/* ixmak.c */
/* Copyright (C) Agfa Compugraphic, 1989, 1990. All rights reserved. */
/*   12-Mar-90   jfd   If requested font is italic, set bit 2 of
 *                     bucket number.
 *    9-Aug-90   awr   Merged Barbara's compression changes.
 *   11-Aug-90   awr   In process_lib() but a check for file directory
 *                     buffer overflow.
 *
 */
#include    <stdlib.h>
#include    <stdio.h>
#include    <fcntl.h>
#include    <string.h>
#include    "port.h"
#include    "debug.h"
#include    "cgconfig.h"
#if COMPRESS
#include    "istypes.h"
#endif

#include    "diskfonttag.h"

/*--------------------------------*/
/*       font index entry         */
/*--------------------------------*/
/*  Bit definitions for bucket_type */
#define B_BOLD          0x0001
#define B_SANS_SERIF    0x0002
#define B_ITALIC        0x0004
#define B_COMPRESS      0x8000

               /* ------------------------------------ */
               /* Typeface Header Structure Definition */
               /* ------------------------------------ */

struct typefaceHeaderType {
  unsigned short      NFACES;
  char                typeFaceName[50];
  char                familyName[20];
  char                weight[20];
  long                typeFaceSet[12];
};

struct descriptorSetType {
       unsigned char  stemStyle;
       unsigned char  stemMod;
       unsigned char  stemWeight;
       unsigned char  slantStyle;
       unsigned char  horizStyle;
       unsigned char  vertXHeight;
       unsigned char  videoStyle;
       unsigned char  copyUsage;
} ;

               /* ------------------------------------ */
               /* Font Alias Data Structure Definition */
               /* ------------------------------------ */

struct fontAliasTableType {
  char                        aliasTableName[20];
  unsigned short              NXREF;
  char                        xRefArray[100];
  unsigned long               CGtfnum;
};

struct fontAliasDataType {
  unsigned short              NFATS;
  struct fontAliasTableType   fontAliasTable[1];
};


/*
  This file contains the definition of the HP/PCL font alias segment
  as an offset into the 100 byte structure
 */

#define FATYPEABBREV	(0)	/* two character typeface abbrev (char)	*/
#define FATREATMENT	(3)	/* one character treatment code	(char)	*/
				/* fixed / prop			(char)	*/
#define FAPCLWIDTH	(7)	/* width type + 5		(int)	*/
				/* reserved			(int)	*/
#define FASTRUCTURE	(12)	/* style structure		(int)	*/
#define FAWIDTH		(15)	/* style width			(int)	*/
#define FAPOSTURE	(17)	/* style posture		(int)	*/
#define FAWEIGHT	(19)	/* weight + 8			(int)	*/
#define FATYPEFAMILY	(22)	/* typeface family		(int)	*/
#define FASERIF		(28)	/* serif style			(int)	*/
#define FASIMPLESERIF	(31)	/* simple serif flag		(int)	*/
#define FACOMPRESSFLG	(33)	/* compressed flag		(int)	*/
#define FASHORTNAME	(35)	/* short typeface name		(char)	*/
#define FASHORTNAMESIZE	(16)
#define FACGNUMBER	(52)	/* CG typeface number		(long)	*/
#define FACLASSINDEX	(59)	/* family class index		(int)	*/

#define FONTALIASNAME "HP                  "


#define MAXLINE 133
#define MAXTF    32
#define MAXLIBNAME 3000

BYTE bucket_descriptors[32] =
{

    /* Type Face 5720  */

        20,    /* stem style:   */
        20,    /* stem mode:    */
        30,    /* stem weight:  */
        10,    /* slant style:  */
        10,    /* horiz. style: */
        20,    /* vert. style:  */
        10,    /* video style:  */
        40,    /* copy usage:   */

    /* Type Face 5721  */

        20,    /* stem style:   */
        20,    /* stem mode:    */
        40,    /* stem weight:  */
        10,    /* slant style:  */
        10,    /* horiz. style: */
        20,    /* vert. style:  */
        10,    /* video style:  */
        40,    /* copy usage:   */

    /* Type Face 5723  */

        10,    /* stem style:   */
        40,    /* stem mode:    */
        30,    /* stem weight:  */
        10,    /* slant style:  */
        10,    /* horiz. style: */
        20,    /* vert. style:  */
        10,    /* video style:  */
        40,    /* copy usage:   */

    /* Type Face 5724  */

        10,    /* stem style:   */
        40,    /* stem mode:    */
        40,    /* stem weight:  */
        10,    /* slant style:  */
        10,    /* horiz. style: */
        20,    /* vert. style:  */
        10,    /* video style:  */
        40    /* copy usage:   */
};



int trace_sw;		       /* Print debug data if not 0.            */
int update;                    /* 0 = normal font index file
                                * 1 = all typeface numbers will be negated
                                * in the font index file.
                                */

MLOCAL BYTE  inline[MAXLINE];  /* Next line from input file             */
MLOCAL BYTE  infile[MAXLINE];  /* Input file name                       */
MLOCAL BYTE  outfile[MAXLINE]; /* Input file name                       */
MLOCAL FILE *instream;         /* Input file stream                     */
MLOCAL WORD  fout;             /* Output file                           */


MLOCAL BYTE  libname[64];
MLOCAL WORD  f;               /* library file */


MLOCAL WORD  entry_ct;
MLOCAL BYTE  libnames[MAXLIBNAME];
MLOCAL UWORD name_off;   /* total length of strings written in above */


/*------------------*/
/*  FMseek_read     */
/*------------------*/
GLOBAL BOOLEAN
FMseek_read(offset, count, buffer)
    LONG   offset;     /* offset within file       */
    UWORD  count;      /* number of BYTEs to read  */
    BYTE  *buffer;     /* buffer to raed data into */
{
    DBG2("                FMseek_read()   offset: %ld   count: %u\n",
                                                          offset, count);

    if(lseek(f, offset, 0) != offset)
    {
        DBG("                    lseek() failed\n");
        return FALSE;
    }

    if(read(f, buffer, count) != count)
    {
        DBG("                    read() failed\n");
        return FALSE;
    }
}




/*-------------*/
/*  error      */
/*-------------*/
MLOCAL BOOLEAN
error()
{
    printf("%s is not a valid library file\n", libname);
    return FALSE;
}




MLOCAL WORD
rWORD()
{
    WORD i;

    if(read(f,(BYTE*)&i,2) != 2)
    {
        DBG("rWORD error\n");
        error();
    }
    return i;
}


MLOCAL LONG
rLONG()
{
    LONG i;

    if(read(f,(BYTE*)&i,4) != 4)
    {
        printf("rLONG error\n");
        error();
    }
    return i;
}


MLOCAL VOID
wBYTE(x)
    BYTE x;
{
    write(fout, (BYTE*)&x, 1);
}

MLOCAL VOID
wWORD(x)
    WORD x;
{
    write(fout, (BYTE*)&x, 2);
}


MLOCAL VOID
wLONG(x)
    LONG x;
{
    write(fout, (BYTE*)&x, 4);
}



/*-------------------------*/
/*  get_line               */
/*-------------------------*/
/*  Load the next non-blank line from text stream "s" into line
 *  buffer "line". Return TRUE if successful, FALSE if end of
 *  file is reached.
 */

MLOCAL BOOLEAN
get_line(s, line)
    FILE *s;
    BYTE *line;
{
    BYTE *cp;

    while(TRUE)
    {
        if(fgets( line, MAXLINE, s) == NULL) return FALSE;
        cp = line;
        while( ((*cp == ' ') || (*cp == '\t')) && *cp != '\n' ) cp++;
        if(*cp != '\n') return TRUE;
    }
}


/*--------------------*/
/*  find_bucket_type  */
/*--------------------*/
MLOCAL BOOLEAN
find_bucket_type(tfnum, globoff, globcount, p_bucket_type, file_type)
    LONG   tfnum;
    ULONG  globoff;
    UWORD  globcount;
    UWORD *p_bucket_type;
    UWORD file_type;
{
    BYTE *fgseg;
    BYTE *typeface_seg;
    BYTE *fontalias_seg;

    BYTE *p, *pp;

    UWORD type;
    ULONG size;

    UWORD key;
    ULONG off;
    UWORD count;

    BYTE  *segptr;


  /*  Fontalias segment */

static BYTE
       faisweights[15] = {0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
                          0x03, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05};

    UWORD    NFATS;
    UWORD    NXREF;
    ULONG    FACENUM;
    UWORD    i,j;
    BOOLEAN  found_table_name;
    BYTE    *fa_table;

  /* Typeface header segment */

    UWORD  NFACES;
    BYTE  *face_classification;

  /* bucket matching */

    WORD  word1, word2, word3;
    UWORD current, closest;


    DBG2("\n        find_bucket_type()  offset: %ld   count: %d\n",
                           globoff, globcount);

  /* get face global buffer */

    if(!(fgseg = malloc(globcount)))
    {
        DBG("            malloc failed\n");
        return error();
    }


  /* read face global data into buffer */
#if COMPRESS
    if(file_type == 'D')
    {
#endif
       if(!FMseek_read(globoff, globcount, fgseg))
       {
           DBG("            face global file error\n");
           free(fgseg);
           return error();
       }
#if COMPRESS
    }
    else
    {
       if(is_expand(f, globoff, fgseg, globcount, IS_EXP_FACEGLOBSEG))
       {
           DBG("            face global file error\n");
           free(fgseg);
           return error();
       }
    }
#endif

  /*  Read segment directory and set segment pointers. First
   *  set segment ptrs to nul- not all segments may be present
   */

    typeface_seg   = (BYTE*)0;
    fontalias_seg  = (BYTE*)0;

    p = fgseg;

    type = *((UWORD*)p);     p += 2;
    size = *((ULONG*)p);     p += 4;
    DBG2("            type = %d    size = %ld\n",type, size);

    DBG("\n            directory\n");
    do
    {
        key =   *((UWORD*)p);     p += 2;
        off =   *((ULONG*)p);     p += 4;
        count = *((UWORD*)p);     p += 2;
        DBG3("            key / offset / count  %d   %ld   %d\n",
                                              key,off,count);

        segptr = (BYTE*)(fgseg + off);
        switch (key)
        {
            case 107:  typeface_seg = segptr + 6;
                       break;
            case 110:  fontalias_seg = segptr + 6;
                       break;
            default:   break;
        }
    }
    while (key != -1);

    if(typeface_seg)
    {
        DBG("\n\n    T y p e f a c e    H e a d e r    S e g m e n t\n\n");
        p = typeface_seg;

        NFACES = *(UWORD*)p;     p += 2;
        DBG1("NFACES = %d\n", NFACES);

        DBG1("    Primary name: --%50.50s--\n", p);
        p += 50;

        DBG1("    Family name: --%20.20s--\n", p);
        p += 20;

        DBG1("    Weight description: --%20.20s--\n", p);
        p += 20;

        pp = p + (4*NFACES);   /*  point to face description array */

        DBG("    Typefaces:\n");
        for(i=0; i<NFACES; i++)
        {
            DBG1("        %lu\n", *(ULONG*)p);
            if(*(ULONG*)p == tfnum) break;
            p += 4;
        }
        /*  i is now set to index of typeface  */

        if(i==NFACES)
        {
            DBG1("tfnum %ld not found in typeface header segment\n", tfnum);
            return FALSE;
        }

        p = pp + (8*i);     /*  index into descriptor array */
        face_classification = p;
        
        DBG("    Face Description:\n");        
        DBG1("        stem style:   %u\n", *p);
        DBG1("        stem mode:    %u\n", *(p+1));
        DBG1("        stem weight:  %u\n", *(p+2));
        DBG1("        slant style:  %u\n", *(p+3));
        DBG1("        horiz. style: %u\n", *(p+4));
        DBG1("        vert. style:  %u\n", *(p+5));
        DBG1("        video style:  %u\n", *(p+6));
        DBG1("        copy usage:   %u\n", *(p+7));
    }

    if(fontalias_seg)
    {
        DBG("\n\n    F o n t a l i a s    S e g m e n t\n\n");

        p = fontalias_seg;
        fa_table = (BYTE*)0;

        NFATS = *(UWORD*)p;    p += 2;
        DBG1("    NFATS = %d\n", NFATS);
        for(i=0; i<NFATS; i++)
        {
            DBG1("    Alias table name: --%20.20s--\n", p);

            found_table_name = (strncmp(FONTALIASNAME, p, 20) == 0);
            p += 20;

            NXREF = *(UWORD*)p;  p += 2;
            DBG1("        NXREF = %d\n", NXREF);
            for(j=0; j<NXREF; j++)
            {
                FACENUM = *(ULONG*)(p+100);
                DBG1("            %lu\n", FACENUM);
                DBG1("            %25.25s\n", p);
                DBG1("            %25.25s\n", p+25);
                DBG1("            %25.25s\n", p+50);
                DBG1("            %25.25s\n", p+75);

                if(found_table_name && (FACENUM == tfnum))
                {
                    fa_table = p;
                    break;
                }
                p += 104;
            }
            if(fa_table) break;
        }
        if(i == NFATS)
        {
            DBG("font alias table not found\n");
            return FALSE;
        }

        DBG("Replacing classification data from font alias table:\n\n");

      /* Stem style */
        DBG("    stem style:\n");
        DBG1("        fa_table[FASIMPLESERIF] = %d\n", 
                                        atoi (&fa_table[FASIMPLESERIF]));
        DBG1("        old stem style = %d\n", face_classification[0]);

        face_classification[0] = (BYTE)
                        (10 * (atoi(&fa_table[FASIMPLESERIF]) + 1));

        DBG1("        new stem style = %d\n", face_classification[0]);

      /* stem weight */

        DBG("    stem weight:\n");
        DBG1("    fa_table[FAWEIGHT]) - 1 = %d\n",
                                             atoi(&fa_table[FAWEIGHT]) - 1);
        DBG1("        old stem weight = %d\n", face_classification[2]);
        face_classification[2] = (BYTE)
                      (10 * faisweights[atoi(&fa_table[FAWEIGHT]) - 1]);
        DBG1("        new stem weight = %d\n", face_classification[2]);

      /* slant style */

        DBG("    slant style:\n");
        DBG1("        fa_table[FAPOSTURE] = %d\n",
                                         atoi(&fa_table[FAPOSTURE]));
        DBG1("        old slant style = %d\n",
                                        face_classification[3]);
        face_classification[3] = (BYTE)
                          (10 * ((atoi(&fa_table[FAPOSTURE])&1) + 1));
        DBG1("        new slant style = %d\n", face_classification[3]);
    }

  /*  Compute limited sensitive bucket */

    closest = 0xffff;
    p = bucket_descriptors;
    for(i=0; i<4; i++)
    {
        word1 = (face_classification[2] - p[2])/10;
        word2 = (face_classification[3] - p[3])/10;
        word3 = (face_classification[0] - p[0])/10;

        current = (((UWORD)word1&0x8000)>>7) +
                  ((UWORD)((ABS(word1))<<9)) +
                  (((UWORD)word2&0x8000)>>11) +
                  ((UWORD)((ABS(word2))<<5)) +
                  (((UWORD)word3&0x8000)>>15) +
                  (UWORD)((ABS(word3))<<1);
               
        if (closest > current)
        {
            closest = current;
            *p_bucket_type = i;
        }
        p += 8;
    }

    if (face_classification[3] > (BYTE)10)
        *p_bucket_type |= B_ITALIC;  /* if italic, set bit 2 */
                                     /* 12-Mar-90 jfd        */
    if(file_type == 'C')
        *p_bucket_type |= B_COMPRESS;  /* if compressed, set bit 3 */

    free(fgseg);
    return TRUE;
}

/*----------------*/
/*  process_face  */
/*----------------*/
MLOCAL BOOLEAN
process_face(tfnum, fhoff, fhcount, type)
    LONG tfnum, fhoff;
    UWORD fhcount;
    UWORD type;
{
    BYTE  *char_dir;   /* face header segment */
    LONG globoff;
    UWORD globcount;
    UWORD  name_len;
    UWORD  bucket_type;

    DBG1("\nType Face %ld\n",tfnum);

    DBG("Face header segment\n");

    if(!(char_dir = malloc(fhcount)))
    {
        DBG("            malloc failed\n");
        return FALSE;
    }


  /* read face header link to face global segment */
#if COMPRESS
    if(type == 'D')
    {
#endif
       if(!FMseek_read(fhoff, (UWORD)6, char_dir))
       {
           DBG("FMseek_read error\n");
           free(char_dir);
           error();
           return FALSE;
       }
#if COMPRESS
    }
    else
    {
        if(is_expand(f, fhoff, char_dir, fhcount, IS_EXP_FACEHDRSEG))
        {
            DBG("is_expand error\n");
            free(char_dir);
            error();
            return FALSE;
        }
    }
#endif

  /* Type face global segment */

    globoff =   *(LONG*)char_dir;
    globcount = *(UWORD*)(char_dir + 4);
    free(char_dir);
    DBG2("    global off / count   %ld  %d\n",globoff, globcount);

    DBG1("                                              total = %d\n",
                                 fhcount+globcount);
  /* Find type of limited sensitive bucket */

    if(!find_bucket_type(tfnum, globoff, globcount, &bucket_type, type))
        return FALSE;

  /* Write entry in output file */


    if(update) tfnum = - tfnum;    /* negate typeface number for update */
    DBG5("%ld %s %ld %u %u\n", tfnum, libname, fhoff, fhcount, bucket_type);

    wLONG(tfnum);
    wWORD(name_off);
    wLONG(fhoff);
    wWORD(fhcount);
    wWORD(bucket_type);          /* bucket number: 0 - 3 */
#if 0
    /* Kodiak's additions to ix entry */
    /* serifFlag */
    if (bucket_type & B_SANS_SERIF)
	/* sans-serif */
	wBYTE(0);
    else
	/* serif */
	wBYTE(1);
    /* stemWeight */
    if (bucket_type & B_BOLD)
	/* bold */
	wBYTE(OTS_Bold);
    else
	/* normal */
	wBYTE((OTS_Book+OTS_Medium)/2);
    /* slantStyle */
    if (bucket_type & B_ITALIC)
	/* italic */
	wBYTE(OTS_Italic);
    else
	/* non-italic */
	wBYTE(OTS_Upright);
    /* horizStyle */
    wBYTE(OTH_Normal);
#endif
    entry_ct++;

    name_len = strlen(libname)+1;	/* inc. null */
    DBG1("    strlen = %d\n", name_len);
    if(name_off + name_len > MAXLIBNAME)
    {
        printf("Library names exceed limit\n");
        return FALSE;
    }

    strcpy(&libnames[name_off], libname);
    name_off += name_len;

    return TRUE;
}




/*----------------*/
/*  process_lib   */
/*----------------*/
MLOCAL VOID
process_lib()
{
    UWORD type;

    WORD  i;
    WORD  num_faces;

    UWORD key,count;      /* File segment directroy */
    LONG off;

    LONG fdoff;           /* File directory */
    UWORD fdcount;

    struct tfseg {
      LONG  tfnum;
      LONG  tfoff;
      UWORD tfcount;
    } tfbuf[MAXTF];


    DBG("process_lib\n");

    type = rWORD();
    DBG1("File type = %d\n",type);
    if(type != 'D' && type != 'C')
    {
        error();
        return;
    }

  /* File segment directory */

    DBG("\nFile segment directory\n");
    do
    {
        key = rWORD();
        off = rLONG();
        count = rWORD();
        DBG3("    key / offset / count  %d   %ld   %d\n",key,off,count);
        if(key == 2)
        {
            fdoff = off;       /* save file directory segment */
            fdcount = count;
        }
    }
    while (key != -1);

  /* File Directory Segment */

    DBG("\nFile directory segment\n");
    if(fdcount > 10 * MAXTF)
    {
        DBG("Error: too many typefaces in library\n");
        return;
    }
    if (type == 'D')
    {
        if( lseek(f, fdoff, 0) != fdoff)
        {
            DBG("seek error\n");
            error();
            return;
        }
        if( read(f, (UBYTE *) tfbuf, fdcount) != fdcount )
        {
            DBG("read error\n");
            error();
            return;
        }
    }
    else
    {
#if COMPRESS
        if(is_expand(f, fdoff, (UBYTE *) tfbuf,
			 MAXTF*sizeof(struct tfseg), IS_EXP_FILEDIR))
        {
            DBG("expand error\n");
            error();
            return;
        }
#else
        printf("    Can't process compressed library\n");
        return;
#endif

    }


    for(i=0; i<MAXTF; i++)
    {
        DBG3("    face / offset / count %ld   %ld  %d\n",
                       tfbuf[i].tfnum, tfbuf[i].tfoff, tfbuf[i].tfcount);
        if(tfbuf[i].tfnum == -1) break;
    }
    num_faces = i;
    DBG1("%d faces in library\n", num_faces);


    for(i=0; i<num_faces; i++)
        if(!process_face(tfbuf[i].tfnum, tfbuf[i].tfoff, tfbuf[i].tfcount,type))
            return;
}



/*------------------*/
/*  write_lib_names */
/*------------------*/
MLOCAL VOID
write_lib_names()
{
    DBG("write_lib_names\n");

  /* Write out all library name strings */

    write(fout, (BYTE*)&libnames[0], name_off);

  /* Write entry count at start of file */

    if( lseek(fout, 0L, 0) != 0L)
    {
        DBG("seek error\n");
        printf("seek error\n");
        return;
    }

    wWORD(entry_ct);
}

#ifdef  DEBUG
kprintf(fmt, arg1, arg2, arg3, arg4, arg5, arg6)
char *fmt, *arg1, *arg2, *arg3, *arg4, *arg5, *arg6;
{
    printf(fmt, arg1, arg2, arg3, arg4, arg5, arg6);
}
#endif


/*----------------*/
/*  main          */
/*----------------*/
GLOBAL VOID
main(argc, argv)
int argc;
char *argv[];
{
    char *s;

#ifdef DEBUG
    printf("trace: ");
    scanf("%d",&trace_sw);
    printf("Update: (0 normal, 1 update libraries) ");
    scanf("%d",&update);
#else
    trace_sw = 0;
    update   = 0;
#endif


    printf("Input data file: "); 
    scanf("%s",infile);
    if (!(instream = fopen(infile, "r")))
    {
        DBG1("File %s not found\n", infile);
        return;
    }

    if ((fout = open("out.fnt", O_CREAT|O_TRUNC|O_WRONLY)) == -1)
    {
        DBG("Cannot open out.fnt\n");
        return;
    }


    name_off = 0;   /* total length of name strings so far */
    entry_ct = 0;   /* total output file entries */
    wWORD(entry_ct);
    while ( get_line(instream, inline))
    {
        DBG1("%s\n", inline);
        sscanf(inline,"%s", libname);
        DBG1("%s\n",libname);
	if ((s = strrchr(libname, '/')) || (s = strrchr(libname, ':')))
	    s++;
	else
	    s = libname;
        DBG1("%s\n", s);
        if ((f = open(s, O_RDONLY)) == -1)
        {
            printf("Library file %s not found\n", s);
        }
        else
        {
            process_lib();
            close(f);
        }
    }
    fclose(instream);

    write_lib_names();
    close(fout);
}

