/* ARP (= Address Resolution Protocol) primitives */

/*
 * standard check on presence of required library & server
 */
if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'socketrexx') then do
	say 'netrexx is not running, trying to start it.'
	address command
	'run >nil: inet:c/socketrexx'
	'waitforport socketrexx'
	if ~showlist('P', 'socketrexx') then do
		say 'Could not start socketrexx, exiting!'
		exit 20
	end
end
call addlib 'socketrexx',-1

/*
 * set an ARP entry for internet addr 192.12.90.11 with phys addr
 * 01:02:03:04:FE:af
 */
pad.domain = "inet"
pad.sin_addr = "192.12.90.11"
pad.sin_port = 0
had.domain = "unspec"
had.sa_data = "01:02:03:04:FE:af"
if socket_arpsetaddr(pad, had, "perm") < 0 then do
	say "socket_arpsetaddr() failed because" socket_errtxt(INETerror)
end
else do
	say "socket_arpsetaddr() OK"
end

/*
 * delete physical address for host 192.12.90.10
 */
sparc.domain = "inet"
sparc.sin_addr = "192.12.90.10"
sparc.sin_port = 0
if socket_arpgetaddr(sparc, sha) < 0 then do
	say "socket_arpgetaddr() failed because" socket_errtxt(INETerror)
	exit
end
else do
	say "proto addr" sparc.sin_addr "=" sha.sa_data
end
