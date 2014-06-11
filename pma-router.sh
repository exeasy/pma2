#!/bin/bash

CONF_DETECT_BIN=./conf_detect
PEM_BIN=./pem
QUAGGA_CONF=/etc/quagga/Quagga.conf
AGENT_IP=`cat ./agent.conf`
AGENT_PORT=8025
PKT_TYPE=73

deploy(){
	cp config/agent.conf.$1 agent.conf
}

bin_start(){
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./command/
	$CONF_DETECT_BIN $QUAGGA_CONF $AGENT_IP $AGENT_PORT $PKT_TYPE 2&>1 > conf_detect.log &
	$CONF_DETECT_BIN $QUAGGA_CONF $AGENT_IP $AGENT_PORT 6 2&>1 > conf_detect.log &
	$PEM_BIN $AGENT_IP 2&>1 > pem.log &
}
bin_stop(){
	killall conf_detect
	killall pem
}

log_clean(){
	> conf_detect.log	
	> pem.log
}

echo $1
if [ "x$1" = "xstart" ]; then
	bin_start
else if [ "x$1" = "xstop" ]; then
	bin_stop
else if [ "x$1" = "xclean" ]; then
	log_clean
else if [ "x$1" = "xdeploy" ]; then
	deploy $2
fi
fi
fi
fi
