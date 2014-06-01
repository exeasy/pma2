#!/usr/bin/expect
log_user 0
if { $argc == 5 } {
	set routerip [lindex $argv 0]
	set inlabel [lindex $argv 1]
	set outlabel [lindex $argv 2]
	set inif [lindex $argv 3]
	set nexthop [lindex $argv 4]
} else {puts "no some arg!\n";exit 1}
set timeout 5
set password 123

spawn telnet $routerip
expect "*Password*"
send "$password\r"
expect ">"
send "system-view\r"
expect "]"
send "static-lsp transit HtoR incoming-interface interface $inif in-label $inlabel nexthop $nexthop out-label $outlabel\r"
expect "]"
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0

