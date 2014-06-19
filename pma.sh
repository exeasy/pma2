#!/bin/bash

opt="-o ConnectTimeout=1"

bgp_pma=(PMA1-1 PMA1-2 PMA2-1 PMA2-2 PMA3-1 PMA3-2 PMA4-1 PMA4-2 PMA5-1 PMA5-2 PMA6-1 PMA6-2 PMA7-1 PMA7-2 PMA8-1 PMA8-2 PMA9-1 PMA11-1 PMA11-2 PMA12-1 PMA12-2 PMA12-3)
bgp_as=(AS1-1 AS1-2 AS2-1 AS2-2 AS3-1 AS3-2 AS4-1 AS4-2 AS5-1 AS5-2 AS6-1 AS6-2 AS7-1 AS7-2 AS8-1 AS8-2 AS9-1 AS11-1 AS11-2 AS12-1 AS12-2 AS12-3)
bgp_id=(11 12 21 22 31 32 41 42 51 52 61 62 71 72 81 82 91 111 112 121 122 123)

igp_pma=(R1 R2 R3 R4 R5 R6 R7 R8 R111 R112 R113 R114 R121 R122 R123 R124 R131 R132 R133 R134 R135 R136 R137 R138 R211 R212 R213 R214 R221 R222 R223 R224 R231 R232 R233 R234 R235 R236 R237 R238)
igp_id=(1 2 3 4 5 6 7 8 111 112 113 114 121 122 123 124 131 132 133 134 135 136 137 138 211 212 213 214 221 222 223 224 231 232 233 234 235 236 237 238 )

bgp_control()
{
	for((i=0;i<22;i++)); do
		idx=${bgp_id[$i]}
		pmahost=${bgp_pma[$i]}
		ashost=${bgp_as[$i]}
	
	if [ $1 == "start" ]; then
		ssh $opt root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh start; " &
		ssh $opt root@$ashost "hostname; cd pma2-router; sh pma-router.sh start; " &
	else if [ $1 == "stop" ]; then
		ssh $opt root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh stop; " &
		ssh $opt root@$ashost "hostname; cd pma2-router; sh pma-router.sh stop; " &
	else if [ $1 == "clear" ]; then
		echo ""
	else if [ $1 == "put" ]; then
		scp $opt pma2-agent.tar.gz root@$pmahost:~/
		scp $opt pma2-router.tar.gz root@$ashost:~/
		scp $opt pma-2.1.tar.gz root@$pmahost:~
		ssh $opt root@$pmahost "hostname; tar xvzf pma-2.1.tar.gz -k --overwrite; tar xvzf pma2-agent.tar.gz -k --overwrite;"
		ssh $opt root@$ashost " hostname; tar xvzf pma2-router.tar.gz -k --overwrite;"
	else if [ $1 == "deploy" ]; then
		ssh $opt root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh deploy $idx;"
		ssh $opt root@$ashost "hostname; cd pma2-router; sh pma-router.sh deploy $idx; "
	fi
	fi
	fi
	fi
	fi
		scp $opt $1 root@$pmahost:~/pma2-agent/
	done
}
igp_control()
{
	for((i=0;i<8;i++)); do
#	for((i=8;i<24;i++)); do
#	for((i=24;i<40;i++)); do
		idx=pma${igp_id[$i]}
		pmahost=${igp_pma[$i]}
	
	if [ $1 == "start" ]; then
		ssh $opt root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh start; " &
		ssh $opt root@$pmahost "hostname; cd pma2-router; sh pma-router.sh start; " &
	else if [ $1 == "stop" ]; then
		ssh $opt root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh stop; " &
		ssh $opt root@$pmahost "hostname; cd pma2-router; sh pma-router.sh stop; " &
	else if [ $1 == "clear" ]; then
		echo ""
	else if [ $1 == "put" ]; then
		scp $opt pma2-agent.tar.gz root@$pmahost:~/
		scp $opt pma2-router.tar.gz root@$pmahost:~/
		scp $opt pma-2.1.tar.gz root@$pmahost:~
		ssh $opt root@$pmahost "hostname; tar xvzf pma-2.1.tar.gz -k --overwrite; tar xvzf pma2-agent.tar.gz -k --overwrite;"
		ssh $opt root@$pmahost " hostname; tar xvzf pma2-router.tar.gz -k --overwrite;"
	else if [ $1 == "deploy" ]; then
		ssh $opt root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh deploy $idx;"
		ssh $opt root@$pmahost "hostname; cd pma2-router; sh pma-router.sh deploy $idx; "
	fi
	fi
	fi
	fi
	fi
		scp $opt $1 root@$pmahost:~/pma2-agent/
	done
}
igp_control $1
