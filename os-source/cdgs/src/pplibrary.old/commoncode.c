
#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/libraries.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>

#include <devices/input.h>
#include <devices/inputevent.h>

#include <cdtv/debox.h>
#include "cdtvkeys.h"
#include "screensaver/screensaver.h"
#include <resources/battclock.h>

#include "playerprefs.h"
#include "playerprefsbase.h"
#include "playerprefs_pragmas.h"
#include "playerprefs_protos.h"
#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/battclock_protos.h>
#include <cdtv/debox_protos.h>

#include "playerprefs_private_pragmas.h"
#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/battclock_pragmas.h>
#include <cdtv/debox_pragmas.h>

struct GfxBase          *GfxBase;
struct DeBoxBase        *DeBoxBase;
void                    *SysBase;
struct PlayerPrefsBase  *PlayerPrefsBase;

struct TimeClock                 CDTVTime = {1991,2,28,12,00};
struct CDTVPrefs                 CDTVPrefs;
WORD                             ViewCenterX,ViewCenterY;

struct BMInfo   *bmi;
struct BMInfo   *bbmi;
struct BitMap   *bbm;
struct BitMap   *bm;
UBYTE           *mask;
struct RastPort Rp;

struct CycleIntInfo     intdata;
struct CDTVInputInfo    input_data;
UBYTE                   *bmask;

char __far TimerDeviceName[] = "timer.device";
char __far  BattClock_Name[] = BATTCLOCKNAME;

struct DisplayFrame Frame1,Frame2;
struct DisplayFrame *CurrentF,*WorkF, MasterF;

WORD FramesPerSec;
LONG MicroPerFrame;

void removehandler(struct PlayerPrefsBase *, struct IOStdReq *, struct Interrupt *);

struct IOStdReq * installhandler(struct PlayerPrefsBase *, struct Interrupt *);



struct IOStdReq *installhandler(register struct PlayerPrefsBase *PlayerPrefsBase, register struct Interrupt *ihdata) {

register struct MsgPort         *reply;
register struct IOStdReq        *io;

    if (!(reply = (struct MsgPort *)CreatePort (NULL, 0))) return (0);

    if (!(io = (struct IOStdReq *)CreateStdIO (reply))) return (0);

    if (OpenDevice ("input.device", 0L, io, 0L)) {

        io->io_Device = NULL;
        return (0);
        }

    io->io_Command  = IND_ADDHANDLER;
    io->io_Data     = (APTR)ihdata;
    if (!DoIO (io)) return (io);

    if (io) {

        if (io->io_Device) CloseDevice(io);
        DeleteStdIO (io);
        }

    if (reply) DeletePort (reply);

    return (NULL);
    }



void removehandler(register struct PlayerPrefsBase *PlayerPrefsBase,
                   register struct IOStdReq *io, struct Interrupt *ihdata) {

    if (io) {

        if (io->io_Device) {

            io->io_Command = IND_REMHANDLER;
            io->io_Data = (APTR) ihdata;
            DoIO (io);
            CloseDevice (io);
            }

        if (io->io_Message.mn_ReplyPort) DeletePort(io->io_Message.mn_ReplyPort);

        DeleteStdIO(io);
        }
    }


GAD2SRender(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

struct Render2S_Info *ri;
struct BitMap *b;

    ri = (struct Render2S_Info *)g->RenderData;
    
    ri = &ri[new];
    if (ri->boxinfo) g->BoxData = ri->boxinfo;      

    b = (ri->scr ? bbm:bm );

    BltBitMapRastPort(b, ri->x, ri->y, rp, g->LeftEdge, g->TopEdge, g->RightEdge-g->LeftEdge, g->BottomEdge-g->TopEdge, 0xc0);

    return(g->Flags & GDFLAGS_BOXOVER);
    }
    
    
GADSTDBox(struct RastPort *rp, struct GadDir *g, int val, UWORD state) {

extern struct BitMap        *bm;
struct BitMap               *dispbm;
register struct GADSTDBox   *gb;
register struct GADImageLoc *gl;

int x,y;

    gb = (struct GADSTDBox *)g->BoxData;
    gl = (val ? &gb->oni:&gb->offi);
    
    x = g->LeftEdge + gb->xoffset;
    y = g->TopEdge + gb->yoffset;
    dispbm = (gl->map ? MasterF.bbm:bm);

    if (gl->mask < 0) BltBitMap(dispbm, gl->xloc, gl->yloc, rp->BitMap, x, y, gb->width, gb->height, 0xc0, 0xff, NULL);

    else BltMaskBitMapRastPort(dispbm, gl->xloc, gl->yloc, rp, x, y, gb->width, gb->height, (ABC|ABNC|ANBC), (gl->mask ? bmask:mask));

    return( 0 );
    }



GADInsDraw(struct RastPort *rp, struct GadDir *g, int val, UWORD state) {

static   BYTE  inslen[INS_MAX+1] = { 1, 3, 3, 3, 1, 1, 1, 1 };
register BYTE *ptr;
register int   x,y;

    x = g->LeftEdge; y = g->TopEdge;

    ptr = (UBYTE *)g->BoxData;
    
    SetDrMd(rp, JAM1);

    while(*ptr != INS_END) {

        switch (*ptr) {

            case INS_PEN:   SetAPen(rp, (long)(val ? ptr[2]:ptr[1]));   break;

            case INS_MOVE:  Move(rp, x+ptr[1],y+ptr[2]);                break;

            case INS_DRAW:  Draw(rp, x+ptr[1],y+ptr[2]);                break;

            case INS_END:                                               return(0);

            case INS_ON_REDRAW:

                 if (val) {

                    g->RenderFunc(rp, MasterF.bbm, g, state, state);
                    return(0);
                    }

                 else break;

            case INS_OFF_REDRAW:

                 if (!val) {

                    g->RenderFunc(rp, MasterF.bbm, g, state, state);
                    return(0);
                    }

                 else break;

            case INS_ONSKIP:
            case INS_OFFSKIP:                                           break;
            }

        ptr += (*ptr < INS_MAX ? inslen[*ptr]:1);
        }

    return(0);
    }
 


GADScrSave(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

int x;

    x = 91 + (new & 0x000f) * 22;

    BltBitMapRastPort(bm, 305, 68,  rp, 305, 68, 27, 57, 0xc0);
    BltBitMapRastPort(bbm,  x, 139, rp, 307, 71, 20, 48, 0xc0);

    x = (new & 0x8000 ? 201:230);                                                           /* down:up */

    BltMaskBitMapRastPort(bbm, x, 132, rp, 305, 68, 27, 57, (ABC|ABNC|ANBC), bmask);
    return(0x80);                                                                           /* no need to rewrite the box */
    }



__regargs PowerTen(register UBYTE dig) {

static WORD dignumarr[] = { 0, 1, 10, 100, 1000, 10000 };

    return((int)dignumarr[dig]);
    }


static __regargs getdigit(register struct GADNumberInfo *gi, register UWORD data, WORD digit, int last) {

register int dignum,dig;
        
    if (data == -1) return(-1);

    if ((gi->flags & GNIF_HALF) && (data & GNIF_HALF) ) dignum = 10; 
    else                                                dignum = 0;

    data &= 0xfff;
    dig = (data % PowerTen(digit+1))/PowerTen(digit);

    if (((gi->flags & GNIF_BLANKLEAD) && (last == gi->offnum) && (dig==0))
        || ((gi->flags & GNIF_BLANKZERO) && !data)
        || ((gi->flags & GNIF_BLANKMAX) && (data >= PowerTen(gi->ndigits+1)))) return((int)gi->offnum);

    else return((dignum + dig));
    }


GADNumber(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

register struct GADNumberInfo *gi;
int                            i;
WORD                           o1, n1;
int                            xloc, w, x, y, offx;

    gi = (struct GADNumberInfo *)g->RenderData;
    
    i    = gi->ndigits;
    n1   =
    o1   = gi->offnum;
    offx = g->LeftEdge+gi->xoffset-(gi->flags & GNIF_LEADONE ? gi->nwidth:0);
    y    = g->TopEdge + gi->yoffset;

    while(i) {

        o1 = getdigit( gi, old, i, o1 );
        n1 = getdigit( gi, new, i, n1 );

        if (o1 != n1) {

            if (n1 < 0) continue;
            if ((i == gi->ndigits) && (gi->flags & GNIF_LEADONE)) {

                xloc = (n1==1 ? gi->oneloc:gi->blankoneloc);
                w    = gi->onewidth;
                x    = g->LeftEdge + gi->oneoffset;
                }

            else {

                xloc = gi->xloc + (n1 * gi->width);
                w    = gi->width;
                x    = offx;
                }

            BltBitMap(bbm, xloc, gi->yloc, rp->BitMap, x, y, w, gi->height, 0xc0, 0xff, NULL);
            }

        offx += gi->nwidth;
        i--;
        }

    return(g->Flags & GDFLAGS_BOXOVER);
    }



RNormal(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) { 

register UWORD *d;

    d = &((UWORD *)g->RenderData)[new+new];

    BltBitMapRastPort(bbm, *d, d[1], rp, g->LeftEdge, g->TopEdge, g->RightEdge-g->LeftEdge, g->BottomEdge-g->TopEdge, 0xc0);

    return(g->Flags & GDFLAGS_BOXOVER);
    }



/*----------------------- ANIMATION ----------------------*/

UWORD *DoAnimation(UWORD *ins, UWORD *org) {

    if (*ins == 0xffff) return(NULL);

    while(intdata.cintd_VCountDown) WaitTOF(); 

    intdata.cintd_VCountDown = *ins++;

    while (*ins != 0xffff) {

        if ((ins[1] == 0xffff) && org) MasterF.gstate[*ins] = org[*ins];
        else                           MasterF.gstate[ *ins ] = ins[1];

        ins += 2;
        }

    UpdateDisplay();
    ins++;
    return(ins);
    }



/************************************************************************
* UpdateDisplay()                                                                                                               *
*                                                                                                                                               *
* This routine is responsible for all the updating of the display.  The *
* goal of the routine is to get 'df' looking exactly like 'md' in as    *
* few moves as possible                                                                                                 *
**                                                                                                                                              *
* However, we also have to consider if the Master state is different    *
* from the Current state, even if it isn't different from the Work              *
* state.                                                                                                                                *
************************************************************************/

/*
 * Master is different from Current; therefore we *must*
 * flip the view.
 **
 * Note: A "more efficient" method of this would involve
 * checking MasterF against CurrentF; if there are any
 * differences, then a full loop would be run to synchronize
 * WorkF with MasterF.  Otherwise, no rendering to WorkF is
 * done, even though MasterF may differ from WorkF (but not
 * from CurrentF).
 */

DrawDisplay() {

register struct DisplayFrame *df, *cf;
struct GadDir *g;
UWORD *mstate,*dfstate,*cfstate;
int i;
int ans = 0;
        
    df = WorkF;
    cf = CurrentF;

    if (intdata.cintd_LoadView) while( intdata.cintd_LoadView );
    
    g = MasterF.GadList;
    mstate = MasterF.gstate;
    dfstate= df->gstate;
    cfstate= cf->gstate;

    for(i=0; i<MasterF.MaxGad; i++,g++,mstate++,dfstate++,cfstate++) {

        if (*mstate != *cfstate) ans = 1;

        if (*mstate == *dfstate) continue;                                                                   /* Master is the same as Work; so we do no work for this one. */

        if ((*mstate & 0x0fff) >= g->MaxStates) *mstate = (*mstate & 0xf000)+(*mstate % g->MaxStates);
                
        if (*mstate < 0) *mstate = g->MaxStates-1;

        if (g->RenderFunc(df->rp,MasterF.bbm, g, *dfstate, *mstate) && (i==df->BoxNo)) df->BoxNo = -1;

        *dfstate = *mstate;
        }
    
    if (MasterF.BoxNo != df->BoxNo) {

        if (df->BoxNo >= 0) {

            g = &MasterF.GadList[ df->BoxNo ];
            g->BoxFunc( df->rp, g, 0, MasterF.gstate[df->BoxNo] );
            }

        if (MasterF.BoxNo >= 0) {

            g = &MasterF.GadList[ MasterF.BoxNo ];
            g->BoxFunc( df->rp, g, 1, MasterF.gstate[MasterF.BoxNo] );
            }

        df->BoxNo = MasterF.BoxNo;
        ans = 1;
        }

    if (ans) MasterF.UpDated = 1;

    return(ans);
    }


VOID __regargs LoadDisplayFrame(register struct CycleIntInfo *ci, register struct DisplayFrame *df) {

struct View *v;

    v = df->view;

    LoadCycleView(ci, v, df->cclist);
    }


VOID __stdargs ToggleFrame(VOID) {

register struct DisplayFrame *ds;
        
    ds        = CurrentF;
    CurrentF  = WorkF;
    WorkF     = ds;
    Rp.BitMap = WorkF->bm;
    }



RefreshDisplay() {

    if (MasterF.UpDated) {

        WaitBlit();
        ToggleFrame();
        LoadDisplayFrame(&intdata, CurrentF);
        MasterF.UpDated = 0;
        return(1);
        }

    return(0);
    }



UpdateDisplay() {

    DrawDisplay();
    return(RefreshDisplay());
    }



void __regargs BoxMove(register struct DisplayFrame *mf, UWORD event) {

register struct GadDir *g;
        
    if (mf->BoxNo < 0) { mf->BoxNo = 0; return; }
    g = &mf->GadList[ mf->BoxNo ];

    switch(event) { 

        case DIR_UP:    event = g->Up;          break;
        case DIR_DOWN:  event = g->Down;        break;
        case DIR_LEFT:  event = g->Left;        break; 
        case DIR_RIGHT: event = g->Right;       break;

        default:        return;
        }

    if (mf->BoxNo != event) mf->D1 = 0;         /* box has moved */

    if (event == PLGAD_NL) return;

    mf->BoxNo = event;
    }


IsNumKey(UWORD event) {

static UBYTE numkeys[] = {

    RAWKEY_0, 0x0a, RAWKEY_1, 0x01, RAWKEY_2, 0x02, RAWKEY_3, 0x03, RAWKEY_4, 0x04,
    RAWKEY_5, 0x05, RAWKEY_6, 0x06, RAWKEY_7, 0x07, RAWKEY_8, 0x08, RAWKEY_9, 0x09
    };

register int    i;
register UBYTE *n;

    if (event & IECODE_UP_PREFIX) return(-1);

    for(i=0,n=numkeys; i<10; i++) {

        if (event == *n++) return(i);
        if (event == *n++) return(i);
        }

    return(-1);
    }



/*-----------------------------------------------------------------------*/

void FadeTo(int lv) {

    intdata.cintd_DestFade0 = lv;
    intdata.cintd_FadeMask  = 0;

    while(intdata.cintd_Fade0 != intdata.cintd_DestFade0) WaitTOF();
    }




handlescrsave(struct CycleIntInfo *ci, int blank, struct PlayerPrefsBase *PlayerPrefsBase) {

    if (blank == -1) {

        ScreenSaverCommand( SCRSAV_FAKEIT );
        ci->cintd_DestFade0 = 0;
        return(0);
        }

    if (ScreenSaverCommand(SCRSAV_ISACTIVE)) {

        if ((blank == 0) && (ci->cintd_Fade0 == 15)) {

            blank = 1;
            ScreenSaverCommand( SCRSAV_UNFAKEIT );
            }

        ci->cintd_DestFade0 = 15;
        }

    else {

        if (blank == 1) {

            blank = 0;
            ScreenSaverCommand( SCRSAV_FAKEIT );
            }

        ci->cintd_DestFade0 = 0;
        }

    return(blank);
    }



/*=========================== CLOCK FUNCTIONS ============================*/


void MakeDate(ULONG time, struct TimeClock *tc) {

long n,m,y;

    n = (time/SECS_DAY) + 671;

    if (n <= 0) {

        y = 1978 ;
        m = 1 ;
        tc->day = 1 ;
        }

    else {

        y       =  (4 * n + 3) / 1461 ;
        n       -= 1461 * y / 4 ;
        y       += 1976;
        m       =  (5 * n + 2) / 153 ;
        tc->day =  n - (153 * m + 2) / 5 + 1 ;
        m       += 3 ;

        if (m > 12) {

            y++;
            m -= 12;
            }
        }

    tc->month = m;
    tc->year  = y;

    n  = (time % SECS_DAY)/60;

    tc->min  = n % 60;
    tc->hour = n / 60;
    }



FillCDTVTime(register struct TimeClock *tc) {

struct Library *BattClockBase;
struct TimeClock tempc;
ULONG tempsec;

    if (!(BattClockBase = OpenResource( BattClock_Name ))) return(1);
        
    tempsec=ReadBattClock();
    MakeDate( tempsec,&tempc );

    CopyMem( &tempc, tc, sizeof(struct TimeClock));

    return(0);
    }




/*========================================================================--*
* InitBase() - Init base libs in local varables                             *
*                                                                                                                                                       *
*---------------------------------------------------------------------------*/


VOID InitVarBase(register struct PlayerPrefsBase *ppb) {

struct View view;

    SysBase         = ppb->ppb_ExecBase;
    GfxBase         = ppb->ppb_GfxBase;
    DeBoxBase       = ppb->ppb_DeBoxBase;
    PlayerPrefsBase = ppb;

    InitView(&view);
    ViewCenterX = view.DxOffset;
    ViewCenterY = view.DyOffset;

    FillCDTVPrefs(&CDTVPrefs);
    FillCDTVTime(&CDTVTime);

    ConfigureCDTV( &CDTVPrefs );
    
    if (GfxBase->DisplayFlags & PAL) {

        FramesPerSec  = 50;
        MicroPerFrame = (1000000/50);
        }

    else {

        FramesPerSec  = 60;
        MicroPerFrame = (1000000/60);
        }
    }





/*----------------------- INPUT HANDLER SETTUP ----------------------*/



VOID __regargs CloseIHandle(register struct CDTVInputInfo *id) {

struct CDTVEvent *ce;

    if (id->input_io) {

        removehandler(PlayerPrefsBase, id->input_io, &id->Inter);

        ClearEvent(id);

        while(ce = (struct CDTVEvent *)GetMsg(&id->returnport)) FreeMem(ce, ce->cdtve_Msg.mn_Length);       /* Free all pending messages */
        }
    }

VOID __asm InputHDR(register __a0 struct InputEvent *, register __a1 struct CDTVInputInfo *);


__regargs StartIHandle(register struct CDTVInputInfo *id) {

    MemSet(id, 0, sizeof(struct CDTVInputInfo));
    NewList(&(id->MsgPort.mp_MsgList));
    NewList(&(id->returnport.mp_MsgList));

    id->returnport.mp_Flags     = id->MsgPort.mp_Flags = PA_IGNORE;

    id->Inter.is_Node.ln_Name   = "cdtv.ihandler";
    id->Inter.is_Data           = (APTR)&input_data;
    id->Inter.is_Code           = InputHDR;
    id->Inter.is_Node.ln_Pri    = 55;

    id->MouseWait               = 150000;
    id->MouseTDist              = 25;

    if ((id->input_io = installhandler(PlayerPrefsBase, &id->Inter))) return(0);

    CloseIHandle(id);
    return(1);
    }


VOID __regargs FreeDisplayFrame(register struct DisplayFrame *df) {

    if (df->gstate) FreeMem( df->gstate, df->MaxGad << 1);
    if (df->view) DeleteView((struct SuperView *)df->view);
    if (df->bm) FreeBitMap(df->bm);

    MemSet(df, 0, sizeof(struct DisplayFrame));
    }



/*----------------------- VBLANK HANDLER SETTUP ----------------------*/

VOID __regargs CloseVBlank(register struct CycleIntInfo *ci) {

    CloseViewMgr(ci);
    }



__regargs StartVBlank(register struct CycleIntInfo *ci, struct BMInfo *bmi, struct DisplayFrame *df) {       

    StartViewMgr(ci, bmi, df->view, df->cclist, 0);
    return(0);
    }



/*----------------------- DISPLAY FRAMES ------------------------*/


VOID DisplayFramePos(struct DisplayFrame *df, struct CDTVPrefs *cp) {

register struct View *v;
        
    v = df->view;
    CenterCDTVView(cp, v, df->bmi->bmi_Width, df->bmi->bmi_Height);
    FindViewRGB(v, df->cclist, 32);
    }



FillDisplayFrame(register struct DisplayFrame *df, register struct BMInfo *bmi,
                          struct RastPort *rp, struct BitMap *orgbm, int maxgad) {

    MemSet(df, 0, sizeof( struct DisplayFrame ));

    if (bmi) {

        df->bmi = bmi;

        if (!(df->bm = AllocBitMap(bmi->bmi_Depth, bmi->bmi_Width,bmi->bmi_Height))) return(1);

        if (!(df->view = (struct View *)CreateView(NULL, df->bm, bmi->bmi_Width, bmi->bmi_Height, bmi->bmi_Modes))) {

            FreeDisplayFrame(df);
            return(1);
            }

        df->vp = df->view->ViewPort;
        df->rp = rp;

        BltBitMap(orgbm, 0, 0, df->bm, 0, 0, bmi->bmi_Width, bmi->bmi_Height, 0xc0, 0xff, NULL);
        DisplayFramePos(df, &CDTVPrefs);
        }
    
    
    if (!(df->gstate = AllocMem(maxgad << 1, MEMF_CLEAR))) {

        FreeDisplayFrame( df );
        return(1);
        }

    df->MaxGad = maxgad;
    df->BoxNo = -1;
    return(0);
    }



/*-------------------------------------------------------------------------*/


StartPPScreen(struct GadDir *gl, struct BuildBG_Info *bg, UBYTE *buttondata, UBYTE initfade) {

    if (!(StartIHandle(&input_data))) {
        
        if (BuildBg(bg, &bmi, &bm)) {

            mask = NULL;

            bmi->bmi_Modes |= EXTRA_HALFBRITE;

            if (GrabBm(NULL, buttondata, &bbmi, &bbm, &bmask)) {

                InitRastPort(&Rp);
        
                if (!(FillDisplayFrame(&Frame1, bmi, &Rp, bm, MAX_PLGAD))) {

                    if (!(FillDisplayFrame(&Frame2, bmi, &Rp, bm, MAX_PLGAD))) {

                        if (!(FillDisplayFrame(&MasterF, NULL, NULL, NULL, MAX_PLGAD))) {

                            MasterF.GadList     = gl;
                            MasterF.bbm         = bbm;
                            CurrentF            = &Frame1;
                            WorkF               = &Frame2;
                            Rp.BitMap           = WorkF->bm;
        
                            if (!(StartVBlank(&intdata, bmi, CurrentF))) {

                                intdata.cintd_DestFade0 = initfade;
                                intdata.cintd_DestFade1 = initfade;

                                return(0);
                                }
                            }
                        }
                    }
                }
            }
        }

    EndCDTVScreen();
    return(1);
    }





/*------------------------ StartCDTVScreen ------------------------*/

VOID __stdargs EndCDTVScreen(VOID) {

    CloseVBlank(&intdata);
    FreeDisplayFrame(&MasterF);
    FreeDisplayFrame(&Frame1);
    FreeDisplayFrame(&Frame2);

    if (mask)        {bm->Planes[bm->Depth++] = mask;}
    if (bm)          FreeBitMap(bm);
    if (bmi)         FreeBMInfo(bmi);
    if (bmask)       {bbm->Planes[bbm->Depth++] = bmask; }
    if (bbm)         FreeBitMap(bbm);
    if (bbmi)        FreeBMInfo(bbmi);

    mask    = bmask = NULL;
    bm      = bbm   = NULL;
    bmi     = bbmi  = NULL;

    CloseIHandle(&input_data);
    }


