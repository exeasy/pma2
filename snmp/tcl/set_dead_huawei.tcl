#!/usr/bin/expect
#huawei router command:ospf timer dead $helloint
set timeout 10
set password 123
if { $argc == 3 } {
	set routerip [lindex $argv 0]
	set ethid [lindex $argv 1]
	set deadint [lindex $argv 2]
} else {puts "no some arg!\n";exit 1}
spawn telnet $routerip
expect "*Password*"
send ${password}\r
expect ">"
send "system-view\r"
expect "]"
send "interface $ethid\r"
expect "]"
send "ospf timer dead $deadint\r"
expect { 
	"Error: The dead interval must be greater than the hello interval."  {send "quit\r";expect "]";send "quit\r";expect ">";send "quit\r";expect eof;exit 1 } 	
	"]" }
send "quit\r"
expect "]" 
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0
