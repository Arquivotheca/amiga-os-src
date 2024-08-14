/* -----------------------------------------------------------------------
 * arexxmenu.h      handshake_src
 *
 * $Locker:  $
 *
 * $Id: arexxmenu.h,v 1.1 91/05/09 14:32:07 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/arexxmenu.h,v 1.1 91/05/09 14:32:07 bj Exp $
 *
 * $Log:	arexxmenu.h,v $
 * Revision 1.1  91/05/09  14:32:07  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Arexx menu items.
*
***/

static struct IntuiText aborttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Abort Macro",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem abortitem =
  {
    NULL,           /* Next menu item           */
    0,20,           /* LE, TE                   */
    110+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &aborttext, /* ItemFill              */
    NULL,           /* SelectFill               */
    'z',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText visitortext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "DOS Window",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem visitoritem =
  {
    &abortitem,     /* Next menu item           */
    0,10,           /* LE, TE                   */
    110+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &visitortext, /* ItemFill            */
    NULL,           /* SelectFill               */
    'w',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText arexxtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Run Macro",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem arexxitem =
  {
    &visitoritem,   /* Next menu item           */
    0,0,            /* LE, TE                   */
    110+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &arexxtext, /* ItemFill              */
    NULL,           /* SelectFill               */
    'x',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

