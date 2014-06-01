#!/usr/bin/expect
log_user 0
if { $argc == 2 } {
	set routerip [lindex $argv 0]
	set desip [lindex $argv 1]
} else {puts "no some arg!\n";exit 1}
set timeout 5
set password 123

spawn -noecho telnet $routerip
expect "*Password*"
send "$password\r"
expect ">"
send "system-view\r"
expect "]"
#send "undo ip route-static $desip $mask $nexthop\r"
#expect "]"
send "undo static-lsp ingress $desip\r"
expect "]"
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0

