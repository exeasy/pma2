#!/usr/bin/expect
log_user 0
if { $argc == 3 } {
	set routerip [lindex $argv 0]
	set inif [lindex $argv 1]
	set inlabel [lindex $argv 2]
} else {puts "no some arg!\n";exit 1}
set timeout 5
set password 123

spawn telnet $routerip
expect "*Password*"
send "$password\r"
expect ">"
send "system-view\r"
expect "]"
send "static-lsp egress RtoH incoming-interface $inif in-label $inlabel\r"
expect "]"
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0

