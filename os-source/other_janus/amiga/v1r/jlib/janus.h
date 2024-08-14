
/**********************************************************************
*
* janus.h -- software conventions for janus subsystem
*
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
*
***********************************************************************/


#ifndef     EXEC_TYPES_I
#include    "exec/types.h"
#endif	    EXEC_TYPES_I

#ifndef     EXEC_LIBRARIES_I
#include    "exec/libraries.h"
#endif	    EXEC_LIBRARIES_I

#ifndef     EXEC_INTERRUPTS_I
#include    "exec/interrupts.h"
#endif	    EXEC_INTERRUPTS_I

/*
** As a coding convenience, we assume a maximum of 32 handlers.
** People should avoid using this in their code, because we want
** to be able to relax this constraint in the future.  All the
** standard commands' syntactically support any number of interrupts,
** but the internals are limited to 32.
*/

#define MAXHANDLER  32

typedef   UWORD     RPTR;


/* JanusAmiga -- amiga specific data structures for janus project */

struct JanusAmiga {
    struct Library ja_LibNode;
    ULONG	ja_IntReq;	    /* software copy of outstanding requests */
    ULONG	ja_IntEna;	    /* software copy of enabled interrupts */
    UBYTE      *ja_ParamMem;	    /* ptr to (byte arranged) param mem */
    UBYTE      *ja_IoBase;	    /* ptr to base of io register region */
    UBYTE      *ja_ExpanBase;	    /* ptr to start of shared memory */
    APTR	ja_ExecBase;	    /* ptr to exec library */
    APTR	ja_SegList;	    /* ptr to loaded code */
    struct Interrupt **ja_IntHandlers;/* base of array of int handler ptrs */
    struct Interrupt ja_IntServer;  /* INTB_PORTS server */
    struct Interrupt ja_ReadHandler;/* JSERV_READAMIGA handler */
    UWORD	ja_KeyboardRegisterOffset;	/* exactly that */
    UWORD	ja_ATFlag;		/* 1 if this is an AT */
    UWORD	ja_ATOffset;		/* offset to the AT ROM bank */
};


/* hide a byte field in the lib_pad field */
#define     ja_SpurriousMask	lib_pad

/* magic constants for memory allocation */
#define MEM_TYPEMASK	0x00ff	/* 8 memory areas */
#define MEMB_PARAMETER	(0)	/* parameter memory */
#define MEMB_BUFFER	(1)	/* buffer memory */

#define MEMF_PARAMETER	(1<<0)	/* parameter memory */
#define MEMF_BUFFER	(1<<1)	/* buffer memory */

#define MEM_ACCESSMASK	    0x3000  /* bits that participate in access types */
#define MEM_BYTEACCESS	    0x0000  /* return base suitable for byte access */
#define MEM_WORDACCESS	    0x1000  /* return base suitable for word access */
#define MEM_GRAPHICACCESS   0x2000  /* return base   "    for graphic access */
#define MEM_IOACCESS	    0x3000  /* return base suitable for io access */

#define TYPEACCESSTOADDR    5	    /* # of bits to turn access mask to addr */



#define JANUSNAME	 "janus.library"

