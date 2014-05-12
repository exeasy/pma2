#!/usr/bin/expect
set timeout 10
set password 123
set loop 1
set i 0
set logname ifrate.log
if { $argc == 1 } {
	set routerip [lindex $argv 0]
#	set ethid [lindex $argv 1]
} else {puts "no some arg!\n";exit 1}
spawn telnet $routerip
expect "*Password*"
send "$password\r"
log_file -noappend $logname
expect ">" 
while { $i < 3 } {
	send "display interface GigabitEthernet0/0/$i | include rate\r"
	expect ">"
	incr i 1
	set loop 1
}
send "quit\r"
expect eof
exit 0
