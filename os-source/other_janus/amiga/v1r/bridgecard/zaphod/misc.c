
/* ***************************************************************************
 * 
 * Miscellaneous Routines for the Zaphod Project
 *
 * Copyright (C) 1986, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY	Name		Description
 * -------	------		--------------------------------------------
 * 23 Feb 86	=RJ Mical=	Created this file
 *
 * **************************************************************************/

#include "zaphod.h"
						
						 
extern UBYTE FakeColorCRT[CRT_IOBLOCKSIZE];


 
struct Task *CreateTask(name, priority, initialPC, stacksize)
UBYTE *name;
UBYTE priority;
APTR initialPC;
ULONG stacksize;
{
    struct Task *taskptr;
    ULONG realsize;
    UBYTE *stackptr;

    taskptr = (struct Task *)AllocRemember(&GlobalKey, sizeof(struct Task), 
	    MEMF_CLEAR);

    realsize = (stacksize + 3) & 0xFFFFFC;
    stackptr = AllocRemember(&GlobalKey, realsize, MEMF_CLEAR | MEMF_PUBLIC);

    if ((taskptr == NULL) || (stackptr == NULL))
	{
	Alert(ALERT_NO_MEMORY);
	return(NULL);
	}

    taskptr->tc_SPLower = (APTR)stackptr;
    taskptr->tc_SPUpper = (APTR)(((LONG)(stackptr) + realsize - 2) & 0xFFFFFE);
    taskptr->tc_SPReg = taskptr->tc_SPUpper;

    taskptr->tc_Node.ln_Type = NT_TASK;
    taskptr->tc_Node.ln_Pri = priority;
    taskptr->tc_Node.ln_Name = name;

    AddTask(taskptr, initialPC, 0);
    return(taskptr);
}

 
CloseEverything()
{
/* === Critical Section (for safety's sake) ========================== */
    Forbid();

    if (GfxBase) CloseLibrary(GfxBase);
    if (LayersBase) CloseLibrary(LayersBase);
    if (IntuitionBase) CloseLibrary(IntuitionBase);
    if (JanusBase) CloseLibrary(JanusBase);

    FreeRemember(&GlobalKey, TRUE);

    Permit();
/* === End of Critical Section ====================================== */
}

		    
DrainPort(port)
struct MsgPort *port;
/* NOTE:  DrainPort() does *not* Forbid() and Permit().  There's little
 * point in doing so.  If the caller needs this quality, the caller 
 * should provide it (and probably needs to for other reasons as well).
 */
{
    struct IntuiMessage *message;

    if (port)
	while (message = GetMsg(port)) ReplyMsg(message);
}

#ifdef DIAG
  
SHORT ZGetChar()
{
     SHORT inkey;

     inkey = kgetchar();
  
     if (inkey >= 'a' && inkey <= 'z') inkey = inkey - 'a' + 'A';

     kprintf("%lc",inkey);

     return(inkey);
}
#endif

ULONG ZAllocSignal()
{
     SHORT i;

    if ((i = AllocSignal(-1)) == -1)
	{
	Alert(ALERT_NO_SIGNALS, NULL);
	return(-1);
	}
    return((ULONG)1 << i);
}


SHORT StupidCreateSignalNumber(mask)
ULONG mask;
/* I will never understand why we are forced to pass around signal numbers
 * when everyone in the universe converts the number to a mask before 
 * doing anything useful with it.
 */
{
    SHORT i;

    i = 0;
    while ((mask & 1) == 0)
	{
	i++;
	mask >>= 1;
	}

    return(i); 
}


	     
TopRightLines(window)
struct Window *window;
{
    SHORT xright, ytop, ybottom;

    xright = window->Width - window->BorderRight - 1;
    ytop = window->BorderTop - 1;
    ybottom = window->Height - window->BorderBottom - 1;

    SetAPen(window->RPort, 0);
    SetDrMd(window->RPort, JAM2);
    Move(window->RPort, window->BorderLeft, ytop);
    Draw(window->RPort, xright, ytop);
}



GetPCDisplay(display)
struct Display *display;
{
#ifdef JANUS
    display->PCDisplay = display->JanusStart + display->PCDisplayOffset;
    display->PCCRTRegisterBase = display->JanusStart;
   
     /* If we're medium-res graphics, use the special graphics offset ... */
    if ( (display->Modes & MEDIUM_RES) 
	    && ((display->Modes & TEXT_MODE) == 0) )
	display->PCDisplay += GRAPHIC_ACCESS_OFFSET;
    else   /* ... else just use the normal byte-oriented access */
	display->PCDisplay += BYTE_ACCESS_OFFSET;
				
    if (display->Modes & COLOR_DISPLAY)
	{
	display->PCDisplay += COLOR_OFFSET;
	display->PCCRTRegisterBase += COLOR_CRT_OFFSET;
	}
    else
	{
	display->PCDisplay += MONO_OFFSET;
	display->PCCRTRegisterBase += MONO_CRT_OFFSET;
	}

#else
    if (display->PCDisplay == NULL)
	if ((display->PCDisplay
		= AllocRemember(&GlobalKey, 0x4000, MEMF_CLEAR) ) 
		== NULL)
	    Alert(ALERT_NO_MEMORY, NULL);

    display->PCCRTRegisterBase = &FakeColorCRT[0];
#endif
}


SetTimer(sec, micro, timermessage)
ULONG sec, micro;
struct IOStdReq *timermessage;
{
    timermessage->io_Command = TR_ADDREQUEST; /* add a new timer request */
    timermessage->io_Actual = sec;    /* seconds */
    timermessage->io_Length = micro;	/* microseconds */
    SendIO(timermessage);	/* post a request to the timer */
}



DrawBox(rport, x, y, width, height, pen, hthick, vthick)
struct RastPort *rport;
SHORT x, y, width, height, pen, hthick, vthick;
{
    SHORT right, bottom, i;
    USHORT savemode, savepen;

    savemode = rport->DrawMode;
    savepen = rport->FgPen;

    SetAPen(rport, pen);
    SetDrMd(rport, JAM1);

    right = x + width - 1;
    bottom = y + height - 1;

    for (i = 0; i < hthick; i++)
	{
	Move(rport, x + i, y);
	Draw(rport, x + i, bottom);
	Move(rport, right - i, y);
	Draw(rport, right - i, bottom);
	}

    x += hthick;
    right -= hthick;

    for (i = 0; i < vthick; i++)
	{
	Move(rport, x, y + i);
	Draw(rport, right, y + i);
	Move(rport, x, bottom - i);
	Draw(rport, right, bottom - i);
	}

    SetDrMd(rport, savemode);
    SetAPen(rport, savepen);
}



RefreshWindowFrame(w)
struct Window *w;
{
    struct RastPort *rport;
    SHORT borderleft, bordertop, borderright, borderbottom;
    SHORT windowbottom, windowright, savemode, savepen;

    rport = w->RPort;
    windowbottom = w->Height - 1;
    windowright = w->Width - 1;
    borderleft = w->BorderLeft;
    borderright = w->BorderRight;
    bordertop = w->BorderTop;
    borderbottom = w->BorderBottom;

    savemode = rport->DrawMode;
    savepen = rport->FgPen;

    SetAPen(rport, 0);
    SetDrMd(rport, JAM1);

    if (bordertop)
	RectFill(rport, 0, 0, windowright, bordertop - 1);

    if (borderbottom)
	RectFill(rport, 0, windowbottom - (borderbottom - 1),
		windowright, windowbottom);

    if (borderleft)
	RectFill(rport, 0, 0, borderleft - 1, windowbottom);

    if (borderright)
	RectFill(rport, windowright - (borderright - 1), 0, 
		windowright, windowbottom);

    if ((w->Flags & BORDERLESS) == 0)
	DrawBox(rport, 0, 0, w->Width, w->Height, w->BlockPen, 
		borderleft >> 1, 1);

    SetWindowTitles(w, w->Title, -1);

    RefreshGadgets(w->FirstGadget, w, NULL);
}



/*???*/
ULONG StupidULongDivide(longa, longb)
LONG longa, longb;
{
    ULONG result;
    ULONG partial;
    LONG tempb;
	  
    longa >>= 1;
    result = 0;

    while (longa >= longb)
	{
	partial = 2;
	tempb = longb << 1;
	while (longa > tempb)
	    {
	    tempb <<= 1;
	    partial <<= 1;
	    }
	partial >>= 1;
	tempb >>= 1;
	result |= partial;
	longa -= tempb;
	}

    return(result << 1);
}


ModifyDisplayProps(display)
struct Display *display;
{
    LONG partial;
    USHORT hbody, vbody;
    struct SuperWindow *superwindow;
    struct Window *window;
    struct PropInfo *pinfo;

    Lock(&display->DisplayLock);

    if (superwindow = display->FirstWindow)
	{
	if (window = superwindow->DisplayWindow)
	    {
	    partial = window->GZZWidth << 16;
	    hbody = StupidULongDivide((ULONG)partial, (ULONG)window->MaxWidth);
	    partial = window->GZZHeight << 16;
	    vbody = StupidULongDivide((ULONG)partial, (ULONG)window->MaxHeight);

	    if (display->Modes & BORDER_VISIBLE)
		{
		pinfo = (struct PropInfo *)&superwindow->HorizInfo;
		ModifyProp(&superwindow->HorizGadget, window, NULL, 
			pinfo->Flags, pinfo->HorizPot, pinfo->VertPot,
			hbody, 0);
		pinfo = (struct PropInfo *)&superwindow->VertInfo;
		ModifyProp(&superwindow->VertGadget, window, NULL, 
			pinfo->Flags, pinfo->HorizPot, pinfo->VertPot,
			0, vbody);
		}
	    else
		{
		superwindow->HorizInfo.HorizBody = hbody;
		superwindow->VertInfo.VertBody = vbody;
		}
	    }
	}

    Unlock(&display->DisplayLock);
}

					     

SHORT StringLength(text)
UBYTE *text;
{
    SHORT i;

    i = 0;
    while (*text++) i++;
    return(i);
}



