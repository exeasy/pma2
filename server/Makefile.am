noinst_LIBRARIES=libpmasocket.a libpmapem.a libpmaicm.a libpmadbm.a
noinst_HEADERS=dbm.h icm.h pem.h server.h socket.h srvconf.h 
libpmasocket_a_SOURCES=server.c\
					   socket.c
libpmapem_a_SOURCES=pem.c\
					server.c\
					socket.c
libpmadbm_a_SOURCES=dbm.c\
					server.c\
					socket.c
libpmaicm_a_SOURCES=icm.c\
					server.c\
					socket.c
libpmapem_a_CFLAGS=-I/usr/include/libxml2
