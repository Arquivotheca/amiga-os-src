/* sample usage of rexx network interface primitives */

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
	call addlib 'socketrexx',-1
end

say "Interfaces:"  socket_IFgetconf()

intf = "ae0"
say intf "routing metric is set to" socket_IFgetmetric(intf)

if socket_IFgetaddr(intf, addr) < 0 then do
	say "Could not get IF addr because" socket_errtxt(INETerror)
end
else do
	say "IF addr is" addr.domain":" addr.sin_addr
end
drop addr

if socket_IFgetdstaddr(intf, addr) < 0 then do
	say "Could not get IF dstaddr because" socket_errtxt(INETerror)
end
else do
	say "IF dst addr is" addr.domain":" addr.sin_addr
end
drop addr

if socket_IFgetbrdaddr(intf, addr) < 0 then do
	say "Could not get IF brd addr because" socket_errtxt(INETerror)
end
else do
	say "IF brd addr is" addr.domain":" addr.sin_addr
end
drop addr

if socket_IFgetnetmask(intf, addr) < 0 then do
	say "Could not get IF netmask because" socket_errtxt(INETerror)
end
else do
	say "IF netmask is" addr.domain":" addr.sin_addr
end
drop addr

say "Interface flags for" intf "are:" socket_IFgetflags(intf)

intf = "sl1"
say ""
say "Try to set address for" intf

if socket_IFgetaddr(intf, addr) < 0 then do
	say "Could not get IF addr because" socket_errtxt(INETerror)
end
else do
	say "IF addr is" addr.domain":" addr.sin_addr
end
drop addr

addr.domain = "inet"
addr.sin_addr = "10.0.0.78"
addr.sin_port = "0"
if socket_IFsetaddr(intf, addr) < 0 then do
	say "Could not set address because" socket_errtxt(INETerror)
end
else do
	drop addr

	if socket_IFgetaddr(intf, addr) < 0 then do
		say "Could not get IF addr because" socket_errtxt(INETerror)
	end
	else do
		say "IF addr is" addr.domain":" addr.sin_addr
	end
	drop addr
end

intf="ae0"
say "Interface flags for" intf "are:" socket_IFgetflags(intf)
intf="lo0"
say "Interface flags for" intf "are:" socket_IFgetflags(intf)
call socket_IFsetflags(intf, "UP NOARP BROADCAST -BIT14")
say "Interface flags for" intf "are:" socket_IFgetflags(intf)
call socket_IFsetflags(intf, "-UP -NOARP -BROADCAST BIT14")
say "Interface flags for" intf "are:" socket_IFgetflags(intf)

exit
