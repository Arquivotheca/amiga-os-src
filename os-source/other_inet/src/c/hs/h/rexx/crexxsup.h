#ifndef REXX_CREXXSUP_H
#define REXX_CREXXSUP_H TRUE

/*+-- crexxsup.h ---------------------------------------------------------+
 *|
 *|   crexxsup.h additional Arexx include file for c programmers 
 *|
 *|   $Header: crexxsup.h,v 00.2 88/08/02 22:57:03 jms Exp $
 *|
 *|   Copyright (c) 1988 by Joseph M. Stivaletta (All Rights Reserved)
 *|
 *+-----------------------------------------------------------------------+
 */

#ifndef REXX_RXSLIB_H
#include "rexx/rxslib.h"
#endif

#ifndef REXX_REXXIO_H
#include "rexx/rexxio.h"
#endif

/* The Token structure is used by the StcToken function to get all the 
 * values returned by the function.
 */

struct Token {
   STRPTR   tk_Token;               /* pointer to token */
   ULONG    tk_Length;              /* length including quotes */
   char     tk_Quote;               /* quote indicator */
   };

typedef struct RexxTask *RXTPTR;    /* Pointer to RexxTask structure */ 
typedef CPTR            ENVPTR;     /* Pointer to current storage environment */
typedef CPTR            BLOCK;      /* Pointer to block of internal memory */
typedef CPTR            PORT;       /* */
typedef unsigned char	*ARGSTRING; /* Pointer to the string buffer of a
                                       RexxArg structure */
typedef LONG            BOOLEAN;    /* TRUE (-1) or FALSE (0) */
typedef struct RexxMsg  *MSGPTR;    /* Pointer to a RexxMsg structure */
typedef struct RexxRsrc *RNODE;     /* Pointer to a RexxRsrc structure */
typedef struct List     *RLIST;     /* Pointer to a Rexx list */
typedef struct Token    *TOKENPTR;  /* Pointer to token */
typedef struct IoBuff   *IOBUFF;    /* Pointer to a Rexx IoBuff */
typedef struct Message  *PACKET;

#endif
