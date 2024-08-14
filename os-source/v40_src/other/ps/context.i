* This file contains an instance of a context and space structure.
* This is used through out the package to enable us to add multiple
* context's at a later stage.
* All Global's should be placed inside DPSSpaceRec.
* March 1991 P.J. © CBM-Amiga
* Submitted to RCS on 05/APR/91

* Temporary Context structure

*=======================================================================
* The DPSSpaceProcsRec may be extended to include system-specific items

	STRUCTURE DPSSpace,0
		LONG	dpss_lastNameIndex
		STRUCT	dpss_stacks,st_SIZEOF			; the 4 stack headers
		APTR	dpss_MemPool
		LONG	dpss_save_level
		APTR	dpss_GState
		STRUCT	dpss_sysdict,pso_SIZEOF
		STRUCT	dpss_shrdict,pso_SIZEOF
		STRUCT	dpss_usrdict,pso_SIZEOF
		STRUCT	dpss_errdict,pso_SIZEOF
		APTR	dpss_last_dict
		APTR	dpss_StdIn
		APTR	dpss_StdOut
		BOOL	dpss_Echo
		BOOL	dpss_PackingOn
		BOOL	dpss_StrokeAdjust
		ULONG	dpss_ObjectFormat
		LONG	dpss_state
		LONG	dpss_seed
	LABEL	dpss_SIZEOF

* typedef struct {
* 	int lastNameIndex;
* *	DPSSpaceProcs procs;*/
* 	Stacks stacks;			* Handle to our suite of stacks
* 	vmem *MemPool;			* What all mem allocs should use
* 	int	save_level;			* Current save-restore level
* 	gstate *GState;			* current graphics state
* 	ps_obj sysdict;			* What all AddOperator calls use implicitly
* 	ps_obj usrdict;
* 	ps_obj errdict;			* Direct handle for interpreter !
* 	ps_obj *last_dict;		* scanned by NameLookUp (used by 'where').
* 	ps_obj *StdIn;				* Standard Input file for this context
* 	ps_obj *StdOut;			* Standard Output file for this context
* 	BOOL Echo;				* T/F for echo/no-echo
* 	BOOL PackingOn;			* T/F for packing/no-packing
* 	int state;				* Debug State
* 	int seed;				* Random number seed for this context
* 
* } DPSSpaceRec, *DPSSpace;

*=======================================================================
	STRUCTURE DPSContext,0
		STRUCT	dpsc_space,dpss_SIZEOF
	LABEL	dpsc_SIZEOF

* * DPSContext record. The current context for stacks, name-lookup etc..
* 
* * **NOTE** Only DPSSpace is defined inside the context record
* 
* typedef struct _t_DPSContextRec {
* *	void *priv;*/
* 	DPSSpace space;
* *	DPSProgramEncoding programEncoding;*/
* *	DPSNameEncoding nameEncoding;*/
* *	DPSProcs procs;*/
* *	void (*textProc)();*/
* *	void (*errorProc)();*/
* *	DPSResults resultTable;*/
* *	unsigned int resultTableLength;*/
* *	int lastNameIndex;*/
* *	struct _t_DPSContextRec *chainParent, *chainChild;
* } DPSContextRec, *DPSContext;
