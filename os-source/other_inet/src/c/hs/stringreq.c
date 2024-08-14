/* -----------------------------------------------------------------------
 * stringreq.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: stringreq.c,v 1.1 91/05/09 15:24:17 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/stringreq.c,v 1.1 91/05/09 15:24:17 bj Exp $
 *
 * $Log:	stringreq.c,v $
 * Revision 1.1  91/05/09  15:24:17  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "termall.h"


/*
* Requester definition
*/

/*
* Filename input Requester
*/
static unsigned char undobuffer [256];
static unsigned char filebuffer [256];

static unsigned short int fqgadgborder[] =
  {
    0,0, 70,0, 70,10, 0,10, 0,0
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
    " CANCEL",      /* Itext                    */
    NULL            /* No next string           */
  };

static struct Gadget cancelgadget =
  {
    NULL,             /* Pointer to next gadget   */
    -80,-12,          /* LeftEdge, TopEdge        */
    70,10,            /* Width, Height of hit box */
    GADGHCOMP  |      /* Flags                    */
    GRELBOTTOM |
    GRELRIGHT,
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &fqborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    &canceltext,    /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static struct IntuiText OKtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    1,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "   OK",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct Gadget OKgadget =
  {
    &cancelgadget,    /* Pointer to next gadget   */
    10,-12,            /* LeftEdge, TopEdge        */
    70,10,            /* Width, Height of hit box */
    GADGHCOMP |       /* Flags                    */
    GRELBOTTOM,
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &fqborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    &OKtext,          /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static struct StringInfo fileinfo =
  {
    filebuffer,  /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    FNAMESIZE,   /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  };

static unsigned short int fgadgborder[] =
  {
    0,0, 285,0, 285,22, 0,22, 0,0
  };

static struct Border fborder = 
  {
    -3,-12,
    0,1,JAM2,
    5,
    fgadgborder,
    NULL
  };

static struct IntuiText filetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    0,              /* LeftEdge                 */
    -10,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "File Name",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct Gadget filegadget =
  {
    &OKgadget,        /* Pointer to next gadget   */
    10,20,            /* LeftEdge, TopEdge        */
    280,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY     |   /* Activation flags         */
    ENDGADGET     |
    ALTKEYMAP,
    STRGADGET     |   /* Type                     */
    REQGADGET,
    (APTR) &fborder,  /* Pointer to string border */
    NULL,             /* SelectRender             */
    &filetext,        /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &fileinfo, /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static unsigned short int freqvect [] =
  {
      0,  0,
    295,  0,
    295, 46,
      0, 46,
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

static struct Requester FileRequester;

/***
*
* Requester handling functions
*
***/

int GetStringFromUser ( title, str, length )
unsigned char   *title,
                *str;
unsigned        length;
  {
    int status = 0;
    unsigned short int done;

    fileinfo.MaxChars = length;
    fileinfo.BufferPos= strlen ( str );
    filetext.IText    = title;
    InitRequester ( &FileRequester );
    FileRequester.LeftEdge  = 20;
    FileRequester.TopEdge   = 20;
    FileRequester.Width     = ( strlen ( title ) < 32 ) ? 300 : strlen ( title ) * 8 + 40;
    FileRequester.Height    = 50;
    FileRequester.ReqGadget = &filegadget;
    FileRequester.ReqBorder = &freqborder;
    FileRequester.BackFill  = 1;
    fileinfo.AltKeyMap      = &OrigKeyMap;
    fgadgborder[2] = fgadgborder[4] = FileRequester.Width - 15;
    freqvect[2]    = freqvect[4]    = FileRequester.Width -  5;
    memcpy ( fileinfo.Buffer, str, length );
    fileinfo.Buffer[length] = NULL;
    Request ( &FileRequester, Window );
    Delay ( 10 );
    ActivateGadget ( &filegadget, Window, &FileRequester );

    for ( done = 0; !done; )
      {
        /*
        Wait ( 1L << Window->UserPort->mp_SigBit );
        */
        ProcessUntil ( IDCMP_EVENT );
        while ( message = (struct IntuiMessage *) GetMsg ( Window->UserPort ) )
          {
            class   = message->Class;
            address = message->IAddress;
            ReplyMsg ( (struct Message *) message );
            if ( class == GADGETUP )
              {
                if ( address == (APTR) &OKgadget  ||
                     address == (APTR) &filegadget )
                  {
                    strcpy ( str, fileinfo.Buffer );
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