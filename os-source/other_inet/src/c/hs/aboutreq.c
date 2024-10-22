/* -----------------------------------------------------------------------
 * aboutreq.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: aboutreq.c,v 1.1 91/05/09 16:03:47 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/aboutreq.c,v 1.1 91/05/09 16:03:47 bj Exp $
 *
 * $Log:	aboutreq.c,v $
 * Revision 1.1  91/05/09  16:03:47  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "termall.h"

/*
* About Requester definition
*/

static unsigned short int fqgadgborder[] =
  {
    0,0, 80,0, 80,10, 0,10, 0,0
  };

static struct Border fqborder = 
  {
    -1,-1,
    0,1,JAM2,
    5,
    fqgadgborder,
    NULL
  };

static struct IntuiText canceltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    1,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " CONTINUE",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct Gadget cancelgadget =
  {
    NULL,             /* Pointer to next gadget   */
    -92,-12,          /* LeftEdge, TopEdge        */
    80,10,            /* Width, Height of hit box */
    GADGHCOMP  |      /* Flags                    */
    GRELBOTTOM |
    GRELRIGHT,
    RELVERIFY,        /* Activation flags         */
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &fqborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    &canceltext,      /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

  static struct IntuiText abouttext12 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    155,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "as \"ehaberfellner\". Usenet ...utgpu!mnetor!becker!haberfellner!eric",
    NULL            /* No next string           */
  };

  static struct IntuiText abouttext11 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    145,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "The author can be reached at (416) 604-2025 after 6 PM EST or on BIX",
    &abouttext12   /* No next string           */
  };

  static struct IntuiText abouttext10 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    125,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Praise or Problems should also be sent to the above address.",
    &abouttext11    /* No next string           */
  };

  static struct IntuiText abouttext9 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    115,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "This will encourage the author to improve and enhance this product.",
    &abouttext10    /* No next string           */
  };

  static struct IntuiText abouttext8 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    95,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "M6P 3C7",
    &abouttext9     /* No next string           */
  };

  static struct IntuiText abouttext7 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    85,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CANADA",
    &abouttext8    /* No next string           */
  };

  static struct IntuiText abouttext6 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    75,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Toronto, Ontario,",
    &abouttext7    /* No next string           */
  };

  static struct IntuiText abouttext5 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    65,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "57 Glenwood Avenue,",
    &abouttext6    /* No next string           */
  };

  static struct IntuiText abouttext4 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    55,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Eric Haberfellner",
    &abouttext5    /* No next string           */
  };

  static struct IntuiText abouttext3 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    35,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "it send a contribution ( $25 suggested ) to:",
    &abouttext4    /* No next string           */
  };

  static struct IntuiText abouttext2 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    25,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "This program is being distributed as SHAREWARE. If you use it and like",
    &abouttext3    /* No next string           */
  };

  static struct IntuiText abouttext1 =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    5,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "HandShake was written by Eric Haberfellner, mostly late at night.",
    &abouttext2    /* No next string           */
  };

static unsigned short int freqvect [] =
  {
      0,  0,
    595,  0,
    595, 176,
      0, 176,
      0,  0
  };

static struct Border freqborder =
  {
    2,2,
    0,1,JAM2,
    5,
    freqvect,
    NULL
  };

static struct Requester aboutrequester;

/***
*
* Requester handling functions
*
***/

void ShowAboutRequester ()
  {
    InitRequester ( &aboutrequester );
    aboutrequester.LeftEdge  = 20;
    aboutrequester.TopEdge   = 20;
    aboutrequester.Width     = 600;
    aboutrequester.Height    = 180;
    aboutrequester.ReqGadget = &cancelgadget;
    aboutrequester.ReqBorder = &freqborder;
    aboutrequester.ReqText   = &abouttext1;
    aboutrequester.BackFill  = 1;
    Request ( &aboutrequester, Window );
  }

int WaitForAbout ()
  {
    int done;
    /*
    Wait ( 1L << Window->UserPort->mp_SigBit );
    */
    ProcessUntil ( IDCMP_EVENT );
    for ( done = 0; !done; )
      {
        while ( message = (struct IntuiMessage *) GetMsg ( Window->UserPort ) )
          {
            class   = message->Class;
            address = message->IAddress;
            ReplyMsg ( (struct Message *) message );
            if ( class == GADGETUP && address == (APTR) &cancelgadget )
                done = 1;
          }
      }
    return ( done );
  }

void RemoveAboutRequester ()
  {
    EndRequest ( &aboutrequester, Window );
  }
