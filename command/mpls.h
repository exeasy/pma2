#ifndef MPLS_H

#define MPLS_H

typedef unsigned int __u32;

void add_mpls_in(__u32 label, char * ifname, char *ipaddress, __u32 *rkey);
void add_mpls_mid(__u32 label, __u32 key);
void add_mpls_end(__u32 label);
void del_mpls(__u32 key);

#endif /* end of include guard: MPLS_H */
