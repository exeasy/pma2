#!/bin/bash

./configure
make


mkdir pma2-agent
cp bm dbm icm pem command config tcl ./icm.ini ./interface.cfg ./pma.db ./pma-agent.sh ./pmalog.conf ./pma-agent.sh -r pma2-agent
tar cvzf pma2-agent.tar.gz pma2-agent

mkdir pma2-router
cp pem conf_detect command pma-router.sh config tcl ./pmalog.conf ./agent.conf  -r  pma2-router
tar cvzf pma2-router.tar.gz pma2-router
