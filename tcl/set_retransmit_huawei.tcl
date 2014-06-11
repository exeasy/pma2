#!/usr/bin/expect
#huawei router command:ospf timer retransmit $retransmitint
set timeout 10
set password 123
if { $argc == 3 } {
	set routerip [lindex $argv 0]
	set ethid [lindex $argv 1]
	set retransmitint [lindex $argv 2]
} else {puts "no some arg!\n";exit 1}
spawn telnet $routerip
expect "*Password*"
send ${password}\r
expect ">"
send "system-view\r"
expect "]"
send "interface $ethid\r"
expect "]"
send "ospf timer retransmit $retransmitint\r"
expect "]" 
send "quit\r"
expect "]" 
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0
