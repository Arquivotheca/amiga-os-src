/* 
 * routing primitive demos
 */
if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end

if ~showlist('P', 'netrexx') then do
	say 'netrexx is not running, trying to start it.'
	address command
	'run >nil: inet:c/netrexx'
	'waitforport netrexx'
	if ~showlist('P', 'netrexx') then do
		say 'Could not start netrexx, exiting!'
		exit 20
	end
end
call addlib 'netrexx',-1

berk.domain = "inet"
berk.sin_addr = "10.0.0.78"
berk.sin_port = 0

srinic.domain = "inet"
srinic.sin_addr = "10.0.0.51"
srinic.sin_port = 0

ae0.domain = "inet"
ae0.sin_addr = "192.12.90.2"
ae0.sin_port = 0

bad.domain = "inet"
bad.sin_addr = "1.2.3.4"
bad.sin_port = 0

call rtdeladdr("host", berk, ae0)
call rtdeladdr("host", srinic, ae0)

say "addrt returns " ||  rtaddaddr("host", berk, ae0)
say "addrt returns " ||  rtaddaddr("host", srinic, ae0)
say "delrt returns " ||  rtdeladdr("host", srinic, ae0)
say "Following route should fail"
say "     delrt returns " ||  rtdeladdr("host", bad, ae0) /* fails */
say "     error code: " || neterrtxt(INETerror)

max = 75
say "Add " || max || " routes to routing table"
do i = 1 to max
	berk.sin_addr = "10.0.0." || i
	call rtaddaddr("host", berk, ae0)
end
say "Delete " || max || " routes to routing table"
do i = 1 to 100
	berk.sin_addr = "10.0.0." || i
	call rtdeladdr("host", berk, ae0)
end
