// This file contains the Context structure.
// All fields towards the end of the Context struct are really part of
// a DPSSpace struct. (If we ever upgrade to DPS, move this sub-struct out of
// the Context).
// This is used through out the package to enable us to add multiple
// context's at a later stage.
// All Global's should be placed inside the Space sub-structure.
// March 1991 P.J. © CBM-Amiga
// May 1991 L.V.
// Submitted to RCS on 05/APR/91

/* Temporary Context structure */


#ifndef CONTEXT_H
#define CONTEXT_H

#include <intuition/intuition.h>
#include <intuition/screens.h>


typedef struct File_List FileList;

struct File_List {
	FileList *next;
	ps_obj *file;
};


/* The DPSSpaceProcsRec may be extended to include system-specific items */

typedef struct {
	int lastNameIndex;

/*	DPSSpaceProcs procs; */

	StackRec stacks;		// Our 4 stacks live in here...

	vmem *MemPool;			// What all mem allocs should use
	int	save_level;			// Current save-restore level
	gstate *GState;			// current graphics state
	ps_obj sysdict;			// What all AddOperator calls use implicitly
	ps_obj shrdict;
	ps_obj usrdict;
	ps_obj errdict;			// Direct handle for interpreter !
	ps_obj *last_dict;		// scanned by NameLookUp (used by 'where').
	ps_obj *StdIn;			// Standard Input file for this context
	ps_obj *StdOut;			// Standard Output file for this context
	BOOL Echo;				// T/F for echo/no-echo
	BOOL PackingOn;			// T/F for packing/no-packing
	BOOL StrokeAdjust;		// T/F for adjusted stroking
	int	ObjectFormat;		// 1..4 number rep in binary obj sequences
	int state;				// Debug State
	int seed;				// Random number seed for this context
	int recurse;			// Recursion level for this context

	char **SysNameTab;		// An array of pointers to the first 256 function
							// Names.

	FileList *flist;		// This is a link list of files currently open by
							// this context.

	stream *ip;				// The initial file object pointer on the e stack
} DPSSpace ;

/************************************************************************/
/* DPSContext record. The current context for stacks, name-lookup etc.. */

/* **NOTE** Only DPSSpace is defined inside the context record 			*/
/************************************************************************/


// A debug structure. Will be removed !!!
struct Db {
	ps_obj *OperSp;
	ps_obj *ExecSp;
	ps_obj *DictSp;
	ps_obj *GsavSp;
	int Onum;
	int Enum;
	int Dnum;
	int Gnum;
	long Del;
	long ExecMode;
};

typedef struct _t_DPSContextRec {
/*	void *priv;*/
	DPSSpace space;				// INSTANCE of a struct !

// stuff needed for the kluged rendering code for testing (will be removed)
	struct IntuitionBase *IntuitionBase;
	struct GfxBase *GfxBase;
	struct Screen *screen;
	struct RastPort *rp;
	struct Window *window;
// some more kluged stuff, this time it's for the debug window 
	struct Window *wind;
	struct Db Debg;

// stdout file object. To use it, Cast to (FILE *)
	int STDOUT;

	void	*hashtab;			// pointer to Name Space hashchain array

/*	DPSProgramEncoding programEncoding;*/
/*	DPSNameEncoding nameEncoding;*/
/*	DPSProcs procs;*/
/*	void (*textProc)();*/
/*	void (*errorProc)();*/
/*	DPSResults resultTable;*/
/*	unsigned int resultTableLength;*/
/*	int lastNameIndex;*/
/*	struct _t_DPSContextRec *chainParent, *chainChild; */
} DPSContextRec, *DPSContext;

#endif
