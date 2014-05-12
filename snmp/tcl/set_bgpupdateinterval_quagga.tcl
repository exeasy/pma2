#!/usr/bin/expect
#quagga router command: neighbor $peerip advertisement-interval $inteval\r"
set timeout 10
set password zebra
if { $argc == 4 } {
	set routerip [lindex $argv 0]
	set areaid [lindex $argv 1]
	set peerip [lindex $argv 2]
	set interval [lindex $argv 3]
} else { puts "must be 4 args!\n";exit 1}
spawn telnet ${routerip} 2605
expect "Password:" 
send "$password\r"
expect ">"
send "enable\r"
expect "bgpd#"
send "configure t\r"
expect "bgpd(config)#"
send "router bgp $areaid\r"
expect "bgpd(config-router)#" 
send "neighbor $peerip advertisement-interval $interval\r"
expect "bgpd(config-router)#" 
send "quit\r"
expect "bgpd(config)#"
send "quit\r"
expect "bgpd# "
send "quit\r"
expect eof
exit 0
