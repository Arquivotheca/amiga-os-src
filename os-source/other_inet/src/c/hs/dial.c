/* -----------------------------------------------------------------------
 * dial.c			handshake_src
 *
 * $Locker:  $
 *
 * $Id: dial.c,v 1.1 91/05/09 16:29:28 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/dial.c,v 1.1 91/05/09 16:29:28 bj Exp $
 *
 * $Log:	dial.c,v $
 * Revision 1.1  91/05/09  16:29:28  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Autodialing requester
*
***/

#include "termall.h"

#define NUMBER_OF_NUMBERS 20
#define NUMBER_SIZE 32

static unsigned char undobuffer [NUMBER_SIZE];
static unsigned short int       abort_dial;

static unsigned short int cancelgadgvect [] =
  {
    0,0, 99,0, 99,10, 0,10, 0,0
  };

static struct Border cancelborder = 
  {
    -1,-1,
    0,1,JAM2,
    5,
    cancelgadgvect,
    NULL
  };

static struct IntuiText canceltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    1,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " DON'T DIAL",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct Gadget cancelgadget =
  {
    NULL,             /* Pointer to next gadget   */
    10,-12,           /* LeftEdge, TopEdge        */
    99,10,            /* Width, Height of hit box */
    GADGHCOMP |       /* Flags                    */
    GRELBOTTOM,
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &cancelborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    &canceltext,      /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };

static unsigned short int phgadgvect[] =
  {
    0,0, 261,0, 261,12, 0,12, 0,0
  };

static struct Border phborder = 
  {
    -3,-2,
    0,1,JAM2,
    5,
    phgadgvect,
    NULL
  };

static unsigned short int dialgadgvect[] =
  {
    0,0, 9,0, 9,8, 0,8, 0,0
  };

static struct Border dialborder = 
  {
    0,0,
    0,1,JAM2,
    5,
    dialgadgvect,
    NULL
  };

static struct Gadget dialgadget[] =
{
  {
    NULL,             /* Pointer to next gadget   */
    8,10,             /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,25,             /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,40,             /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,55,             /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,70,             /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,85,             /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,100,            /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,115,            /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,130,            /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    8,145,            /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,    /* Pointer to next gadget   */
    308,10,           /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,25,           /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,40,           /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,55,           /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,70,           /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,85,            /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,100,           /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,115,          /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    308,130,          /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &cancelgadget,    /* Pointer to next gadget   */
    308,145,          /* LeftEdge, TopEdge        */
    10,8,             /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    RELVERIFY |       /* Activation flags         */
    ENDGADGET,
    BOOLGADGET |      /* Type                     */
    REQGADGET,
    (APTR) &dialborder,/* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    NULL,             /* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  }
};

static struct StringInfo phoneinfo[] =
{
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  },
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    NUMBER_SIZE, /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  }
};

static struct Gadget phnumbergadget[] =
{
  {
    &dialgadget[0],   /* Pointer to next gadget   */
    25,10,            /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[0],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[1],   /* Pointer to next gadget   */
    25,25,            /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[1],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[2],   /* Pointer to next gadget   */
    25,40,            /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[2],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[3],   /* Pointer to next gadget   */
    25,55,            /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[3],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[4],   /* Pointer to next gadget   */
    25,70,            /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[4],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[5],     /* Pointer to next gadget   */
    25,85,            /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[5],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[6],   /* Pointer to next gadget   */
    25,100,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[6],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[7],   /* Pointer to next gadget   */
    25,115,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[7],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[8],   /* Pointer to next gadget   */
    25,130,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[8],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[9],   /* Pointer to next gadget   */
    25,145,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[9],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[10],  /* Pointer to next gadget   */
    325,10,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[10],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[11],  /* Pointer to next gadget   */
    325,25,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[11],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[12],  /* Pointer to next gadget   */
    325,40,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[12],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[13],  /* Pointer to next gadget   */
    325,55,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[13],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[14],  /* Pointer to next gadget   */
    325,70,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[14],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[15],  /* Pointer to next gadget   */
    325,85,           /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[15],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[16],  /* Pointer to next gadget   */
    325,100,          /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[16],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[17],  /* Pointer to next gadget   */
    325,115,          /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[17],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[18],  /* Pointer to next gadget   */
    325,130,          /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[18],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    &dialgadget[19],  /* Pointer to next gadget   */
    325,145,          /* LeftEdge, TopEdge        */
    256,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &phborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &phoneinfo[19],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  }
};

static unsigned short int dreqvect [] =
  {
      0,  0,
    595,  0,
    595,176,
      0,176,
      0,  0
  };

static struct Border dreqborder =
  {
    2,2,
    0,1,JAM2,
    5,
    dreqvect,
    NULL
  };

static struct Requester dialrequester =
  {
    NULL,
    20,15,
    600,180,
    0,0,
    &phnumbergadget[0],
    &dreqborder,
    NULL,
    NULL,
    1,
    NULL,
    NULL,
    NULL
  };

/***
*
* Requester handling functions
*
***/

int DialRequest ()
  {
    short int status;
    short int i;
    unsigned short int done;
    unsigned char dialstring[NUMBER_SIZE];

    TWindow = NULL;
    WWindow = NULL;
    for ( i = 0; i < NUMBER_OF_NUMBERS - 1 ; i++ )
        dialgadget[i].NextGadget = &phnumbergadget[i+1];

    for ( i = 0; i < NUMBER_OF_NUMBERS; i++ )
      {
        phoneinfo[i].AltKeyMap = &OrigKeyMap;
        nvmodes.phonestr[i][ NUMBER_SIZE - 1] = 0x00;
        phoneinfo[i].Buffer    = nvmodes.phonestr[i];
      }

    Request ( &dialrequester, Window );

    status = 0;
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
                if ( address == (APTR) &cancelgadget )
                  {
                    status = 0;
                    done = 1;
                  }
                for ( i = 0; i < NUMBER_OF_NUMBERS; i++ )
                    if ( address == (APTR) &dialgadget[i] )
                      {
                        strcpy ( dialstring, phoneinfo[i].Buffer );
                        status = 1;
                        break;
                      }
                if ( status )
                    done = 1;
              }
          }
      }
    if ( status )
      Dial ( dialstring );
    return ( (int) status );
  }

void Dtr ( state )
unsigned char state;
  {
#define CIAB_PRA  (*(char *) 0xbfd001)
    Disable ();
    if ( state )
        CIAB_PRA &= ~0x80;       /* Turn off DTR */
    else
        CIAB_PRA |=  0x80;       /* Turn off DTR */
    Enable ();

  }

void HangUp ()
  {
    SetDialMessage ( "Hanging up" );
    Dtr ( 0 );
    Delay ( 75 );
    if ( !DialPuts ( "+++" ) )
      {
        abort_dial = 1;
        return;
      }
    Delay ( 75 );
    if ( !DialPuts ( "ATH\r" ) )
      {
        abort_dial = 1;
        return;
      }
    Delay ( 42 );
    Dtr ( 1 );
  }

unsigned short int Dputc ( c )
unsigned char c;
  {
    unsigned long int   signals,
                        timeoutmask,
                        serialmask;

    unsigned short int  count;
    unsigned char       ch[2];

    *ch = c; /* Get character on an even boundary. */

    timeoutmask  = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    serialmask   = 1L << ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;

    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Command = CMD_WRITE;
    ser_req.IOSer.io_Data    = (APTR) ch;
    ser_req.IOSer.io_Length  = 1;
    SendIO ( (struct IORequest *) &ser_req );

    for ( count = 0;; )
      {
        StartTimer ( &timeout_req, 1 );

        signals = Wait ( timeoutmask | serialmask );

        if ( signals & serialmask )
         {
            if ( !( signals & timeoutmask )
              {
                AbortIO ( (struct IORequest *) &timeout_req );
                Wait    ( timeoutmask );
              }
            GetMsg  ( timeout_port );
            GetMsg  ( ser_port );

            if ( CheckForWindowAbort () )
                return ( 0 );

            return ( 1 );
          }
        else
          {
            WaitIO ( (struct IORequest *) &timeout_req );
            if ( CheckForWindowAbort () || ++count == 3 )
              {
                AbortIO ( (struct IORequest *) &ser_req );
                Wait ( serialmask );
                GetMsg ( ser_port );
                return ( 0 );
              }
          }
      }
    return ( 0 );
  }

unsigned char Dgetc ()
  {
    unsigned long int   signals,
                        timeoutmask,
                        serialmask;

    unsigned short int  i;
    unsigned char       ch[2];

    timeoutmask  = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;
    serialmask   = 1L << ser_req.IOSer.io_Message.mn_ReplyPort->mp_SigBit;

    ser_req.IOSer.io_Command = CMD_READ;
    ser_req.IOSer.io_Data    = (APTR) ch;
    ser_req.IOSer.io_Flags  &= ~IOF_QUICK;
    ser_req.IOSer.io_Length  = 1;
    SendIO ( (struct IORequest *) &ser_req );


    for ( i = 0; i < nvmodes.dialtime; i++ )
      {
        StartTimer ( &timeout_req, 1 );

        signals = Wait ( timeoutmask | serialmask );

        if ( signals & serialmask )
          {
            if ( !( signals & timeoutmask ) )
              {
                AbortIO ( (struct IORequest *) &timeout_req );
                Wait    ( timeoutmask );
              }
            GetMsg  ( timeout_port );
            GetMsg  ( ser_port );

            if ( CheckForWindowAbort () )
              {
                abort_dial = 1;
                return  ( (char) 0xff );
              }

            return  ( *ch );
          }
        else
          {
            WaitIO ( (struct IORequest *) &timeout_req );
            if ( CheckForWindowAbort () )
              {
                abort_dial = 1;
                AbortIO ( (struct IORequest *) &ser_req );
                Wait    ( serialmask  );
                GetMsg  ( ser_port );
                return  ( (char) 0xff );
              }
          }
      }
    AbortIO ( (struct IORequest *) &ser_req );
    Wait    ( serialmask );
    GetMsg  ( ser_port );
    return  ( 0 );
  }

unsigned short int DialPuts ( str )
unsigned char *str;
  {
    if ( str[ 2 ] != 'H' && str[ 2 ] != '+' )
       SetDialMessage ( "Dialing" );
    while ( *str && !abort_dial )
      {
        if ( !Dputc ( *str ) )
            return ( 0 );
        if ( *str == '\r' )
          {
            StartTimer ( &timeout_req, 1 );
            Wait ( 1L <<
                   timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit );
          }
        str++;
      }
    return ( 1 );
  }

void ReDial ()
  {
        Dial ( 0 );
  }

void HangUpPhone ()
  {
    abort_dial = 0;    
    HangUp ();
  }

void ReDialDelay ()
  {
    unsigned long int   timeoutmask;

    unsigned short int  i;

    timeoutmask  = 1L << timeout_req.tr_node.io_Message.mn_ReplyPort->mp_SigBit;

    SetDialMessage ( "Delaying between dial attempts" );
    for ( i = 0; i < nvmodes.dialdelay; i++ )
      {
        StartTimer ( &timeout_req, 1 );

        Wait ( timeoutmask );

        WaitIO ( (struct IORequest *) &timeout_req );
        if ( CheckForWindowAbort () )
          {
            abort_dial = 1;
            return;
          }
      }
  }

unsigned short int Connected ()
  {
    unsigned char   response_buffer[80],
                    *ptr,
                    c;

    SetDialMessage ( "Waiting for carrier tone" );
    SleepTasks ();

try_again:;
    while ( ( c = Dgetc () ) && c != 0xff && !abort_dial )
      {
        if ( c == '\n' )
            break;
      }
    if ( !c || ( c == 0xff && abort_dial ) )
        goto connect_failed;

    for ( ptr = response_buffer;
        ( c = Dgetc () ) &&
        c != 0x0d        &&
        c != 0xff        &&
        !abort_dial      &&
        ( ptr < response_buffer + 80 );
        *ptr++ = c );
    if ( !c || ( c == 0xff || abort_dial ) )
        goto connect_failed;

    *ptr = 0;
    if ( strncmp ( "BUSY",        response_buffer, 4  ) == 0 )
      {
        SetDialMessage ( "Line Busy" );
        Delay ( 60 );
        goto connect_failed;
      }
    if ( strncmp ( "NO CARRIER",  response_buffer, 10 ) == 0 )
      {
        SetDialMessage ( "No Carrier" );
        Delay ( 60 );
        goto connect_failed;
      }
    if ( strncmp ( "NO DIALTONE", response_buffer, 11 ) == 0 )
      {
        SetDialMessage ( "No Dial Tone" );
        Delay ( 60 );
        goto connect_failed;
      }
    if ( strncmp ( "NO ANSWER",   response_buffer, 9  ) == 0 )
      {
        SetDialMessage ( "No Answer" );
        Delay ( 60 );
        goto connect_failed;
      }
    if ( strncmp ( "CONNECT",     response_buffer, 7  ) != 0 )
        goto try_again;

    WakeTasks ();

    if ( abort_dial || c == 0xff )
        return ( -1 );
    

    if ( strncmp ( "CONNECT 1200",     response_buffer, 12  ) == 0 )
      {
        nvmodes.lspeed = 1200;
        ConfigureSerialPorts ();
      }
    else if ( strncmp ( "CONNECT 2400",response_buffer, 12  ) == 0 )
      {
        nvmodes.lspeed = 2400;
        ConfigureSerialPorts ();
      }
    else if ( strncmp ( "CONNECT 9600",response_buffer, 12  ) == 0 )
      {
        nvmodes.lspeed = 9600;
        ConfigureSerialPorts ();
      }
    RefreshMenu ();
    return ( 1 );

connect_failed:
    si_msg.opcode = SER_IN_WAKE;
    PutMsg ( si_task_port, (struct Message *) &si_msg );
    ProcessUntil ( SI_SUBTASK_AWAKE );

    if ( abort_dial || c == 0xff )
        return ( -1 );

    return ( 0 );
  }

void ConnectBeep ()
  {
    MakeBeep (  500, 32 );
    MakeBeep ( 1250, 32 );
    MakeBeep (  500, 32 );
    MakeBeep ( 1250, 32 );
  }

unsigned short int Dial ( str )
unsigned char *str;
  {
    static unsigned char        last_number[NUMBER_SIZE];
    static unsigned char        dialstr[NUMBER_SIZE+63];
    unsigned char               *tmp_ptr;
    unsigned short int          auto_redial;
    unsigned short int          connect_status;
    unsigned short int          dial_count = 0;
    unsigned int                baud;

    if ( str )
      {
        strcpy ( last_number, str );
        strcpy ( dialstr, nvmodes.dial_str );
        tmp_ptr = stptok ( str, dialstr+strlen(nvmodes.dial_str), 30, " :" );
        strcat ( dialstr, "\r" );
        if ( *tmp_ptr == ':' )
          {
            tmp_ptr += stcd_i ( ++tmp_ptr, &baud );
            if ( baud >= 112 && baud <=292000 )
                nvmodes.lspeed = baud;
            if ( *tmp_ptr == ',' )
              {
                switch ( *++tmp_ptr )
                  {
                    case '7':
                        nvmodes.bpc = 7;
                        break;
                    case '8':
                        nvmodes.bpc = 8;
                        break;
                    default:
                        break;
                  }
                if ( *++tmp_ptr == ',' )
                  {
                    switch ( *++tmp_ptr )
                      {
                        case 'n':
                        case 'N':
                            nvmodes.parity = 0;
                            break;
                        case 'e':
                        case 'E':
                            nvmodes.parity = 2;
                            break;
                        case 'o':
                        case 'O':
                            nvmodes.parity = 1;
                            break;
                        case 'm':
                        case 'M':
                            nvmodes.parity = 3;
                            break;
                        case 's':
                        case 'S':
                            nvmodes.parity = 4;
                            break;
                        default:
                            break;
                      } 
                    if ( *++tmp_ptr == ',' )
                      {
                        switch ( *++tmp_ptr )
                          {
                            case '1':
                                nvmodes.stpbits = 1;
                                break;
                            case '2':
                                nvmodes.stpbits = 2;
                                break;
                            default:
                                break;
                          }
                      }
                  }
              }
            ConfigureSerialPorts ();
            RefreshMenu ();
          }
      }
    if ( str[ strlen(str) - 1 ] == '*' || nvmodes.dial_mode )
      {
        auto_redial = 1;
        OpenTransferWindow ( "Auto Re-dial",350, 65 );
      }
    else
      {
        auto_redial = 0;
        OpenTransferWindow ( "Dialing",350, 65 );
      }


    SetPhoneNumber ( last_number );
    SetAnswerTimeout ( (unsigned short int) nvmodes.dialtime  );
    SetRetryDelay ( (unsigned short int) nvmodes.dialdelay );
    SetDialAttempts ( dial_count );

    for ( abort_dial = 0, connect_status = 0;
          !connect_status && !abort_dial;
          ReDialDelay () )
      {
        if ( nvmodes.hangup || dial_count )
            HangUp ();
        if ( abort_dial )
            break;
        SetDialAttempts ( ++dial_count );
        if ( !DialPuts ( dialstr ) )
          {
            abort_dial = 1;
            break;
          }
        if ( connect_status = Connected () )
            break;
        if ( !auto_redial || abort_dial )
            break;
     }

    if ( auto_redial )
      {
        if ( abort_dial || connect_status == -1 )
          {
            abort_dial = 0;
            HangUp ();
          }
      }
    CheckForWindowAbort ();
    CloseTransferWindow ();
    if ( connect_status == 1)
      {
        ConnectBeep ();
        return ( 1 );
      }
    return ( 0 );
  }


void SetDialMessage ( message )
unsigned char   *message;
  {
    unsigned char   message_buffer[ 81 ];
    
    if ( WWindow )
	  {
		sprintf ( message_buffer, "Status: %-30s", message );
    	Move ( WWindow->RPort, 8, 60 );
    	Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
	  }
	else
		return;
    if ( !TWindow )
       return;
    Move ( TWindow->RPort, 8, 60 );
    Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
  }

void SetDialAttempts ( count )
unsigned int   count;
  {
    unsigned char   message_buffer[ 81 ];
    
	if ( WWindow )
	  {
	    sprintf ( message_buffer, "Attempt number: %3d", count );
    	Move ( WWindow->RPort, 8, 50 );
    	Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
	  }
	else
		return;
    if ( !TWindow )
       return;
    Move ( TWindow->RPort, 8, 50 );
    Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
  }

void SetRetryDelay ( delay )
unsigned int delay;
  {
    unsigned char   message_buffer[ 81 ];
    
    if ( WWindow )
	  {
		sprintf ( message_buffer, "Re-dial delay   %3d seconds", delay );
    	Move ( WWindow->RPort, 8, 40 );
    	Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
	  }
	else
		return;

    if ( !TWindow )
       return;
    Move ( TWindow->RPort, 8, 40 );
    Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
  }

void SetAnswerTimeout ( timeout )
unsigned int   timeout;
  {
    unsigned char   message_buffer[ 81 ];
    
    if ( WWindow )
	  {
		sprintf ( message_buffer, "Answer timeout  %3d seconds", timeout );
    	Move ( WWindow->RPort, 8, 30 );
    	Text ( WWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
	  }
	else
		return;
    if ( !TWindow )
       return;
    Move ( TWindow->RPort, 8, 30 );
    Text ( TWindow->RPort, message_buffer, (long) strlen ( message_buffer ) );
  }

void SetPhoneNumber ( phone_number )
unsigned char   *phone_number;
  {
	if ( WWindow )
	  {
    	Move ( WWindow->RPort, 8, 20 );
    	Text ( WWindow->RPort, "Dialing: ", 9 );
    	Text ( WWindow->RPort, phone_number, (long) strlen ( phone_number ) );
	  }
	else
		return;
    if ( !TWindow )
       return;
    Move ( TWindow->RPort, 8, 20 );
    Text ( TWindow->RPort, "Dialing: ", 9 );
    Text ( TWindow->RPort, phone_number, (long) strlen ( phone_number ) );
  }



unsigned char *StrToUpper ( string )
unsigned char *string;
  {
    unsigned char *orig_string = string;

    for ( ; *string; string++ )
        *string = toupper ( *string );

    return ( orig_string );
  }

unsigned short int DialInitialNumber ( name )
unsigned char   *name;
  {
    unsigned char       dial_str[ NUMBER_SIZE ];
    unsigned char       *name_ptr;
    unsigned short int  phone_idx;
    unsigned short int  status = 0;

    if ( !*name )
        return ( status );

    ClearMenuStrip ( Window );
    for ( phone_idx = 0; phone_idx < NUMBER_OF_NUMBERS; phone_idx++ )
      {
        name_ptr = stptok ( nvmodes.phonestr[ phone_idx ],
                            dial_str, NUMBER_SIZE - 1, " " );
        
        for ( ; *name_ptr == ' '; name_ptr++ );

        if ( name_ptr )
          {
            stptok ( name_ptr, dial_str, NUMBER_SIZE - 1, "*" );

            StrToUpper ( dial_str );
            StrToUpper ( name );

            if ( !strcmp ( dial_str, name ) )
              {
                status = Dial ( nvmodes.phonestr[ phone_idx ] );
                break;
              }
          }
      }
    InitMenu ();
    return ( status );
  }

