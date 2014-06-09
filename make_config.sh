#!/bin/bash

TEMPLATE=pma.conf

RLIST="1 2 3 4 5 6 7 8 111 112 113 114 121 122 123 124 131 132 133 134 135 136 137 138 211 212 213 214 221 222 223 224 231 232 233 234 235 236 237 238" 

for i in $RLIST; do
	cat $TEMPLATE | awk '{ if (NR == 7) print "<pma_id>0.0.0.'$i'</pma_id>";
	else if (NR==28) print "<router_ip>127.0.0.1</router_ip>"; 
	else if (NR==29) print "<local_ip>192.168.3.'$i'</local_ip>"; 
	else if (NR==30) print "<hello_ip>10.10.'$i.$i'</hello_ip>"; 
	else print $0 }'  > config/pma.conf.pma$i
done
