/* general macros for C programming */

#define MAX(a,b) 	 ((a)>(b)?(a):(b))
#define MIN(a,b) 	 ((a)<(b)?(a):(b))
#define ABS(x) 		 ((x<0)?(-(x)):(x))
#define CLAMP(a,b,c) ((b)<(a)?(a): ((b)>(c)?(c):(b)) )

#define MAKE_EVEN(n) (((n) + 1) & ~1L)

#define unless(a)	if (!(a))

void *AllocMem();

#define	MakeStruct(x)          (x = AllocMem(sizeof *x,0) )
#define	MakeClearStruct(x)     (x = AllocMem(sizeof *x,MEMF_CLEAR) )
#define	MakeChip(x)            (x = AllocMem(sizeof *x,MEMF_CHIP) )
#define	MakePublic(x)          (x = AllocMem(sizeof *x,MEMF_PUBLIC) )
#define	MakeStructArray(x,n)   (x = AllocMem(n * (sizeof *x),0) )
#define	MakeChipArray(x,n)     (x = AllocMem(n * (sizeof *x),MEMF_CHIP) )
#define	MakeClearArray(x,n)    (x = AllocMem(n * (sizeof *x),MEMF_CLEAR) )
#define	MakePublicArray(x,n)   (x = AllocMem(n * (sizeof *x),MEMF_PUBLIC) )

#define	UnMakeStruct(x)        if (x) FreeMem(x,sizeof *x)
#define	UnMakeStructArray(x,n) if (x) FreeMem(x,n * (sizeof *x))
#define	DeleteStruct(x)        if (x) { FreeMem(x,sizeof *x); x = NULL; }
#define	DeleteStructArray(x,n) if (x) { FreeMem(x,n * (sizeof *x)); x = NULL; }

#define	FIELD_OFFSET(type,field) (LONG)&(((type *)0)->field)

typedef struct FileHandle *AFILE;

/* standard list traversing functions */

#ifdef EXEC_LISTS_H

#define FIRSTNODE(list)  (((struct MinList)(list)).mln_Head)
#define NEXTNODE(n) 	 (((struct MinNode *)(n))->mln_Succ)
#define MNODE(n)		 (struct MinNode *)(n)
#define WALKLIST(list,n) for (MNODE(n)=FIRSTNODE(list); NEXTNODE(n); MNODE(n) = NEXTNODE(n))
#define LISTEMPTY(list)  ( NEXTNODE(FIRSTNODE(list)) == NULL )

#endif
