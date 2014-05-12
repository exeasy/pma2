#ifndef NETLINK_H

#define NETLINK_H

#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#define NL_PKT_BUF_SIZE 4096

/* Socket interface to kernel */
struct nlsock
{
  int sock;
  int seq;
  struct sockaddr_nl snl;
  const char *name;
}; 
/* Message structure. */
struct message
{
  int key;
  const char *str;
};

int
netlink_socket (struct nlsock *nl, unsigned long groups);


int
netlink_request (int family, int type, struct nlsock *nl);


int
netlink_parse_info (int (*filter) (struct sockaddr_nl *, struct nlmsghdr *),
                    struct nlsock *nl);

void
netlink_parse_rtattr (struct rtattr **tb, int max, struct rtattr *rta,
                      int len);

int
netlink_interface (struct sockaddr_nl *snl, struct nlmsghdr *h);

int interface_lookup_netlink(struct nlsock netlink_cmd);



#endif /* end of include guard: NETLINK_H */
