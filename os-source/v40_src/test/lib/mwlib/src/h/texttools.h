/****** mwhead/TextTools **********************************************************

    Mitchell/Ware Systems, Inc      Version 1.00            19 Oct 1990

    Structs for TextTools - ITLIST

    ITLIST allows the user to define an array to create an IntuiText
    chain. All draw modes default to JAM1.

    ITLIST Array must be terminated by a NULL aggarate entry.

****************************************************************************/

typedef struct ITList  {
    short   left, top, pen;
    UBYTE   *string;         /* Must contain a string - NULL termination
                                   checkpoint */
    /* New additions as of 8-26-90 - Font handling
    */
    struct TextAttr ta;         /* Text Attr structure (optional) or NULL if not used */

    struct IntuiText *itext;    /* (set by Tools) */
    struct TextFont *tf;        /* Font used (set by tools) - MUST be inited to ZERO */
    struct ITList *fchain;      /* Font chain - next ITList in list to use this font- or NULL */

    long reserved[4];           /* Set to NULL for now */
    } ITLIST, ITList;

