; structure of the very first block on a hard disk

STRUCTURE HDB,0
	UWORD	HDB_MAGIC1		; 0xBABE indicates valid block 
	UWORD	HDB_FILL1	;  Skip 16 bits	
	ULONG	HDB_PT_BBLOCKS	;  Pointer to 1st bad block record
	ULONG	HDB_CYLINDERS	;  # of cylinders on drive
	ULONG	HDB_PRECOMP	;  Pre-comp cylinder #
	ULONG	HDB_REDUCE	;  Reduced write current cylinder
	ULONG	HDB_PARK	;  Cylinder to park heads at (0 = don't)
	UWORD	HDB_SECTORS	;  Sectors per track
	UWORD	HDB_HEADS	;  Surfaces on device
	STRUCT HDB_ENVIRONMENT,EN_SIZE ; AmigaDOS Environment for 1st partition
	UWORD	HDB_HOFFSET	;  physical offset if boot time hdlr 
	UWORD	HDB_HSIZE	;  physical size if boot time hdlr 
	STRUCT	HDB_HNAME,32	;  name of handler proc 
	ULONG	HDB_TBUFFS	;  # of track buffers (if applicable)
	STRUCT  HDB_PARTS,SDB_SIZE*6


; stripped down environment vector struct for each partition 

STRUCTURE SDB,0
	UWORD	SDB_SECTORORIGIN	; start at 0 or 1 
	UWORD	SDB_SECTORSPERBLOCK
	UWORD	SDB_BLOCKSPERTRACK
	UWORD	SDB_RESERVEDBLOCKS
	ULONG	SDB_STARTUPVALUE
	UWORD	SDB_INTERLEAVE
	ULONG	SDB_LOWERCYLINDER
	ULONG	SDB_UPPERCYLINDER
	UWORD	SDB_BUFFERS
	UWORD	SDB_HOFFSET	;  physical offset if boot time hdlr 
	UWORD	SDB_HSIZE	;  physical size if boot time hdlr 
	STRUCT	SDB_NAME,32
	LABEL	SDB_SIZE

STRUCT HDB1,0			; second disk block info 
	UWORD	HDB1_MAGIC1
	UWORD	HDB1_PAD1
	STRUCT  HDB1_PARTS,8*SDB_SIZE
	LABEL HDB1_SIZE

STRUCT ENVEC,0
	ULONG ENV_ENVECSIZE	; size of this structure in longwords  
	ULONG ENV_SECTORSIZE	; size of one sector (usually 512) 	
	ULONG ENV_SECTORORIGIN	; lowest sector number (0 ???)		
	ULONG ENV_SURFACES	; number of surfaces		
	ULONG ENV_SECTORSPERBLOCK	; number of sectors in one block 
	ULONG ENV_BLOCKSPERTRACK	; number of blocks in one track 
	ULONG ENV_RESERVEDBLOCKS	; blocks reserved for dos (2)	 
	ULONG ENV_PREALLOCATED	; ??? this ain`t used no more		
	ULONG ENV_INTERLEAVE	; very rarely used			
	ULONG ENV_LOWERCYLINDER	; start cylinder of this partition 
	ULONG ENV_UPPERCYLINDER	; end cylinder of this partition 
	ULONG ENV_BUFFERS;	; number of buffers to allocate 
	ULONG ENV_BUFFMEMTYPE	; type of memory buffers need		
	APTR  ENV_STARTUP	; startup Parameter 
	APTR ENV_HANDLERNAME	; name if used with mount 
	UWORD ENV_HANDLERSIZE	; size of handler or 0 
	UWORD ENV_HANDLEROFFSET	; offset to handler or 0 
	LABEL ENVEC_SIZE
