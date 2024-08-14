/* 
 * dump NFS directory
 */
args min max

if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'NFSd') then do
	say 'NFSd is not running'
	exit 20
end
call addlib 'NFSd',-1

/*
 * get statistics from NFSd
 */
if query(stat) < 0 then do
	say "Could not get server statistics"
	exit 10
end

say "--- Dump of NFS directory ---"
do i=0 to stat.map_nummap-1
	path = getpath(i)
	say "Entry " || i || " is " || path
end

exit
