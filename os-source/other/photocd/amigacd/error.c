/*** error.c *****************************************************************
 *
 *  $Id: error.c,v 1.2 94/02/18 15:55:29 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Error-Handling Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	error.c,v $
 * Revision 1.2  94/02/18  15:55:29  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 * 
 * Revision 1.1  94/01/06  11:56:54  jjszucs
 * Initial revision
 *
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* Amiga includes */
#include <exec/types.h>

#include <dos/dos.h>

/* ANSI includes */
#include <stdio.h>

/* Local includes */
#include "global.h"

/****************************************************************************
 *                                                                          *
 *  fatalError() - fatal error                                              *
 *                                                                          *
 ****************************************************************************/

void fatalError(struct appContext *appContext,STRPTR message)
{

    printf("%s: %s\n",PROGRAM_NAME,message);

    goodbye(appContext,RETURN_FAIL);

}
