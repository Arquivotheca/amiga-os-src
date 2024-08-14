/* cgif.h */
/* Copyright (C) 1989, 1990 by Agfa Compugraphic, Inc. All rights reserved. */
/*
 *     21-Jul-90   awr   moved segment definitions for CGIFsegments()
 *                       from segments.h to cgif.h
 *     23-Jul-90   awr   added Blake's typePath to IF_CONFIG
 *     26-Jun-90   bjg   Added IFOUTLINE OUTLINE_LOOP and OUTLINE_CHAR.  Also
 *                       added error flags for outline or bitmap not installed.
 *     17-Aug-90   awr   corrected byte count comment for FONTCONTEXT
 *
 */
/*----------------------------------*/
/*         Memory Pools */
/*----------------------------------*/
#define CACHE_POOL   0
#define BUFFER_POOL  1

/*----------------------------------*/
/*     CGIFkern() query codes */
/*----------------------------------*/
#define  TEXT_KERN    0
#define  DESIGN_KERN  1

/*-----------------------------------*/
/* CGIFsegments() data segment codes */
/*-----------------------------------*/
#define FACE_GLOBAL_KEY   0    /* Entire face global segment */
#define GIF_KEY         100    /* Global Intellifont segment */
#define TRACK_KERN_KEY  101    /* track kerning */
#define TEXT_KERN_KEY   102    /* text kerning */
#define DESIGN_KERN_KEY 103    /* designer kerning */
#define WIDTH_KEY       104    /* character width segment */
#define ATTRIBUTE_KEY   105    /* Attribute header */
#define RASTER_KEY      106    /* Raster parameter */
#define TF_HEADER_KEY   107    /* Typeface Header segment */
#define COMPOUND_KEY    108    /* Compound character */
#define DISPLAY_KEY     109    /* Display header */
#define FONT_ALIAS_KEY  110    /* Font Alias segment */
#define COPYRIGHT_KEY   111    /* Copyright notice */

/*----------------------------------*/
/*        Error Return Codes */
/*----------------------------------*/
#define ERR_bad_bitmap_width   1
#define ERR_no_font_index      3   /* can't open font index file */
#define ERR_rd_font_index      4   /* can't allocate or read font index */
#define ERR_missing_plugin     5   /* missing one or more plugin lib */
#define ERR_no_symbol_set      6   /* can't open symbol set file */
#define ERR_rd_symbol_set      7   /* can't allocate or read symbol set */
#define  ERR_bad_pool          21
#define  ERR_CACinit           32
#define  ERR_no_font           53
#define  ERR_IXnew_ss          54
#define  ERR_bad_chID          61
#define  ERR_bm_buff           63
#define  ERR_no_cgnum          64  /* ss id does not map to a cgnum */
#define  ERR_no_fi_entry       65  /* no font index */
#define  ERR_open_lib          66  /* Error opening library file */
#define  ERR_mem_face_hdr      67  /* Mallac failed for face header */
#define  ERR_face_abort        68
#define  ERR_find_cgnum        69  /* Can't find cgnum in char. index */
#define  ERR_rd_char_data      70  /* Can't read character data */
#define  ERR_buffer_too_small  71
#define  ERR_bad_kern_code     81
/* pcleo.c */
#define ERRnot_ready_lpt      100  /* printer not ready during write */
#define ERRwrite              101  /* fwrite() error */
#define ERRopen_printer       102  /* LPT not ready during open_file() */
#define ERRopen_file          103  /* fopen() error   "      "    " */
#define ERRfstat              104  /* fstat() error   "      "    " */
#define ERRrastparam_missing  105  /* missing raster parameter segment */
#define ERRattribseg_missing  106  /*    "    font attribute      " */
#define ERRdispheader_missing 107  /*    "    display header      " */
#define ERRfat_missing        108  /*    "    font alias table */
#define ERRbadchar            109  /* Inconsistent skeletal char data */
#define ERRvertesc            110  /* Non-zero vertical escapement */
/*---------*/
#define  ERR_bm_gt_oron       599  /* Char bitmap too big for on array */
#define  ERR_bm_too_big       600  /* Char bitmap too big */
#define  ERR_mem_char_buf     602  /* Can't malloc character buffer */
#define  ERR_if_init_glob     605  /* if_init_glob() failed */
#define  ERR_comp_pix         606  /* com_pix() failed */
#define  ERR_fixed_space      607  /* character is a fixed space */
#define  ERR_skeletal_init    610
#define  ERR_y_skel_proc      611
#define  ERR_x_skel_proc      612
#define  ERR_bold_buf         613
#define  ERR_no_contour       701
#define  ERR_not_hq3          702
#define  ERR_ov_char_buf      703
#define  ERR_out_of_handles   800
#define  ERR_bad_handle       801
#define  ERR_lost_mem         802  /* CGIFdefund() didnot free whole block */
#define  ERR_IXopen_file      803
#define  ERR_no_buck_mem      804  /* BUCKnew couldn't get memory */
#define  ERR_cc_complex       805  /* Compound character hs too many parts */
#define  ERR_no_cc_part       806  /* Can't find compound character part */
#define  ERR_missing_block    807  /* CGIFdefund() can't find the block */
#define  ERR_max_callers      900  /* maximum number of callers */
#define  ERR_bad_ID           901  /* Caller ID is out of range */
#define  ERR_not_valid_ID     902  /* ID passed is not active */
#define  ERR_ccBuff_alloc     903  /* cannot allocate comp-char buffer */
#define  ERRtoo_many_dup_skel 904  /* too many duplicate skel points */


/*  output processor installation errors */
#define  ERR_bitmap_not_installed    951
#define  ERR_outline_not_installed   952
#define  ERR_quadratic_not_installed 953
#define  ERR_linear_not_installed    954


/*----------------------------------*/
/* Intellifont Configuration Block */
/*----------------------------------*/

typedef struct
{
    UWORD  max_char_size;   /* max cached character bitmap size */
    UBYTE *cc_buf_ptr;      /* compound character buffer pointer */
    UWORD  cc_buf_size;     /*    "        "        "       " */
    UWORD  num_files;       /* max number of open library files */
    BYTE   typePath[PATHNAMELEN];    /* location of typeface files. */
} IFCONFIG;




/*----------------------------------*/
/*       Font Context */
/*----------------------------------*/
/*  48 bytes */
typedef struct
{
    LONG    font_id;      /* font number */
    char   *name;         /* font library file name */
    ULONG   fhoff;        /* offset in library file for face header */
    UWORD   fhcount;      /* BYTE count of face header */
    WORD    bucket_num;   /* Bit flags */
    WORD    point_size;   /* point size of this character in 1/8th pts */
    WORD    set_size;     /* set   size of this character in 1/8th pts */
    POINTFX shear;        /* cosine & sine of vertical stroke angle */
    POINTFX rotate;       /* cosine & sine of baseline rotation */
    ULONG   xres;         /* output pixels per meter */
    ULONG   yres;
    FIXED   xspot;
    FIXED   yspot;
    WORD    xbold;
    WORD    ybold;
    UWORD   ssnum;
    UWORD   format;       /* output format */
                          /* bit 15-14    0  Auto-quality */
                          /*              1  quality level 1 */
                          /*              2  quality level 2 */
                          /*              3  quality level 3 */

                   
}
FONTCONTEXT;



/*----------------------------------*/
/*    Character Bitmap */
/*----------------------------------*/

#define HEADERSIZE   34  /*  size of header in IFBITMAP structure */

typedef struct
{
    struct MinNode mn;     /* doubly linked list pointers */
    UWORD   notmru;        /* set to 0 if recently made most recently used */
    struct _FONT *iffont;       /* Font handle of the owner of this IFBITMAP */
    WORD    index;         /* index to IFBITMAP in above FONT's table */

    WORD    width;         /* bit map width (bytes) */
    WORD    depth;         /*  "   "  depth (pixels) */
    WORD    left_indent;
    WORD    top_indent;
    WORD    black_width;
    WORD    black_depth;
    LONG    xorigin;       /*  (1/16th pixel) */
    LONG    yorigin;       /*  (1/16th pixel) */
    WORD    x0,y0,x1,y1;
    LONG    escapement;    /* character body width (design units) */

    UBYTE   *bmp;	   /* pointer to bm */
    UBYTE   bm[4];         /* character bit map */
} IFBITMAP;

typedef struct
{
    struct MinNode mn;     /* doubly linked list pointers                  */
    UWORD   notmru;        /* set to 0 if recently made most recently used */
    struct _FONT *iffont;     /* Font handle of the owner of this IFBITMAP    */
    WORD    index;         /* index to IFBITMAP in above FONT's table      */

    WORD    size;          /* size of header w/out metrics */
    WORD    depth;         /*  always 1                */
    WORD    left;
    WORD    top;
    WORD    right;
    WORD    bottom;
    WORD    xscale;
    WORD    yscale;
    WORD    escapement;    /* character body width (design units)  */

    UBYTE   bm[4];         /* character bit map                     */
} IFOUTLINE;

typedef struct {
    UWORD num_segmts;
    UWORD num_coords;
    UWORD polarity;
    UWORD segmt_offset;
    UWORD coord_offset;
} OUTLINE_LOOP;

typedef struct {
  UWORD size;
  UWORD num_loops;
  OUTLINE_LOOP loop[4];
} OUTLINE_CHAR;

/*----------------------------------*/
/*    Kerning Pair */
/*----------------------------------*/

typedef struct
{
    UWORD chId0, chId1;
    WORD  adj;
}  KERN_PAIR;

EXTERN  UWORD  ENTRY  CGIFinit();
EXTERN  UWORD  ENTRY  CGIFconfig();
EXTERN  UWORD  ENTRY  CGIFfund();
EXTERN  UWORD  ENTRY  CGIFenter();
EXTERN  UWORD  ENTRY  CGIFexit();
EXTERN  UWORD  ENTRY  CGIFfont();
EXTERN  UWORD  ENTRY  CGIFwidth();
EXTERN  UWORD  ENTRY  CGIFkern();
EXTERN  UWORD  ENTRY  CGIFmove_block();

#if CACHE
EXTERN  UWORD  ENTRY  CGIFchar();
#else
EXTERN  UWORD  ENTRY  CGIFchar_size();
EXTERN  UWORD  ENTRY  MAKbitMap();
#endif

#if PCLEO
EXTERN UWORD   ENTRY  CGIFpcleo();
#endif

#if SEGACCESS
EXTERN UWORD   ENTRY  CGIFsegments();
#endif

