/************************************************************************
			 Miscellaneous handy stuff
*************************************************************************/

// Handy macros...
//----------------

#define VM	ctxt->space.MemPool
#define pso	ps_obj

#define PDEBUG	printf
#define	QUOTED	"\"%s\""	// to quote any string variables in printf

#define RUN_MARKER	12345	// magic number for "RUN" context marker

// Macros to be used in operator code.
//------------------------------------

#define	OBJ_TYPE(OB)	((OB)->type & TYPE_MASK)
#define	OBJ_ATTR(OB)	((OB)->type & ATTR_MASK)
#define	OBJ_AXES(OB)	((OB)->type & ACCESS_MASK)
#define	SAVE_LEVEL(OB)	(OB)->tag = ctxt->space.save_level
#define	NEED_ARGS(N)	if (NUMOPER(ctxt) <(N)) return ERR_stackunderflow
#define	ENOUGH_ROOM(N)	if (NUMOPER(ctxt)+(N) > O_SIZE-1) return ERR_stackoverflow
#define	ENOUGH_EROOM(N)	if (NUMEXEC(ctxt)+(N) > E_SIZE-1) return ERR_execstackoverflow
#define MUST_BE_INT(I)	if (OBJ_TYPE(I)!=TYPE_INT) return ERR_typecheck
#define	MUST_BE_NUMERIC(A)	if (OBJ_TYPE(A)!=TYPE_INT && OBJ_TYPE(A)!=TYPE_REAL) return ERR_typecheck
#define	MUST_BE_PROC(P)	if ((OBJ_TYPE(P)!=TYPE_ARRAY && OBJ_TYPE(P)!=TYPE_PACKED) || OBJ_ATTR(P)!=ATTR_EXECUTE) return ERR_typecheck
#define	FORCE_REAL(N)	if (OBJ_TYPE(N) == TYPE_INT) (N)->obj.realval = (float) (N)->obj.intval
#define UNIQUE_STRPTR(S)	AddName(ctxt,S);
#define PUSHNULL		PUSHOPER(ctxt,&n_obj)
#define PUSHMARK		PUSHOPER(ctxt,&m_obj)
#define PUSHTRUE		PUSHOPER(ctxt,&t_obj)
#define PUSHFALSE		PUSHOPER(ctxt,&f_obj)

#define	MAX_STRING		65535		// according to Appendix B
#define	MAX_ARRAY		65535		// according to Appendix B

#define	OP	((FILE *)ctxt->STDOUT)
// Globals

IMPORT int		verbose;

// Global skeleton objects (handy for pushing an int or a real or a ...)
//----------------------------------------------------------------------

IMPORT pso		t_obj,f_obj,n_obj,m_obj;			// TRUE,FALSE,NULL,MARK
IMPORT pso		int_obj,real_obj,name_obj,dict_obj;
