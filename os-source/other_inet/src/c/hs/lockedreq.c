/* -----------------------------------------------------------------------
 * lockedreq.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: lockedreq.c,v 1.1 91/05/09 16:23:54 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/lockedreq.c,v 1.1 91/05/09 16:23:54 bj Exp $
 *
 * $Log:	lockedreq.c,v $
 * Revision 1.1  91/05/09  16:23:54  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

#include    "termall.h"

/*
* Requester definition
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
    1,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " Continue",    /* Itext                    */
    NULL            /* No next string           */
  };

static struct IntuiText resetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    1,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " Restart",     /* Itext                    */
    NULL            /* No next string           */
  };

static struct IntuiText lockedtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    10,             /* LeftEdge                 */
    10,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Timeout sending character.",/* Itext        */
    NULL            /* No next string           */
  };

static struct Gadget resetgadget =
  {
    NULL,             /* Pointer to next gadget   */
    10,-12,           /* LeftEdge, TopEdge        */
    90,10,            /* Width, Height of hit box */
    GADGHCOMP  |      /* Flags                    */
    GRELBOTTOM,
    RELVERIFY  |      /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &cborder,  /* Pointer to string border */
    NULL,             /* SelectRender             */
    &resetext,       /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static struct Gadget continuegadget =
  {
    &resetgadget,     /* Pointer to next gadget   */
    -100,-12,         /* LeftEdge, TopEdge        */
    90,10,            /* Width, Height of hit box */
    GADGHCOMP  |      /* Flags                    */
    GRELBOTTOM |
    GRELRIGHT,
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &cborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    &continuetext,    /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static unsigned short int lkreqvect [] =
  {
      0,  0,
    295,  0,
    295, 46,
      0, 46,
      0,  0
  };

static struct Border lkreqborder =
  {
    2,2,
    0,1,JAM2,
    5,
    lkreqvect,
    NULL
  };

static struct Requester lockedrequester;

/***
*
* Requester handling functions
*
***/

void HandleLockedLine ( ser_req )
struct IOExtSer *ser_req;
  {
    unsigned short int done;
    
    InitRequester ( &lockedrequester );
    lockedrequester.LeftEdge  = 20;
    lockedrequester.TopEdge   = 20;
    lockedrequester.Width     = 300;
    lockedrequester.Height    = 50;
    lockedrequester.ReqGadget = &continuegadget;
    lockedrequester.ReqBorder = &lkreqborder;
    lockedrequester.ReqText   = &lockedtext;
    lockedrequester.BackFill  = 1;

    Request ( &lockedrequester, Window );

    for ( done = 0; !done; )
      {
        ProcessUntil ( IDCMP_EVENT );
        while ( message = (struct IntuiMessage *) GetMsg ( Window->UserPort ) )
          {
            class   = message->Class;
            address = message->IAddress;
            ReplyMsg ( (struct Message *) message );
            if ( class == GADGETUP && address == (APTR) &continuegadget )
              {
                done = 1;
              }
            else if ( class == GADGETUP && address == (APTR) &resetgadget )
              {
                CloseDevice ( (struct IORequest *) &ser_req   );
                ser_port = InitSerialIO ( (struct IOExtSer *) &ser_req,
                                           NULL );
                ser_out_req.IOSer.io_Message.mn_ReplyPort = ser_out_port;
                done = 1;
              }
          }
      }
  }
