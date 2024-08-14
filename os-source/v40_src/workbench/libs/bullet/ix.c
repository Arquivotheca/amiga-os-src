/*
**	$Id: ix.c,v 8.1 91/03/28 15:06:01 kodiak Exp $
**
**	(C) Copyright 1990 Robert R. Burns
**	    All Rights Reserved
**
**	$Log:	ix.c,v $
 * Revision 8.1  91/03/28  15:06:01  kodiak
 * check for 0 Open() error (not -1)
 * 
 * Revision 8.0  91/03/24  12:17:06  kodiak
 * Phase 2 Beta 1 (38.2)
 * 
 * Revision 7.0  91/03/19  18:36:28  kodiak
 * after Amigaizing memory, lists, and adding UniCode.  Before cache work.
 * 
 * Revision 6.0  91/03/18  15:27:09  kodiak
 * folded in Bullet 1.30
 * 
 * Revision 5.1  91/03/18  08:57:35  kodiak
 * eliminate old style symbol table lookup
 * 
 * Revision 5.0  91/02/26  10:36:59  kodiak
 * Phase 2 Alpha 1
 * 
 * Revision 3.1  91/02/07  15:39:47  kodiak
 * Kodiak Phase 1 Final Release
 * 
*/
/* ix.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/*   12-Apr-90   jfd   In routine IXget_fnt_index, filter out bit 2
 *                     (italic/nonitalic flag) of "bucket_num" because
 *                     it is being used as an index into "plugin"
 *    05-May-90  awr   Character buffer is fixed size- allocated once
 *    07-Jun-90  dah   Modified FMalloc_rd() and FMseek_read(); added
 *                     parameters for font library compression.
 *    01-Jul-90  awr   Add "screen_fnt_ct" to determine if generic
 *                     screen fonts were installed.
 *    23-Jul-90  awr   Merged Blake's default typeface path code in
 *    18-Aug-90  awr   Correct buildpath() to not insert a "\" at the
 *                     beginning if source1 is null.
 *    04-Sep-90  awr   Removed find_compound()- no longer referenced.
 *    03-Dec-90  awr   In BUCKfree(), reset if_init_face (last face processed
 *                     by if_init())
 *    04-Dec-90  jfd   Moved "include" statement for  "cgconfig.h" to line
 *                     following "port.h"
 *		14-Jan-91  tnc   Window-ized file "close" and "read".
 *
 */


#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <dos/dos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>

#include "debug.h"
#include "port.h"
#include "cgconfig.h"
#include "segments.h"
#include "cgif.h"
#include "ix.h"


#define ZERO      0    /* constant 0 */
#define NO_CMP    0    /* no compression code indicator */

extern struct Library *SysBase;
extern struct Library *DOSBase;

/* gid rid of - make same for regular and compound */
EXTERN WORD  h_esc;                  /* Horizontal escapement of character */
EXTERN WORD  v_esc;                  /* Vertical       "      "     "      */
EXTERN WORD  num_parts;              /* Number of parts of character       */
EXTERN BOOLEAN is_compound;




/* mem.c */
EXTERN VOID *MemAlloc();
EXTERN VOID MemFree();

/* maker.c */
EXTERN VOID    clear_mem();
EXTERN LONG    if_init_face;

/* fm.c */
EXTERN BOOLEAN     FMseek_read();
EXTERN UBYTE      *FMalloc_rd();
EXTERN UWORD       FMload_font();
EXTERN WORD        FMchar_index();

MLOCAL VOID        buildpath();

/* Character buffer */

GLOBAL UBYTE *char_buf = 0;

/* Open files */

MLOCAL UWORD num_open_files;      /* current number of open library files */
GLOBAL UWORD max_open_files;      /* maximum   "     "   "     "      "   */
GLOBAL BYTE  typePath[PATHNAMELEN] = { 0 };

/* Buckets */

#if SCREEN_FONTS
GLOBAL UWORD screen_fnt_ct;       /*  Number of Generic screen fonts
                                   *  installed. Must be 2 to enable screen
                                   *  font substitution
                                   */
#endif
GLOBAL struct MinList lBUCKlru;   /*  Least recently used list for BUCKETs */
MLOCAL INDEX_ENTRY *plugin[5];
MLOCAL BOOLEAN small_plugins;     /*  True if small plugin set is used.
                                   *  This set has the universals and the
                                   *  normal lim sens serif and sans serif.
                                   *  It does not have the bold lim sens
                                   *  serif and sans serif.
                                   */

GLOBAL FONTINDEX   *fi = 0;        /* Font Index */

/* Symbol set */

MLOCAL UWORD         symbol_set_size;
MLOCAL SS_ENTRY      *symbol_set;         /* current symbol set            */





/*------------------*/
/* IXmak_font_index */
/*------------------*/
GLOBAL UWORD
IXmak_font_index(fname)
    BYTE *fname;        /*  file name of font index file     */
{
    BPTR        f;
    UWORD       index_size;
    BYTE       *buf;
    BYTE        pathname[PATHNAMELEN];
    BYTE        *s1, *s2;
    WORD           i;
    INDEX_ENTRY   *p;

    DBG1("IXmak_font_index(%s)\n", fname);

  /* Build full pathname */

    buildpath (pathname, "Fonts:_Bullet", fname);
    
  /* Read font index table */

    DBG1("open \"%s\"\n", pathname);
    if ((f = Open(pathname, MODE_OLDFILE)) == 0)
    {
        DBG1("Font index file %s not found\n", pathname);
        return ERR_no_font_index;
    }
    DBG1("  file %ld\n", f);
    Seek(f, 0, OFFSET_END);
    index_size = (UWORD)Seek(f, 0, OFFSET_CURRENT);
    DBG1("index_size = %ld\n", index_size);

    buf = FMalloc_rd(f, ((index_size+sizeof(INDEX_ENTRY)+PATHNAMELEN+
	    sizeof(FONTINDEX))+1)&0xfffffffe,
	    (LONG)0, index_size, ZERO, NO_CMP);
    Close(f);
    if(!buf)
    {
        DBG("read font index failed\n");
        return ERR_rd_font_index;
    }

    fi = (FONTINDEX *)
	    (buf + ((index_size+sizeof(INDEX_ENTRY)+PATHNAMELEN+1)&0xfffe));
    fi->fnt_index = buf;
    fi->entry_ct = (*((WORD*)buf))+1;
    fi->index_entry = (INDEX_ENTRY *) (buf+2);
    fi->libnames  = buf+2+(fi->entry_ct*sizeof(INDEX_ENTRY));
    fi->cur_tf_num = 0;
    /* make space for another face */
    for (i = 0, p = (INDEX_ENTRY *) (buf+2); i < fi->entry_ct-1; i++, p++)
	p->name_off += PATHNAMELEN;
    s1 = buf + index_size;
    s2 = s1 + (sizeof(INDEX_ENTRY)+PATHNAMELEN);
    while (s1 != ((char *) p))
	*--s2 = *--s1;
    p->name_off = 0;
    p->tfnum = 0x8000000;	/* impossible value for now */
	
#ifdef DEBUG
        DBG("\n\nF o n t    I n d e x\n");
        DBG1("entry_ct = %ld\n", fi->entry_ct);
        DBG1("    size of entry = %ld\n", sizeof(INDEX_ENTRY));
        for(i=0, p = fi->index_entry; i<fi->entry_ct-1; i++, p++)
            DBG5("%ld %s %ld %ld %ld\n", p->tfnum,
                    fi->libnames+p->name_off,
                    p->fhoff, p->fhcount, p->bucket_num);
#endif

    return SUCCESS;
}

/*------------------*/
/*  init_font_index */
/*------------------*/
GLOBAL UWORD
init_font_index()
{
    UWORD         status;
    WORD          i;
    INDEX_ENTRY  *p;
    UWORD         plug_ct;

    DBG("init_font_index\n");

  /* Allocate and read font index table */

    if(status = IXmak_font_index("if.fnt")) return status;   

  /* Find plugins and generic screen fonts */

#if SCREEN_FONTS
    screen_fnt_ct = 0;
#endif
    plug_ct = 0;
    small_plugins = TRUE;
    p = fi->index_entry;
    for(i=0; i<fi->entry_ct; i++)
    {
             if(p->tfnum == FACE_UNIV)       {plugin[0] = p;  plug_ct++;}
        else if(p->tfnum == FACE_LS_S_NORM)  {plugin[1] = p;  plug_ct++;}
        else if(p->tfnum == FACE_LS_S_BOLD)  {plugin[2] = p;  plug_ct++;
                                                    small_plugins = FALSE;}
        else if(p->tfnum == FACE_LS_SS_NORM) {plugin[3] = p;  plug_ct++;}
        else if(p->tfnum == FACE_LS_SS_BOLD) {plugin[4] = p;  plug_ct++;
                                                    small_plugins = FALSE;}
#if SCREEN_FONTS
        else if(p->tfnum == FACE_S_SCR)      {screen_fnt_ct++;}
        else if(p->tfnum == FACE_SS_SCR)     {screen_fnt_ct++;}
#endif

        p++;
    }
    if((plug_ct < 5 && !small_plugins) || (plug_ct != 3 && small_plugins))
    {
        DBG("    missing plugin\n");
        return ERR_missing_plugin;
    }

    return SUCCESS;
}


/*------------------*/
/*  IXinit          */
/*------------------*/
GLOBAL UWORD
IXinit()
{
    ULONG         f;                    /* symbol set file */
    ULONG         ss_long;
    BYTE          pathname[PATHNAMELEN];

    UWORD         status;

    DBG("IXinit()\n");
    num_open_files = 0;                   /* number of open library files */
    if_init_face = 0;                 /* Last face processed by if_init() */
    NewList(&lBUCKlru);                     /*  Set BUCKET lru to empty.  */

  /* Allocate character buffer */

    if(!(char_buf = AllocVec(CHARBUFSIZE, 0)))
        return ERR_mem_char_buf;

  /* Initialize font index and find the plugins */

    if(status = init_font_index())
	return status;   

  /* Symbol set */

    DBG("init_symbol_set()\n");

    /* Build full pathname */

    buildpath (pathname, "Fonts:_Bullet", "if.uc");

    /* Open symbol set file and read number of entries */

    if ((f = Open(pathname, MODE_OLDFILE)) == 0)
    {
        DBG("Symbol set file if.uc not found\n");
        return ERR_no_symbol_set;
    }
    if ((Read(f, (BYTE*) &ss_long, 4) != 4) || (ss_long != 0x55430000) ||
	    (Read(f, (BYTE*)&symbol_set_size, 2) != 2)) {
        DBG("problem w/ if.uc\n");
        status = ERR_rd_symbol_set;
    }
    else {
	symbol_set = (SS_ENTRY *) FMalloc_rd(f,
		symbol_set_size*sizeof(SS_ENTRY), 8,
		symbol_set_size*sizeof(SS_ENTRY), ZERO, NO_CMP);
	if (!symbol_set)
	    status = FAILURE;
    }
    Close(f);

    return(status);
}


/*------------------*/
/*  IXget_fnt_index */
/*------------------*/
/*  Search the FONTINDEX whose handle is the GLOBAL hfi for the typeface
 *  INDEX_ENTRY matching tfnum. Set 4 fields in the FONTINDEX:
 *
 *        fi->cur_tf_num
 *        fi->hcur_index[0]
 *        fi->hcur_index[1]
 *        fi->hcur_index[2]
 */

GLOBAL UWORD
IXget_fnt_index(tfnum)
    LONG tfnum;
{
    WORD           i;
    INDEX_ENTRY   *p;
    WORD           bucket_num;

    DBG1("\nIXget_fnt_index(tfnum = %ld)\n", tfnum);

  /*  If we've changed type faces, then find the font index entry
   *  for the new face.
   */

    if(fi->cur_tf_num == tfnum) return SUCCESS;

    p = fi->index_entry;
    for(i=0; i<fi->entry_ct;  i++)
    {
        if(tfnum == p->tfnum)
        {
#if !COMPRESS
          /*  If library file compression is not enabled, then return
           *  failure if this is a compressed file.
           */
            if(p->bucket_num & B_COMPRESS)
                return ERR_no_fi_entry;
#endif

            fi->cur_tf_num = tfnum;
            fi->cur_index[UNIVERSAL]     = plugin[0];
            fi->cur_index[TF_SENSITIVE]  = p;

           /*  Limited sensitive codes from font index entry:
            *      serif      0 normal, 1 bold,
            *      sans serif 2 normal, 3 bold
            *  If(small_plugins) the bolds aren't loaded so use normals
            *  instead.
            */

            bucket_num = p->bucket_num & 3;     /* 04-12-90 jfd */
            if(small_plugins) bucket_num &= 2;
            fi->cur_index[LIM_SENSITIVE] = plugin[bucket_num + 1];
            return SUCCESS;
        }
        p++;
    }
    DBG("    not found\n");
    return ERR_no_fi_entry;
}





/*-------------------*/
/*   IXclose_file    */
/*-------------------*/
GLOBAL VOID
IXclose_file(b)
    BUCKET   *b;
{
    Close(b->f);
    b->f = -1;                               /* just in case */
    b->file_open = FALSE;
    num_open_files--;
}




/*-------------------*/
/*   IXopen_file     */
/*-------------------*/
GLOBAL UWORD
IXopen_file(ix, bucket)
    INDEX_ENTRY *ix;
    BUCKET      *bucket;
{
    BUCKET      *b;
    BYTE        pathname[PATHNAMELEN];

    if(bucket->file_open) return SUCCESS;

  /*  Make sure too many files aren't open  */

    while(num_open_files >= max_open_files)
    {
	DBG("IXopen_file -- too many files open\n");
        b = (BUCKET *) lBUCKlru.mlh_Tail;     /* lru BUCKET in lru list */
        while(b->mn.mln_Pred)
        {
            if(b->file_open)
            {
                IXclose_file(b);
                break;
            }
            b = (BUCKET *) b->mn.mln_Pred;    /* previous BUCKET in the list */
        }
        if(!b->mn.mln_Pred)
        {
            DBG("    couldn't close file\n");
            return ERR_IXopen_file;
        }
    }

  /*  Open the file  */

    buildpath (pathname, typePath, fi->libnames + ix->name_off);
    if((bucket->f = Open(pathname, MODE_OLDFILE)) == 0)
    {
        DBG("        Open lib file failed\n");
        return ERR_open_lib;
    }
    bucket->file_open = TRUE;
    num_open_files++;
    return SUCCESS;
}




/*-------------------*/
/*    BUCKfree       */
/*-------------------*/
GLOBAL VOID
BUCKfree(b)
    BUCKET *b;
{

    DBG1("BUCKfree $%lx\n", b);
    if (b->file_open)
        IXclose_file(b);

    if (b->tfnum == if_init_face) /* Since we are freeing this bucket, make */
       if_init_face = 0;          /* make sure we reset if_init_face (last  */
                                  /* face processed by if_init()) 12-03-90 awr */ 

    MemFree(b->pface_header_seg);
    MemFree(b->pfgseg);

    Remove((struct Node *) b);    /* remove from active lru list */
    MemFree(b);
}



/*-------------------*/
/*    BUCKnew        */
/*-------------------*/
MLOCAL UWORD
BUCKnew(ix, p_bucket)
    INDEX_ENTRY  *ix;
    BUCKET      **p_bucket;
{
    BUCKET  *b;
    UWORD    status;

    DBG("BUCKnew()\n");

    b = MemAlloc(sizeof(BUCKET));
    if (!b)
	return(ERR_no_buck_mem);
    *p_bucket = b;
    clear_mem(b, (ULONG)sizeof(BUCKET));

    if(status = FMload_font(b, ix))
    {
        DBG("    BUCKnew() failed\n");
	MemFree(b);
    }
    else
	/* link at start of BUCKET lru list */
        AddTail((struct List *) &lBUCKlru, (struct Node *) b);
    return status;
}




/*-------------------*/
/*  IXfind_bucket    */
/*-------------------*/
GLOBAL UWORD
IXfind_bucket(i, p_bucket)
    UWORD i;
    BUCKET **p_bucket;
{
    INDEX_ENTRY *ix;
    BUCKET      *b;
    UWORD        status;

    DBG1("IXfind_bucket(%ld)\n", i);

    ix = fi->cur_index[i];
    DBG5("    INDEX_ENTRY: %ld %s %ld %ld %ld\n", ix->tfnum,
                             fi->libnames+ix->name_off,
                             ix->fhoff, ix->fhcount, ix->bucket_num);

  /* Search for BUCKET containing ix->tfnum */

    b = (BUCKET *) lBUCKlru.mlh_Head;   /* first BUCKET in lru list */
    while(b->mn.mln_Succ)
    {
        if(b->tfnum == ix->tfnum) break;
        b = (BUCKET *) b->mn.mln_Succ;  /* next BUCKET in the list */
    }
    if(b->mn.mln_Succ)
    {                                                    /* found it */
      /*  Make BUCKET most recently used */
        DBG("    search succeeded\n");
        Remove((struct Node *) b);
        AddTail((struct List *) &lBUCKlru, (struct Node *) b);
    }
    else                                 /* not found, make a new one */
    {
        DBG("    search failed- making new BUCKET\n");
        if(status = BUCKnew(ix, &b)) return status;
    }
    *p_bucket = b;
    return SUCCESS;
}




/*-------------------*/
/* IXsetup_chr_def   */
/*-------------------*/
/*  Setup chr_def, the character definition structure for chId.
 *  Returns a pointer to the BUCKET in which chId was found.
 *  If chId references a compound character, then all parts of
 *  the compound character must be in the same bucket that listed
 *  chId in its compound character segment.
 */
/*!!! here is where unicode matching will take place */
GLOBAL UWORD
IXsetup_chr_def(chId, chr_def, p_bucket)
    UWORD    chId;
    CHR_DEF *chr_def;
    BUCKET  **p_bucket;
{
    WORD         a, i, j, k, m;
    WORD	 poke0, poke, lo, hi, ucCnt;
    UWORD	 ucMap[MAX_UC_MAPS];
    UWORD        cgnum;
    UWORD        status;
    BUCKET      *b;
    BYTE        *p;
    UWORD        num_cc;    /* number of cc defs */
    UWORD        cc_cgnum;
    CHR_DEF     *cd;

    DBG1("IXsetup_chr_def(chId = %ld)\n", chId);

   /*  Search symbol map for corresponding cgnum[s] */
    lo = 0;
    hi = symbol_set_size - 1;
    ucCnt = 0;
    while (lo <= hi) {
	poke = poke0 = (lo + hi) / 2;
	DBG4("%ld .. %ld .. %ld: $%04lx\n", lo, poke, hi,
		symbol_set[poke].unicode);
	while ((cgnum = symbol_set[poke].unicode) == 0xffff) {
	    poke--;
	    DBG2("  .. %ld .. : $%04lx\n", poke, symbol_set[poke].unicode);
	}
	if (cgnum == chId) {
	    do {
		ucMap[ucCnt++] = symbol_set[poke++].cgnum;
		DBG2("  cgnum #%ld: %ld\n", ucCnt, ucMap[ucCnt-1]);
	    }
		while (symbol_set[poke].unicode == 0xffff);
	    break;
	}
	else
	    if (cgnum < chId)
		lo = poke0+1;
	    else
		hi = poke-1;
    }


    /* search for each cgnum alternative */
    for (a = 0; a < ucCnt; a++) {
	DBG1("cgnum = %ld\n", cgnum);
	cgnum = ucMap[a];
       /*  Search each BUCKET for cgnum. The order of the search is
	*  typeface sensitive, limited sensitive, universal.
	*/
	for(i=0; i<3; i++) {
	    if(status = IXfind_bucket(i, p_bucket))
		return status;

	    if(cgnum == 812) return SUCCESS;
	    if((chr_def->index = FMchar_index(*p_bucket, cgnum)) >= 0)
	    {
		DBG("    found as primary\n");
		is_compound = FALSE;
		num_parts = 1;
		chr_def->cgnum    = cgnum;
		chr_def->bucknum  = i;
		chr_def->offset.x = 0;
		chr_def->offset.y = 0;
		return SUCCESS;
	    }
	    else
	    {
		DBG("    searching compound\n");

		is_compound = TRUE;
		b = *p_bucket;
		if(!b->pcompound_seg) continue;
		p = b->pcompound_seg;

		num_cc = *(UWORD*)p;    p += 2;
		DBG1("    num_cc %ld\n", num_cc);

		for(j=0; j<num_cc; j++)
		{
		    cc_cgnum  = *(UWORD*)p;    p += 2;
		    h_esc     = *(WORD*)p;     p += 2;
		    v_esc     = *(WORD*)p;     p += 2;
		    num_parts = *(WORD*)p;     p += 2;

		  /*  Protect against over-running chr_def array. This means
		   *  we won't process characters in a font that contains
		   *  compound characters with too many parts even if these
		   *  complex compound characters are never used.
		   */

		    if(num_parts > MAX_CC_PARTS) return ERR_cc_complex;

		  /* speed up by skiping if no match */

		    cd = chr_def;
		    for(k=0; k<num_parts; k++)
		    {
			cd->cgnum = *(UWORD*)p;             p += 2;
			cd->offset.x   = *(WORD*)p;         p += 2;
			cd->offset.y   = *(WORD*)p;         p += 2;
			cd++;
		    }
		    if(cc_cgnum == cgnum)
		    {
		      /*  Found the compound character. */

			DBG4("cgnum %ld, h_esc %ld, v_esc %ld, num_parts %ld\n",
				cgnum, h_esc, v_esc, num_parts);

		      /*  For each compound character part, fill in its
		       *  bucket number and its character index.
		       */

			cd = chr_def;
			for(m=0; m<num_parts; m++)
			{
			    DBG3("  cgnum %ld, xoffset %ld, yoffset %ld\n",
				    cd->cgnum, cd->offset.x, cd->offset.y);

			    cd->bucknum  = i;
			    if((cd->index = FMchar_index(b, cd->cgnum)) < 0)
				return ERR_no_cc_part;
			    cd++;
			}

			return SUCCESS;
		    }
		}
	    }
	}
    }
    DBG("    ... not found\n");
    return ERR_find_cgnum;
}




/*------------------*/
/*  IXexit          */
/*------------------*/
GLOBAL VOID
IXexit()
{
    DBG("\n\nIXexit()\n");

  /* Free the character buffer */

    DBG1("  Free char_buf $%lx\n", char_buf);
    FreeVec(char_buf);
    char_buf = 0;

   /* Free all BUCKETs */

    while(lBUCKlru.mlh_Head->mln_Succ) {
	DBG1("  Free lBUCKlru element $%lx\n", lBUCKlru.mlh_Head);
        BUCKfree(lBUCKlru.mlh_Head);
    }
  /* Free FONTINDEX */

    DBG1("  Free fi->fnt_index $%lx\n", fi->fnt_index);
    MemFree(fi->fnt_index);

  /* Free symbol set directory */

    DBG1("  Free symbol_set $%lx\n", symbol_set);
    MemFree(symbol_set);
}



/*------------------*/
/*  buildpath       */
/*------------------*/
/*  Build the a full pathname in dest.  Assume source2 is either a simple
 *  filename or already the full pathname. Use the presence of a "\" to
 *  determine. source1 if the default directory path.
 *  If source1 is null or source2 contains a slash, copy source2 to dest.
 *  Otherwise, concatinate source1 and source2 in dest making sure that
 *  there is a "\" between them.
 */
MLOCAL VOID
buildpath(dest, source1, source2)
   BYTE  *dest,*source1, *source2;
{
    DBG3("buildpath(0x%lx, \"%s\", \"%s\")", dest, source1, source2);

    if (strchr(source2, ':'))
	strcpy(dest, source2);
    else {
	strcpy(dest, source1);
	AddPart(dest, source2, 256);
    }
    DBG1(": \"%s\"\n", dest);
}
