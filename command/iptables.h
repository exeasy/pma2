#ifndef IPTABLES_H

#define IPTABLES_H


int add_iptables( char *src, char *dst, __u32 key);
int del_iptables(__u32 num);
int get_iptables(char *src, char *dst,  __u32 *num,__u32 *key);

#endif /* end of include guard: IPTABLES_H */
