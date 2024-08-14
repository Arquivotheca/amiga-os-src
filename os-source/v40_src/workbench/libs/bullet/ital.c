/* ital.c */
/* Copyright (C) Agfa Compugraphic, 1989. All rights reserved. */

#include "debug.h"
#include "port.h"
#include "if_type.h"
#include "adj_skel.h"
#include "segments.h"

EXTERN VOID  first_loop();     /*  if_init.c  */
EXTERN LOOP* next_loop();

EXTERN  FACE                face;
EXTERN  IF_CHAR             c;
EXTERN  adjusted_skel_type  x_skel[];
EXTERN  adjusted_skel_type  y_skel[];

GLOBAL VOID
italic()
{
    adjusted_skel_type *xskel, *yskel;

    UBYTE *x_num_skel_loop;
    UBYTE *y_num_skel_loop;

    UWORD *x_skel_to_contr;
    UWORD *y_skel_to_contr;

    UWORD  loopct;
    LOOP  *lp;

    UWORD  x_vecnum;
    UWORD  y_vecnum;

    WORD  x_nsk;      /*  Number of skeletal points in loop */
    WORD  y_nsk;
    WORD  xnum;       /*  Number of skel pts processed so far */
    WORD  ynum;

    WORD  yold,  ynew;
    WORD  yold0, yold1, yold_first;
    WORD  ynew0, ynew1, ynew_first;
    WORD  num, den;
    WORD  xadj;


    DBG("italic()\n");

    xskel = x_skel;
    yskel = y_skel;

    x_num_skel_loop = c.xskel.num_skel_loop;
    y_num_skel_loop = c.yskel.num_skel_loop;

    x_skel_to_contr = c.xskel.skel_to_contr;
    y_skel_to_contr = c.yskel.skel_to_contr;

    x_vecnum = *x_skel_to_contr++;

    /*  For each loop in the character */
    first_loop();
    for(loopct = 0; loopct < c.num_loops; loopct++)
    {
        lp = next_loop();

        x_nsk = *(x_num_skel_loop++);
        y_nsk = *(y_num_skel_loop++);
        DBG3("loop %ld has %ld xskels and %ld yskels\n", loopct, x_nsk, y_nsk);

        xnum  = 0;
        
        yold_first = yskel->original;
        ynew_first = yskel->adjusted;

        yold1 = (yskel + y_nsk - 1)->original;
        ynew1 = (yskel + y_nsk - 1)->adjusted;

        DBG1("\n    0th yskel pair has start index y_vecnum = %ld\n",
                                           *(y_skel_to_contr + y_nsk - 1));

      /*  For each y skel scaling segment in this loop  */

        for(ynum=0; ynum<=y_nsk; ynum++)
        {
          /*  Next pair of y skeletal points along loop  */

            yold0 = yold1;
            ynew0 = ynew1;

          /*  If at end of loop, the last y skeletal is the first  */

            if(ynum == y_nsk)
            {
                y_vecnum = lp->npnt;  /* To stop x at the end of the loop */
                yold1 = yold_first;
                ynew1 = ynew_first;
            }
            else
            {
                y_vecnum = *y_skel_to_contr++;
                yold1 = yskel->original;
                ynew1 = yskel->adjusted;
                yskel++;
            }
            DBG2("\n    %ldth yskel pair has ending index y_vecnum = %ld\n",
                                                            ynum, y_vecnum);
            DBG2("    yold0, yold1 %ld %ld\n", yold0, yold1);
            DBG2("    ynew0, ynew1 %ld %ld\n", ynew0, ynew1);

            num = ynew1 - ynew0;
            den = yold1 - yold0;
            DBG2("    num, den%ld %ld\n", num, den);

            while((x_vecnum < y_vecnum) && (xnum<x_nsk))
            {
                DBG2("\n        %ldth xskel has index x_vecnum = %ld\n",
                                                         xnum, x_vecnum);

                yold = *(lp->y + x_vecnum) & 0x3fff;

              /*  Scale y coordinate of this x skeletal */
                if(den)
                    ynew = (WORD)(((LONG)(yold - yold0)*(LONG)num)
                                                           /den) + ynew0;
                else
                    ynew = yold - yold0 + ynew0;
                DBG2("        yold, ynew   %ld %ld\n", yold, ynew);

              /*  Adjust x coordinate based on angle */
                if(face.glob_ital_ang && !c.italic_flag)
                {
                    DBG1("        Adjust by global %ld\n", face.glob_ital_ang);
                    xadj = (WORD)
                         (
                           (
                               ( (LONG)face.glob_ital_ang * (LONG)ynew ) -
                                 ( (LONG)yold * (LONG)c.loc_ital_ang )
                           ) >> 15
                         );
                }
                else
                {
                    DBG1("        Adjust by local %ld\n", c.loc_ital_ang);
                    xadj = (WORD)
                         (
                           ((LONG)c.loc_ital_ang * (LONG)(ynew - yold)) >> 15
                         );
                }
                DBG1("        xadj = %ld\n", xadj);
                xskel->intermed += xadj;
                xskel->adjusted = xskel->intermed;

              /*  Bump to next x skeletal */

                xnum++;
                x_vecnum = *x_skel_to_contr++;
                xskel++;
            }
        }
    }
}

