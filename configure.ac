#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT(pma, 2.1, catchgreen@163.com)
AC_CONFIG_SRCDIR([pma_bm.c])
AC_CONFIG_HEADERS([config.h])
AC_PROG_RANLIB
AM_INIT_AUTOMAKE
# Checks for programs.
${CFLAGS="-g -O0"}
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
# Checks for libraries.
AC_SEARCH_LIBS(pthread_create, pthread)
AC_SEARCH_LIBS(dlopen, dl)
AC_SEARCH_LIBS(snmp_sess_init, netsnmp, 
			   [AC_SUBST(LIB_SNMP_INCLUDE, ./),
				AC_SUBST(LIB_SNMP_LIB, ./)],
			   [AC_SUBST(LIB_SNMP_INCLUDE, /usr/local/snmp/include/),
				AC_SUBST(LIB_SNMP_LIB, /usr/local/snmp/lib/)],[])
AC_SEARCH_LIBS(xmlStrcmp, xml2)

# Checks for header files.
AC_SUBST(LIB_XML_INCLUDE, /usr/include/libxml2/)


# Self defined
AC_ARG_ENABLE([ospf],
			  [AC_HELP_STRING([--enable-ospf],
							  [Enable ospf support])],
			[case "${enableval}" in 
			 (yes) ospf=true ;;
			 (no) ospf=false ;;
			 (*) AC_MSG_ERROR([bad value ${enableval} for --enable-ospf]) ;;
		 esac],
		 [ospf=false])
AC_ARG_ENABLE([bgp],
			  [AC_HELP_STRING([--enable-bgp],
							  [Enable bgp support])],
			[case "${enableval}" in 
			 (yes) bgp=true ;;
			 (no) bgp=false ;;
			 (*) AC_MSG_ERROR([bad value ${enableval} for --enable-bgp]) ;;
		 esac],
		 [bgp=false])
		
		if test "$ospf" = true; then
			AC_MSG_WARN([Ospf Version will be installed])
			fi
		if test "$bgp" = true; then
			AC_DEFINE([EXCHANGE_DISABLE],[1],[Disable the exchange module])
			AC_DEFINE([FLOOD_DISABLE],[1],[Disable the flood module])
			AC_DEFINE([SPF_DISABLE],[1],[Disable the spf function])
			AC_MSG_WARN([Bgp Version will be installed])
		fi


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
