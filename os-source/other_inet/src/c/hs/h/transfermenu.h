/* -----------------------------------------------------------------------
 * transfermenu.h       handshake_src
 *
 * $Locker:  $
 *
 * $Id: transfermenu.h,v 1.1 91/05/09 14:28:52 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/transfermenu.h,v 1.1 91/05/09 14:28:52 bj Exp $
 *
 * $Log:	transfermenu.h,v $
 * Revision 1.1  91/05/09  14:28:52  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* Transfer menu level 2 Binary protocol selection.
*
***/

static struct IntuiText xprotocoltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "External Protocol",/* Itext                */
    NULL            /* No next string           */
  };

static struct MenuItem xprotocol =
  {
    NULL,           /* Next menu item           */
    150,70,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xffffff7f,     /* MutualExclude            */
    (APTR) &xprotocoltext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText kermittexttext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "KERMIT Text",  /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem kermittext =
  {
    &xprotocol,      /* Next menu item          */
    150,60,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xffffffbf,     /* MutualExclude            */
    (APTR) &kermittexttext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText kermit7bittext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "KERMIT 7-bit Binary",/* Itext              */
    NULL            /* No next string           */
  };

static struct MenuItem kermit7bit =
  {
    &kermittext,    /* Next menu item           */
    150,50,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xffffffdf,     /* MutualExclude            */
    (APTR) &kermit7bittext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText kermitext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "KERMIT 8-bit Binary",/* Itext              */
    NULL            /* No next string           */
  };

static struct MenuItem kermit =
  {
    &kermit7bit,    /* Next menu item           */
    150,40,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xffffffef,     /* MutualExclude            */
    (APTR) &kermitext,/* ItemFill               */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText ymodembatchtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "YMODEM Batch", /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem ymodembatch =
  {
    &kermit,           /* Next menu item           */
    150,30,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &ymodembatchtext,/* ItemFill         */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText ymodemtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "YMODEM",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem ymodem =
  {
    &ymodembatch,   /* Next menu item           */
    150,20,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &ymodemtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText xmodemcrctext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "XMODEM/CRC",   /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem xmodemcrc =
  {
    &ymodem,        /* Next menu item           */
    150,10,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &xmodemcrctext,/* ItemFill           */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText xmodemchecksumtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "XMODEM",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem xmodemchecksum =
  {
    &xmodemcrc,     /* Next menu item           */
    150,00,         /* LE, TE                   */
    160+CHECKWIDTH,10,/* Width, Height          */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &xmodemchecksumtext,/* ItemFill      */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

/***
*
* Transfer menu level 2 Transmit carriage return processing.
*
***/

static struct IntuiText txcrignoretext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "ignore",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txcrignore =
  {
    NULL,           /* Next menu item           */
    150,30,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &txcrignoretext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txcrnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "LF",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txcrnl =
  {
    &txcrignore,    /* Next menu item           */
    150,20,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &txcrnltext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txcrcrtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txcrcr =
  {
    &txcrnl,        /* Next menu item           */
    150,10,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &txcrcrtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txcrcrnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR, LF",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txcrcrnl =
  {
    &txcrcr,        /* Next menu item           */
    150,00,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffe,      /* MutualExclude           */
    (APTR) &txcrcrnltext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

/***
*
* Transfer menu level 2 Transmit line feed processing.
*
***/

static struct IntuiText txnlignoretext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "ignore",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txnlignore =
  {
    NULL,           /* Next menu item           */
    150,30,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffff7,      /* MutualExclude            */
    (APTR) &txnlignoretext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txnlnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,    /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "LF",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txnlnl =
  {
    &txnlignore,    /* Next menu item           */
    150,20,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &txnlnltext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txnlcrtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,    /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txnlcr =
  {
    &txnlnl,        /* Next menu item           */
    150,10,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffd,      /* MutualExclude            */
    (APTR) &txnlcrtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txnlcrnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,    /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR, LF",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem txnlcrnl =
  {
    &txnlcr,        /* Next menu item           */
    150,00,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &txnlcrnltext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

/***
*
* Transfer menu level 2 Receive carriage return processing.
*
***/

static struct IntuiText rxcrignoretext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "ignore",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxcrignore =
  {
    NULL,           /* Next menu item           */
    150,30,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &rxcrignoretext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxcrnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "LF",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxcrnl =
  {
    &rxcrignore,    /* Next menu item           */
    150,20,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &rxcrnltext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxcrcrtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxcrcr =
  {
    &rxcrnl,        /* Next menu item           */
    150,10,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &rxcrcrtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxcrcrnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR, LF",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxcrcrnl =
  {
    &rxcrcr,        /* Next menu item           */
    150,00,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &rxcrcrnltext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

/***
*
* Transfer menu level 2 receive linefeed processing.
*
***/

static struct IntuiText rxnlignoretext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,    /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "ignore",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxnlignore =
  {
    NULL,           /* Next menu item           */
    150,30,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffff7,     /* MutualExclude            */
    (APTR) &rxnlignoretext,/* ItemFill          */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxnlnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "LF",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxnlnl =
  {
    &rxnlignore,    /* Next menu item           */
    150,20,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffb,     /* MutualExclude            */
    (APTR) &rxnlnltext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxnlcrtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR",           /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxnlcr =
  {
    &rxnlnl,        /* Next menu item           */
    150,10,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffd,     /* MutualExclude            */
    (APTR) &rxnlcrtext,/* ItemFill              */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxnlcrnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5+CHECKWIDTH,   /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "CR, LF",       /* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem rxnlcrnl =
  {
    &rxnlcr,        /* Next menu item           */
    150,00,         /* LE, TE                   */
    60+CHECKWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    CHECKIT     |
    ITEMENABLED |
    HIGHCOMP,
    0xfffffffe,     /* MutualExclude            */
    (APTR) &rxnlcrnltext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

/***
*
* Transfer menu level 1 items
*
***/

static struct IntuiText xprinittext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Init Ext. Protocol",/* Itext               */
    NULL            /* No next string           */
  };

static struct MenuItem xprinititem =
  {
    NULL,           /* Next menu item           */
    0,100,          /* LE, TE                   */
    176+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &xprinittext,/* ItemFill             */
    NULL,           /* SelectFill               */
    'X',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText xmodemsendtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Transmit Binary File",/* Itext             */
    NULL            /* No next string           */
  };

static struct MenuItem xmodemsenditem =
  {
    &xprinititem,   /* Next menu item           */
    0,80,           /* LE, TE                   */
    176+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &xmodemsendtext,/* ItemFill          */
    NULL,           /* SelectFill               */
    't',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText xmodemreceivetext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Receive Binary File",/* Itext              */
    NULL            /* No next string           */
  };

static struct MenuItem xmodemreceiveitem =
  {
    &xmodemsenditem,/* Next menu item           */
    0,90,           /* LE, TE                   */
    176+COMMWIDTH,10,/* LE, TE, Width, Height   */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &xmodemreceivetext,/* ItemFill       */
    NULL,           /* SelectFill               */
    'r',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText protocoltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Binary Protocol",/* Itext                  */
    NULL            /* No next string           */
  };

static struct MenuItem protocolitem =
  {
    &xmodemreceiveitem,/* Next menu item        */
    0,70,           /* LE, TE                   */
    176+COMMWIDTH,10,/* LE, TE, Width, Height   */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &protocoltext,/* ItemFill            */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &xmodemchecksum, /* First subitem           */
    0               /* Next Select              */
  };

static struct IntuiText asciisendtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Send ASCII File",/* Itext                  */
    NULL            /* No next string           */
  };

static struct MenuItem asciisenditem =
  {
    &protocolitem,  /* Next menu item           */
    0,60,           /* LE, TE                   */
    176+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &asciisendtext,/* ItemFill           */
    NULL,           /* SelectFill               */
    'a',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText captureofftext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "End ASCII Rec.",/* Itext                   */
    NULL            /* No next string           */
  };

static struct MenuItem captureoffitem =
  {
    &asciisenditem, /* Next menu item           */
    0,50,           /* LE, TE                   */
    176+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &captureofftext,/* ItemFill          */
    NULL,           /* SelectFill               */
    'e',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText captureontext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Receive ASCII",/* Itext                    */
    NULL            /* No next string           */
  };

static struct MenuItem captureonitem =
  {
    &captureoffitem,/* Next menu item           */
    0,40,           /* LE, TE                   */
    176+COMMWIDTH,10,/* Width, Height           */
    ITEMTEXT    |   /* Flags                    */
    COMMSEQ     |
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &captureontext,/* ItemFill           */
    NULL,           /* SelectFill               */
    'c',            /* Command                  */
    NULL,           /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Receive  LF as ...",  /* Itext              */
    NULL            /* No next string           */
  };

static struct MenuItem rxnlitem =
  {
    &captureonitem, /* Next menu item           */
    0,30,140,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &rxnltext,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &rxnlcrnl,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText rxcrtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Receive  CR as ...",  /* Itext             */
    NULL            /* No next string           */
  };

static struct MenuItem rxcritem =
  {
    &rxnlitem,      /* Next menu item           */
    0,20,140,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &rxcrtext,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &rxcrcrnl,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txnltext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Transmit LF as ...",/* Itext               */
    NULL            /* No next string           */
  };

static struct MenuItem txnlitem =
  {
    &rxcritem,      /* Next menu item           */
    0,10,140,10,    /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &txnltext,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &txnlcrnl,      /* First subitem            */
    0               /* Next Select              */
  };

static struct IntuiText txcrtext =
  {
    0,1,            /* FrontPen, BackPen        */
    JAM1,           /* DrawMode                 */
    5,              /* LeftEdge                 */
    1,              /* TopEdge                  */
    NULL,           /* ITextFont                */
    "Transmit CR as ...",  /* Itext             */
    NULL            /* No next string           */
  };

static struct MenuItem txcritem =
  {
    &txnlitem,      /* Next menu item           */
    0,0,140,10,     /* LE, TE, Width, Height    */
    ITEMTEXT    |   /* Flags                    */
    ITEMENABLED |
    HIGHCOMP,
    0,              /* MutualExclude            */
    (APTR) &txcrtext,/* ItemFill                */
    NULL,           /* SelectFill               */
    0,              /* Command                  */
    &txcrcrnl,      /* First subitem            */
    0               /* Next Select              */
  };

