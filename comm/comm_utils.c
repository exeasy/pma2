/**
 * @file comm_utils.c
 * @Brief  comm_utils contains some common function address related
 * @author Zhang Hong
 * @version 2.1
 * @date 2014-06-10
 */
#include "utils/common.h"
#include "utils/utils.h"
#include "comm_utils.h"

char ip_address[INET6_ADDRSTRLEN] = {0};

void free_buf(void *buf)
{
	if( buf != NULL)
	{
		free(buf);
		buf = NULL;
	}
}
void *get_inet_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	} else {
		return &(((struct sockaddr_in6 *)sa)->sin6_addr);
	}
}
char * get_peer_ip(int fd)
{
	size_t addr_size = sizeof(struct sockaddr_in6);
	struct sockaddr *sa = malloc(addr_size);

	if (getpeername(fd, sa, &addr_size) == -1) {
		return "\0";
	}
	memset(ip_address, 0, INET6_ADDRSTRLEN);
	if (NULL == inet_ntop(sa->sa_family,
				get_inet_addr(sa),
				ip_address, INET6_ADDRSTRLEN)) {

		return "\0";
	}
	free(sa);
	return ip_address;
}

u16 get_peer_port(int fd)
{
	size_t addr_size = sizeof(struct sockaddr_in6);
	struct sockaddr *sa = malloc(addr_size);

	if (getpeername(fd, sa, &addr_size) == -1) {
		return 0;
	}
	free(sa);
	return ((struct sockaddr_in *)sa)->sin_port;
}

