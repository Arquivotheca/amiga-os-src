
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
#include <clib/battclock_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include "/playerprefs_private_pragmas.h"
#include <pragmas/battclock_pragmas.h>


/********      System Definitions      ********/

extern struct GfxBase         *GfxBase;


/********      External Functions      ********/

extern void __regargs NegState(int s, WORD val);


/********       Global Variables       ********/

ULONG Vir;



/******************************************************************************
 *                                                                            *
 * The state of the language is requester is as follows:                      *
 *                                                                            *
 *                      bxxx xrrr llll llll                                   *
 *                                                                            *
 * b         is the active bit (when b is set, the gadget is highlighted).    *
 * rrr       is a number from 0 to 6, zero being the non animation state.     *
 * llll llll is the language value.                                           *
 *                                                                            *
 * MAX_PREFSLANG contains the languages for the requester.                    *
 *                                                                            *
 ******************************************************************************/

static BYTE gdata[] = {

    87,8,0, 0,65,8, 95,9,73, -1,-1,-1, 13,0,14, 34,0,14, 55,0,14,
    104,6,0, 15,71,6, 110,6,76, 0,4,10, 17,0,14, 38,0,14, 59,0,14,
    116,3,0, 10,76,3, 119,3,79, 0,2,12, 19,0,14, 40,0,14, 61,0,14,
    -1,-1,-1, 4,82,0, -1,-1,-1, 1,0,14, 22,0,14, 43,0,14, 64,0,14,
    -1,-1,-1, 0,82,0, -1,-1,-1, 4,0,14, 25,0,14, 46,0,14, 67,0,14,
    122,2,0, 0,80,2, -1,-1,-1, 7,0,14, 28,0,14, 49,0,14, 70,0,12,
    124,5,0, 0,77,5, -1,-1,-1, 10,0,14, 31,0,14, 52,0,14, 73,0,9
    };



/******************************************************************************
 *                                                                            *
 * CDTVLangGad - is a remap table to convert over to my numberin system       *
 *                                                                            *
 *              WARNING - THIS IS MOSTLIKLY THE WRONG ORDER!!!!!              *
 *                                I NEED THE REAL LIST!!!!!                   *
 *                                                                            *
 ******************************************************************************/

UBYTE CDTVLangGAD[] = {

    CDTVLANG_AMERICAN,      CDTVLANG_DANISH,        CDTVLANG_GERMAN,        
    CDTVLANG_ENGLISH,       CDTVLANG_SPANISH,       CDTVLANG_FRENCH,
    CDTVLANG_ITALIAN,       CDTVLANG_DUTCH,         CDTVLANG_NORWEGIAN,
    CDTVLANG_PORTUGUESE,    CDTVLANG_SWEDISH,       CDTVLANG_FINNISH,
    CDTVLANG_KOREAN,        CDTVLANG_CHINESE,       CDTVLANG_JAPANESE
    };



/****************************************** Select a Language ******************************************/

void dolang() {

UWORD event;
BYTE key = 1;
BYTE dir = 0;
BYTE upkey = 0;
UBYTE lang;
int i = 0 ;
ULONG vir = 0;
long countup;
        
    lang = MasterF.gstate[PEGAD_LANG];
    countup = intdata.cintd_VCountUp + 240;

    while ((i != 0) || key) {
 
        if (key) {

            event = GetEvent (&input_data);

            if (event) countup = intdata.cintd_VCountUp + 240;
            else if ((intdata.cintd_VCountUp > countup) && upkey) key = 0;

            if (event == (RAWKEY_RETURN | IECODE_UP_PREFIX)) {

                if (upkey) key = 0;
                else       upkey = 1;
                }

            event &= 0xffff;

            switch(event) {

                case (IECODE_LBUTTON|IECODE_UP_PREFIX): 
                     if (upkey) key = 0;
                     else upkey = 1;
                     break;

                case RAWKEY_ARROWUP:
                case DIR_UP:            upkey = 1; dir = 1;             break;

                case RAWKEY_ARROWDN:
                case DIR_DOWN:          upkey = 1; dir = -1;            break;

                case RAWKEY_ARROWLT:
                case DIR_LEFT:          upkey = 1; vir = (vir+vir) | 1; break;

                case RAWKEY_ARROWRT:
                case DIR_RIGHT:         upkey = 1; vir += vir;          break;                                                    
                }

            if ((i == 0) && !intdata.cintd_VCountDown && dir ) {

                intdata.cintd_VCountDown = 40;

                if (dir == -1) {

                    i = 6; 
                    if (++lang >= MAX_PREFSLANG) lang = 0;
                    }

                else i = 1;
                }
            }

        MasterF.gstate[PEGAD_LANG] = ((i<<8)+lang) | 0x8000;

        if (i) {

            if (dir == 1) {

                if (i == 6) {

                    i = 0;
                    if (lang-- <= 0) lang = (MAX_PREFSLANG-1);
                    }

                else i++;
                }

            else i--;

            if (!i) dir = 0;
            }

        UpdateDisplay();
        WaitTOF();
        }

    Vir = vir;
    NegState(PEGAD_LANG, -1);
    MasterF.gstate[PEGAD_LANG] = lang;
    CDTVPrefs.Language = GADtoLanguage();
    UpdateDisplay();
    WaitTOF();
    }




/************************************** Return Selected Language ***************************************/

GADtoLanguage() {

    return((int)CDTVLangGAD[(UBYTE)MasterF.gstate[PEGAD_LANG]]);
    }



/************************************* Return Language Gadget ID# **************************************/

LanguagetoGAD() {

register int u,i;

    u = CDTVPrefs.Language;

    if (u == CDTVLANG_UNKNOWN) u = (GfxBase->DisplayFlags & PAL ? CDTVLANG_DEFAULT_PAL:CDTVLANG_DEFAULT_NTSC);

    for(i=0; i<MAX_PREFSLANG; i++) if (CDTVLangGAD[i] == u) return(i);

    return(1);
    }



/*======================================= Mod Value to Gadget Index ===================================*/

static langno(register int no) {

    while (no < 0) no += MAX_PREFSLANG;
    return((no % MAX_PREFSLANG));
    }



/******************** Return Y Position of Selected Language from prefsgadgets.pic *********************/

static langoffy(register int no) {

register int y;

    no = langno(no); 

    if (no < 8) y = 106;

    else {

        no -= 8;
        y = 130;
        }

    return((int)(y + (no * 22)));
    }



/******************** Return X Position of Selected Language from prefsgadgets.pic *********************/

static langoffx(register int no) {

    return((langno(no) >= 8 ? 262:0));
    }





/*************************************** Activate Language Gadget **************************************/

int GADLanguage(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

register BYTE *dptr;
struct BitMap sbm;
extern ULONG Vir;
int i;
int lang,lc;

    old = new & 0x8000;
    new &= 0x7fff;
    lang = new & 0xff;
    new  = (new>>8)%7;
    dptr = &gdata[new*21];
    
    for(i=0; i<3; i++,dptr+=3) {

        if (*dptr == -1) continue;
        BltBitMap(bbm, 260, *dptr, rp->BitMap, 26, 149+dptr[2], 92, dptr[1], 0xc0, 0xff, NULL);
        }

    CopyMem(bbm, &sbm, sizeof(struct BitMap));

    if (Vir == 381677295) sbm.Planes[0] = sbm.Planes[1] = sbm.Planes[3];
        
    if (Vir == 155193616) sbm.Planes[0] = sbm.Planes[1] = sbm.Planes[2];

    for(i=0,lc=lang-2; i<4; i++,dptr+=3,lc++) {

        if (*dptr == -1) continue;
        BltBitMap(&sbm, langoffx(lc), langoffy(lc)+dptr[1], rp->BitMap, 26+2, 149+*dptr, 90, dptr[2], 0xc0, 0x03, NULL);
        }

    if (!new) {

        BltBitMap(&sbm, langoffx(lang-2), (langoffy(lang-2)+15), rp->BitMap, (26+2), (149+0), 90, 6, 0xc0, 0x03, NULL);

        BltBitMap(&sbm, langoffx(lang+2), (langoffy(lang+2)+15), rp->BitMap, (26+2), (149+76), 90, 6, 0xc0, 0x03, NULL);

        sbm.Planes[2] = sbm.Planes[0];

        BltBitMap(&sbm, langoffx(lang), langoffy(lang), rp->BitMap, (26+2), (149+34), 90, 14, 0xc0, 0x04, NULL);
        }

    g->BoxFunc(rp, g, (MasterF.BoxNo == PEGAD_LANG ? 1:0), new);

    if (old) {                          /* High bit set draw box around the thing */

        SetAPen(rp, 13);
        SetDrMd(rp, JAM1);

        Move(rp, 26,  182);
        Draw(rp, 118, 182);

        Draw(rp, 118, 196);
        Draw(rp, 26,  196);
        Draw(rp, 26,  182);
        }

    return(0);
    }


