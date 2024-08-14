/* 
 * initialization script for NFSd 
 */

/*----------------------------------------------------------------------*/
/* 		   USER DEFINABLE PARAMETERS START HERE			*/
/*----------------------------------------------------------------------*/

/*
 * The mapfile variable sets the location where the NFS directory will
 * be stored.  It will be of length mapsize*128 bytes.  If the exported
 * filesystems are small enough you may wish to locate the directory in
 * a ram disk.  If the mapentry is kept on slow media, eg floppy disk,
 * be sure to make cachesize large.
 */
mapfile = 'big:mapfile'

/*
 * The cachesize variable sets the size of the in ram cache of NFS
 * directory entries.  The larger cachesize is set, the better directory
 * read/lookup performance will be.  The amount of ram consumed per entry
 * is about 142 bytes.
 */
cachesize = 2000

/*
 * The exports variable declares the partitions you wish to export.
 */
exports = 'ram: BIG:'

/*
 * The mapsize variable declares the maximum number of files the server
 * can export.  A good rule of thumb in setting this variable is to allow
 * one entry per 2048 bytes of disk space.  Note that only one bit is allocated
 * per entry, so a 20000 entry bitmap is 2500 bytes long.
 */
mapsize = 15000

/*
 * The fsentries variable sets the maximum number of Amiga volumes that
 * can be exported.
 */
fsentries = 3

/*
 * syncperiod sets the time between cache/bitmap flushes.  Too small a value
 * and performance suffers; too large a value may damage the NFS directory
 * if a crash occurs between flushes.  Time is specified in seconds.
 */
syncperiod = 30

/*----------------------------------------------------------------------*/
/* 		   DO NOT EDIT BEYOND THIS POINT			*/
/*----------------------------------------------------------------------*/

/* standard check for presence of libraries and NFSd */
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
 * Check if the mapfile already exists; if not, then build one
 */
builtmap = 0
if ~exists(mapfile) then do
	if buildmap(mapfile, mapsize, fsentries) < 0 then do
		say 'Build of mapping file failed'
		exit 20
	end
	builtmap = 1
end

if initmap(mapfile) < 0 then do
	say 'Could not initialize map file'
	exit 20
end

if initcache(cachesize) < 0 then do
	say 'Could not initialize cache'
	exit 20
end

/*
 * For each volume mentioned in exports, do a buildvol().  buildvol()
 * recursively reads the filesystem and enters disk files/directories
 * into the NFS directory.  Note that if the volume has already been
 * exported, then the buildvol is not done.
 */
i = 1
do forever
	volume = upper(word(exports, i))
	if volume == "" then break
	if ~exported(volume) then do
		if buildvol(volume) < 0 then do
			say 'Could not export volume ' || volume
		end
	end
	i = i + 1
end

/*
 * Main loop: periodically flush bitmap & open filehandle caches
 */
do forever
	call flushcache()
	call flushbitmap()
	call delay syncperiod*50
end
exit

/*
 * Check to see whether a volume has been exported already.  Return 0
 * if the volume is not exported, or 1 if it has.
 */
exported:
	arg str

	drop stat
	if query(stat) < 0 then do
		return 0
	end

	do j=stat.map_fs to stat.map_fs+stat.map_numfs-1
		path = getpath(j)
		if upper(path) = upper(str) then do
			return 1
		end
	end
	return 0
