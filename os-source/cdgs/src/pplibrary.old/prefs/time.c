
#include <devices/inputevent.h>
#include <graphics/gfxbase.h>

#include <cdtv/cdtvprefs.h>

#include "/playerprefs.h"
#include "/playerprefsbase.h"

#include "/cdtvkeys.h"
#include "/basicio/keyclick.h"

#include <resources/battclock.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <cdtv/debox_protos.h>
#include <cdtv/battclock_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include "/playerprefs_private_pragmas.h"
#include <pragmas/battclock_pragmas.h>


/********      System Definitions      ********/

extern struct GfxBase         *GfxBase;


/********     External Definitions     ********/

extern struct TimeClock     CDTVTime;
extern UBYTE                prefnumenter;
extern char __far           BattClock_Name[];
extern struct BitMap       *bbm;
extern struct GadDir        PrefsGADList[];
extern struct GadDir        altdate[];




/********      Global Definitions      ********/

static UBYTE monthdays[2][13] = {

    {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
    };



/*============================== Get Number of Days in Specified Month ================================*/

static MaxMonthDays(int month, int year) {

    return( (int) monthdays[IsLeap(year)][month] );
    }



/************************************** Make sure Time is Valid ****************************************/

VOID MaxMinTime(struct TimeClock *time, int wrap) {

static WORD    vals[] = {1979, 2043, 1, 12, 1, 31, 0, 23, 0, 59, -1};
register WORD *w, mm;
register WORD *tptr;
        
    tptr = (WORD *)time;
    w = vals;
    
    while(*w != -1) {
        mm = *tptr;

        if (wrap) {

            if (mm < *w)    *tptr = (w[1]+1) - (*w - mm);
            if (mm > w[1])  *tptr = (*w-1) + (mm - w[1]);
            }

        else {

            if (mm < *w)    *tptr = *w;
            if (mm > w[1])  *tptr = w[1];
            }

        tptr++;
        w += 2;
        }

    if (time->day > (mm = MaxMonthDays(time->month,time->year))) time->day = mm;
    }





/******************************* Store Enterred Time into Permanent Field ******************************/

void EnterTimeClock(int wrap) {

register UWORD no, temp;
        
    prefnumenter = 0;

    if (MasterF.BoxNo < 0) return;

    no = MasterF.gstate[MasterF.BoxNo];

    switch(MasterF.BoxNo) {

        case PEGAD_AM:
            if (no != 2) {

                CDTVPrefs.Flags |= CDTVPF_AMPM;
                CDTVTime.hour = (CDTVTime.hour % 12) + (no ? 12:0);
                }

            else CDTVPrefs.Flags &= ~CDTVPF_AMPM;

            break;

        case PEGAD_HOUR:
            if (CDTVPrefs.Flags & CDTVPF_AMPM) no = no % 12;

            temp = no;

            if (CDTVPrefs.Flags & CDTVPF_AMPM) {

                if ((no > 0) && (no <= 12)) {

                    temp = (CDTVTime.hour >= 12 ? 12:0);
                    if (no == 12) no = 0;
                    temp += no;
                    }
                }

            CDTVTime.hour = temp;
            break;

        case PEGAD_MIN:     CDTVTime.min   = no; break;
        case PEGAD_MONTH:   CDTVTime.month = no; break;
        case PEGAD_DAY:     CDTVTime.day   = no; break;
        case PEGAD_YEAR:    CDTVTime.year  = no; break;

        default: return;
        }

    if (wrap) MaxMinTime(&CDTVTime, 1);

    PrefsToMasterF(&CDTVPrefs, &CDTVTime);
    }




/************************************** Is This Year a Leap Year? **************************************/

IsLeap(int year) {

    return ((int)((year%4 == 0 && year % 100 != 0) || year % 400 == 0));
    }





/********************************** Convert Date to Seconds from 1978 **********************************/

ULONG GetRawTime(struct TimeClock *tc) {

ULONG         sec;
register int  d,m,y;
UBYTE        *l;

    y = tc->year - 1978;

    d = y * 365 + (y+2)/4;
    if (((m = tc->month) < 3) && IsLeap(tc->year)) d--;
    l = &monthdays[0][1];
    while(--m > 0) d += *l++;

    d+=tc->day-1;

    sec=d*SECS_DAY;
    sec+=tc->hour*(60*60);
    sec+=tc->min*60;

    return(sec);
    }



/****************************** Store Date/Time In Battery Backed-Up Clock *****************************/

SaveCDTVTime(register struct TimeClock *tc, register struct CDTVPrefs *c) {

register struct Library *BattClockBase;
ULONG                    temp;

    if (!(BattClockBase = OpenResource(BattClock_Name))) return(1);

    temp=GetRawTime(tc);

    WriteBattClock(temp);                                                                       /* Attempt to set the batt clock mode */
    if (BattClockBase->lib_Version >= 38) SetBattClockMode((c->Flags & CDTVPF_AMPM ? 1:0));

    ReadBattClock();

    return(0);
    }



/********************************** Convert Date to European Format ************************************/

VOID MakeAltDate() {       

    BltBitMap(bbm, 2,51, WorkF->bm,    23, 111, 45, 17, 0xc0, 0xff, NULL); 
    BltBitMap(bbm, 2,51, CurrentF->bm, 23, 111, 45, 17, 0xc0, 0xff, NULL); 

    CopyMem(altdate, &PrefsGADList[PEGAD_MONTH], (sizeof(struct GadDir)*2));

    PrefsGADList[PEGAD_YEAR].Left = PEGAD_MONTH;
    PrefsGADList[PEGAD_HOUR].Down = PEGAD_DAY;
    PrefsGADList[PEGAD_AM].Right  = PEGAD_DAY;
    }       



/*************************************************************************
 *                                                                       *
 * PrefsToMasterF - copy prefs into the MasterF structure.               *
 *                                                                       *                                                                       *
 * NOTE: this will modifiy prefs to fit the max/min values               *
 *                                                                       *                                                                        *
 *************************************************************************/

PrefsToMasterF(register struct CDTVPrefs *cprefs, register struct TimeClock *time) {

register int i;
        
    MaxMinTime(time, 0);

    if (cprefs->Flags & CDTVPF_AMPM) {

        if ((i = time->hour) >= 12 ) {

            MasterF.gstate[PEGAD_AM] = 1;
            if (i != 12) i -= 12;
            }

        else {

            if (!time->hour) i = 12;
            MasterF.gstate[PEGAD_AM] = 0;
            }
        }

    else {

        MasterF.gstate[PEGAD_AM] = 2;
        i = time->hour;
        }

    MasterF.gstate[ PEGAD_HOUR ]    = i;
    MasterF.gstate[ PEGAD_MIN  ]    = time->min;
    MasterF.gstate[ PEGAD_DAY  ]    = time->day;
    MasterF.gstate[ PEGAD_MONTH]    = time->month;
    MasterF.gstate[ PEGAD_YEAR ]    = time->year;
    MasterF.gstate[ PEGAD_LACE ]    = (cprefs->Flags & CDTVPF_LACE ? 1:0);
    MasterF.gstate[ PEGAD_CLICK  ]  = (cprefs->Flags & CDTVPF_KEYCLICK ? 0:1);
    MasterF.gstate[ PEGAD_SCRSAVE ] = SavertoGAD(cprefs->SaverTime); 

    return(0);
    }



