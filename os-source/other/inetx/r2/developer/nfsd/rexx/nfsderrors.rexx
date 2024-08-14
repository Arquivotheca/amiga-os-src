/* 
 * print NFSd errors
 */

if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'NFSd') then do
	say 'NFSd is not running.'
	exit
end
call addlib 'NFSd',-1

do e=0 to 100
	say 'NFS error ' || e || ' is ' || nfsderrtxt(e)
end
