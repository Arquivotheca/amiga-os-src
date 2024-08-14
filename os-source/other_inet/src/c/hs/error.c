/* -----------------------------------------------------------------------
 * error.c 		handshake_src
 *
 * $Locker:  $
 *
 * $Id: error.c,v 1.1 91/05/09 16:16:53 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/error.c,v 1.1 91/05/09 16:16:53 bj Exp $
 *
 * $Log:	error.c,v $
 * Revision 1.1  91/05/09  16:16:53  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "termall.h"

/*
* Error Requester
*/

static unsigned short int cgadgborder[] =
  {
    0,0, 90,0, 90,10, 0,10, 0,0
  };

static struct Border cborder = 
  {
    -1,-1,
    0,1,JAM2,
    5,
    cgadgborder,
    NULL
  };

static struct IntuiText continuetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    4,              /* LeftEdge                 */
    4,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " CONTINUE",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct IntuiText errortext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    10,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "",             /* Itext                    */
    NULL            /* No next string           */
  };

static struct Requester errorrequester;

/***
*
* Requester handling functions
*
***/

int Error ( str )
unsigned char *str;
  {
    unsigned long length;
    
    errortext.IText = str;
    length = ( ( strlen( str ) * 8 ) > 300 ) ? (long)strlen ( str ) * 8 : 300L;
    AutoRequest ( Window, &errortext, &continuetext, &continuetext,
                  0L, 0L, length, 60L );
    return ( 0 );
  }
