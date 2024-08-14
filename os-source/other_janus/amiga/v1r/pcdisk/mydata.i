		INCLUDE	"exec/types.i"

MAXFILES	EQU	8			max number of files open
DBUFSIZE	EQU	8192			8K data buffer
LIBVERSION	EQU	0

	STRUCTURE onefile,0
		ULONG	of_Handle		the file handle
		ULONG	of_Position		current position in file
	LABEL	of_SIZEOF

	STRUCTURE mydata,0
		APTR	md_JanusLib		janus library pointer
		APTR	md_ExecLib		exec library pointer
		APTR	md_DosLib		DOS library pointer
		STRUCT	md_Files,MAXFILES*of_SIZEOF	array of opened volumes
		UWORD	md_FilesOpen		number of volumes open
		UWORD	md_SigBit		signal bit allocated
		ULONG	md_SigMask		signal mask for Wait
		APTR	md_JanusSig		the janus sig struct
		APTR	md_ParamBlock		all commands/errors go here
		APTR	md_DataBuffer		all data goes here
	LABEL md_SIZEOF
