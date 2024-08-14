/* -----------------------------------------------------------------------
 * titlemenu.h      handshake_src
 *
 * $Locker:  $
 *
 * $Id: titlemenu.h,v 1.1 91/05/09 14:33:31 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/titlemenu.h,v 1.1 91/05/09 14:33:31 bj Exp $
 *
 * $Log:	titlemenu.h,v $
 * Revision 1.1  91/05/09  14:33:31  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Title Line Menus
*
***/
static struct Menu arexx =
  {
    NULL,           /* No further Menus         */
    445,0,90,0,     /* LE, TE, Width, Height    */
    MENUENABLED,    /* FLags                    */
    "AREXX",        /* MenuName                 */
    &arexxitem      /* First MenuItem           */
  };

static struct Menu transfer =
  {
    &arexx,         /* No further Menus         */
    265,0,90,0,     /* LE, TE, Width, Height    */
    MENUENABLED,    /* FLags                    */
    "TRANSFER",     /* MenuName                 */
    &txcritem       /* First MenuItem           */
  };

static struct Menu phone =
  {
    &transfer,      /* No further Menus         */
    355,0,90,0,     /* LE, TE, Width, Height    */
    MENUENABLED,    /* FLags                    */
    "PHONE",        /* MenuName                 */
    &dialitem       /* First MenuItem           */
  };

static struct Menu terminal =
  {
    &phone,         /* No further Menus         */
    165,0,100,0,    /* LE, TE, Width, Height    */
    MENUENABLED,    /* FLags                    */
    "TERMINAL",     /* MenuName                 */
    &modeitem       /* First MenuItem           */
  };

static struct Menu setup =
  {
    &terminal,      /* No further Menus         */
    90,0,75,0,      /* LE, TE, Width, Height    */
    MENUENABLED,    /* FLags                    */
    "SETUP",        /* MenuName                 */
    &bauditem       /* First MenuItem           */
  };

static struct Menu project =
  {
    &setup,         /* No further Menus         */
    0,0,90,0,       /* LE, TE, Width, Height    */
    MENUENABLED,    /* FLags                    */
    "PROJECT",      /* MenuName                 */
    &quititem       /* First MenuItem           */
  };

