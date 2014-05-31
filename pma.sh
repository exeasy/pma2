#!/bin/bash

pma_start(){
	./bm > bm.log 2>&1 &
	sleep 2
	./dbm > dbm.log 2>&1 &
	./icm > icm.log 2>&1 &
	./pem > pem.log 2>&1 &
}
pma_stop(){
	killall pem
	killall icm
	killall dbm
	killall bm
}

log_clear(){
	> bm.log
	> dbm.log
	> pem.log
	> icm.log
}

if [ $1 == "start" ]; then
	pma_start
else if [ $1 == "stop" ]; then
	pma_stop
else if [ $1 == "clear" ]; then
	log_clear
fi
fi
fi
