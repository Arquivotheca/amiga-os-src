/*
**	$Id: fm.c,v 7.1 91/03/28 15:05:41 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	fm.c,v $
 * Revision 7.1  91/03/28  15:05:41  kodiak
 * change DEBUG statements
 * 
 * Revision 7.0  91/03/19  18:35:57  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:26:20  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 5.1  91/03/18  08:57:00  kodiak
 * eliminate old style symbol table lookup
 * 
 * Revision 5.0  91/02/26  10:43:08  kodiak
 * Phase 2 Alpha 1
 * 
 * Revision 3.0  90/11/09  17:09:06  kodiak
 * Kodiak's Alpha 1
 * 
*/
/* fm.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/*  12-Mar-90   jfd     Storing the pointers to the Typeface Header
 *                      and Font Alias Segments in this routine now
 *                      (keys 107 and 110)
 *  05-May-90   awr     Character buffer is fixed size- allocated once
 *  07-Jun-90   dah     Modified FMalloc_rd() and FMseek_read(); added
 *                      parameters for font library compression.
 *  08-Jun-90   dET     Close files at end of functions because Windows
 *                      is brain dead.
 *  29-Jul-90   awr     Fixed FMrd_char. Was clobbering cd->index with
 *                      with index into update library. Later, old library
 *                      becomes active- meaningless index.
 *  12-Oct-90   dET     Close files at end of functions if running either
 *                      WINDOWS or BOW
 *  04-Dec-90  jfd      Moved "include" statement for  "cgconfig.h" to line
 *                      following "port.h"
 *                     
 */

#include <exec/types.h>
#include <exec/nodes.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include "debug.h"
#include "port.h"
#include "cgconfig.h"
#include "cgif.h"
#include "segments.h"
#include "ix.h"
#include "istypes.h"

extern struct Library *DOSBase;

/* mem.c */
EXTERN VOID *MemAlloc();
EXTERN VOID MemFree();

/* if_init.c */
EXTERN BOOLEAN    if_init_glob();
EXTERN UWORD      if_init_char();
/* maker.c */
EXTERN LONG       if_init_face;        /* Last face processed by if_init */
/* ix.c */
EXTERN FONTINDEX *fi;                       /* Font Index handle */
EXTERN UWORD      IXmak_font_index();
EXTERN UWORD      IXget_fnt_index();
EXTERN UWORD      IXfind_bucket();
EXTERN UWORD      IXopen_file();
EXTERN UWORD      IXclose_file();

/* character buffer */

EXTERN UBYTE *char_buf;




/*------------------*/
/*  FMseek_read     */
/*------------------*/
/*  In file "f", do an lseek to "offset" an then read "count" bytes
 *  into "buffer". Return TRUE if success.
 *  If file is compressed, use flag to decompress and read.
 */
GLOBAL BOOLEAN
FMseek_read(f, offset, count, buffer, flag, input_filetype)
    BPTR   f;          /* file handle              */
    LONG   offset;     /* offset within file       */
    UWORD  count;      /* number of BYTEs to read  */
    BYTE  *buffer;     /* buffer to read data into */
    UWORD  flag;
    UWORD  input_filetype;  /* "D" or "C" for normal and compressed */
{

    DBG2("                FMseek_read()   offset: %ld   count: %ld\n",
                                                          offset, count);
#if COMPRESS
    if(!(input_filetype & B_COMPRESS))
    {
#endif
        if(Seek(f, offset, OFFSET_BEGINNING) == -1)
        {
            DBG2("    Seek(,%ld,) failed %ld\n", offset, IoErr());
            return FALSE;
        }

#ifdef MSWINDOWS
        if(_lread(f, buffer, count) != count)
#else
        if(Read(f, buffer, count) != count)
#endif
        {
            DBG("                    read() failed\n");
            return FALSE;
        }
#if COMPRESS
    }
    else
    {
      if(is_expand(f, offset, buffer, count, flag))
      {
        DBG("                      is_expand failed\n");
        return FALSE;
      }
    }	      
#endif	
    return TRUE;
}


/*------------------*/
/*  FMalloc_rd      */
/*------------------*/
/*  alloc "size" BYTEs and then do an lseek() to "offset" in file "f" and
 *  read "count" BYTEs into the allocated buffer. Return the buffer pointer
 *  or null on failure.
 */
GLOBAL UBYTE *
FMalloc_rd(f, size, offset, count, flag, input_filetype)
    LONG    f;
    UWORD   size;                          /* Size of buffer to allocated */
    LONG    offset;
    UWORD   count;
    UWORD   flag;
    UWORD   input_filetype;
{
    UBYTE      *buf;

  /*  Acquire size BYTES  */
    buf = MemAlloc(size);
    if (!buf)
	return(0);

  /*  Read data into new buffer.  If error, free buffer  */

    if(!FMseek_read(f, offset, count, buf, flag, input_filetype))
    {
        MemFree(buf);
        return(0);
    }

    return(buf);
}




/*-------------------*/
/*  FMload_font      */
/*-------------------*/
/*  Load the global data from a font file into memory.
 *  The source of this load are defined by the parameter "ix" which
 *  points to an entry in the FONTINDEX table defining the typeface.
 *
 *  The destination of the load is defined by the parameter "b" which is
 *  a pointer to a BUCKET structure.
 */
GLOBAL UWORD
FMload_font(b, ix)
    BUCKET      *b;
    INDEX_ENTRY *ix;
{
    UWORD          status;
    BYTE          *face_header_seg;      /* Face header seqment  */
    CH_DIR_ENTRY  *chptr;
    UWORD          ch_count;
    LONG           globoff;
    UWORD          globcount;
    BYTE          *p;


    UWORD          key;
    ULONG          off;
    UWORD          count;
    BYTE          *fgseg;        /* face global segment */
    BYTE          *seg;

    DBG("    FMload_font()\n");

    b->tfnum      = ix->tfnum;
    b->bucket_num = ix->bucket_num;


  /* Open library file */


    if(status = IXopen_file(ix, b)) return status;

  /* Read face header (= character directory) */

    DBG("        face_header (=character directory)\n");

    if(!(b->pface_header_seg = FMalloc_rd(b->f, ix->fhcount,
            ix->fhoff, ix->fhcount, IS_EXP_FACEHDRSEG, ix->bucket_num)))
    {
        IXclose_file(b);
        return ERR_mem_face_hdr;
    }
    face_header_seg = b->pface_header_seg;

  /*  Set up character directory   (skip segment link)  and count characters
   */

    b->pchar_dir = (CH_DIR_ENTRY *) (b->pface_header_seg + 6);

  /* Count number of characters in directory */

    chptr = (CH_DIR_ENTRY*)(face_header_seg + 6);
    ch_count = 0;
    while(chptr->charnum != -1)
    {
        ch_count++;
        chptr++;
    }
    b->ch_count = ch_count;

  /* Type face global segment */

    globoff = *(LONG*)(face_header_seg);        /* file offset    */
    globcount = *(UWORD*)(face_header_seg + 4); /* segment length */


    DBG2("\n        face_global()  offset: %ld   count: %ld\n",
                           globoff, globcount);

  /* get face global buffer */

    if(!(b->pfgseg = FMalloc_rd(b->f, globcount, globoff, globcount,
                                  IS_EXP_FACEGLOBSEG, ix->bucket_num)))
    {
        IXclose_file(b);
        MemFree(b->pface_header_seg);
        return ERR_face_abort;
    }
    fgseg = b->pfgseg;
    b->sfgseg = globcount;

  /*  Read segment directory and set segment memory handles */

#ifdef  DEBUG
    p = fgseg;

    {
	UWORD          type;
	ULONG          size;
	type = *((UWORD*)p);     p += 2;
	size = *((ULONG*)p);     p += 4;
	DBG2("            type = %ld    size = %ld\n",type, size);
	DBG("\n            directory\n");
    }
#else
    p = fgseg + 6;
#endif

    do
    {
        key =   *((UWORD*)p);     p += 2;
        off =   *((ULONG*)p);     p += 4;
        count = *((UWORD*)p);     p += 2;
        DBG3("            key / offset / count  %ld   %ld   %ld\n",
                                              key,off,count);

        seg   = b->pfgseg + off + 6;
        count -= 6;
        switch (key)
        {
            case 100:  b->pgif = seg - 6;          /* Intellifont        */
                       b->gifct = count + 6;
                       break;
            case 101:  b->ptrack_kern_seg = seg;   /* track kerning      */
                       b->strack_kern_seg = count;
                       break;
            case 102:  b->ptext_kern_seg = seg;    /* text kerning       */
                       b->stext_kern_seg = count;
                       break;
            case 103:  b->pdesign_kern_seg = seg;  /* designer kerning   */
                       b->sdesign_kern_seg = count;
                       break;
            case 104:  b->pwidth_seg =             /* character widths   */
			   (CHARWIDTH *) seg;
                       b->swidth_seg = count;
                       break;
            case 105:  b->pattribute =             /* attribute hdr      */
			   (FACE_ATT *) seg;
                       b->sattribute = count;
                       break;
            case 106:  b->prast = seg;             /* raster parameter    */
                       b->srast = count;
                       break;
            case 107:  b->ptf_header_seg = seg;    /* typeface header     */
                       b->stf_header_seg = count;
                       break;
            case 108:  b->pcompound_seg = seg;     /* compound characters */
                       b->scompound_seg = count;
                       break;
            case 109:  b->pdisplay =               /* display header      */
			   (DISPLAY *) seg;
                       b->sdisplay = count;
                       break;
            case 110:  b->pfont_alias_seg = seg;   /* font alias          */
                       b->sfont_alias_seg = count;
                       break;
            case 111:  b->pcopy = seg - 6;         /* Copyright segment   */
                       b->scopy = count + 6;
                       break;
            default:   break;
        }
    }
    while (key != -1);

#if (defined(MSWINDOWS) | defined(BOW))   /* 10-12-90 dET */
    IXclose_file(b);   /* brain dead Windows does not allow open files */
#endif

    return SUCCESS;
}




/*-------------------*/
/*  FMchar_index    */
/*-------------------*/
GLOBAL WORD
FMchar_index(b, cgnum)
    BUCKET  *b;
    UWORD    cgnum;
{
    CH_DIR_ENTRY  *chptr;
    WORD          i,lo,hi;
    UWORD         charnum;

  /* find entry in character directory */

    DBG1("        FMchar_index(%ld)\n", cgnum);

    chptr = b->pchar_dir;
    lo = 0;
    hi = (WORD)b->ch_count - 1;
    while(lo <= hi)
    {
        i = (lo + hi)/2;
        charnum = (chptr+i)->charnum;
        if(charnum  == cgnum)
        {
            DBG1("            index = %ld\n", i);
            return i;
        }
        else if(charnum < cgnum) lo = i+1;
        else hi = i-1;
    }
    DBG("            character not found\n");
    return -1;
}



/*-------------------*/
/*    rd_char        */
/*-------------------*/
MLOCAL UWORD
rd_char(bucknum, index)
    UWORD  bucknum;
    WORD   index;
{
    UWORD          status;
    BUCKET        *b;
    CH_DIR_ENTRY  *chptr;

    DBG2("        rd_char(bucknum = %ld, index = index = %ld)\n",
                                                    bucknum, index);


    if(status = IXfind_bucket(bucknum, &b)) return status;
    if(status = IXopen_file(fi->cur_index[bucknum], b))
        return status;

  /* Initialize typeface global data if need be */

    if(if_init_face != b->tfnum)
    {
        if_init_face = 0;
        if(!if_init_glob(b))
        {
#if (defined(MSWINDOWS) | defined(BOW))   /* 10-12-90 dET */
          /* brain dead Windows does not allow open files */
            IXclose_file(b);
#endif

            return ERR_if_init_glob;  
        }
        if_init_face = b->tfnum;
    }

  /*  character directory entry  */

    chptr = b->pchar_dir + index;

  /*  read character data */

    if(chptr->charcount > CHARBUFSIZE)
    {

#if (defined(MSWINDOWS) | defined(BOW))   /* 10-12-90 dET */
      /* brain dead Windows does not allow open files */
        IXclose_file(b);
#endif
        return ERR_ov_char_buf;
    }

    if(!FMseek_read(b->f, chptr->charoff, chptr->charcount, char_buf,
                                        IS_EXP_CHARSEG, b->bucket_num))
    {

#if (defined(MSWINDOWS) | defined(BOW))   /* 10-12-90 dET */
      /* brain dead Windows does not allow open files */
        IXclose_file(b);
#endif
        return ERR_rd_char_data;
    }


#if (defined(MSWINDOWS) | defined(BOW))   /* 10-12-90 dET */
  /* brain dead Windows does not allow open files */
    IXclose_file(b);
#endif

  /*  Initialize character data structures  */

    return if_init_char(char_buf);
}



/*-------------------*/
/*  FMrd_char        */
/*-------------------*/
/*  Find character with cgnum = n, allocate a character buffer, read the
 *  character data into the buffer, and initialize character data structures.
 *  The function rd_char() above does all of the work. This function
 *  tries to substitute characters from an auxilary library if the character
 *  being processed is in hq2 format.
 */
GLOBAL UWORD
FMrd_char(cd)
    CHR_DEF *cd;
{
    UWORD       status;
    FONTINDEX  *fi_sav;
    LONG        update_tfnum;
    BUCKET     *b;
    WORD        temp;

    DBG1("        FMrd_char(cgnum = %ld)\n", cd->cgnum);

    if(status = rd_char(cd->bucknum, cd->index))
    {
      /* If we failed because the character was not hq3 format, then try
       * to find the character in the substitute hq3 libraries.
       */

        if(status != ERR_not_hq3) return status;  /* some other error */

      /* Read in the font index for the substitute libraries */

        fi_sav = fi;    /* save old font index */
        if(status = IXmak_font_index("hq3.fnt")) return ERR_not_hq3;

      /*  Set the three bucket index entries, the primary TF_SENSITIVE
       *  bucket entry will contain our typeface. The update typeface
       *  number is negative so that FMfont_change() won't get confused
       *  when we go back to processing normal typefaces. This way he
       *  won't think that a BUCKET already has normal typeface header
       *  info loaded when in fact it has update typeface header info.
       */

        update_tfnum = - ((fi_sav->cur_index[cd->bucknum])->tfnum);
        DBG1("    update_tfnum = %ld\n", update_tfnum);

        if(!(status = IXget_fnt_index(update_tfnum)))
        {

            if(!(status = IXfind_bucket(0, &b)))
            {
                if((temp = FMchar_index(b, cd->cgnum)) >= 0)
                    status = rd_char(0, temp);
                else
                    status = ERR_not_hq3;
            }
        }

        MemFree(fi->fnt_index);   /* free temp. font index */
        fi = fi_sav;                            /* restore font index    */
        if(status) return ERR_not_hq3;
    }

    return SUCCESS;
}

