#include <exec/types.h>
#include <exec/exec.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/intuition.h>
/* #include <local/sccs.h> */

#include "share.h"

#include "auto.h"
#include "man.h"
#include "df0.h"
#include "df1.h"
#include "pc_a.h"
#include "pc_b.h"
#include "menu.h"
#include "window.h"

/* We leave these here temporarily */

#define AUTOSELECT { AutoGadget.Flags |= SELECTED; \
ManGadget.Flags &= ~SELECTED; }

#define MANSELECT {AutoGadget.Flags &= ~SELECTED; \
ManGadget.Flags |= SELECTED; }

#define DF0SELECT {AmigaGadget.Flags |= SELECTED; \
PCGadget.Flags &= ~SELECTED; }

#define ASELECT {AmigaGadget.Flags &= ~SELECTED; \
PCGadget.Flags |= SELECTED; }

#define REFRESH { RefreshGList (&ManGadget, muxer, NULL, 4);}

/* static char SccsId[] = SCCS_STRING; */

struct IntuitionLibrary *IntuitionBase;
struct Library *GfxBase;
struct Window *muxer;
struct Gadget *AmigaGadget, *PCGadget;

/* 
    The state of things... like what disk the user wants to be 
    active and so on an so on.
*/

void PatchDisplay(void)
{
    /* Link the gadgets... */
    AmigaGadget = ((amiga_number == AMIGA_DF1) ? &DF1Gadget : &DF0Gadget);
    PCGadget = ((pc_number == PC_B) ? &PC_BGadget : &PC_AGadget);
    AutoGadget.NextGadget = &ManGadget;
    ManGadget.NextGadget = AmigaGadget;
    AmigaGadget->NextGadget = PCGadget;
    FlipperWindow.FirstGadget = &AutoGadget;
    if((xposition >= 0) && (yposition >= 0)) {
        FlipperWindow.LeftEdge = xposition;
        FlipperWindow.TopEdge = yposition;
    }
}

int OpenDisplay(void)
{
    GfxBase = OpenLibrary("graphics.library", 0);
    if(GfxBase == NULL) {
        Debug0("OpenDisplay: Open GfxBase failed\n");
        return 0;
    }
    IntuitionBase = OpenLibrary("intuition.library", 0);
    if(IntuitionBase == NULL) {
        Debug0("OpenDisplay: Open Intuition failed\n");
        CloseLibrary(GfxBase);
        return 0;
    }
    PatchDisplay();
    muxer = OpenWindow(& FlipperWindow);
    if( muxer == NULL) {
        Debug0("OpenDisplay: Open Window failed\n");
        CloseLibrary(GfxBase);
        CloseLibrary(IntuitionBase);
        return 0;
    }
    if(currentmode == MODE_AUTO) {
        OffGadget(AmigaGadget, muxer, 0);
        OffGadget(PCGadget, muxer, 0);
    }
    SetMenuStrip(muxer, &FlipperMenu);
    user_mask = ( 1 << muxer->UserPort->mp_SigBit);
    return 1;
}

void CloseDisplay(void)
{
    Debug0("Closing display stuff\n");
    ClearMenuStrip(muxer);
    if(muxer != NULL) {
        CloseWindow(muxer);
        muxer = NULL;
    }
    if(IntuitionBase != NULL) {
        CloseLibrary(IntuitionBase);
        IntuitionBase = NULL;
    }
    if(GfxBase != NULL) {
        CloseLibrary(GfxBase);
        GfxBase = NULL;
    }
}

void UpdateDisplay(mode, owner)
int mode, owner;
{
    if(mode & MODE_AUTO) {
        OffGadget(AmigaGadget, muxer, 0);
        OffGadget(PCGadget, muxer, 0);
        AUTOSELECT;
    }
    if(mode & MODE_MANUAL) {
        OnGadget(AmigaGadget, muxer, 0);
        OnGadget(PCGadget, muxer, 0);
        MANSELECT;
    }
    if(owner == SELECT_PC) {
        AmigaGadget->Flags &= ~SELECTED;
        PCGadget->Flags |= SELECTED;
    }
    if(owner == SELECT_AMIGA) {
        AmigaGadget->Flags |= SELECTED;
        PCGadget->Flags &= ~SELECTED;
    }
    RefreshGList (&AutoGadget, muxer, NULL, 4);
}
int HandleUser(void)
{
    struct IntuiMessage *newmessage;
    int Class, Code, Qualifier;
    APTR Item;
    struct Gadget *g;

    if(!muxer)
        return 0;
    while( ( newmessage = (struct IntuiMessage *) \
        GetMsg(muxer->UserPort)) != NULL) {
        Class = newmessage->Class;
        Code = newmessage->Code;
        Item = newmessage->IAddress;
        Qualifier = newmessage->Qualifier;
        g = (struct Gadget *) newmessage->IAddress;
        ReplyMsg((struct Message *) newmessage);

        switch(Class) {
            int newmode;
            case GADGETUP:
                if (g == &AutoGadget) {
                    newmode = MODE_AUTO;
                }
                else if (g == &ManGadget) {
                    newmode = MODE_MANUAL;
                }
                else if (g == AmigaGadget) {
                    newmode = (MODE_MANUAL | SELECT_AMIGA);
                }
                else if (g == PCGadget) {
                    newmode = (MODE_MANUAL | SELECT_PC);
                }
                return newmode;
                break;
            case CLOSEWINDOW:
                Debug0("supposed to close window!\n");
                going = 0;
                break;
            default:
                break;
        }
        return 0;
    }
}

void Update(void)
{
/*
    Debug2("Mode: %s select %s", modes[currentmode], disks[currentselect]);
    Debug2(" disk %s owner %s\n", disks[currentdisk], disks[owner]);
*/
    if(!muxer)
        return;

    if( currentmode == MODE_AUTO) {
        AutoGadget.Flags |= SELECTED;
	ManGadget.Flags &= ~SELECTED;
    } else {
        ManGadget.Flags |= SELECTED;
	AutoGadget.Flags &= ~SELECTED;
    }
    if( owner == SELECT_PC) {
        PCGadget->Flags |= SELECTED;
	AmigaGadget->Flags &= ~SELECTED;
    }
    else {
        AmigaGadget->Flags |= SELECTED;
	PCGadget->Flags &= ~SELECTED;
    }
    RefreshGList (&AutoGadget, muxer, NULL, 4);
}
                            
