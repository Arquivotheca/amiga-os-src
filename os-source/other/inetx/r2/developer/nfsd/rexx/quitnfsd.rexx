/* 
 * shutdown NFS server
 */

if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'NFSd') then do
	say 'NFSd is not running.'
end
call addlib 'NFSd',-1

call quit()
