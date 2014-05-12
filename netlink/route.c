#include <utils/common.h>
#include <utils/utils.h>
#include <stdio.h>
#include "route.h"
#include <netlink.h>

struct route* route_list;

char* getmask(char * mask, int len)
{
	if(len<0||len > 32)return NULL;
	int i=0;int dmask=0;
	while(i++<len)
		dmask = (dmask <<= 1)|1;
	//dmask = htonl(dmask);
	struct in_addr m ;
	m.s_addr = dmask;
	strcpy(mask, inet_ntoa(m));
	return mask;
}

int prefixv4_cmp(struct prefix_ipv4 a, struct prefix_ipv4 b)
{
	if(a.prefix.s_addr == b.prefix.s_addr&&a.prefixlen == b.prefixlen)
		return 1;
	else return 0;	
}
int add_route(struct route* rlist, struct route *r)
{
	r->next = rlist->next;
	rlist->next = r;
	r->dirty = FRESH_STATUS;
}
int is_route_same_exactly(struct route a, struct route b)
{
	if(a.gateway.s_addr == b.gateway.s_addr)
	{
		if(a.interface_id == b.interface_id)
		{
			if(a.metric == b.metric)
				return 1 ;
		}
	}
	return 0;
}
int update_route(struct route* before, struct route* r)
{
	struct route* ori = (before)->next;
	r->next = ori->next;
 	free(ori);
	before->next = r;
 	r->dirty = NEW_STATUS;
}
int refresh_route_status(struct route* rlist)
{
	struct route * r = rlist;
	while( r->next )
	{
		r->dirty = OLD_STATUS;
		r = r->next;
	}
}

char* get_single_route(struct route*h)
{
	static char data[100];
	memset(data, 0x00, 100);
	int len = 0;
	char mask[24];
	getmask(mask,h->prefix.prefixlen);
	sprintf(data+len,"%s %s ",inet_ntoa(h->prefix.prefix),mask);
	len = strlen(data);
	if(h->gateway.s_addr)
	{
	sprintf(data+len,"%s ",inet_ntoa(h->gateway));
	len = strlen(data);
	}
	else
	{
		sprintf(data+len, "0.0.0.0 ");	
		len = strlen(data);
	}

	sprintf(data+len,"%d %s %d",h->interface_id,h->type?"remote":"local",h->metric);	
	len = strlen(data);
	return data;
}

int show_single_route(struct route* h)
{
	char mask[24];
	getmask(mask,h->prefix.prefixlen);
	printf("%s\t%s\t",inet_ntoa(h->prefix.prefix),mask);
	if(h->gateway.s_addr)
	printf("%s\t",inet_ntoa(h->gateway));
	else printf("0.0.0.0\t");	
	printf("%d\t%d\t%d",h->interface_id,h->type,h->metric);	
}

int watch_route_change(struct route* rlist)
{
	int ischanged = 0;
	struct route *r = rlist;
	while(r->next)
	{
		struct route * h = r->next;
		if( h->dirty == NEW_STATUS )
		{
			//printf(" EXISTED\n");
		}
		else if ( h->dirty == FRESH_STATUS)
		{
			show_single_route(h);
			printf(" ADDED\n");
			ischanged = 1;
		}
		else //delete old route
		{
			show_single_route(h);
			printf("DELETED\n");
			r->next = h->next;
			free(h);
			ischanged = 1;
			continue;
		}
		r = r->next;
	
	}
	return ischanged;
}
struct route* find_before_route(struct route *rlist, struct route rt)
{
	struct route* p = rlist;
	while(p->next)
	{
		int ret = prefixv4_cmp(p->next->prefix,rt.prefix);
		int ret1 = is_route_same_exactly(*(p->next),rt);	
		if(ret&&ret1)
			return p;
		p = p->next;
	}
	return NULL;
}

int nexthop_ipv4_ifindex_add(struct route* r, struct in_addr* gate, struct in_addr* src, int ifdex)
{
	struct in_addr ipv4 = *gate;
	if(src)
	{
//	printf("Gateway %s ",inet_ntoa(ipv4));
//	printf("Src %s\n",inet_ntoa(*src));
	r->gateway = *gate;
	}
	else
	{
//	printf("Gateway %s ",inet_ntoa(ipv4));
//	printf(" Ifindex %d\n",ifdex);
		r->gateway = *gate;
		r->interface_id = ifdex;
	}
	
}

int nexthop_ipv4_add(struct route* r, struct in_addr* gate, struct in_addr* src)
{
//	printf("Gateway %s ",inet_ntoa(*gate));
//	printf("Src %s\n",inet_ntoa(*src));
	r->gateway = *gate;

}

int  nexthop_ifindex_add(struct route* r, int ifdex){
//	printf("Gateway 0.0.0.0 ");
//	printf(" Ifindex %d\n",ifdex);
	r->interface_id = ifdex;
}
int show_route_table(struct route* rlist)
{
	struct route* p = rlist->next;
	while(p)
	{
		char mask[24];
		getmask(mask,p->prefix.prefixlen);
		printf("%s %s ",inet_ntoa(p->prefix.prefix),mask);
		if(p->gateway.s_addr)
		printf("%s ",inet_ntoa(p->gateway));
		else printf("0.0.0.0 ");	
		printf("%d %d 0 %d\n",p->interface_id,p->type,p->metric);	
		p = p ->next;
	}
}
int route_add_ipv4(int type, int flags, struct prefix_ipv4 *p,
	struct in_addr *gate, struct in_addr *src, 
		unsigned int ifindex, u_int32_t vrf_id,
		u_int32_t metric, u_char distance,int safi)
		{
			/* Set default distance by route type. */
	int len = p->prefixlen;
	struct in_addr from = p->prefix;
	struct route* new_route = (struct route*)malloc(sizeof(struct route));
	memset(new_route,0x0,sizeof(struct route));
	new_route->prefix = *p;
	new_route->type = type;
//	printf("Dst:%s\\%d ",inet_ntoa(from),len);
  if (distance == 0)
    {
 //     printf("metric 0 ");
    }
//	printf("Metric %d ",metric );
	new_route->metric = metric;
		 /* Nexthop settings. */
  if (gate)
    {
      if (ifindex)
	nexthop_ipv4_ifindex_add (new_route, gate, src, ifindex);
      else
	nexthop_ipv4_add (new_route, gate, src);
    }
  else
    nexthop_ifindex_add (new_route, ifindex);
	
	struct route* rr = find_before_route(route_list,*new_route);
	if(rr==NULL)
	{
		add_route(route_list,new_route);
	}		
	else
	{
		update_route(rr,new_route);
	}
}
int count = 0;
/* Looking up routing table by netlink interface. */
static int
netlink_routing_table (struct sockaddr_nl *snl, struct nlmsghdr *h)
{

  int len;
  struct rtmsg *rtm;
  struct rtattr *tb[RTA_MAX + 1];
  u_char flags = 0;

  char anyaddr[16] = { 0 };

  int index;
  int table;
  int metric;

  void *dest;
  void *gate;
  void *src;

  rtm = NLMSG_DATA (h);

  if (h->nlmsg_type != RTM_NEWROUTE)
    return 0;
  if (rtm->rtm_type != RTN_UNICAST)
    return 0;

  table = rtm->rtm_table;

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct rtmsg));
  if (len < 0)
    return -1;

  memset (tb, 0, sizeof tb);
  netlink_parse_rtattr (tb, RTA_MAX, RTM_RTA (rtm), len);

  if (rtm->rtm_flags & RTM_F_CLONED)
  {
  	printf("here 1\n");
    return 0;
  }
  if (rtm->rtm_protocol == RTPROT_REDIRECT)
    {
  	printf("here 2\n");
    return 0;
  }
  if (rtm->rtm_protocol == RTPROT_KERNEL)
   {
count++;
  //	printf("here 3\n");
   // return 0;
  }

  if (rtm->rtm_src_len != 0)
   {
  	printf("here 4\n");
    return 0;
  }

  index = 0;
  metric = 0;
  dest = NULL;
  gate = NULL;
  src = NULL;

  if (tb[RTA_OIF])
    index = *(int *) RTA_DATA (tb[RTA_OIF]);

  if (tb[RTA_DST])
    dest = RTA_DATA (tb[RTA_DST]);
  else
    dest = anyaddr;

  if (tb[RTA_PREFSRC])
    src = RTA_DATA (tb[RTA_PREFSRC]);

  if (tb[RTA_GATEWAY])
    gate = RTA_DATA (tb[RTA_GATEWAY]);

  if (tb[RTA_PRIORITY])
    metric = *(int *) RTA_DATA(tb[RTA_PRIORITY]);

  if (rtm->rtm_family == AF_INET)
    {
      struct prefix_ipv4 p;
      p.family = AF_INET;
      memcpy (&p.prefix, dest, 4);
      p.prefixlen = rtm->rtm_dst_len;

   
   if (!tb[RTA_MULTIPATH])
   {
   	
   		
         route_add_ipv4 (rtm->rtm_protocol, flags, &p, gate, src, index,
    
	                    table, metric, 0, 0);
   }
	  else
        {
          /* This is a multipath route */

          /*
struct rib *rib;
          struct rtnexthop *rtnh =
            (struct rtnexthop *) RTA_DATA (tb[RTA_MULTIPATH]);

          len = RTA_PAYLOAD (tb[RTA_MULTIPATH]);

          rib = XCALLOC (MTYPE_RIB, sizeof (struct rib));
          rib->type = ZEBRA_ROUTE_KERNEL;
          rib->distance = 0;
          rib->flags = flags;
          rib->metric = metric;
          rib->table = table;
          rib->nexthop_num = 0;
          rib->uptime = time (NULL);

          for (;;)
            {
              if (len < (int) sizeof (*rtnh) || rtnh->rtnh_len > len)
                break;

              rib->nexthop_num++;
              index = rtnh->rtnh_ifindex;
              gate = 0;
              if (rtnh->rtnh_len > sizeof (*rtnh))
                {
                  memset (tb, 0, sizeof (tb));
                  netlink_parse_rtattr (tb, RTA_MAX, RTNH_DATA (rtnh),
                                        rtnh->rtnh_len - sizeof (*rtnh));
                  if (tb[RTA_GATEWAY])
                    gate = RTA_DATA (tb[RTA_GATEWAY]);
                }

              if (gate)
                {
                  if (index)
                    nexthop_ipv4_ifindex_add (rib, gate, src, index);
                  else
                    nexthop_ipv4_add (rib, gate, src);
                }
              else
                nexthop_ifindex_add (rib, index);

              len -= NLMSG_ALIGN(rtnh->rtnh_len);
              rtnh = RTNH_NEXT(rtnh);
            }

          if (rib->nexthop_num == 0)
            XFREE (MTYPE_RIB, rib);
          else
            rib_add_ipv4_multipath (&p, rib, SAFI_UNICAST);*/
             printf("rib_add_ipv4_multipath\n");
        }
       
    }

	else
	{
	printf("Not a AF_INET route\n");
	}
  return 0;
}



int get_route_table(struct nlsock nl)
{
	if ( route_list == NULL )
		route_list = malloc_z(struct route);
  	/* Get IPv4 routing table. */
	int ret = 0;
  	ret = netlink_request (AF_INET, RTM_GETROUTE, &nl);
  	if (ret < 0)
	    printf("Request error\n");
  	ret = netlink_parse_info (netlink_routing_table, &nl);
  	if (ret < 0)
    	printf("Read error\n");

}

//int main()
//{
//	struct nlsock nl;
//	int ret =netlink_socket(&nl,0);
//	if(ret!=0)return 0;
//	get_route_table(nl);
//
//}
