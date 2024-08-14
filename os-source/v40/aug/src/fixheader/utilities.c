/*
**  $Header: HD:Sources/FixHeader/RCS/utilities.c,v 1.4 92/09/03 14:42:30 peter Exp $
**
**  Conversion routines used in FixHeader 
**
**  (C) Copyright 1989 Commodore-Amiga, Inc.
**      All Rights Reserved
*/

/***************************************************************\
*								*
*	UTILITIES.C						*
*	------------------------------------------------------	*
*								*
*	Table of Contents:					*
*								*
*		Functions Defined:				*
*		------------------				*
*								*
*		ltoa(char *, long)				*
*			Convert a long value to an ASCII string	*
*								*
*****************************************************************
*	Revision History					*
*	----------------					*
*								*
*	Original Coding 05-May-89		Kevin Klop	*
*								*
\***************************************************************/

#include "fixheader.h"

static char	*BufferPtr;
static char	 IterationCount;

static void cvrt_LtoA( long LongVal );

char	*ltoa(String,LongVal)
char	*String;
long	 LongVal;
{
	BufferPtr = String;
	IterationCount = 0;
	cvrt_LtoA(LongVal);
	return (String);
}

static void cvrt_LtoA(LongVal)
long	LongVal;
{
	if (LongVal != 0)
	{
		IterationCount++;
		cvrt_LtoA(LongVal/10L);
		*(BufferPtr++) = (char)((LongVal % 10L) & 0xffL) + '0';
	}
	else
	{
		if (!IterationCount)
			*(BufferPtr++) = '0';
		*BufferPtr = '\0';
	}
}
