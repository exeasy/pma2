#!/usr/bin/expect
log_user 0
if { $argc == 1 } {
	set routerip [lindex $argv 0]
} else {puts "no some arg!\n";exit 1}
set timeout 5
set password 123

spawn telnet $routerip
expect "*Password*"
send "$password\r"
expect ">"
send "system-view\r"
expect "]"
send "undo static-lsp ingress RtoH\r"
expect "]"
send "undo static-lsp transit RtoH\r"
expect "]"
send "undo static-lsp egress RtoH\r"
expect "]"
send "quit\r"
expect ">"
send "quit\r"
expect eof
exit 0

