
/**************************************************************************
 *
 *      PropDemo.c
 *
 *      Copyright 1989, Commodore-Amiga, Inc.
 *
 *      Revision History:
 *
 *      07-Jul-89               Now accepts command line arguments
 *      01-Jun-89               Created this file.  Peter Cherna
 *
 **************************************************************************/


/*------------------------------------------------------------------------*/

#include <exec/types.h>
#include <intuition/intuition.h>

/* Prototypes for functions defined in propgad.c */
VOID FindPropValues(USHORT total,
                    USHORT visible,
                    USHORT first,
                    USHORT *body,
                    USHORT *pot);
USHORT FindPropFirst(USHORT total,
                             USHORT visible,
                             USHORT pot);


/*------------------------------------------------------------------------*/

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/*------------------------------------------------------------------------*/

/*------------------------------------------------------------------------*/

/*/ FindPropValues()
 *
 * Function to calculate the Body and Pot values of a proportional gadget
 * given the three values total, visible, and first, representing the
 * total number of items in a list, the number of items visible at one
 * time, and the first item to be displayed.  For example, a file requester
 * may be able to display 10 entries at a time.  The directory has 20
 * entries in it, and the first one visible is number 3 (the fourth one,
 * counting from zero), then total = 20, visible = 10, and first = 3.
 *
 */

VOID FindPropValues(total,visible,first,body,pot)

    USHORT total, visible, first;
    USHORT *body, *pot;
    {
    /*  If we're too far down, reduce first: */

    if (first > (total - visible))
       first = MAX(total - visible,0);

    /*  body is the relative size of the proportional gadget's body.
        Its size in the container represents the fraction of the total
        that is in view.  If there are zero or one lines total, body
        should be full-size (0xFFFF).  Otherwise, body should be the
        fraction of (the number of displayed lines - 1) over
        (the total number of lines - 1).  The "-1" is so that when the
        user scrolls by clicking in the container of the scroll gadget,
        then there is a one line overlap between the two views. */

    if (total <= 1)
        (*body) = 0xFFFF;
    else
        (*body) = (USHORT) (((MIN(visible-1, total-1) << 16L) - 1) /
            (total-1));

    /*  pot is the position of the proportional gadget body, with zero
        meaning that the scroll gadget is all the way up (or left),
        and full (0xFFFF) meaning that the scroll gadget is all the way
        down (or right).  If we can see all the lines, pot should be zero.
        Otherwise, pot is the number of lines first at the beginning
        divided by the number of unseen lines (total - visible). */

    if (visible >= total)
        (*pot) = 0;
    else
        (*pot) = (USHORT) MIN(0xFFFF, (first << 16L) / (total - visible));
    }


/*------------------------------------------------------------------------*/

/*/ FindPropFirst()
 *
 * Function to calculate the first line that is visible in a proportional
 * gadget, given the total number of items in the list and the number
 * visible, as well as the HorizPot or VertPot value.
 *
 */

USHORT FindPropFirst(total,visible,pot)

    USHORT total;
    USHORT visible;
    USHORT pot;

    {
    USHORT first;

    first = ((total - MIN(total, visible)) * ((LONG) pot) +
        (1L << 15)) >> 16;

    /*  Once you get back the new value of first, only redraw your list
        if first changed from its previous value.  The proportional gadget
        may not have moved far enough to change the value of first. */

    return(first);
    }

/*------------------------------------------------------------------------*/
