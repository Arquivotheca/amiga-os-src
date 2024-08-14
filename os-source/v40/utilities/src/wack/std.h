/* $Id: std.h,v 1.4 91/04/24 20:54:21 peter Exp $	*/
/****** std.h *******************************************************/
/*								    */
/*		Confidential Information: Amiga Computer, Inc.	    */
/*		Copyright (c) Amiga Computer, Inc.		    */
/*								    */
/*		    Modification History			    */
/*  date    :	author :    Comments				    */
/*  -------	------	    --------------------------------------- */
/*  6-Aug-84	MJS	    added CUB and CUW types		    */
/*  7-Aug-84	MJS	    added void and VOID types		    */
/*  8-24-84	Dale	    added this header file		    */
/*								    */
/********************************************************************/

/************************************************************************
 *  std.h -- The Standard AMIGA Header for C Types			*
 *		(for Whitesmith C Compiler)				*
 ************************************************************************/

/*  Storage Classes */
#define GLOBAL	extern	    /* The declaratory use of an external */
#define IMPORT	extern	    /* reference to an external */
#define STATIC	static	    /* a local static variable */
#define REGISTER register   /* a (hopefully) register variable */

typedef int		VOID;
typedef int		void;
typedef unsigned char	CUB;	    /* compatibility unsigned byte */
typedef unsigned short	CUW;	    /* compatibility unsigned word */

typedef long		LONG;	    /* signed 32-bit quantity */
typedef unsigned long	ULONG;	    /* unsigned 32-bit quantity */
typedef unsigned long	LONGBITS;   /* 32 bits manipulated individually */
typedef short		SHORT;	    /* dl */
typedef unsigned short	USHORT;	    /* "  */
typedef short		WORD;	    /* signed 16-bit quantity */
typedef unsigned short	UWORD;	    /* unsigned 16-bit quantity */
typedef unsigned short	WORDBITS;   /* 16 bits manipulated individually */
typedef char		BYTE;	    /* signed 8-bit quantity */
typedef unsigned char	UBYTE;	    /* unsigned 8-bit quantity */
typedef unsigned char	BYTEBITS;   /* 8 bits manipulated individually */
typedef unsigned char	*SPTR;	    /* string pointer */
typedef SPTR		*APTR;	    /* absolute memory pointer */

/*	Types with specific semantics */
typedef float		FLOAT;
typedef double		DOUBLE;
typedef short		COUNT;
typedef unsigned short	UCOUNT;
typedef short		BOOL;
typedef unsigned char	TEXT;
typedef short		FILE;

/*  System Constants */
#define STDIN		0
#define STDOUT		1
#define STDERR		2
#define TRUE		1
#define FALSE		0
#define NULL		0
#define EOF		-1
#define BYTEMASK	0xFF

