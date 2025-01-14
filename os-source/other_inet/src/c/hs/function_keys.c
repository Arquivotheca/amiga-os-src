/* -----------------------------------------------------------------------
 * function_keys.c		handshake_src
 *
 * $Locker:  $
 *
 * $Id: function_keys.c,v 1.1 91/05/09 16:17:29 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/RCS/function_keys.c,v 1.1 91/05/09 16:17:29 bj Exp $
 *
 * $Log:	function_keys.c,v $
 * Revision 1.1  91/05/09  16:17:29  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Function Key Requester
*
***/

#include "termall.h"

#define NUMBER_OF_KEYS 6
#define KEY_SIZE 64

static unsigned char undobuffer [KEY_SIZE];

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

static unsigned short int chargadgvect[] =
  {
    0,0, 21,0, 21,11, 0,11, 0,0
  };

static struct Border charborder = 
  {
    -3,-2,
    0,1,JAM2,
    5,
    chargadgvect,
    NULL
  };


static unsigned char  crstring[8];
static unsigned char  nlstring[8];

static struct StringInfo pfk_crinfo =
  {
    crstring,    /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer position      */
    2,           /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* Position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  };

static struct StringInfo pfk_nlinfo =
  {
    nlstring,    /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer position      */
    2,           /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* Position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  };

static struct IntuiText canceltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    1,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    " ALL DONE",    /* Itext                    */
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

static struct Gadget pfk_crgadget =
  {
    &cancelgadget,    /* Pointer to next gadget   */
    220,-12,          /* LeftEdge, TopEdge        */
    16,10,            /* Width, Height of hit box */
    GADGHCOMP |       /* Flags                    */
    GRELBOTTOM,
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &charborder,/* Pointer to string border*/
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR)&pfk_crinfo,/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };
  
static struct Gadget pfk_nlgadget =  
  {
    &pfk_crgadget,    /* Pointer to next gadget   */
    400,-12,          /* LeftEdge, TopEdge        */
    16,10,            /* Width, Height of hit box */
    GADGHCOMP |       /* Flags                    */
    GRELBOTTOM,
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &charborder,/* Pointer to string border*/
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &pfk_nlinfo,/* Pointer to special info */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  };


static unsigned short int keygadgvect[] =
  {
    0,0, 517,0, 517,12, 0,12, 0,0
  };

static struct Border keyborder = 
  {
    -3,-2,
    0,1,JAM2,
    5,
    keygadgvect,
    NULL
  };


static struct StringInfo keystringinfo[] =
{
  {
    NULL,        /* IO buffer            */
    undobuffer,  /* Undo buffer          */
    0,           /* Buffer Position      */
    KEY_SIZE,    /* Maximum size         */
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
    KEY_SIZE,    /* Maximum size         */
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
    KEY_SIZE,    /* Maximum size         */
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
    KEY_SIZE,    /* Maximum size         */
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
    KEY_SIZE,    /* Maximum size         */
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
    KEY_SIZE,    /* Maximum size         */
    0, 0,        /* Initial positions    */
    0,           /* Initial no. of chars */
    0, 0, 0,     /* position vars        */
    NULL,        /* Layer                */
    NULL,        /* Long int             */
    NULL         /* Alternate keymap     */
  }
};


static struct Gadget keydefgadget[] =
{
  {
    NULL,             /* Pointer to next gadget   */
    48,10,            /* LeftEdge, TopEdge        */
    512,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &keyborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR)&keystringinfo[0],/* Pointer to special info*/
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    48,25,            /* LeftEdge, TopEdge        */
    512,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &keyborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR)&keystringinfo[1],/* Pointer to special info*/
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    48,40,            /* LeftEdge, TopEdge        */
    512,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &keyborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &keystringinfo[2],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    48,55,            /* LeftEdge, TopEdge        */
    512,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &keyborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &keystringinfo[3],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    48,70,            /* LeftEdge, TopEdge        */
    512,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &keyborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &keystringinfo[4],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
  {
    NULL,             /* Pointer to next gadget   */
    48,85,            /* LeftEdge, TopEdge        */
    512,10,           /* Width, Height of hit box */
    GADGHCOMP,        /* Flags                    */
    GADGIMMEDIATE |   /* Activation flags         */
    ALTKEYMAP,
    STRGADGET |       /* Type                     */
    REQGADGET,
    (APTR) &keyborder, /* Pointer to string border */
    NULL,             /* SelectRender             */
    NULL,             /* Pointer to GadgetText    */
    0,                /* No mutual exclude        */
    (APTR) &keystringinfo[5],/* Pointer to special info  */
    0,                /* No ID                    */
    NULL              /* No pointer to user data  */
  },
};

static unsigned short int kreqvect [] =
  {
      0,  0,
    595,  0,
    595, 116,
      0, 116,
      0,  0
  };

static struct Border kreqborder =
  {
    2,2,
    0,1,JAM2,
    5,
    kreqvect,
    NULL
  };

static struct IntuiText crtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    120,            /* LeftEdge                 */
    108,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Imbedded CR",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct IntuiText nltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    300,            /* LeftEdge                 */
    108,            /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Imbedded LF",  /* Itext                    */
    &crtext         /* Next string              */
  };

static struct IntuiText f10text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    85,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "F10:",         /* Itext                    */
    &nltext         /* Next string              */
  };

static struct IntuiText f9text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    70,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "F9:",          /* Itext                    */
    &f10text        /* Next string              */
  };

static struct IntuiText f8text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    55,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "F8:",          /* Itext                    */
    &f9text         /* Next string              */
  };

static struct IntuiText f7text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    40,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "F7:",          /* Itext                    */
    &f8text         /* Next string              */
  };

static struct IntuiText f6text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    25,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "F6:",          /* Itext                    */
    &f7text         /* Next string              */
  };

static struct IntuiText f5text =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM2,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    10,             /* TopEdge                  */
    NULL,           /* ITextFont                */
    "F5:",          /* Itext                    */
    &f6text         /* Next string              */
  };

static struct Requester pfkrequester =
  {
    NULL,
    20,15,
    600,120,
    0,0,
    &keydefgadget[0],
    &kreqborder,
    &f5text,
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

int SetFunctionKeys ()
  {
    unsigned short int done;
    int status;
    int i;

    TWindow = NULL;
    WWindow = NULL;
    for ( i = 0; i < NUMBER_OF_KEYS - 1 ; i++ )
        keydefgadget[i].NextGadget = &keydefgadget[i+1];
    keydefgadget[i].NextGadget = &pfk_nlgadget;

    for ( i = 0; i < NUMBER_OF_KEYS; i++ )
      {
        keystringinfo[i].AltKeyMap = &OrigKeyMap;
        nvmodes.keystr[i][ KEY_SIZE - 1] = 0x00;
        keystringinfo[i].Buffer    = nvmodes.keystr[i];
      }

    pfk_crinfo.AltKeyMap = &OrigKeyMap;
    pfk_nlinfo.AltKeyMap = &OrigKeyMap;
    *crstring = nvmodes.pfk_cr;
    crstring[1]  = '\0';
    *nlstring = nvmodes.pfk_nl;
    nlstring[1]  = '\0';
    
    Request ( &pfkrequester, Window );

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
                for ( i = 0; i < NUMBER_OF_KEYS; i++ )
                    if ( address == (APTR) &keydefgadget[i] )
                      {
                        status = 1;
                        break;
                      }
                if ( status )
                    done = 1;
              }
          }
      }
    nvmodes.pfk_nl = *nlstring;
    nvmodes.pfk_cr = *crstring;
    
    UpdateFunctionKeys ();
    return ( status );
  }


