
/***************************************************************************
 * 
 * Your Text Here
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 16 Dec 86  =RJ Mical=   Created this file
 *
 **************************************************************************/

#include "zaphod.h"
#include <stdio.h>

extern struct Library *IntuitionBase;

#define ADJUST_MICROS   100

static struct Window *AdjustWindow;

static UBYTE SIBuffer1[8];

static struct StringInfo GadgetSI1 = {
   SIBuffer1,  /* buffer where text will be edited */
   NULL,       /* optional undo buffer */
   0L,          /* character position in buffer */
   8L,          /* maximum number of characters to allow */
   0L,          /* first displayed character buffer position */
   0L,0L,0L,0L,0L,  /* Intuition initialized and maintained variables */
   0L,          /* Rastport of gadget */
   0L,          /* initial value for integer gadgets */
   NULL        /* alternate keymap (fill in if you set the flag) */
};

static SHORT BorderVectors1[] = {
   0L,0L,
   73L,0L,
   73L,10L,
   0L,10L,
   0L,0L
};
static struct Border Border1 = {
   -2L,-1L,            /* Border XY origin relative to container TopLeft */
   1L,0L,JAM1,         /* front pen, back pen and drawmode */
   5L,                /* number of XY vectors */
   BorderVectors1,   /* pointer to XY vectors */
   NULL              /* next border in list */
};

static struct IntuiText IText2 = {
   1L,2L,JAM1,   /* front and back text pens, drawmode and fill byte */
   -204L,-10L,   /* XY origin relative to container TopLeft */
   &SafeFont/*NULL*/,       /* font pointer or NULL for default */
   (UBYTE *)"between transmission of key events:",   /* pointer to text */
   NULL        /* next IntuiText structure */
};

static struct IntuiText IText1 = {
   1L,2L,JAM1,   /* front and back text pens, drawmode and fill byte */
   -151L,-19L,   /* XY origin relative to container TopLeft */
   &SafeFont/*NULL*/,       /* font pointer or NULL for default */
   (UBYTE *)"Enter number of microseconds",   /* pointer to text */
   &IText2     /* next IntuiText structure */
};

static struct Gadget AdjustMicros = {
   NULL,             /* next gadget */
   211L,26L,           /* origin XY of hit box relative to window TopLeft */
   70L,9L,             /* hit box width and height */
   NULL,             /* gadget flags */
   RELVERIFY+ENDGADGET+STRINGRIGHT+LONGINT,   /* activation flags */
   STRGADGET,        /* gadget type flags */
   (APTR)&Border1,   /* gadget border or image to be rendered */NULL,   /* alternate imagery for selection */
   &IText1,          /* first IntuiText structure */
   0L,                /* gadget mutual-exclude long word */
   (APTR)&GadgetSI1, /* SpecialInfo structure */
   ADJUST_MICROS,    /* user-definable data */
   NULL              /* pointer to user-definable data */
};

/* Gadget list */

static struct NewWindow NewWindowStructure1 = {
   10L, 12L,        /* window XY origin relative to TopLeft of screen */
   300L,55L,        /* window width and height */
   0L,1L,           /* detail and block pens */
   NULL,          /* IDCMP flags */
   WINDOWDRAG|WINDOWDEPTH|SIMPLE_REFRESH | ACTIVATE | NOCAREREFRESH,   /* other window flags */
   NULL,          /* first gadget in gadget list */
   NULL,          /* custom CHECKMARK imagery */
   "PCWindow",          /* window title */
   NULL,          /* custom screen pointer */
   NULL,          /* custom bitmap */
   5L,5L,           /* minimum width and height */
   640L,200L,       /* maximum width and height */
   WBENCHSCREEN   /* destination screen type */
};

/* end of PowerWindows source generation */



#define AdjustSInfo     GadgetSI1
#define AdjustNewWindow NewWindowStructure1

/* === Main Data ======================================================== */
static struct ReqSupport AdjustSupport = 
   {

   /* struct Requester Requester; */
      {
      /*   struct Requester *OlderRequest;*/
      NULL,

      /*   SHORT LeftEdge, TopEdge;*/
      2L, 20L,

      /*   SHORT Width, Height;*/
      294L, 44L, 

      /*   SHORT RelLeft, RelTop;*/
      0L, 0L,

      /*   struct Gadget *ReqGadget;*/
      &AdjustMicros,
   
      /*   struct Border *ReqBorder;*/
      NULL,
      
      /*   struct IntuiText *ReqText;*/
      NULL,
   
      /*   USHORT Flags;*/
      NOISYREQ,

      /*   UBYTE BackFill;*/
      2L,

      /*   struct ClipRect ReqCRect;*/
      { NULL },

      /*   struct BitMap *ImageBMap;*/
      NULL,

      /*   struct BitMap ReqBMap;*/
      { NULL },

      },   /* end of Requester structure */

   /* struct Window *Window; */
   NULL,

   /* LONG (*StartRequest)(); */
   NULL,

   /* LONG (*GadgetHandler)(); */
   NULL,

   /* LONG (*NewDiskHandler)(); */
   NULL,

   /* LONG (*KeyHandler)(); */
   NULL,

   /* LONG (*MouseMoveHandler)(); */
   NULL,

   /* SHORT SelectedGadgetID; */
   0L,
};


/****i* PCWindow/AdjustStart ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/

static BOOL AdjustStart(void)
{
   BOOL success;

   success=ActivateGadget(&AdjustMicros, AdjustWindow, &AdjustSupport.Requester);

   return(success);
}

/****i* PCWindow/AdjustKeyTiming ******************************************
*
*   NAME   
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
VOID AdjustKeyTiming(struct Display *display)
{
   if (FlagIsSet(display->Modes, OPEN_SCREEN))
   {
      NewWindowStructure1.Type   = CUSTOMSCREEN;
      NewWindowStructure1.Screen = display->FirstWindow->DisplayScreen;
   }
   else
      NewWindowStructure1.Type = WBENCHSCREEN;

   if(IntuitionBase->lib_Version>=36L)
   {
      AdjustWindow = OpenWindowTags(&AdjustNewWindow,
	                                WA_InnerHeight,52L,
	                                WA_InnerWidth, 300L,
	                                                 TAG_DONE);
   } else {
      AdjustWindow = OpenWindow(&AdjustNewWindow);
   }
   if (AdjustWindow)
   {
      AdjustSupport.StartRequest = (LONG (*)())AdjustStart;

      AdjustSupport.Requester.LeftEdge = AdjustWindow->BorderLeft;
      AdjustSupport.Requester.TopEdge = AdjustWindow->BorderTop;
      AdjustSupport.Requester.Width = AdjustWindow->Width-
	                                  AdjustWindow->BorderLeft-
	                                  AdjustWindow->BorderRight;
      AdjustSupport.Requester.Height = AdjustWindow->Height-
	                                  AdjustWindow->BorderTop-
	                                  AdjustWindow->BorderBottom;

      AdjustSInfo.LongInt = KeyDelayMicros;

      sprintf(AdjustSInfo.Buffer, "%ld", KeyDelayMicros);

      AdjustSupport.Window = AdjustWindow;

      DoRequest((struct ReqSupport *)&AdjustSupport);

      KeyDelayMicros = ABS(AdjustSInfo.LongInt);
      if (KeyDelayMicros == 0L) 
	     KeyDelayMicros = 1L;
      KeyDelaySeconds = KeyDelayMicros / MILLION;
      KeyDelayMicros = KeyDelayMicros % MILLION;
      CloseWindow(AdjustWindow);
   }
}
