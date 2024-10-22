/* -----------------------------------------------------------------------
 * tabreq.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: tabreq.c,v 1.1 91/05/09 16:27:41 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/tabreq.c,v 1.1 91/05/09 16:27:41 bj Exp $
 *
 * $Log:	tabreq.c,v $
 * Revision 1.1  91/05/09  16:27:41  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "termall.h"

/*
* Tab Requester definition
*/

#define TABLINESIZE 132

static unsigned char undobuffer [TABLINESIZE+1];
static unsigned char tabbuffer  [TABLINESIZE+1];

static unsigned short int tqgadgborder[] =
  {
    0,0, 70,0, 70,10, 0,10, 0,0
  };

static struct Border tqborder = 
  {
    -1,-1,
    0,1,JAM2,
    5,
    tqgadgborder,
    NULL
  };

static struct IntuiText canceltext =
  {
    0,1,              /* FrontPen, BackPen        */
    JAM2,             /* DrawMode                 */
    1,                /* LeftEdge                 */
    1,                /* TopEdge                  */
    NULL,             /* ITextFont                */
    " CANCEL",        /* Itext                    */
    NULL              /* No next string           */
  };

static struct Gadget cancelgadget =
  {
    NULL,             /* Pointer to next gadget   */
    -80,-12,          /* LeftEdge, TopEdge        */
    70,10,            /* Width, Height of hit box */
    GADGHCOMP  |      /* Flags                    */
    GRELBOTTOM |
    GRELRIGHT,
    RELVERIFY  |      /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &tqborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    &canceltext,      /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static struct IntuiText OKtext =
  {
    0,1,              /* FrontPen, BackPen        */
    JAM2,             /* DrawMode                 */
    1,                /* LeftEdge                 */
    1,                /* TopEdge                  */
    NULL,             /* ITextFont                */
    "   OK",          /* Itext                    */
    NULL              /* No next string           */
  };

static struct Gadget OKgadget =
  {
    &cancelgadget,    /* Pointer to next gadget   */
    10,-12,           /* LeftEdge, TopEdge        */
    70,10,            /* Width, Height of hit box */
    GADGHCOMP |       /* Flags                    */
    GRELBOTTOM,
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &tqborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    &OKtext,          /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static struct StringInfo tabinfo =
  {
    tabbuffer,   /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    132,         /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  };

static unsigned short int tgadgborder[] =
  {
    0,0, 656,0, 656,22, 0,22, 0,0
  };

static struct Border tborder = 
  {
    0, 0,
    0,1,JAM2,
    0,
    tgadgborder,
    NULL
  };

static struct IntuiText tabtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    -10,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Place a \"T\" in Tab positions", /* Itext    */
    NULL            /* No next string           */
  };

static struct Gadget tabgadget =
  {
    &OKgadget,  /* Pointer to next gadget   */
    0,20,             /* LeftEdge, TopEdge        */
    SCR_WIDTH,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &tborder,  /* Pointer to string border */
    NULL,             /* SelectRender             */
    &tabtext,         /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &tabinfo,  /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static unsigned short int treqvect [] =
  {
      0,  0,
    654,  0,
    654, 46,
      0, 46,
      0,  0
  };

static struct Border treqborder =
  {
    2,2,
    0,1,JAM2,
    5,
    treqvect,
    NULL
  };

static struct Requester tabrequester;

/***
*
* Requester handling functions
*
***/

int TabRequest ( str )
unsigned char *str;
  {
    int status = 0;
    unsigned short int done;

    InitRequester ( &tabrequester );
    tabrequester.LeftEdge  = 0;
    tabrequester.TopEdge   = 20;
    tabrequester.Width     = SCR_WIDTH;
    tabrequester.Height    = 50;
    tabrequester.ReqGadget = &tabgadget;
    tabrequester.ReqBorder = &treqborder;
    tabrequester.BackFill  = 1;
    tabinfo.AltKeyMap      = &OrigKeyMap;
    str[131] = 0;
    strcpy ( tabinfo.Buffer, str );
    Request ( &tabrequester, Window );

    for ( done = 0; !done; )
      {
        ProcessUntil ( IDCMP_EVENT );
        while ( message = (struct IntuiMessage *) GetMsg ( Window->UserPort ) )
          {
            class   = message->Class;
            address = message->IAddress;
            ReplyMsg ( (struct Message *) message );
            if ( class == GADGETUP )
              {
                if ( address == (APTR) &OKgadget )
                  {
                    strcpy ( str, tabinfo.Buffer );
                    status = 1;
                    done = 1;
                  }
                else if ( address == (APTR) &cancelgadget )
                  {
                    status = 0;
                    done = 1;
                  }
              }
          }
      }
    return ( status );
  }

void TabSet ()
  {
    int     i,
            j;
    
    for ( i = strlen ( nvmodes.tabvec ); i < 132; i++ )
        nvmodes.tabvec[i] = ' ';
    for ( i = 0; i < 132; i++ )
        vcb.tabs[i] = 132;
    for ( j = 0, i = 1; i < 132; i++ )
        if ( nvmodes.tabvec[i] != ' ' )
            while ( j < i )
                vcb.tabs[j++] = i + 1;
  }

