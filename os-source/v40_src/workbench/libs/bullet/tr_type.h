/* tr_type.h */

/* 46 BYTEs */
typedef struct
{

  /* the current transform */

    WORD  num,den;
    WORD  old0;
    WORD  new0;     /* adjusted coordinate values */
    WORD  half_den;

/*-------- above is locked for assembly language access */

    WORD  old1;
    WORD  new1;

    UBYTE *num_skel_loop;   /* Number of skeletal points in next loop */
    adjusted_skel_type *skel;    /*  ptr to original & adjusted values
                                  *  of next segment end
                                  */
    UWORD *skel_to_contr;   /*  skel indices to contr             */

    WORD  nsk;  /* number of skels in current loop */

  /* Current character */

        WORD  margin;


    WORD end;   /* end vector number in scaling  segment */

    WORD sct;   /* number of skels processed in current loop  */


    /* Values at the start of a loop. These values will also */
    /* be used at the end of the loop.                       */

        WORD  num0,den0;
        WORD  half_den0;
        WORD  end0;
        WORD  old00;
        WORD  new00;
} TRAN;
