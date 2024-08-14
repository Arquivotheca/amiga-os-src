/* -----------------------------------------------------------------------
 * sslib.h              defines for use with ss.library
 *
 * $Locker$
 *
 * $Id$
 *
 * $Revision$
 *
 * $Header$
 *
 * $Log$
 *
 * bj
 *------------------------------------------------------------------------
 */


#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/execbase.h>
#include <libraries/dosextens.h>

/*
 * sslib library structure - this is C version of that defined
 * in sslib_def.i
 */

struct SSLib {
        struct Library          ss_SSLib ;
        struct ExecBase        *ss_SysLib ;
        struct DosLibrary      *ss_DosLib ;
        struct InetLibrary     *ss_InetLib ;
        APTR                    ss_SegList ;
        UBYTE                   ss_Flags ;
        UBYTE                   ss_pad ;
        } ;


#define NODE_ID 100

#define SSLIBNAME "sslib.library"


/* ---- end ---- */
