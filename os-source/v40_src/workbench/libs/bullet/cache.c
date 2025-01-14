/* cache.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */

/*
 *    29-Jul-90  awr   changed conditional compile on MEMstat() from DEBUG
 *                     to MEM_TRACE
 *    05-Aug-90  awr   In make_char() passed making_bold flag to
 *                     MAKfontSize.
 *    05-Aug-90  awr   added bitMapCount in FONTCONTEXT to improve cache
 *                     performance. If last bitmap is freed from a font
 *                     and it's not the current font, then the font is freed.
 *    20-Nov-90  jfd   Compiling all routines conditionally based on CACHE;
 *                     Added  #include "cgconfig.h".
 *    04-Dec-90  jfd   Moved "include" statement for  "cgconfig.h" to line
 *                     following "port.h"
 *
 */
#include <exec/types.h>
#include <exec/nodes.h>
#include "debug.h"
#include "port.h"
#include "cgconfig.h"                  /* 11-20-90 jfd */
#include "cgif.h"
#include "cache.h"

#define MEM_TRACE  0        /* set to 1 to trace memory */

#define NO_INDEX 32767

#if CACHE                              /* 11-20-90 jfd */

/* cgif.c */
EXTERN HIFFONT hfontCur;               /* Handle of current selected FONT */
EXTERN FONT   *pfontCur;               /* Pointer to above FONT           */
/* dll.c */
EXTERN BOOLEAN dll_empty ();           /* Returns TRUE if p is empty list */
EXTERN VOID    dll_null();             /* Makes p an empty list           */
EXTERN VOID    dll_remove ();          /* Removes p from a list           */
EXTERN VOID    dll_after ();           /* Links q into p's list after p   */

/* fc.c */
EXTERN BOOLEAN FCis_equal();       /* Check two FONTCONTEXTs for equality */
EXTERN VOID    FCcopy_fc();        /* Copy one FONTCONTEXT to another     */

/* mem.c */
EXTERN  ULONG      mem_avail[];            /* Available BYTEs of memory   */
EXTERN  MEM_HANDLE MEMalloc();             /* Allocate memory to caller   */
EXTERN  VOID       MEMfree();              /* Free memory from caller     */
EXTERN  BYTE      *MEMptr();               /* Convert handle to a pointer */
#if MEM_TRACE
EXTERN  VOID       MEMstat();
#endif

/* maker.c */
EXTERN   UWORD          MAKfontSize();
EXTERN   UWORD  ENTRY   MAKbitMap();
/* me: forward reference */
EXTERN   VOID      FNTfree();

/*------------------------------------------------------------------------*/
GLOBAL MEM_HANDLE  hFNTlru;            /* Least recently used fonts       */
GLOBAL MEM_HANDLE  hBMlru;             /* LRU character IFBITMAPs         */
GLOBAL UWORD       max_char_size;      /* max cached bitmap size          */
MLOCAL HIFBITMAP   hBigBM;             /* buffer handle of big character  */
MLOCAL HIFBITMAP   hBoldBuf1;          /* Handles for bold buffers        */
MLOCAL HIFBITMAP   hBoldBuf2;
/*------------------------------------------------------------------------*/




/*----------------------*/
/*       BMfree         */
/*----------------------*/
/*  Free an IFBITMAP. Remove all references to this bitmap. References can
 *  be from FONTs or from three special references.
 */
GLOBAL VOID
BMfree(hbm)
    HIFBITMAP hbm;              /* handle of IFBITMAP that is to be freed */
{
    IFBITMAP *bm;
    HIFFONT hf;
    FONT *f;

  /*  If we're freeing any of the seperately remembered IFBITMAPs, then
   *  forget.
   */

    if(hbm == hBigBM)    hBigBM    = (HIFBITMAP)0L;
    if(hbm == hBoldBuf1) hBoldBuf1 = (HIFBITMAP)0L;
    if(hbm == hBoldBuf2) hBoldBuf2 = (HIFBITMAP)0L;



    bm = (IFBITMAP*)MEMptr(hbm);       /* Recover pointer to the IFBITMAP */

  /*  Remove the reference to the IFBITMAP from the font. If there are
   *  no more bitmaps in the font and the font is not the current font
   *  then free up the font also.
   */

    if(bm->index != NO_INDEX)     /* character index within FONT (if any) */
    {
        hf = bm->hiffont;                      /* handle to my FONT owner */
        f = (FONT*)MEMptr(hf);
        *(f->hbuf+bm->index) = (HIFBITMAP)0L;  /* forget about me         */
        if(!(--f->bitmapCount) && (hf != hfontCur)) FNTfree(hf);
    }

    dll_remove(hbm);                    /*  Remove IFBITMAP from lru list */
    MEMfree(CACHE_POOL, hbm);           /*  Free the IFBITMAP memory      */
}




/*----------------------*/
/*         BMnew        */
/*----------------------*/
/*  Return the IFBITMAP handle of a new IFBITMAP.
 *  Remember the FONT owner of this IFBITMAP in case the
 *  IFBITMAP has to be reclaimed later.
 *
 *  Normally, this function will not fail.
 *  If there is not enough IFBITMAP memory (managed by mem.c), lru
 *  IFBITMAPs are reclaimed until there is enough.
 *
 */
MLOCAL HIFBITMAP
BMnew(hiffont, size, index)
    HIFFONT hiffont;
    UWORD size;
    UWORD index;
{
    HIFBITMAP hbm;
    IFBITMAP *bm;

  /*  Get IFBITMAP memory */

    while(!(hbm = (HIFBITMAP)MEMalloc(CACHE_POOL, size)))
    {
        if(dll_empty(hBMlru))
            return (HIFBITMAP)0L;
        else
            BMfree( ((DLL*)MEMptr(hBMlru))->b );     /* free lru IFBITMAP */
    }
    bm = (IFBITMAP*)MEMptr(hbm);                       /* Recover pointer */

  /*  make least recently used */

    bm->notmru = 0;              /* Keep CGIFchar() from calling me again */
    dll_after(hBMlru, hbm);      /* link at start of IFBITMAP lru list    */

  /*  Remember my new owner so that I can remove all references to
   *  myself if I have to be reclaimed. Tell new owner there's one more
   *  of us bitmaps.
   */

    if((bm->index = index) != NO_INDEX)
    {
        bm->hiffont = hiffont;
        ((FONT*)MEMptr(hiffont))->bitmapCount++;
    }
    return hbm;
}




/*                              I F B I T M A P                           */
/*========================================================================*/
/*                                 F O N T                                */




/*----------------------*/
/*    FNTfree           */
/*----------------------*/
/*  Free a currently active FONT and associated IFBITMAPs */

GLOBAL VOID
FNTfree(hf)
    HIFFONT hf;                             /* Handle of FONT to be freed */
{
    FONT    *f;                            /* Pointer to FONT to be freed */
    HIFBITMAP *phbm;
    WORD     i;

    f = (FONT*)MEMptr(hf);                             /* Recover pointer */

  /*  For each IFBITMAP in the font. Bump the font's bitmapCount by 1 so
   *  that it won't go to 0 when the last bitmap is freed. Otherwise,
   *  BMfree() will try to free me.
   */

    f->bitmapCount++;                 /* One more than I really have      */
    phbm = f->hbuf;                   /* start of array of bitmap handles */
    for(i = 0; i<SSMAX; i++)
    {
        if (*phbm) BMfree(*phbm);
        phbm++;
    }

    dll_remove(hf);                         /*  Remove FONT from lru list */
    MEMfree(CACHE_POOL, hf);                /*  Free the FONT memory      */
}




/*----------------------*/
/*    FNTnew            */
/*----------------------*/
/*  Return a handle to a newly created FONT. The FONT is initialized
 *  to contain no characters and is most recently used in FNTlru. The
 *  parameter FONTCONTEXT is copied to the new FONT.
 */
MLOCAL HIFFONT
FNTnew(fc)
    FONTCONTEXT *fc;
{
    WORD  i;
    HIFFONT hf;
    FONT  *f;
    HIFBITMAP *phbm;

  /*  Allocate a new FONT */

    while(!(hf = (HIFFONT)MEMalloc(CACHE_POOL, sizeof(FONT))))
    {
        if(dll_empty(hFNTlru))
        {
            if(dll_empty(hBMlru)) return (HIFFONT)0L;
            BMfree( ((DLL*)MEMptr(hBMlru))->b );     /* free lru IFBITMAP */
        }
        else
            FNTfree(((DLL*)MEMptr(hFNTlru))->b);
    }

    f = (FONT*)MEMptr(hf);                             /* Recover pointer */

    /*  Make new FONT most recently used */

    dll_after(hFNTlru, hf);

  /* Initialize font data. */

    f->fc = *fc;                          /* Copy FONTCONTEXT to new FONT */    
    f->bitmapCount = 0;          /* No character IFBITMAPs are cached yet */
    phbm = f->hbuf;                  /* point to list of IFBITMAP handles */
    for(i=0; i < SSMAX; i++)
        *phbm++ = (HIFBITMAP)0L;

    return hf;
}




/*-------------------*/
/*    FNTfind        */
/*-------------------*/
/*  Find the active FONT that matches the given FONTCONTEXT. If no
 *  match, then create a new FONT.
 */
GLOBAL HIFFONT
FNTfind(fc)
    FONTCONTEXT *fc;
{
    HIFFONT   hf;
    FONT   *f;
    HIFBITMAP hbm;
    IFBITMAP *bm;

  /*  Search active fonts for a match with
   *  the user's device font context
   */

    for(hf = ((DLL*)MEMptr(hFNTlru))->f; hf != hFNTlru; hf = f->link.f)
    {
        f = (FONT*)MEMptr(hf);
        if (FCis_equal(fc, &f->fc)) break;
    }

    if(hf != hFNTlru)
    {
      /*  Make FONT most recently used */
        dll_remove(hf);
        dll_after(hFNTlru, hf);
    }
    else
    {
      /*  No active FONT matches the app's FONTCONTEXT. Make a new FONT */

        if(!(hf = FNTnew(fc))) return (HIFFONT)0L;
        f = (FONT*)MEMptr(hf);
    }

  /*  Set all IFBITMAPs' notmru flags to 1 indicating that they have not
   *  been moved to the front of the lru list during calls to CGIFchar() for
   *  the current FONT. CGIFchar() checks the notmru flag for the IFBITMAP
   *  it is about to return. If it is 1, then a call is made to BMmru()
   *  above to move the IFBITMAP to the head of the lru list making it
   *  "most recently used". At that time the notmru flag is cleared so that
   *  it will be moved to the head of the list only once during a sequence
   *  of calls to CGIFchar() for a given FONT. This is to keep CGIFchar()'s
   *  performance up. All IFBITMAPs that need to have their
   *  notmru flags set are at the beginning of the lru list.
   */
    hbm = ((DLL*)MEMptr(hBMlru))->f;        /* first IFBITMAP in lru list */
    bm = (IFBITMAP*)MEMptr(hbm);
    while( (hbm != hBMlru) && (bm->notmru == 0))
    {
        bm->notmru = 1;
        hbm = bm->link.f;                    /* next IFBITMAP in the list */
        bm = (IFBITMAP*)MEMptr(hbm);
    }

    return hf;
}




/*----------------------*/
/*       CACinit        */
/*----------------------*/
GLOBAL UWORD
CACinit()
{
    pfontCur = (FONT*)0;                  /* ptr to current FONT          */
    dll_null(hFNTlru);                    /*  Set FONT lru list to empty. */
    dll_null(hBMlru);                     /*  Set IFBITMAP lru to empty.  */
    hBigBM = (HIFBITMAP)0L;               /*  No big character yet        */
    hBoldBuf1 = (HIFBITMAP)0L;            /*  No bold bufferss            */
    hBoldBuf2 = (HIFBITMAP)0L;

  /*  Make sure enough cache memory is left to at least allocate
   *  one IFBITMAP.
   */

    if((LONG)max_char_size + (LONG)sizeof(FONT) > mem_avail[CACHE_POOL])
        return ERR_CACinit;

    return SUCCESS;
}




/*----------------------*/
/*       CACexit        */
/*----------------------*/
GLOBAL VOID
CACexit()
{
    DBG("\n\nCACexit()\n");

  /* Free all FONTs */

    while(!dll_empty(hFNTlru))
        FNTfree(((DLL*)MEMptr(hFNTlru))->b);

#if MEM_TRACE
    MEMstat(CACHE_POOL);
#endif
}




/*-------------------*/
/*    make_char      */
/*-------------------*/
/*  Make a character bitmap and insert it into the cache */
GLOBAL UWORD
make_char(hf, chId, phbm)
  HIFFONT    hf;                                /* Handle of current FONT */
  WORD       chId;                              /* character ID           */
  HIFBITMAP *phbm;                              /* returned bitmap handle */
{
  FONT   *f;
  UWORD   status;
  UWORD   size;             /*  Size in BYTEs of memory needed for bitmap */
  BOOLEAN making_bold;      /*  TRUE if pseudo bold                       */
  UBYTE  *bold_buf1;        /*  bold IFBITMAP pointers                    */
  UBYTE  *bold_buf2;

    f = (FONT*)MEMptr(hf);                              /* Recover handle */

  /*  Free up big character IFBITMAP if any */

    if (hBigBM)
    {
        BMfree(hBigBM);
        hBigBM = (HIFBITMAP)0L;
     }

    making_bold = ((f->fc.xbold | f->fc.ybold) != 0L);
        
  /* get font bitmap size */

    if (status = MAKfontSize(&f->fc, chId, &size, making_bold))
         return status;

  /* Acquire bold IFBITMAPs if neccessary  */

    if(making_bold)
    {
        /*  We'll check for allocation errors after we allocate
         *  the character IFBITMAP. This could cause reclaimation
         *  of the bold IFBITMAPs.
         */
        hBoldBuf1 = BMnew(hf, size, NO_INDEX);
        hBoldBuf2 = BMnew(hf, size, NO_INDEX);
    }
    else
    {
        making_bold = FALSE;
        bold_buf1 = (IFBITMAP*)0;
        bold_buf2 = (IFBITMAP*)0;
    }

  /*  Get a new IFBITMAP */

    if(!(*phbm = BMnew(hf, size, chId)))
        return ERR_bm_buff;

  /*  If character is too big to cache, remember IFBITMAP in hBigBM.
   *  This IFBITMAP will be freed during the next call to make_char().
   */
    if (size > max_char_size)
        hBigBM = *phbm;

  /*  One last check to make sure our bold IFBITMAPs didn't get reclaimed */

    if(making_bold)
    {
        if(!hBoldBuf1 || !hBoldBuf2)
        {
            if(hBoldBuf1) BMfree(hBoldBuf1);
            if(hBoldBuf2) BMfree(hBoldBuf2);
            return ERR_bold_buf;
        }
        else
        {
            bold_buf1 = ((IFBITMAP*)MEMptr(hBoldBuf1))->bm;
            bold_buf2 = ((IFBITMAP*)MEMptr(hBoldBuf2))->bm;
        }
    }

  /* make the character bit map */

    status = MAKbitMap((IFBITMAP*)MEMptr(*phbm), bold_buf1, bold_buf2);

  /* Free the bold IFBITMAPs */

    if (making_bold)
    {
        BMfree(hBoldBuf1);
        BMfree(hBoldBuf2);
    }

    return status;
}

#endif
