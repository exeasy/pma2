#!/usr/bin/expect
log_user 0
if { $argc == 5 } {
	set routerip [lindex $argv 0]
	set desip [lindex $argv 1]
	set mask [lindex $argv 2]
	set nexthop [lindex $argv 3]
	set label [lindex $argv 4]
} else {puts "no some arg!\n";exit 1}
set timeout 5
set password 123

spawn -noecho telnet $routerip
expect "*Password*"
send "$password\r"
expect ">"
send "system-view\r"
expect "]"
#send "ip route-static $desip $mask $nexthop\r"
#expect "]"
send "static-lsp ingress $desip destination $desip $mask nexthop $nexthop out-label $label\r"
expect "]"
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0

