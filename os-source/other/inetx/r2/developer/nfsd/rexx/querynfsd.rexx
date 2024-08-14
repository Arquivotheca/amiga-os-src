/* 
 * sample script to query NFSd
 */
args min max

if ~show('L','rexxsupport.library') then do
	if ~addlib("rexxsupport.library",0,-30) then do
		say 'rexxsupport.library not found, exiting!'
		exit 20
	end
end
if ~showlist('P', 'NFSd') then do
	say 'NFSd is not running, trying to start it.'
	address command
	'run >nil: inet:serv/NFSd'
	'waitforport NFSd'
	if ~showlist('P', 'NFSd') then do
		say 'Could not start netrexx, exiting!'
		exit 20
	end
end
call addlib 'NFSd',-1

/*
 * get statistics from NFSd
 */
if query(stat) < 0 then do
	say "Could not get server statistics"
	exit 10
end

say "--- NFS server statistics ---"
say "Reads:           " || stat.nfsd_read
say "Writes:          " || stat.nfsd_write
say "Directory reads: " || stat.nfsd_readdir
say " "

say "--- MNT server statistics ---"
say "Mounts:          " || stat.mntd_mnt
say "Unmounts:        " || stat.mntd_umnt
say "Dumps:           " || stat.mntd_dump
say " "

say "--- Call activity ---"
say "Number of Rexx calls made is  " || stat.rexx_call
say "Number of MNT calls made is   " || stat.mntd_call
say "Number of NFS calls made is   " || stat.nfsd_call
say " "

say "--- Mapping table statistics ---"
say "Number of filesystem slots is " || stat.map_numfs
say "Number of mapping entries is  " || stat.map_nummap
say "Number of bitmap entries is   " || stat.map_bsize
say " "

exit
