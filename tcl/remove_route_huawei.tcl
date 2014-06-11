#!/usr/bin/expect
#huawei router command:ip route-static $desip $mask $nexthop
set timeout 10
set password 123
if { $argc == 4 } {
	set routerip [lindex $argv 0]
	set desip [lindex $argv 1]
	set mask [lindex $argv 2]
	set nexthop [lindex $argv 3]
} else {puts "no some arg!\n";exit 1}
spawn telnet $routerip
expect "*Password*"
send ${password}\r
expect ">"
send "system-view\r"
expect "]"
send "undo ip route-static $desip $mask $nexthop\r"
expect {
	"The route already exists." {send "quit\r";expect ">";send "quit\r";expect eof;exit 1} 
	"Error: The specified route does not exist." {send "quit\r";expect ">";send "quit\r";expect eof;exit 0} 
	"]" }
#expect "]"
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0
