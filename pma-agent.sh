#!/bin/bash

RUN_BIN="bm dbm icm "

deploy(){
	cp config/pma.conf.$1 pma.conf
#	cp config/interface.cfg.$1 interface.cfg
}

bin_start(){
	for bin in $RUN_BIN ; do
		echo "starting $bin"
		./$bin 2&>1 > $bin.log &
		sleep 8
	done
}

bin_stop(){
	for bin in $RUN_BIN ; do
		echo "stoping $bin"
		killall $bin
	done
}

log_clean(){
	for bin in $RUN_BIN ; do
		echo "cleanning $bin.log"
		> $bin.log
	done
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
