
#include "keytoy.h"		/* standard includes & definitions */
#ifndef EXEC_IO_H
#include "exec/io.h"
#endif
#ifndef EXEC_DEVICES_H
#include "exec/devices.h"
#endif
#ifndef EXEC_LIBRARIES_H
#include "exec/libraries.h"
#endif

extern struct Window *OpenWindow();	/* function to open a window */
extern struct Gadget LShift;
extern struct Gadget RShift;
extern struct Gadget Contr;
extern struct Gadget LAlt;
extern struct Gadget RAlt;

UBYTE kTitle[] = "KeyShow";	/* KeyToy title with version no. */
UWORD version = 0;
WORD BoardType = EURO;
UWORD Mode = KC_NOQUAL;
UWORD NewMode = KC_NOQUAL;
BOOL mult = FALSE;

struct GfxBase *GfxBase = 0;	/* pointer to Graphics library */
struct IntuitionBase *IntuitionBase = 0; /* pointer to Intuition library */

struct Window *window;		/* pointer to clock window */
struct RastPort *rport;		/* pointer to clock-window RastPort */
struct Remember *RememberKey = NULL;
struct IntuiMessage *message;	/* pointer to clock IDCMP messages */
struct KeyTop Key[NUMKEYS];
struct KeyMap KMap;

struct TextAttr Topaz = {	/* KeyToy text attributes */
    "topaz.font",	/* font name */
    TOPAZ_EIGHTY,	/* font height */
    FS_NORMAL, 0	/* font style, preferences */
};

struct TextAttr TopazI = {	/* KeyToy text attributes */
    "topaz.font",	/* font name */
    TOPAZ_EIGHTY,	/* font height */
    FSF_ITALIC, 0	/* font style, preferences */
};

struct TextAttr TopazBI = {	/* KeyToy text attributes */
    "topaz.font",	/* font name */
    TOPAZ_EIGHTY,	/* font height */
    FSF_BOLD|FSF_ITALIC, 0 /* font style, preferences */
};

struct IntuiText cancelText = {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE, AUTOTOPEDGE,
    &Topaz, "Cancel", AUTONEXTTEXT
};

struct IntuiText bodyText2 = {
    2, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE, AUTOTOPEDGE+12,
    &Topaz, 0, AUTONEXTTEXT
};
struct IntuiText bodyText1 = {
    AUTOFRONTPEN, AUTOBACKPEN, AUTODRAWMODE, AUTOLEFTEDGE, AUTOTOPEDGE,
    &Topaz, 0, &bodyText2
};

struct NewWindow nw = {		/* clock-window definition */
    0, 0,		/* LeftEdge, TopEdge */
    640, 103,		/* Width, Height */
    -1,-1,		/* DetailPen, BlockPen */
    CLOSEWINDOW|GADGETDOWN|RAWKEY|REFRESHWINDOW /* IDCMPFlags */
    ACTIVATE|WINDOWDEPTH|WINDOWDRAG|WINDOWCLOSE|SMART_REFRESH,	/* Flags */
    NULL,		/* FirstGadget */
    NULL,		/* CheckMark */
    kTitle,		/* Title */
    NULL,		/* Screen */
    NULL,		/* BitMap */
    550, 103,		/* MinWidth, MinHeight */
    550, 103,		/* MaxWidth, MaxHeight */
    WBENCHSCREEN	/* Type */
};


/****************************************************************************
 *				  M A I N				    *
 ****************************************************************************/

main()
{	
    UWORD i;			/* general purpose index variable */
    UBYTE *LoTypes;
    USHORT mod;
    struct IOStdReq ior;
/*     struct Library *library; */
    struct Gadget *GadgetPnt, *GP;

    /* open libraries, bail out if there is a problem */
    if((IntuitionBase = (struct IntuitionBase *)
	OpenLibrary("intuition.library", 33)) == NULL)exit(20);
    if((GfxBase = (struct GfxBase *)
	OpenLibrary("graphics.library", 0)) == NULL)
	failNotice("Internal system error", "Cannot open graphics.library");

    if((window = OpenWindow(&nw)) == NULL)
	failNotice("Internal System Error", "Unable to open window");

    rport = window->RPort;
 
    SetWindowTitles(window, -1, kTitle);

    SetDrMd(rport, JAM1);

    /* open console device, read KeyMap, close console device */
    ior.io_Data = (APTR)window;
    ior.io_Length = sizeof(*window);
    if((OpenDevice("console.device", 0, &ior, 0)) != NULL)
	failNotice("Internal system error", "Cannot open console.device");
/*     library = ior.io_Device; */
/*     version = library->lib_Version; */
    version = ior.io_Device->dd_Library.lib_Version;
/* kprintf("console.device version %ld\n", version); */
    ior.io_Command = CD_ASKKEYMAP;
    ior.io_Length = sizeof(struct KeyMap);
    ior.io_Data = &KMap;
    DoIO(&ior);
    CloseDevice(&ior);

    /* determine keyboard type (European or USA) */
    LoTypes = KMap.km_LoKeyMapTypes;
    BoardType = (LoTypes[0x2b] == KCF_NOP) ? USA : EURO;

    InitImages();
    MakeKeys();
    DrawBoard();

    /* THE MAIN LOOP */

    FOREVER			/* and I mean it */
    {
	Wait(1 << window->UserPort->mp_SigBit);

	while(message = (struct IntuiMessage *)GetMsg(window->UserPort))
	{
	    switch(message->Class)
	    {
		case CLOSEWINDOW: 
		    ReplyMsg(message);
		    KillToy();
		    break;
		case GADGETDOWN:
		    GadgetPnt = message->IAddress;
		    ReplyMsg(message);
		    GP = (struct Gadget *)GadgetPnt->UserData;
		    if(GP != NULL)
		    {
			if(GadgetPnt->Flags & SELECTED)
			    GP->Flags |= SELECTED;
			else GP->Flags &= ~SELECTED;
		    }
		    CheckMode();
		    break;
		case RAWKEY:
		    mod = message->Qualifier & 0x003b;
		    ReplyMsg(message);
		    if((mod & 0x0003) && ((mod & 0x0003) == mod))
		    {	/* Left or Right SHIFT */
			if(mult)mult = FALSE;
			else
			{
			    LShift.Flags ^= SELECTED;
			    RShift.Flags ^= SELECTED;
			}
		    }
		    else if((mod & 0x0008) && ((mod & 0x0008) == mod))
		    {	/* CTRL */
			if(mult)mult = FALSE;
			else Contr.Flags ^= SELECTED;
		    }
		    else if((mod & 0x0030) && ((mod & 0x0030) == mod))
		    {	/* Left or Right Alt */
			if(mult)mult = FALSE;
			else
			{
			    LAlt.Flags ^= SELECTED;
			    RAlt.Flags ^= SELECTED;
			}
		    }
		    else if(mod != 0)mult = TRUE;
		    CheckMode();
		    break;
		case REFRESHWINDOW: 
		    BeginRefresh(window);
		    DrawBoard();
		    EndRefresh(window, TRUE);
		    ReplyMsg(message);
		    break;
		default: 
		    ReplyMsg(message);
	    }
	}
    }    
}


CheckMode()
{
    NewMode = ((LShift.Flags & SELECTED) ? KCF_SHIFT : 0) |
	((Contr.Flags & SELECTED) ? KCF_CONTROL : 0) |
	((LAlt.Flags & SELECTED) ? KCF_ALT : 0);
    if(NewMode != Mode)
    {
	Mode = NewMode;
	RefreshGList(&LShift, window, NULL, -1);
	DrawKeys();
    }
}


/****************************************************************************
 *				KillToy					    *
 *	free any allocated memory, close window if open, close any open	    *
 *	libraries, and make sure WorkBench is open			    *
 ****************************************************************************/

KillToy()
{
    FreeRemember(&RememberKey, TRUE);
    if(window != NULL)
	CloseWindow(window);
    if(IntuitionBase != NULL)
	CloseLibrary(IntuitionBase);
    if(GfxBase != NULL)
	CloseLibrary(GfxBase);
    OpenWorkBench();
    exit(0);
}


failNotice(string1, string2)
UBYTE *string1, *string2;
{
    bodyText1.IText = string1;
    bodyText2.IText = string2;
    AutoRequest(0, &bodyText1, 0, &cancelText, 0, 0, 440, 60);
    KillToy();
}

/*=========================== END OF FILE ==================================*/



