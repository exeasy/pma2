#!/usr/bin/expect
#quagga router command: ip route $desip $mask $nexthop
set timeout 10
set password 111
if { $argc == 4 } {
	set routerip [lindex $argv 0]
	set desip [lindex $argv 1]
	set mask [lindex $argv 2]
	set nexthop [lindex $argv 3]
} else {puts "no some arg!\n";exit 1}
spawn telnet ${routerip} 2601
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
expect "*Password*" 
send "zebra\r"
expect "#"
send "configure t\r"
expect "(config)#"
send "no ip route $desip $mask $nexthop\r"
expect "(config)#" 
send "quit\r"
expect "#"
send "quit\r"
expect eof
exit 0
