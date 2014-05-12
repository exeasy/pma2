#include <stdio.h>
#include <utils/common.h>
#include "netlink.h"


struct nlsock netlink      = { -1, 0, {0}, "netlink-listen"},     /* kernel messages */
  netlink_cmd  = { -1, 0, {0}, "netlink-cmd"};        /* command channel */

static const struct message nlmsg_str[] = {
  {RTM_NEWROUTE, "RTM_NEWROUTE"},
  {RTM_DELROUTE, "RTM_DELROUTE"},
  {RTM_GETROUTE, "RTM_GETROUTE"},
  {RTM_NEWLINK,  "RTM_NEWLINK"},
  {RTM_DELLINK,  "RTM_DELLINK"},
  {RTM_GETLINK,  "RTM_GETLINK"},
  {RTM_NEWADDR,  "RTM_NEWADDR"},
  {RTM_DELADDR,  "RTM_DELADDR"},
  {RTM_GETADDR,  "RTM_GETADDR"},
  {0, NULL}
};
static const char *nexthop_types_desc[] =
{
  "none",
  "Directly connected",
  "Interface route",
  "IPv4 nexthop",
  "IPv4 nexthop with ifindex",
  "IPv4 nexthop with ifname",
  "IPv6 nexthop",
  "IPv6 nexthop with ifindex",
  "IPv6 nexthop with ifname",
  "Null0 nexthop",
};

/* Make socket for Linux netlink interface. */
int
netlink_socket (struct nlsock *nl, unsigned long groups)
{
  int ret;
  struct sockaddr_nl snl;
  int sock;
  int namelen;
  int save_errno;

  sock = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
  if (sock < 0)
    {
      printf("Can't open %s socket: %s", nl->name,
            strerror(errno));
      return -1;
    }

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;
  snl.nl_groups = groups;

 
  ret = bind (sock, (struct sockaddr *) &snl, sizeof snl);
  save_errno = errno;
  

  if (ret < 0)
    {
      printf("Can't bind %s socket to group 0x%x: %s",
            nl->name, snl.nl_groups, strerror (save_errno));
      close (sock);
      return -1;
    }

  /* multiple netlink sockets will have different nl_pid */
  namelen = sizeof snl;
  ret = getsockname (sock, (struct sockaddr *) &snl, (socklen_t *) &namelen);
  if (ret < 0 || namelen != sizeof snl)
    {
     printf("Can't get %s socket name: %s", nl->name,
            strerror (errno));
      close (sock);
      return -1;
    }

  nl->snl = snl;
  nl->sock = sock;
  return ret;
}

/* Get type specified information from netlink. */
int
netlink_request (int family, int type, struct nlsock *nl)
{
  int ret;
  struct sockaddr_nl snl;
  int save_errno;

  struct
  {
    struct nlmsghdr nlh;
    struct rtgenmsg g;
  } req;


  /* Check netlink socket. */
  if (nl->sock < 0)
    {
     printf("%s socket isn't active.", nl->name);
      return -1;
    }

  memset (&snl, 0, sizeof snl);
  snl.nl_family = AF_NETLINK;

  memset (&req, 0, sizeof req);
  req.nlh.nlmsg_len = sizeof req;
  req.nlh.nlmsg_type = type;
  req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
  req.nlh.nlmsg_pid = nl->snl.nl_pid;
  req.nlh.nlmsg_seq = ++nl->seq;
  req.g.rtgen_family = family;


 
  ret = sendto (nl->sock, (void *) &req, sizeof req, 0,
                (struct sockaddr *) &snl, sizeof snl);
  save_errno = errno;


  if (ret < 0)
    {
      printf("%s sendto failed: %s", nl->name,
            strerror (save_errno));
      return -1;
    }

  return 0;
}

/* Receive message from netlink interface and pass those information
   to the given function. */
int
netlink_parse_info (int (*filter) (struct sockaddr_nl *, struct nlmsghdr *),
                    struct nlsock *nl)
{
  int status;
  int ret = 0;
  int error;

  while (1)
    {
      char buf[NL_PKT_BUF_SIZE];
      struct iovec iov = { buf, sizeof buf };
      struct sockaddr_nl snl;
      struct msghdr msg = { (void *) &snl, sizeof snl, &iov, 1, NULL, 0, 0 };
      struct nlmsghdr *h;

      status = recvmsg (nl->sock, &msg, 0);
      if (status < 0)
        {
          if (errno == EINTR)
            continue;
          if (errno == EWOULDBLOCK || errno == EAGAIN)
            break;
       
          continue;
        }

      if (status == 0)
        {
         printf("%s EOF", nl->name);
          return -1;
        }

      if (msg.msg_namelen != sizeof snl)
        {
         printf("%s sender address length error: length %d",
                nl->name, msg.msg_namelen);
          return -1;
        }
      
      for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, (unsigned int) status);
           h = NLMSG_NEXT (h, status))
        {
          /* Finish of reading. */
          if (h->nlmsg_type == NLMSG_DONE)
            return ret;

          /* Error handling. */
          if (h->nlmsg_type == NLMSG_ERROR)
            {
              struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);
	      int errnum = err->error;
	      int msg_type = err->msg.nlmsg_type;

              /* If the error field is zero, then this is an ACK */
              if (err->error == 0)
                {
                 /* 
                      printf("%s: %s ACK: type=%s(%u), seq=%u, pid=%u",
                                 __FUNCTION__, nl->name,
                                 lookup (nlmsg_str, err->msg.nlmsg_type),
                                 err->msg.nlmsg_type, err->msg.nlmsg_seq,
                                 err->msg.nlmsg_pid);
                   */

                  /* return if not a multipart message, otherwise continue */
                  if (!(h->nlmsg_flags & NLM_F_MULTI))
                    {
                      return 0;
                    }
                  continue;
                }

              if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
                {
                 printf("%s error: message truncated",
                        nl->name);
                  return -1;
                }

              /* Deal with errors that occur because of races in link handling */
	      if (nl == &netlink_cmd
		  && ((msg_type == RTM_DELROUTE &&
		       (-errnum == ENODEV || -errnum == ESRCH))
		      || (msg_type == RTM_NEWROUTE && -errnum == EEXIST)))
		{
		   /* printf("%s: error: %s type=%s(%u), seq=%u, pid=%u",
				nl->name, strerror (-errnum),
				lookup (nlmsg_str, msg_type),
				msg_type, err->msg.nlmsg_seq, err->msg.nlmsg_pid);*/
		  return 0;
		}

	      /*
printf("%s error: %s, type=%s(%u), seq=%u, pid=%u",
			nl->name, strerror (-errnum),
			lookup (nlmsg_str, msg_type),
			msg_type, err->msg.nlmsg_seq, err->msg.nlmsg_pid);*/
              return -1;
            }


          /* skip unsolicited messages originating from command socket */
          if (nl != &netlink_cmd && h->nlmsg_pid == netlink_cmd.snl.nl_pid)
            {
            
              continue;
            }

          error = (*filter) (&snl, h);
          if (error < 0)
            {
             printf("%s filter function error", nl->name);
              ret = error;
            }
        }

      /* After error care. */
      if (msg.msg_flags & MSG_TRUNC)
        {
         printf("%s error: message truncated", nl->name);
          continue;
        }
      if (status)
        {
         printf("%s error: data remnant size %d", nl->name,
                status);
          return -1;
        }
    }
  return ret;
}

/* Utility function for parse rtattr. */
void
netlink_parse_rtattr (struct rtattr **tb, int max, struct rtattr *rta,
                      int len)
{
  while (RTA_OK (rta, len))
    {
      if (rta->rta_type <= max)
        tb[rta->rta_type] = rta;
      rta = RTA_NEXT (rta, len);
    }
}

/* Called from interface_lookup_netlink().  This function is only used
   during bootstrap. */
int
netlink_interface (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
  int len;
  struct ifinfomsg *ifi;
  struct rtattr *tb[IFLA_MAX + 1];
  struct interface *ifp;
  char *name;

  ifi = NLMSG_DATA (h);

  if (h->nlmsg_type != RTM_NEWLINK)
    return 0;

  len = h->nlmsg_len - NLMSG_LENGTH (sizeof (struct ifinfomsg));
  if (len < 0)
    return -1;

  /* Looking up interface name. */
  memset (tb, 0, sizeof tb);
  netlink_parse_rtattr (tb, IFLA_MAX, IFLA_RTA (ifi), len);
  

  if (tb[IFLA_IFNAME] == NULL)
    return -1;
  name = (char *) RTA_DATA (tb[IFLA_IFNAME]);

//	iface_count++;
//	strcpy(int_name[ifi->ifi_index],name);

  /* Add interface. */
//  ifp = if_get_by_name (name);
  //set_ifindex(ifp, ifi->ifi_index);
//  ifp->flags = ifi->ifi_flags & 0x0000fffff;
//  ifp->mtu6 = ifp->mtu = *(uint32_t *) RTA_DATA (tb[IFLA_MTU]);
//  ifp->metric = 1;

  /* Hardware type and address. */
//  ifp->hw_type = ifi->ifi_type;
 // netlink_interface_update_hw_addr (tb, ifp);

//  if_add_update (ifp);
//*/
  return 0;
}

int interface_lookup_netlink(struct nlsock netlink_cmd)
{
	int ret;

  /* Get interface information. */
  ret = netlink_request (AF_PACKET, RTM_GETLINK, &netlink_cmd);
  if (ret < 0)
    return ret;
  ret = netlink_parse_info (netlink_interface, &netlink_cmd);
  if (ret < 0)
    return ret;
}

//int show_interface()
//{
//	int i =0;
//	int j =0;
//	for(;i<iface_count;i++)
//	{
//	if(strlen(int_name[j])>0)
//		printf("ID[%d] Name[%s]\n",j,int_name[j]);
//	j++;
//}
//}
