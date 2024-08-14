/*** scaling.h ***************************************************************
 *
 *  $Id: scaling.h,v 1.3 94/03/01 16:18:55 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  Scaling Header
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	scaling.h,v $
 * Revision 1.3  94/03/01  16:18:55  jjszucs
 * Changes to support quick scaling.
 * 
 * Revision 1.2  94/02/18  15:59:44  jjszucs
 * Changes through PhotoCD (Amiga CD) 40.8
 * 
 * Revision 1.1  94/01/13  17:09:36  jjszucs
 * Initial revision
 *
*/

#ifndef APP_SCALING_H

#define APP_SCALING_H

/****************************************************************************
 *                                                                          *
 *  Prototypes                                                              *
 *                                                                          *
 ****************************************************************************/

/* from scaling.c */
void scale(struct appContext *appContext,
            ULONG *srcBuffer, UWORD srcWidth, UWORD srcHeight,
            ULONG *destBuffer, UWORD destWidth, UWORD destHeight,
            UWORD destModulo);

#endif /* APP_SCALING_H */
