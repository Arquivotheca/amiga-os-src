/****** mwhead/WindowTools **********************************************************************

    Mitchell/Ware Systems, Inc          Version 1.00            19 Oct 1990

        Structs for WindowTools - BTP

        Brush-to-pointer structure created by the BTP program.

****************************************************************************/

typedef struct  btp {
    USHORT *pointer;        /* pointer to sprite data (must be in CHIP ram) */
    short  width, height;   /* width & height of pointer (width <= 16) */
    short  xoff, yoff;      /* offset (negative location of select point) */
    } BTP;


