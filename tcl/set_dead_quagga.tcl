#!/usr/bin/expect
#quagga router command: ospf dead-interval $deadint
set timeout 10
set password 111
if { $argc == 3 } {
	set routerip [lindex $argv 0]
	set ethid [lindex $argv 1]
	set deadint [lindex $argv 2]
} else { puts "no some arg!\n";exit 1}
spawn telnet ${routerip} 2604
#expect "password:" { send "$password\r" }
#expect {
#	"*yes/no*" { send "yes\r"}
#	"password:" { send "$password\r" } }
#expect "#"
#send "telnet localhost zebra\r"
expect "Password:" 
send "zebra\r"
expect ">"
send "enable\r"
expect "ospfd#"
send "configure t\r"
expect "ospfd(config)#"
send "interface $ethid\r"
expect "ospfd(config-if)#" 
send "ospf dead-interval $deadint\r"
expect "ospfd(config-if)#" 
send "quit\r"
expect "ospfd(config)#"
send "quit\r"
expect "ospfd# "
send "quit\r"
expect eof
exit 0
