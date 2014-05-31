#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(pma, 2.1, catchgreen@163.com)
AC_CONFIG_SRCDIR([pma_bm.c])
AC_CONFIG_HEADERS([config.h])
AC_PROG_RANLIB
AM_INIT_AUTOMAKE
# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_SEARCH_LIBS(pthread_create, pthread)
AC_SEARCH_LIBS(dlopen, dl)
AC_SEARCH_LIBS(snmp_sess_init, netsnmp)
AC_SEARCH_LIBS(xmlStrcmp, xml2)
# Checks for libraries.

# Checks for header files.
AC_DEFINE(OSPF_VERSION,,support the ospf protocol)
#AC_DEFINE(BGP_VERSION,,support with bgp)

AC_SUBST(LIB_XML_INCLUDE, /usr/include/libxml2/)
AC_SUBST(LIB_SNMP_INCLUDE, /usr/local/snmp/include/)
AC_SUBST(LIB_SNMP_LIB, /usr/local/snmp/lib/)
# Checks for typedefs, structures, and compiler characteristics.  
# Checks for library functions.
AC_CONFIG_FILES(Makefile
				utils/Makefile
				policy/Makefile
				conf/Makefile
				control/Makefile
				command/Makefile
				comm/Makefile
				lib/Makefile
				reg/Makefile
				logger/Makefile
				mmanager/Makefile
                router/Makefile
				server/Makefile
				timer/Makefile
				snmp/Makefile
				netlink/Makefile
				lsdb/Makefile
				db/Makefile
				ddc/Makefile
                lsd/Makefile
				vty/Makefile
				test/Makefile)
AC_OUTPUT
