
typedef struct PhonySegList {
	BPTR	psl_NextSeg;		    /*  BPTR to next element in list  */
	UWORD	psl_JMP;		        /*  A 68000 JMP abs.l instruction  */
	LONG	(*psl_EntryPoint)();	/*  The address of the function  */
	UWORD	idunno;
} PHONYSEGLIST;

