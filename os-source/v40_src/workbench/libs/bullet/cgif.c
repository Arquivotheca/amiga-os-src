/*
**	$Id: cgif.c,v 8.0 91/03/24 12:16:44 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	cgif.c,v $
 * Revision 8.0  91/03/24  12:16:44  kodiak
 * Phase 2 Beta 1 (38.2)
 * 
 * Revision 7.0  91/03/19  18:35:38  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:25:29  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 5.1  91/03/18  08:56:37  kodiak
 * eliminate old style symbol table lookup
 * 
 * Revision 5.0  91/02/26  10:40:12  kodiak
 * Phase 2 Alpha 1
 * 
 * Revision 3.1  91/02/07  15:40:01  kodiak
 * Kodiak Phase 1 Final Release
 * 
*/
/* cgif.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/*
 *  History:
 *  original  awr 
 *  18-May-90 dET add instance handling for MS-WINDOWS
 *  23-Jul-90 awr added Blake's typePath in CGIFconfig()
 *  07-Aug-90 awr in CGIFfont(), free previous FONT if no bitmaps
 *  13-Aug-90 awr added return SUCCESS in CGIFexit
 *  18-Aug-90 awr added making_bold parameter in call to MAKfontSize()
 *                in CGIFchar_size()
 *  24-Aug-90 jfd in CGIFpcleo(), changed "ENTRY UWORD" to "UWORD ENTRY";
 *                in CGIFpcleo(), changed argument to INSTsetState()
 *                from "CallerId" to "uCallerId"
 *  28-Aug-90 dET moved uInitFirstTime and uEnterFirstTime declarations
 *                and reset these upon CGIFexit().
 *  27-Nov-90 bg  In INSTinit(), changed line "pInst->status == FREE;" to
 *                "pInst->status = FREE;".
 *  03-Dec-90 dET Moved "cgconfig.h" to after "port.h" and before "cgif.h"
 *
 */

#include <string.h>
#include <exec/types.h>
#include <exec/nodes.h>
#include "debug.h"
#include "port.h"
#include "cgconfig.h"
#include "cgif.h"
#include "cache.h"
#include "if_type.h"
#include "ix.h"

/* fc.c     */
EXTERN VOID     FCcopy_fc();
/* cache.c   */
EXTERN UWORD    max_char_size;   /* max cached character bitmap size */
EXTERN UWORD    CACinit();               /* Initialize the cache */
EXTERN VOID     CACexit();
EXTERN UWORD    make_char();                    /* make a chracter IFBITMAP */
EXTERN FONT    *FNTfind();            /* Find a FONT with given FONTCONTEXT */
EXTERN VOID     FNTfree();             /* Free a FONT                      */
EXTERN VOID     BMmru();               /* make IFBITMAP most recently used */
/* ix.c     */
EXTERN UWORD    IXinit();
EXTERN VOID     IXexit();
EXTERN UWORD    IXget_fnt_index();
EXTERN UWORD    max_open_files;
EXTERN BYTE     libPath[];
EXTERN BYTE     typePath[];
EXTERN FONTINDEX *fi;
/* maker.c  */
EXTERN UBYTE   *cc_buf;          /* compound character buffer        */
EXTERN UWORD    cc_size;         /* size of comp. char. buffer       */
EXTERN UWORD    MAKfontSize();
EXTERN UWORD    MAKchar_width();
EXTERN WORD     MAKkern();
/* if_init.c */
EXTERN  IF_CHAR    c;

/*-----------------------------------------*/
#if CACHE
GLOBAL  FONT       *fontCur;           /* ptr to current FONT            */
MLOCAL  LONG        cur_font_id;
#else
MLOCAL  FONTCONTEXT fcCur;              /* current FONTCONTEXT            */
#endif
/*-----------------------------------------*/

/*-----------------------------------------*/

/*----------------*/
/*  CGIFinit      */
/*----------------*/
/* This function initializes the memory manager. */
UWORD ENTRY
CGIFinit()
{
    return SUCCESS;
}


/*----------------*/
/*  CGIFconfig    */
/*----------------*/
/*  The application calls this function to set or change the
 *  configuration parameters.
*/
UWORD ENTRY
CGIFconfig(cfg)
    IFCONFIG *cfg;
{
  /*  Set configuration values  */

#if CACHE
    max_char_size = cfg->max_char_size; /* max cached character IFBITMAP size */
#endif
    max_open_files = cfg->num_files;  /* max number of open library files */

    cc_buf  = cfg->cc_buf_ptr;                /* compund character buffer */
    cc_size = cfg->cc_buf_size;

    strcpy(typePath, cfg->typePath);

    return SUCCESS;
}


/*----------------------*/
/*    CGIFenter         */
/*----------------------*/
/*  This function is called once after the memory pools have been funded
 *  to initialize the cache and the font management subsystem.
 */
UWORD  ENTRY
CGIFenter()
{
#if CACHE
    UWORD status;

    if(status = CACinit())         /* Initialize the cache */
    {
        DBG1("CACinit() failed: %ld\n", status);
        return status;
    }
#endif

  /*  Initialize the font management subsystem */
  
    return IXinit();
}


/*----------------------*/
/*    CGIFexit          */
/*----------------------*/
UWORD  ENTRY
CGIFexit()
{
    DBG("CGIFexit()\n");

#if CACHE
    CACexit();
#endif
    IXexit();
    return SUCCESS;
}


/*----------------------*/
/*    CGIFfont          */
/*----------------------*/
/*  The application calls this function to change the current font.
 *  A new symbol set is loaded if it changed.
 *
 *  CGIFchar may also call this function (via INSTsetState()) if CGIFchar
 *  detects that the user id has changed from the previous caller.
 */
UWORD  ENTRY
CGIFfont(fc)
    FONTCONTEXT  *fc;
{
    INDEX_ENTRY *ie;

#if CACHE
  /*  Free old FONT if it has no bitmaps */

    if(fontCur)
        if(!fontCur->bitmapCount) FNTfree(fontCur);

  /*  Find or create a FONT matching FONTCONTEXT fc */

    if(!(fontCur  = FNTfind(fc))) return ERR_no_font;
    cur_font_id = fc->font_id;

#else
    FCcopy_fc(fc, &fcCur);  /* Local copy of the FONTCONTEXT */
#endif

   /*
    * update fi (KODIAK)
    */
    ie = (fi->index_entry)+(fi->entry_ct-1);
    ie->tfnum = fc->font_id;
    ie->fhoff = fc->fhoff;
    ie->fhcount = fc->fhcount;
    ie->bucket_num = fc->bucket_num;
    strcpy((char *)(ie+1), fc->name);

    return SUCCESS;
}


#if CACHE
/*----------------------*/
/*    CGIFchar          */
/*----------------------*/
/*  Return a pointer to the requested character IFBITMAP.
 *  Return nil if the character is not in the cache and cannot be
 *  made or if bad input data.
 *  If caller ID has changed since the last call to this function, then the 
 *  saved state (from CGIFconfig, CGIFfont) of the caller must be restored.
 */
UWORD ENTRY
CGIFchar(chId, ppbm, alt_width)
    UWORD    chId;
    IFBITMAP **ppbm;
    WORD     alt_width;
{
    IFBITMAP  *pbm;                   /* pointer to the new character IFBITMAP */
    UWORD    status;

    DBG1("CGIFchar(%ld)\n", chId);

 /* point to handle of character's IFBITMAP in current FONT */

    pbm = fontCur->buf + chId;

#ifndef PROFILE
    if(! *pbm)                            /* Is character in the cache ? */
#endif
    {                                              /* It isn't so make it */
        c.alt_width = alt_width;

        if(status = make_char(fontCur, chId, pbm))
        {
            DBG("    make_char() failed\n");
            *ppbm = (IFBITMAP*)0;
            return status;
        }
    }

  /*  Make buffer most recently used if it hasn't
   *  already been made so during this current font's reign.
   */
    if( pbm->notmru )
    {
        pbm->notmru = 0;               /* Keep from doing this again       */
        Remove(pbm);                  /* Remove from lru list             */
        AddTail(pBMlru, pbm);      /* Link at the head of the lru list */
    }

  /*  Return character bitmap pointer */

    *ppbm = pbm;
    return SUCCESS;
}
#else    /* not using cache */

/*----------------------*/
/*    CGIFchar_size     */
/*----------------------*/
UWORD  ENTRY
CGIFchar_size(chId, size, alt_width)
    UWORD    chId;
    UWORD   *size;
    WORD     alt_width;
{
    DBG1("CGIFchar_size(%ld)\n", chId);

  /* Check for bad character id */

    c.alt_width = alt_width;        /* Alternate width */

    return MAKfontSize(&fcCur, chId, size,
         /* are we makeing bold? */   ((fcCur.xbold | fcCur.ybold) != 0L));
}
#endif


/*----------------------*/
/*    CGIFwidthsetup    */
/*----------------------*/
UWORD  ENTRY
CGIFwidthsetup()
{
    UWORD status;

    DBG("CGIFwidthsetup()\n");

#if CACHE
    if(status = IXget_fnt_index(cur_font_id)) return status;
#else
    if(status = IXget_fnt_index(fcCur.font_id)) return status;
#endif
    return SUCCESS;
}


/*----------------------*/
/*    CGIFkern          */
/*----------------------*/
UWORD  ENTRY
CGIFkern( code, num_pairs, kern)
    UWORD      code;
    UWORD      num_pairs;
    KERN_PAIR *kern;
{
    UWORD status;
    UWORD i;

    if((code != TEXT_KERN) && (code != DESIGN_KERN))
        return ERR_bad_kern_code;
#if CACHE
    if(status = IXget_fnt_index(cur_font_id)) return status;
#else
    if(status = IXget_fnt_index(fcCur.font_id)) return status;
#endif

    for(i=0; i<num_pairs; i++, kern++)
        kern->adj = MAKkern(code, kern->chId0, kern->chId1);
    return SUCCESS;
}


#if PCLEO

EXTERN UWORD pcleo();

UWORD ENTRY  /* 08-24-90 jfd */
CGIFpcleo(fileName, pcount)
    BYTE  *fileName;
    ULONG *pcount;
{
    UWORD status;

#if CACHE
    if(status = IXget_fnt_index(cur_font_id)) return status;
    return pcleo(fileName, pcount, cur_font_id);
#else
    if(status = IXget_fnt_index(fcCur.font_id)) return status;
    return pcleo(fileName, pcount, fcCur.font_id);
#endif

}

#endif
