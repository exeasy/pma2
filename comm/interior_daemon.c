#include <utils/utils.h>
#include <utils/common.h>
#include <handler.h>
#include <header.h>
#include <interior_daemon.h>
#include <mmanager/module_manager.h>
#include <conf/conf.h>
#include <server/srvconf.h>

struct ctl * module_handle;
int bm_status = 0;
struct global_buff xml_policy_buff;
struct global_buff global_pma_buff;

int basemodule_check_status()
{
	if(bm_status > 1)
	{
		return 1;
	}
	else return 0;
}

int interior_daemon_init()
{
	module_handle = create_ctl(1024);
	add_ctl(module_handle, MODREG,				handle_local_module_register,		1);
	add_ctl(module_handle, LSDB_INFO,			handle_lsdb_snapshoot_dbm_to_pms,	1);
	add_ctl(module_handle, NETWORK_INFO,		handle_network_info_dbm_to_pms,		1);
	add_ctl(module_handle, ADDLSA,				handle_add_lsa_ic_to_dbm,			1);
	add_ctl(module_handle, UPDATELSA,			handle_update_lsa_ic_to_dbm,		1);
	add_ctl(module_handle, ADDROUTE,			handle_add_route_ic_to_pms,			1);
	add_ctl(module_handle, OSPF_SPF,			handle_ospf_spf_signal_ic_to_dbm,	1);
	add_ctl(module_handle, POLICY_INFO,			handle_send_policy_dbm_to_pea,		1);
	add_ctl(module_handle, UP_ROUTE_INFO,		handle_route_table_ic_to_pms,		1);
	add_ctl(module_handle, UP_DEVICE_INFO,	handle_device_info_ic_to_pms,			1);
	add_ctl(module_handle, UP_OSPF_INTERFACE_INFO,		handle_ospf_interface_ic_to_pms,		1);
	add_ctl(module_handle, UP_BGP_INTERFACE_INFO,		handle_bgp_interface_ic_to_pms,		1);
	add_ctl(module_handle, UP_TRAFFICE_INFO,		handle_traffice_info_ic_to_pms,		1);
	add_ctl(module_handle, UP_BGP_PATH_TABLE_INFO,		handle_bgp_path_info_ic_to_pms,		1);
	global_pma_buff.length = 0;
	global_pma_buff.buff = (char*) malloc(MAXBUF);
	interior_daemon_start();
}
	static int
create_and_bind_tcp( char *port )
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0)
	{
		DEBUG (ERROR, "getaddrinfo: %s", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1)
			continue;
		int n=1;
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(int));
		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			break;
		}

		close (sfd);
	}

	if (rp == NULL)
	{
		DEBUG (ERROR, "Could not bind");
		return -1;
	}

	freeaddrinfo (result);

	int flag = 1;
	int len = sizeof(int);
	if( setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) == - 1)
	{
		DEBUG( ERROR, "bind error=[%s]",  strerror(errno) );
		return -1;
	}

	return sfd;
}

	static int
make_socket_non_blocking (int sfd)
{
	int flags, s;

	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}

	return 0;
}
	int
interior_daemon_start()
{
	int sfd, s;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	signal(SIGPIPE, SIG_IGN);//pipe broken

	sfd = create_and_bind_tcp ( SERVER_PORT );
	if (sfd == -1)
	{
		return -1;
	}

	int flag = 1;
	int len = sizeof(int);
	if( setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &flag, len) == - 1)
	{
		DEBUG(ERROR,"Bind error %s", strerror(errno));
		return -1;
	}

	s = make_socket_non_blocking (sfd);
	if (s == -1)
	{
		return -1;
	}

	s = listen (sfd, SOMAXCONN);
	if (s == -1)
	{
		DEBUG(ERROR,"Listen error %s", strerror(errno));
		return -1;
	}

	efd = epoll_create1 (0);
	if (efd == -1)
	{
		DEBUG(ERROR,"epool_create error %s", strerror(errno));
		return -1;
	}

	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;  //ET or LT?
	s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
	if (s == -1)
	{
		DEBUG(ERROR,"epool_ctl error %s", strerror(errno));
		return -1;
	}

	// Buffer where events are returned
	events = calloc (MAXEVENTS, sizeof event);

	// The event loop
	while (1)
	{
		int n, i;

		n = epoll_wait (efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++)
		{
			if ((events[i].events & EPOLLERR) ||
					(events[i].events & EPOLLHUP) ||
					(!(events[i].events & EPOLLIN)))
			{
				// An error has occured on this fd, or the socket is not
				// ready for reading (why were we notified then?)
				DEBUG(ERROR,"epool error %s", strerror(errno));
				s = epoll_ctl (efd, EPOLL_CTL_DEL, events[i].data.fd, &event);
				if (s == -1)
				{
					DEBUG(ERROR,"epool_ctl %s", strerror(errno));
					return -1;
				}
				close (events[i].data.fd);
				continue;
			}
			else if (sfd == events[i].data.fd)
			{
				// We have a notification on the listening socket, which
				// means one or more incoming connections.

				while (1)
				{
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

					in_len = sizeof in_addr;
					infd = accept (sfd, &in_addr, &in_len);
					if (infd == -1)
					{
						if ((errno == EAGAIN) ||
								(errno == EWOULDBLOCK))
						{
							// We have processed all incoming connections.
							DEBUG(ERROR,"Block here %s", strerror(errno));
							break;
						}
						else
						{
							DEBUG(ERROR,"accept %s", strerror(errno));
							break;
						}
					}

					DEBUG(ERROR,"infd=[%d]", infd);
					/* Make the incoming socket non-blocking and add it to the
					   list of fds to monitor. */
					s = make_socket_non_blocking (infd);
					if (s == -1)
					{
						close(infd);
						DEBUG(ERROR,"Make socket non blocking error %s", strerror(errno));
						break;
					}

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
					if (s == -1)
					{
						close(infd);
						DEBUG(ERROR,"epool_ctl error  %s", strerror(errno));
						break;
					}
				}

				continue;
			}
			else
			{
				child_process( (void *)&(events[i].data.fd) );
			}
		}
	}

	free (events);

	DEBUG(INFO,"local_daemon close");
	close (sfd);

	return EXIT_SUCCESS;
}

void child_process( void *arg )
{

	int fd = *( (int*)arg );
	if( fd <= 0 )
	{
		DEBUG(INFO,"socket fd error");
		return;
	}
	DEBUG(INFO,"socket fd=[%d]",fd);

	// We have data on the fd waiting to be read. Read and
	// display it. We must read whatever data is available
	// completely, as we are running in edge-triggered mode
	// and won't get a notification again for the same data.

	int done = 0;
	char buf[MAXBUF];


	while(1)//read until no data left
	{
		memset( buf, 0x00, sizeof(buf) );	// Reset the space for safety
		int numbytes = 0;
		local_module_header header;
		numbytes = read(fd,&header,sizeof(local_module_header));

		DEBUG(INFO,"Sizeof header=[%d]",sizeof(local_module_header));

		if(numbytes<=0)
		{
			break;
		}

		int pkt_len = header.pkt_len - sizeof(local_module_header);
		DEBUG(INFO,"Pkt_len=[%d]",pkt_len);
		local_module_header *pkt = (local_module_header *)malloc(header.pkt_len);


		DEBUG(INFO, "pkt_type=[%d],len=[%u],recv num=[%u]\n", header.pkt_type, pkt_len, numbytes );

		if( pkt_len > MAXBUF )
		{
			fprintf( stdout, "[%s][%d]n", __FILE__, __LINE__ );
			DEBUG(ERROR,"Preceive data's length is longer than default buffer's length");
			return;
		}
		pkt->pkt_type = header.pkt_type;
		pkt->pkt_version = header.pkt_version;
		pkt->pkt_len = header.pkt_len;

		DEBUG(INFO,"type[%d],version[%d],length[%d]",pkt->pkt_type,pkt->pkt_version,pkt->pkt_len);

		//read the data util all the data readed
		if(pkt_len > 0)
		{
			char* cur_pos = (char*)pkt;
			cur_pos += numbytes;
			while(pkt_len > 0)
			{
				numbytes = read(fd,cur_pos,pkt_len);
				if(numbytes == -1)
				{
					if(errno == EAGAIN || errno == EWOULDBLOCK)
					{
						return;
					}
					close(fd);
					DEBUG(ERROR,"close fd, error=[%s]",  strerror(errno));
					return ;
				}
				else if(numbytes == 0)
				{
					close(fd);
					DEBUG(ERROR,"close fd:the connection has been closed by peer" );
					return ;
				}
				DEBUG(ERROR, "sockfd=[%d],pkt_type=[%d],len=[%d]num=[%u]\n",  fd, header.pkt_type, pkt_len, numbytes );
				cur_pos += numbytes;
				pkt_len -= numbytes;
			}
		}

		//1.process data

		module_data_handle( (char*)pkt, pkt->pkt_len, fd );

		//2.send data to client
		//write( fd, "ok", sizeof("ok") );
	}
	return;
}

	int
module_data_handle( char *buf, int bufflen, int sockfd )
{
	if( buf == NULL )
	{
		DEBUG(ERROR, "input param error");
		return -1;
	}

	local_module_header *header = (local_module_header *)buf;
	int ret = 0;
	int len = 0;
	DEBUG(INFO,"this program.type[%d]",header->pkt_type);
	if(basemodule_check_status() == 0&&header->pkt_type!=MODREG)//not finish init the module
	{
		DEBUG(INFO,"PMA Not ready for use!\nWaiting the other module...\n");
		free(buf);
		return 0;
	}
	struct pkt * info = (struct pkt*)malloc(sizeof(struct pkt));
	info->data = buf;
	info->fd = sockfd;
	run_ctl( module_handle,header->pkt_type, info); 
	free(buf);
	free(info);
	return;
}

int send_pkt(int sock,local_module_header* pkt,int length)
{
	int ret;
	if(sock==0)
	{
		//DEBUG(ERROR,"There is no socket fd");
		return -1;
	}

	ret = send(sock,(void*)pkt,length,0);
	if (ret == -1)
	{
		//DEBUG(ERROR, "send_packet send failed :type:[%d] --%s",pkt->pkt_type, strerror(errno));
		return -1;
	}
	if(ret == 0)
	{
		return -2;//Connection disconnect
	}
	//DEBUG(INFO, "%d %d %u %u send_packet send", global_bm_sock,pkt->pkt_type, pkt->pkt_len, ret);
	return ret;
}

int pack_response_message( local_module_header* pkt, int des, int ret, char *info )
{
	if( pkt == NULL || info == NULL )
	{
		DEBUG(ERROR, "input param error");
		return -1;
	}
	pkt->pkt_type = ACK;
	pkt->pkt_version = PMA_VERSION;
	pkt->pkt_len = sizeof(local_module_header)+ sizeof(struct response_message);


	struct response_message *body = (struct response_message*)pkt->pkt;
	body->des_mod = des;
	body->ret = ret;
	strcpy( body->info, info ); 
	return ( sizeof(local_module_header)+sizeof(struct response_message) ) ;
}

int send_message_to_module( int module , int pkg_type , char* data, int length)
{
	struct module_register_table *mod = find_register_table_by_id(module);
	if(mod == NULL)
	{
		DEBUG(ERROR,"The Module %s hasn't Register",modlist[module]);
		return -1;
	}
	int sock = mod->m_sockfd;
	int len = length + sizeof(local_module_header);
	local_module_header *buff = (local_module_header*)malloc(len);
	buff->pkt_len = len;
	buff->pkt_type = pkg_type;
	buff->pkt_version = PMA_VERSION;
	buff->checksum = 0x123456;
	memcpy(buff->pkt, data, length);
	int ret = 0;
	ret = send_pkt( sock , buff , len);
	if(buff)
		free(buff);
	return ret;
}

int send_config_to_module(int module)
{
	struct module_register_table* mod = find_register_table_by_id(module);
	if(mod == NULL)
	{
		fprintf(stdout,"The Module %d hasn't registed\n",module);
		return -1;
	}

	printf("About to send the config to Module %d\n",mod->m_id);
	int sock = mod->m_sockfd;
	local_module_header* pkt = (local_module_header*)malloc(100+sizeof(local_module_header));
	pkt->pkt_version = PMA_VERSION;
	pkt->checksum = 12345;
	if(module == ICM)
	{
		pkt->pkt_type = ICM_CONF;
		pkt->pkt_len = sizeof(local_module_header)+ sizeof(struct ic_conf);
		memcpy(pkt->pkt,(char*)&pma_conf.ic_config,sizeof(struct ic_conf));

	}
	else if(module == DBM)
	{
		pkt->pkt_type = DBM_CONF;
		pkt->pkt_len =  sizeof(local_module_header)+ sizeof(struct dbm_conf);
		memcpy(pkt->pkt,(char*)&pma_conf.dbm_config,sizeof(struct dbm_conf));
	}
	else if(module == PEM)
	{
		pkt->pkt_type = PEM_CONF;
		pkt->pkt_len =  sizeof(local_module_header)+ sizeof(struct pea_conf);
		memcpy(pkt->pkt,(char*)&pma_conf.pea_config,sizeof(struct pea_conf));
	}
	printf("About to send the config to Module %d\n",mod->m_id);
	send_pkt(sock,pkt,pkt->pkt_len);
	printf("send the config to %d success\n",module);
	free(pkt);
	return 0;
}

