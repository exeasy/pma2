#!/usr/bin/expect
#huawei router command:peer $peerip route-update-interval $interval\r"
set timeout 10
set password 123
if { $argc == 4 } {
	set routerip [lindex $argv 0]
	set areaid [lindex $argv 1]
	set peerip [lindex $argv 2]
	set interval [lindex $argv 3]
} else {puts "must be 4 args!\n";exit 1}
spawn telnet $routerip
expect "*Password*"
send ${password}\r
expect ">"
send "system-view\r"
expect "]"
send "bgp $areaid\r"
expect "]"
send "peer $peerip route-update-interval $interval\r"
expect "]" 
send "quit\r"
expect "]" 
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0
