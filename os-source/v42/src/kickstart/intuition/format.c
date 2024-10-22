/*** fmt.c *****************************************************************
 *
 *  format string/args processing for EZRequester
 *
 *  $Id: format.c,v 40.0 94/02/15 18:03:03 davidj Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (C) 1989, Commodore-Amiga, Inc.
 *  All Rights Reserved
 *
 ****************************************************************************/

#include "intuall.h"
#include "ezreq.h"
#include <exec/memory.h>

/*****************************************************************************/

#define D(x)	;

/*****************************************************************************/

static struct IntuiText *getITextList (LONG num, struct Remember **remkey)
{
    struct IntuiText *itext = NULL;
    struct IntuiText *it;

    D (printf ("getITL num > 0 %ld\n", num));
    if ((num > 0) && (itext = (struct IntuiText *) AllocRemember (remkey, num * (sizeof *itext), MEMF_CLEAR)))
    {
	it = itext;
	while (--num)
	{
	    it->FrontPen = AUTOFRONTPEN;
	    it->BackPen  = AUTOBACKPEN;
	    it->DrawMode = AUTODRAWMODE;
	    it->NextText = it + 1;
	    it = it->NextText;
	}
    }

    D (printf ("gITL returning %lx\n", itext));
    return (itext);
}

/*****************************************************************************/

/* converts format/args into a list of ITList nodes. returns 'next' position in arglist. */
struct ITList *formatITList (UBYTE *fmt, UWORD *arglist, UWORD **nextargp, struct Remember **remkey, LONG separator)
{
    struct ITList *itl;

    D (printf ("formatITList format string %s\n", fmt));

    if (itl = (struct ITList *) AllocRemember (remkey, sizeof *itl, MEMF_CLEAR))
    {
	D (printf ("fITL got itl at %lx\n", itl));
	D (printf ("fITL call pass1 buffcount at %lx, numit at %lx\n",
		   &itl->itl_BufferSize, &itl->itl_NumIText));

	fmtPass1 (fmt, arglist, (LONG)&itl->itl_BufferSize, (LONG)&itl->itl_NumIText, separator);

	D (printf ("fITL got buffsize (from pass1) %ld numit %lx\n",
		   itl->itl_BufferSize, itl->itl_NumIText));

	if (itl->itl_BufferSize
	    && (itl->itl_Buffer =
		(UBYTE *) AllocRemember (remkey, itl->itl_BufferSize, 0))
	    && (itl->itl_IText = getITextList (itl->itl_NumIText, remkey)))
	{
	    /* format and point into buffer  */
	    D (printf ("calling pass2\n"));
	    /* update 'arglist' pointer to "next" argument */
	    arglist = fmtPass2 (fmt, arglist, (LONG)itl->itl_Buffer, (LONG)itl->itl_IText, separator);
	    D (printf (" pass2 returned buffer %lx\n", itl->itl_Buffer));
	}
	else
	{
	    D (printf ("fITL bailing out and freeing, itext %lx\n",
		       itl->itl_IText));
	    itl = NULL;
	}
    }

    /* pass "next" pointer back to caller	*/
    if (nextargp)
	*nextargp = arglist;

    D (printf ("fITL returning %lx\n", itl));
    return (itl);
}

/*****************************************************************************/

/* The following functions are now in assembler in aformat.asm */

#if 0
void freeITList (struct ITList *itl)
{
    struct IntuiText *itext;

    if (itl)
    {
	/* free buffer */
	jfreeMem (itl->itl_Buffer, itl->itl_BufferSize);

	/* free IntuiText list	*/
	jfreeMem (itl->itl_IText, itl->itl_NumIText * sizeof (struct IntuiText));

	/* free ITList itself */
	FreeMem (itl, sizeof *itl);
    }
}

/*****************************************************************************/

/* count up total characters in formatted string,
 * also count up number of 'separated' string components */
void fmtPass1 (fmt, arglist, ccountp, itcountp)
UBYTE *fmt;			/* format string	*/
UWORD *arglist;			/* format args		*/
UWORD *ccountp;			/* character count	*/
UWORD *itcountp;		/* string count		*/
{
    *ccountp = 0;
    *itcountp = 1;

    while (*fmt != '\0')
    {
	if (*fmt == SEPARATOR)
	    (*itcountp)++;
	(*ccountp)++;
	fmt++;
    }
}

/*****************************************************************************/

/*
 * does formatting into 'buffer', replacing separators with '\0',
 * and pointing to components a list of IntuiText structs.
 *
 * returns next member in arglist
 */
UWORD *fmtPass2 (UBYTE *fmt, UWORD *arglist, UBYTE *buffer, struct IntuiText *itext)
{
    if (itext)
	itext->IText = buffer;

    while ((*buffer = *fmt++) != '\0')
    {
	if (*buffer == SEPARATOR)
	{
	    *buffer = '\0';
	    if (itext && (itext = itext->NextText))
	    {
		itext->IText = buffer + 1;
	    }
	}

	buffer++;
    }
    return (NULL);		/* someday will return 'next' arglist */
}
#endif
