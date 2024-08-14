/* if_type.h */
/* History
 *    05-May-90 awr  moved char_buf into IF_CHAR structure
 *    15-Jun-90 awr  added max_des field to COORD_DATA
 *    28-Jun-90 bjg  added OUTLINE_PARAMS structure definition. 
 *    27-Jul-90 awr  corrected structure sizes in comments
 *    29_Nov-90 tbh  Changed COORD_DATA structure by adding field 
 *                   "st_variation[4]"
 *    15-Jan-91 jfd  Changed field "baseline_offset" in structure IF_CHAR
 *                   from UWORD to WORD.
 */


#define  MAX_DESIGN  0x3fff    /* 16K - 1, max design unit coordinate */
#define  MAX_LOOPS    64
#define  MAX_YCLASS  256
#define  MAX_YLINES  256
#define  MAX_SKEL    96
#define ROOT            (0xff)  /*  init value: assoc's parent spec   */
#define PROCESSED       1       /*  status of skel pnt: processed     */
#define NOT_PROC        0       /*  status of skel pnt: not processed */
#define NOT_ALIGNED     -1      /*  alignment status for skel points  */

#define R_TYPES         1       /*  1 turns on "R" type functionality */
#define R_TWO_I         2       /*  "R" type index of two             */


typedef struct
{
    WORD x;
    WORD y;
}
XY;

typedef struct
{
    XY ll;
    XY ur;
}
BOX;

typedef struct
{
    BOX bound_box;
    BOX escape_box;
    WORD halfline;
    WORD centerline;
}
METRIC;


typedef struct
{
    WORD xyoff;
    BYTE spare;
    BYTE polarity;
    WORD child;
    WORD sibling;
}
CTREE;

/* 20 BYTEs */
typedef struct
{
    UWORD npnt;
    UWORD nauxpt;
    WORD  *x;
    WORD  *y;
    BYTE  *xa;
    BYTE  *ya;
}
LOOP;

/* 38 BYTEs */
typedef struct
{
    UBYTE  num_skel;        /* number of skels */
    UBYTE *num_skel_loop;   /*  num of skel per loop              */
    UWORD *skel_to_contr;   /*  skel indices to contr             */
    UBYTE  num_trees;       /* number of trees (roots) */
    UBYTE  num_nodes;       /* number of tree nodes */
    UBYTE *tree_skel_i;     /* skeletal indices */
    UBYTE *tree_attrib;     /* skeletal attributes */

    UBYTE  stan_dim_i;

/** I6.2.3   Interpolated Association data */

    UBYTE num_intrp;              /* number of pairs of interp skels */
    UBYTE *intrp1_skel_i;         /* 1st interp skel indices */
    UBYTE *intrp2_skel_i;         /* 2nd interp skel indices */
    UBYTE *num_intrp_assoc;       /* number of interps per pair */
    UBYTE  tot_num_intrp_assoc;   /* number of interp assocs */
    UBYTE *intrp_assoc_skel_i;    /* interp assoc indices */
}
SKEL;

/* 162 BYTEs */
typedef struct
{
    BYTE   *char_buf;      /* character data buffer */

    METRIC *metric;

    WORD   loc_ital_ang;
    UWORD  italic_flag;

    SKEL   xskel;
    SKEL   yskel;

    WORD  baseline_offset;

  /* y-classes */
/*** I6.2.1.7   Y-class Character (local) data */
    UBYTE   num_yclass_ass;    /* number of y class assignments */
    UBYTE  *num_root_p_ass;    /* number of roots per y class */
    UBYTE  *root_skel_i;     /* root indices in y class order */
    UBYTE  *yclass_i;      /* y class indices */

/** local y classes */

    UBYTE  num_yclass_def;     /* number of local y class definitions */
    UBYTE *yline_high_i;       /* high y line indices */
    UBYTE *yline_low_i;        /* low y line indices */

/*** local y class definitions */

    UWORD  num_ylines;     /* number of local y lines */
    UWORD *ylines;      /* local y line values */


    UWORD   num_loops;             /* number of loops */

    UWORD   contour_flag;      /* Bits in this word are processed to  */
                               /* the next 3 variables.               */
    UWORD   PreBrokenContrs;   /* Boolean flag for contour data with  */
                               /* local and global extreme points.    */
    UWORD   HqFormat;          /* Hi-Q data format: 1 = HQ1 (tangent) */
                               /* 2 = HQ2 (contact) 3 = HQ3 (compact) */
    UWORD   ConnectingChar;



    CTREE  *ctree;

    BYTE   *contour_data;          /* start of contour data */
    BYTE   *next_contour_loop;     /* next contour loop     */
    LOOP   lp;                     /* current contour loop  */

    WORD  alt_width;        /* if>0, alternate character width */
}
IF_CHAR;


/* 66 bytes */  /* 11-29-90 tbh */
typedef struct
{
    WORD    orig_pixel;     /*  Original imprecise pixel size       */
    WORD    p_pixel;        /*  precise pixel - shifted so that     */
                            /*    16K <= p_size < 32K               */
    WORD    p_half_pix;     /*  half precise pixel                  */
    WORD    bin_places;     /*  bits after the binary point above   */
    WORD    pixel_size;     /*  pixel dimension (power of 2)        */
    WORD    half_pixel;     /*  half pixel dimension (power of 2)   */
    WORD    grid_align;     /*  x & grid_align is truncated to grid */
    WORD    grid_shift;     /*  x >> grid_shift is grid number      */

    LONG    round[4];       /*  rounding value for 4 "R" types      */

    WORD    shut_off;       /*  value of "1-1/4" pixel shut-off     */
    WORD    con_size;       /*  min value for constrained dim       */

    WORD    margin;         /* add to work sp coord to shift to     */
                            /* bitmap coordinate                    */

    WORD    max_des;        /* grid aligned max design coordinate   */

  /* Standard Dimensions */

    WORD stand_value;       /* Standard dimension in design units   */
    WORD st_value[4];       /* These two arrays are results of      */
    WORD st_pixels[4];      /* pixel_align() using all 4 R-types    */
    WORD st_variation[4];   /* AMount stan dim rounds 11-29-90 tbh  */

}  COORD_DATA;


/*  Values for quadrant field below  */

#define ROT0    1
#define ROT90   2
#define ROT180  3
#define ROT270  4

/* 142 BYTEs */
typedef struct
{
    COORD_DATA   x;
    COORD_DATA   y;

    COORD_DATA  *px;
    COORD_DATA  *py;

    WORD     origBaseline; /* Design units                        */
    WORD     aBaselnVal;   /* grid aligned baseline value (DU)    */

    WORD     bmwidth;   /* width of bitmap in BYTEs               */
    WORD     bmdepth;   /* depth  "   "     " pixels              */
    BOX      tbound;    /* bounding box in working bitmap space   */
    UWORD    quadrant;  /* 90 rotations: ROT0 ROT90 ROT180 ROT270 */
} IF_DATA;

typedef struct 
{
   WORD current_part;
   WORD current_loop;
   WORD tot_loops;
   BOX  bound;
} OUTLINE_PARAMS;

