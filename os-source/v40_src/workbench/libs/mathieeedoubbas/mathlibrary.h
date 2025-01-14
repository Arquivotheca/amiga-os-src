#ifndef	LIBRARIES_MATHLIBRARY_H
#define	LIBRARIES_MATHLIBRARY_H 
/*
**	$Id: mathlibrary.h,v 1.6 90/07/13 13:24:13 dale Exp $
**
**	Data structure returned by OpenLibrary of:
**	mathieeedoubbas.library,mathieeedoubtrans.library
**	mathieeesingbas.library,mathieeesingtrans.library
**
**	(C) Copyright 1987,1988 Commodore-Amiga, Inc.
**	    All Rights Reserved
*/

#ifndef EXEC_LIBRARIES_H
#include <exec/libraries.h>
#endif

struct MathIEEEBase
{
	struct Library MathIEEEBase_LibNode;
	unsigned char	MathIEEEBase_reserved[18];
	int	(*MathIEEEBase_TaskOpenLib)();
	int	(*MathIEEEBase_TaskCloseLib)();
	/*	This structure may be extended in the future */
};
/*
* Math resources may need to know when a program opens or closes this
* library. The functions TaskOpenLib and TaskCloseLib are called when 
* a task opens or closes this library. They are initialized to point to
* local initialization pertaining to 68881 stuff if 68881 resources 
* are found. To override the default the vendor must provide appropriate 
* hooks in the MathIEEEResource. If specified, these will be called 
* when the library initializes.
*/

#endif	/* LIBRARIES_MATHLIBRARY_H */
