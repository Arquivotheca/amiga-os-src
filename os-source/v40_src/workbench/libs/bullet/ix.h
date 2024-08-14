/* ix.h */
/*  01-Apr-90  awr  Expanded SS_DIRECTORY to include pcleo info
 *  12-Apr-90  jfd  Added handles for Typeface Header and Font
 *                    Alias segments to BUCKET structure
 *  07-Jun-90  dah  Added input_filetype to INDEX_ENTRY and BUCKET structs.
 *  01-Jul-90  awr  Corrected comments, added bit flag definitions and
 *                  screen font definitions.
 *  15-Aug-90  awr  added BUCKET's handle to BUCKET structure.
 *  17-Sep-90  jfd  Changed CHARBUFSIZE from 2048 to 4096
 */

#define MAX_CC_PARTS  10    /* maximum number of parts in a compound ch. */
#define MAX_UC_MAPS    4    /* maximum # of CG codes mapped to one UniCode */

#define CHARBUFSIZE  4096   /* character buffer size */

/* Bucket Types */

#define  TF_SENSITIVE    0
#define  LIM_SENSITIVE   1
#define  UNIVERSAL       2

/*  Typeface number definitions for generic screen faces and plugin faces */

#define FACE_S_SCR       5500L  /* serif screen font                     */
#define FACE_SS_SCR      5501L  /* sans serif screen font                */
#define FACE_UNIV        5719L  /* Universals                            */
#define FACE_LS_S_NORM   5720L  /* Limited sensitive  serif,      normal */
#define FACE_LS_S_BOLD   5721L  /*                    serif,      bold   */
#define FACE_LS_SS_NORM  5723L  /*                    sans-serif, normal */
#define FACE_LS_SS_BOLD  5724L  /*                    sans-serif, bold   */

/*--------------------------------*/
/*       font index entry         */
/*--------------------------------*/

#define B_BOLD          0x0001
#define B_SANS_SERIF    0x0002
#define B_ITALIC        0x0004
#define B_COMPRESS      0x8000

typedef struct
{
    LONG    tfnum;        /* typeface number                          */
    UWORD   name_off;     /* offset to full path name of library file */
    ULONG   fhoff;        /* offset in library file for face header   */
    UWORD   fhcount;      /* BYTE count of face header                */
    WORD    bucket_num;   /* Bit flags
                           *
                           *   Bit 15 = 0    Normal library file
                           *            1    Compressed library file
                           *
                           *
                           *   Bit definitions:
                           *      bit         0           1
                           *       0         normal      bold
                           *       1         serif       sans-serif
                           *       2         non-italic  italic
                           *       3-14      reserved
                           *
                           * Associated limited sensitive face:
                           *     0 = 5720    Serif         Normal
                           *     1 = 5721    Serif         Bold
                           *     2 = 5723    Sans-serif    Normal
                           *     3 = 5724    Sans-serif    Bold
                           */
} INDEX_ENTRY;


/*--------------------------------*/
/*     FONTINDEX                  */
/*--------------------------------*/
/*  Memory for the entire font index file and this structure is obtained
 *  ... The font index file is read into the
 *  beginning of this block. The following structure is at the end of the
 *  block.
 */
typedef struct
{
    UBYTE       *fnt_index;      /* font index buffer                    */
    WORD         entry_ct;        /* number of entries in index           */
    INDEX_ENTRY *index_entry;     /* font index entries                   */
    char        *libnames;        /* path names of library files in index */

  /*  The current typeface. */

    LONG         cur_tf_num;      /* current type face number             */
    INDEX_ENTRY *cur_index[3];   /* Index entrie handles for the three
                                   * buckets for the current typeface.
                                   */
} FONTINDEX;

/*--------------------------------*/
/*     BUCKET                     */
/*--------------------------------*/
/*  106 BYTEs  */

typedef struct
{
    struct MinNode mn;
    BOOLEAN   file_open;       /* Library file is open                   */
    LONG      tfnum;    
    ULONG     f;               /* (really BPTR) File handle */
    UWORD     bucket_num;      /* Copy of ix->buket_num */

  /*  character directory  (face header segment) */

    UBYTE         *pface_header_seg;
    struct _CH_DIR_ENTRY *pchar_dir;     /* Character directory  */
    UWORD          ch_count;             /* Number of characters */

  /*  face global segment */

    UBYTE     *pfgseg;           /* Handle of face global segment */
    UWORD      sfgseg;
    UBYTE     *pgif;              /* Global Intellifont segment */
    UWORD      gifct;
    UBYTE     *ptrack_kern_seg;   /* track kerning              */
    UWORD      strack_kern_seg;
    UBYTE     *ptext_kern_seg;    /* text kerning               */
    UWORD      stext_kern_seg;
    UBYTE     *pdesign_kern_seg;  /* designer kerning           */
    UWORD      sdesign_kern_seg;
    struct _CHARWIDTH *pwidth_seg;        /* character width segment    */
    UWORD      swidth_seg;
    struct _FACE_ATT  *pattribute;        /* Attribute header           */
    UWORD      sattribute;
    UBYTE     *prast;             /* Raster parameter           */
    UWORD      srast;
    UBYTE     *ptf_header_seg;    /* Typeface Header segment    */
    UWORD      stf_header_seg;
    UBYTE     *pcompound_seg;     /* Compound character         */
    UWORD      scompound_seg;
    struct _DISPLAY   *pdisplay;          /* Display header             */
    UWORD      sdisplay;
    UBYTE     *pfont_alias_seg;   /* Font Alias segment         */
    UWORD      sfont_alias_seg;
    UBYTE     *pcopy;             /* Copyright notice           */
    UWORD      scopy;
} BUCKET;

/*--------------------------------*/
/*--------------------------------*/

typedef struct
{
    UWORD cgnum;
    UWORD bucknum;
    WORD  index;
    WORDPOINT    offset;
    WORDPOINT    bmorigin;
} CHR_DEF;


/*--------------------------------*/
/*        Symbol set entry        */
/*--------------------------------*/

typedef struct
{
    UWORD unicode;
    UWORD cgnum;
} SS_ENTRY;
