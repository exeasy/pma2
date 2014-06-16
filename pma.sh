#!/bin/bash

pma=(PMA1-1 PMA1-2 PMA2-1 PMA2-2 PMA3-1 PMA3-2 PMA4-1 PMA4-2 PMA5-1 PMA5-2 PMA6-1 PMA6-2 PMA7-1 PMA7-2 PMA8-1 PMA8-2 PMA9-1 PMA11-1 PMA11-2 PMA12-1 PMA12-2 PMA12-3)
as=(AS1-1 AS1-2 AS2-1 AS2-2 AS3-1 AS3-2 AS4-1 AS4-2 AS5-1 AS5-2 AS6-1 AS6-2 AS7-1 AS7-2 AS8-1 AS8-2 AS9-1 AS11-1 AS11-2 AS12-1 AS12-2 AS12-3)
id=(11 12 21 22 31 32 41 42 51 52 61 62 71 72 81 82 91 111 112 121 122 123)


for((i=0;i<22;i++)); do
	idx=${id[$i]}
	pmahost=${pma[$i]}
	ashost=${as[$i]}

if [ $1 == "start" ]; then
	ssh root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh start; "
	ssh root@$ashost "hostname; cd pma2-router; sh pma-router.sh start; "
else if [ $1 == "stop" ]; then
	ssh root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh stop; "
	ssh root@$ashost "hostname; cd pma2-router; sh pma-router.sh stop; "
else if [ $1 == "clear" ]; then
	echo ""
else if [ $1 == "put" ]; then
	scp pma2-agent.tar.gz root@$pmahost:~/
	scp pma2-router.tar.gz root@$ashost:~/
	scp pma-2.1.tar.gz root@$pmahost:~
	ssh root@$pmahost "hostname; tar xvzf pma-2.1.tar.gz -k --overwrite; tar xvzf pma2-agent.tar.gz -k --overwrite;"
	ssh root@$ashost " hostname; tar xvzf pma2-router.tar.gz -k --overwrite;"
else if [ $1 == "deploy" ]; then
	ssh root@$pmahost "hostname; cd pma2-agent; sh pma-agent.sh deploy $idx;"
	ssh root@$ashost "hostname; cd pma2-router; sh pma-router.sh deploy $idx; "
fi
fi
fi
fi
fi
	scp $1 root@$pmahost:~/pma2-agent/
done
