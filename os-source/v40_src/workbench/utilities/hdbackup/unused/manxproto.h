/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */

/*
 *	This file declares the types of the external functions for use
 *	with the Manx C compiler.  We cannot include <functions.h>
 *	because the types declared therein do not match the types
 *	declared by Lattice's proto files.  It would make the code
 *	too messy to try to work around this, so we just create our
 *	own subset version of functions.h that have compatible type
 *	declarations.
 */

extern BPTR Open ();
extern BPTR CurrentDir ();
extern BPTR Lock ();
extern char *strchr ();
extern struct Message *GetMsg ();
extern struct Screen *OpenScreen ();
extern struct Library *OpenLibrary ();
extern void *AllocMem ();
extern struct ViewPort *ViewPortAddress ();
extern struct TextFont *OpenFont ();
extern void CloseFont ();
extern struct TextFont *OpenDiskFont ();
extern struct Window *OpenWindow ();
extern struct Task *FindTask ();
extern struct MsgPort *CreatePort ();
