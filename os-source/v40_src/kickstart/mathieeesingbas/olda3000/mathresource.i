	ifd	MATHRESOURCE_I
MATHRESOURCE_I	equ 1

	ifd	EXEC_TYPES_I
	include "exec/types.i"
	endc

	ifd EXEC_NODES_I
	include "exec/nodes.i"
	endc

*
*       The 'Init' entries are only used if the corresponding
*       bit is set in the Flags field.
*
*       So if you are just a 68881, you do not need the Init stuff
*       just make sure you have cleared the Flags field.
*
*       This should allow us to add Extended Precision later.
*
*       For Init users, if you need to be called whenever a task
*	opens this library for use, you need to change the appropriate
*	entries in MathIEEELibrary.
*

	STRUCTURE MathIEEEResource,0
		STRUCT	MathIEEE_Node,LN_SIZ
		USHORT	MathIEEE_Flags
		APTR	MathIEEE_BaseAddr	* ptr to 881 if exists *
		APTR	MathIEEE_DblBasInit
		APTR	MathIEEE_DblTransInit
		APTR	MathIEEE_SglBasInit
		APTR	MathIEEE_SglTransInit
		APTR	MathIEEE_ExtBasInit
		APTR	MathIEEE_ExtTransInit
	LABEL	MathIEEEResource_SIZE

* definations for MathIEEE_FLAGS *
	BITDEF	MATHIEEEF,DBLBAS,0
	BITDEF	MATHIEEEF,DBLTRANS,1
	BITDEF	MATHIEEEF,SGLBAS,2
	BITDEF	MATHIEEEF,SGLTRANS,3
	BITDEF	MATHIEEEF,EXTBAS,4
	BITDEF	MATHIEEEF,EXTTRANS,5

	endc !RESOURCES_MATHRESOURCE_I

