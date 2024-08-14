
#include <devices/inputevent.h>
#include <graphics/gfxbase.h>

#include <cdtv/cdtvprefs.h>

#include "/playerprefs.h"
#include "/playerprefsbase.h"

#include "/cdtvkeys.h"
#include "/basicio/keyclick.h"

#include <cdtv:include/cdtv/cdtv.h>

#include <resources/battclock.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <cdtv/debox_protos.h>
#include <cdtv/battclock_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include "/playerprefs_private_pragmas.h"



/********          Defines             ********/

#define PREFSTATE_BUTTONDOWN    (1<<0)
#define TIMEOUTSECS 60                              /* Number of seconds before bailout */


/********      System Definitions      ********/

extern void                   *SysBase;
extern struct GfxBase         *GfxBase;
extern struct DeBoxBase       *DeBoxBase;
extern struct PlayerPrefsBase *PlayerPrefsBase;


/********     External Definitions     ********/

extern struct CardStatus      CardStatus;
extern struct CDTVPrefs       CDTVPrefs;
extern struct TimeClock       CDTVTime;
extern struct CycleIntInfo    intdata;
extern struct CDTVInputInfo   input_data;
extern struct GadDir          PrefsGADList[];
extern struct DisplayFrame   *CurrentF, *WorkF, MasterF;
extern UBYTE  __far           prefsbuttons_cr[];
extern UBYTE  __far           prefsdata_cr[];
extern UBYTE  __far           prefsstrip[];
extern WORD                   FramesPerSec;
extern WORD                   ViewCenterX, ViewCenterY;


/********      External Functions      ********/

extern VOID __stdargs EndCDTVScreen(VOID);
extern VOID           InitVarBase(register struct PlayerPrefsBase *ppb);
extern                StartPPScreen(struct GadDir *gl, struct BuildBG_Info *bg, UBYTE *buttondata, UBYTE initfade);
extern ULONG __asm    GetEvent(register __a1 struct InputData *id);


/********       Global Variables       ********/

BYTE  *ViewGState;
WORD   ClockMod;
UBYTE  prefnumenter;
UBYTE  savertimes[] = { 0,1,5,10,30 };

static struct MsgPort      *cdreply;
static struct IOStdReq     *cdio;

/********       Global Structures      ********/

struct BuildBg_Info __far PrefsBg = {

    NULL,prefsstrip, NULL,prefsdata_cr,
    352,296,6,
    (9+7),59,
    NULL
    };



/**************************************** Turn on Numeric Keypad ***************************************/

void EnableNumericKeypad() {

    if (cdreply = (struct MsgPort *)CreatePort(0, 0)) {

        if (cdio = (struct IOStdReq *)CreateStdIO(cdreply)) {

            if (!OpenDevice("cdtv.device", 0, cdio, 0)) {

                cdio->io_Message.mn_ReplyPort = cdreply;

                cdio->io_Command = CDTV_FRONTPANEL;
                cdio->io_Data    = NULL;
                cdio->io_Offset  = 0;
                cdio->io_Length  = CDTV_PANEL_ENABLED | CDTV_PANEL_PLAY_EJECT;
                SafeDoIO(cdio);

                CloseDevice(cdio);
                }

            DeleteStdIO(cdio);
            }

        DeletePort(cdreply);
        }
    }



/********************************* Wait n/60ths or n/50ths of a second *********************************/

void DoDelay(int len,struct PlayerPrefsBase *PlayerPrefsBase) {

    while(len--) WaitTOF();
    }



/********************************************** Set State **********************************************/

void __regargs NegState( int s, WORD val ) {

    WorkF->gstate[s] = CurrentF->gstate[s] = val;
    }



/*************************** Get Timeout Value Associated With Gadget Number ***************************/

GADtoSaver(int no) {

    return((int)savertimes[(no%5)]);
    }



/*************************** Get Gadget Number Associated With Timeout Value ***************************/

SavertoGAD(register int no) {

register int i;

    for(i=4;i>=0;i--) if (no >= savertimes[i]) return(i);

    return(0);
    }



/************************************* Interpret Preferences Event *************************************/

doprefskey (ULONG sevent) {

UWORD                   event;
int                     temp;
static UWORD            cardkey;
register struct GadDir *g;

    event = sevent;

    g = &MasterF.GadList[(MasterF.BoxNo < 0 ? PEGAD_NL:MasterF.BoxNo)];

    if (event & BOXMOVE) KeyClickCommand(CLKCMD_CLICK,0,0,0,0,0);

    switch(event) {

        case RAWKEY_ESC:
            EnterTimeClock(0);
            UpdateDisplay();
            return(RAWKEY_ESC);

        case IECODE_RBUTTON:
        case RAWKEY_ENTER:
            EnterTimeClock(0);
            UpdateDisplay();
            return(RAWKEY_ENTER);

        case RAWKEY_ARROWRT:    event = DIR_RIGHT;      break;
        case RAWKEY_ARROWLT:    event = DIR_LEFT;       break;                    
        case RAWKEY_ARROWUP:    event = DIR_UP;         break; 
        case RAWKEY_ARROWDN:    event = DIR_DOWN;       break;
        case RAWKEY_RETURN:     event = IECODE_LBUTTON; break;

        case (RAWKEY_RETURN | IECODE_UP_PREFIX):    event = (IECODE_LBUTTON | IECODE_UP_PREFIX);    break;
        }

    if (event == IECODE_LBUTTON) { 

        input_data.CurrButtonPos |= CURRBUTTONA;
        EnterTimeClock(0); 

        switch(g->routine) {

            case 2:
                ClockMod = 1;
                temp = MasterF.gstate[ MasterF.BoxNo ] + 1;
                temp = temp % 3;
                MasterF.gstate[ MasterF.BoxNo ] = temp;
                if ((temp == 2) && (CDTVTime.hour != 12))
                        CDTVTime.hour = CDTVTime.hour % 12;
                EnterTimeClock(0);
                break;

            case 3:
                AdjScreen();
                input_data.CurrButtonPos &= ~(CURRBUTTONA);
                break;

            case 4: 
                switch(MasterF.BoxNo) {

                    case PEGAD_LACE:
                            CDTVPrefs.Flags ^= CDTVPF_LACE;
                            NegState( PEGAD_SCREEN, 0x7F7F );
                            break;

                    case PEGAD_CLICK:
                            CDTVPrefs.Flags ^= CDTVPF_KEYCLICK;break;
                    }

                PrefsToMasterF( &CDTVPrefs, &CDTVTime );
                ConfigureCDTV( &CDTVPrefs );
                break;

            case 5:
                CDTVPrefs.SaverTime = GADtoSaver(MasterF.gstate[PEGAD_SCRSAVE]+1 );
                PrefsToMasterF( &CDTVPrefs, &CDTVTime );
                MasterF.gstate[PEGAD_SCRSAVE] |= 0x8000;
                UpdateDisplay();
                DoDelay(5,PlayerPrefsBase);
                MasterF.gstate[PEGAD_SCRSAVE] &= ~(0x8000);
                UpdateDisplay();
                break;

            case 8:
                dolang();
                input_data.CurrButtonPos &= ~(CURRBUTTONA);
                return(0);

            case 9:
                cardkey = MasterF.BoxNo;
                MasterF.gstate[MasterF.BoxNo] = 1;
                UpdateDisplay();
                docaution();
                break;

            case 10:
                MasterF.gstate[MasterF.BoxNo] = 1;
                UpdateDisplay();
                docard(cardkey);
                break;
            }
        }

    if (event == (IECODE_LBUTTON | IECODE_UP_PREFIX)) {

        input_data.CurrButtonPos &= ~(CURRBUTTONA);
        if (g->routine == 1) EnterTimeClock(0);
        }

    if (((temp = IsNumKey(event)) != -1) && (g->routine == 1)) {

        ClockMod = 1;

        if (prefnumenter) {

            temp += MasterF.gstate[ MasterF.BoxNo ] * 10;
            temp = temp % g->MaxStates;
            while (temp > 3000) temp-=1000; 
            }

        else prefnumenter = 1;

        MasterF.gstate[ MasterF.BoxNo ] = temp;
        }
        
    if (event & BOXMOVE) {

        if (prefnumenter) EnterTimeClock(0);

        if ((input_data.CurrButtonPos & CURRBUTTONA) && (g->routine == 1)) {

            ClockMod = 1;
            if (event == DIR_UP)    MasterF.gstate[ MasterF.BoxNo ]++;
            if (event == DIR_DOWN)  MasterF.gstate[ MasterF.BoxNo ]--;
            EnterTimeClock(1);
            }

        else {
            EnterTimeClock(0);
            BoxMove( &MasterF,event );
            }
        }
                    
    return(0);
    }



/****** playerprefs.library/DoPrefs ****************************************
*
*   NAME   
*       DoPrefs -- Bring up CDTV Preferences editor screen.
*
*   SYNOPSIS
*       modified = DoPrefs ();
*          D0
*
*       LONG DoPrefs (void);
*
*   FUNCTION
*       Displays and allows the user to edit the CDTVPrefs Bookmark.
*
*   RESULT
*       modified - 0: user edited and saved preferences
*                  1: edited but not saved
*                 -1: unable to open pref screen
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
*/


LONG __asm DoPrefs(register __a6 struct PlayerPrefsBase *ppb) {

ULONG event;
LONG  ans = -1;
LONG  timeout;
int   i;

    InitVarBase(ppb);
    InstallKeyClick();
    InstallJoyMouse();
    KeyClickCommand(CLKCMD_SETREPEAT,0,0,0,0,0);
    InstallCard();
    EnableNumericKeypad();

    timeout = TIMEOUTSECS * FramesPerSec;

    if (!StartPPScreen(PrefsGADList, &PrefsBg, prefsbuttons_cr, 15)) {

        if (GfxBase->DisplayFlags & PAL) MakeAltDate();                             /* Switch to European time mode */
    
        ViewGState = (BYTE *)&MasterF.gstate[ PEGAD_SCREEN ];
        ViewGState[0] = CDTVPrefs.DisplayX - ViewCenterX;
        ViewGState[1] = CDTVPrefs.DisplayY - ViewCenterY;
    
        for (i=0; i<MAX_PEGAD; i++) WorkF->gstate[i] = CurrentF->gstate[i] = -1;
        
        NegState( PEGAD_SCREEN, 0x7f7f );
    
        input_data.MouseTDist= 100;
    
        PrefsToMasterF(&CDTVPrefs, &CDTVTime);
        MasterF.gstate[PEGAD_LANG] = LanguagetoGAD();
        
        MasterF.BoxNo = PEGAD_SCREEN;
        UpdateDisplay();

        FadeTo(0);

        ClearEvent(&input_data);
        event = 0;
    
        while(!event) {

        if (CardStatus.CStatus == -1) UpdateCardStatus();

            while (event = GetEvent(&input_data)) { 
                if (event = doprefskey (event)) break; 
                intdata.cintd_VCountUp = 0;
                } 
            
            if (intdata.cintd_VCountUp > timeout) break;                            /* If the timeout value is reached then just break out of loop */

            UpdateDisplay();
            }
    
        intdata.cintd_DestFade0 = 15;
    
        if (event == RAWKEY_ENTER) {
            SaveCDTVPrefs(&CDTVPrefs);    
            if (ClockMod) SaveCDTVTime(&CDTVTime, &CDTVPrefs);
            }

        FadeTo(15);
        ans = 0;
        }    

    RemoveCard();
    RemoveJoyMouse();
    KeyClickCommand(CLKCMD_DIE,0,0,0,0,0);
    EndCDTVScreen();
    return(ans);
    }


