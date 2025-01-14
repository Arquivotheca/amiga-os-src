

#include <exec/types.h>
#include <exec/interrupts.h>
#include <exec/memory.h>
#include <exec/libraries.h>
#include <exec/io.h>

#include <devices/timer.h>
#include <devices/input.h>
#include <devices/inputevent.h>

#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>

#include <cdtv/debox.h>
#include <cdtv/cdtv.h>
#include <cdtv/cdtvprefs.h>

#include <hardware/dmabits.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/blit.h>

#include <libraries/dos.h>

#include "/playerprefsbase.h"
#include "/playerprefs_private_pragmas.h"
#include "/playerprefs_protos.h"

#include "/basicio/viewmgr.h"
#include "/screensaver/screensaver.h"

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <cdtv/debox_protos.h>

#include <pragmas/exec_old_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>

#define TMPIC_SIZE      16008
#define TMDATA_SIZE     1140
#define TMRAW_SIZE      (TMPIC_SIZE + TMDATA_SIZE)


extern struct Custom __far      custom;

extern char __far TMVBName[] = "tmvblank";



/* 
 * The Task here is to make the copyright screen work.
 * I dont think I'll make it a task since I want to conserve memory.
 *
 * Process should go as follows--:
 *
 * if (ShowTM(diskTM)) {
 * 
 *              CDTVTitle( TITLESTATUS_LEAVE );
 *              DisplayTM(); 
 *              startboot.
 *              }
 *
 * else {
 * 
 *              CDTVTitle( TITLESTATUS_ERROR );
 *              continue with scaning process.
 *              }
 *
 * DisplayTM() will trap LoadView and display the TRADEMARK screen.  A
 * call to UnDisplayTM() must be called before the system will be able 
 * to display real screens.
 */

#define TRADEMARK_WIDTH         320
#define TRADEMARK_BYTESPERROW   ((TRADEMARK_WIDTH+15)>>3&0xFFFE)
#define TRADEMARK_HEIGHT        200
#define TRADEMARK_DEPTH         2
#define TRADEMARK_PLANESIZE     (TRADEMARK_BYTESPERROW*TRADEMARK_HEIGHT)
#define TRADEMARK_PLANE0        0x1800
#define TRADEMARK_PLANE1        (0x1800+TRADEMARK_PLANESIZE)
#define TRADEMARK_COLOROFFSET   (0x1800+TRADEMARK_PLANESIZE*TRADEMARK_DEPTH)
#define TRADEMARK_MODES         (HIRES | LACE)
#define TRADEMARK_NOCOLORS      4


struct LoadView ** __asm TrapLoadView(register __a1 struct GfxBase *);

VOID    __asm UntrapLoadView(register __a0 struct View **);
VOID    __asm OrgLoadView(register __a0 struct View **, register __a1 struct View *);


/*
 * Spoof the #pragmas
 */

#define SysBase         (PlayerPrefsBase->ppb_ExecBase)
#define DeBoxBase       (PlayerPrefsBase->ppb_DeBoxBase)
#define GfxBase         (PlayerPrefsBase->ppb_GfxBase)



struct TMData {

    struct View            *view;
    struct BitMap           bm;
    UBYTE                  *tmdata;
    LONG                    datasize;
    struct View           **altlv;
    struct UCopList         ucop;
    };


extern struct CompHeader __far cdtv_tm;


/****i* playerprefs.library/DecompTradeMark ********************************
*
*   NAME
*       DecompTradeMark -- Decompress CDTV ROM trademark image.
*
*   SYNOPSIS
*       success = DecompTradeMark (dataptr, sizeptr);
*         D0                         A0        A1
*
*       LONG    success;
*       UBYTE   **dataptr;
*       LONG    *sizeptr;
*
*   FUNCTION
*       This function decompresses the ROM copy of the CDTV trademark image.
*       A pointer to the decompressed data is placed in the pointer
*       designated by 'dataptr.'  The size of the decompressed data is left
*       in the LONG pointed to by 'sizeptr.'
*
*       You may safely specify NULL for 'dataptr' and 'sizeptr.'  If
*       'dataptr' is NULL, the image is not decompressed.  If 'sizeptr' is
*       NULL, the size is not recorded.
*
*       If any internal operation fails, zero is returned and nothing is
*       allocated; the pointer referenced by 'dataptr' is written with NULL.
*
*   INPUTS
*       dataptr - pointer to a UBYTE pointer
*       sizeptr - pointer to a LONG
*
*   RESULT
*       success - non-zero if all went well; zero otherwise
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
__asm LIBDecompTradeMark(register __a0 UBYTE **data, register __a1 LONG *size,
                         register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

struct CompHeader *h2;
UBYTE *d;
ULONG ts;
        
    h2 = NextComp( &cdtv_tm, NULL );
    if (!size) size = &ts;

    *size = cdtv_tm.ci_Size + h2->ci_Size;

    if (data) {

        if (!(d = *data = AllocMem(*size, MEMF_CHIP))) return(0);

        if (!DecompData(&cdtv_tm, NULL, d)) goto errxit;

        if (!DecompData(h2,NULL,&d[cdtv_tm.ci_Size])) {

errxit:
             FreeMem(*data, *size);
             *data = NULL;
             return(0);
             }
        }

    return(1);
    }


void fadeblack(UWORD *omap,UWORD *nmap, int noc, int lv, struct PlayerPrefsBase *PlayerPrefsBase) {

    while (noc--)*nmap++ = LevelColor(*omap++,lv);
    }


/****i* playerprefs.library/FreeTM *****************************************
*
*   NAME
*       FreeTM -- Take down CDTV trademark screen.
*
*   SYNOPSIS
*       FreeTM ();
*
*   FUNCTION
*       This routine fades out the CDTV trademark screen, takes down and
*       frees the associated View, ViewPort, and bitmap.
*
*       LoadView() is restored to normal operation.  The last View that was
*       loaded using LoadView() is brought up.
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
*       PrepareTM(), DisplayTM()
*
*****************************************************************************
*/

void __asm LIBFreeTM(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

register struct TMData *tm;
int ans;
UWORD cmap[ TRADEMARK_NOCOLORS ];
struct ViewPort *vp;

struct MsgPort *AnimPort;
struct MsgPort *ReplyPort;
struct Message *ShutdownMsg;

    if (AnimPort = FindPort("Startup Animation")) {                             /* See if animation is running */

        if (ReplyPort = CreateMsgPort()) {                                      /* Create a message reply port */

            if (ShutdownMsg = (struct Message *)                                /* Allocate a message to send  */
                AllocMem(sizeof(struct Message), MEMF_PUBLIC | MEMF_CLEAR)) {

                ShutdownMsg->mn_Length    = 0;                                  /* Tell the animation to       */
                ShutdownMsg->mn_ReplyPort = ReplyPort;                          /* shutdown and clean up       */
                PutMsg(AnimPort, ShutdownMsg);

                /* You can use this time to process data while waiting for    */
                /* the animation to fade to black and free itself if you want */

                WaitPort(ReplyPort);                                            /* Wait for message to return  */

                while(GetMsg(ReplyPort));

                FreeMem(ShutdownMsg, sizeof(struct Message));                   /* Free the message            */
                }

            DeleteMsgPort(ReplyPort);                                           /* Delete MsgPort              */
            }
        }

    ObtainSemaphore(&PlayerPrefsBase->ppb_LibLock);
    tm = PlayerPrefsBase->ppb_TMData;

    if (tm) {

        if (tm->altlv) {

            for(ans =0;ans <= 15; ans++) {

                fadeblack((UWORD *)&tm->tmdata[TRADEMARK_COLOROFFSET ], cmap, TRADEMARK_NOCOLORS, ans, PlayerPrefsBase);
                LoadRGB4(tm->view->ViewPort,cmap,TRADEMARK_NOCOLORS);
                WaitTOF();
                }

            UntrapLoadView(tm->altlv);
            }

        if (tm->view) {

            vp = tm->view->ViewPort;

            if (vp->UCopIns) {

                FreeCopList(vp -> UCopIns -> FirstCopList);
                vp->UCopIns = NULL;
                }

            DeleteView((struct SuperView *)tm->view);
            }

        if (tm->tmdata) FreeMem(tm->tmdata,tm->datasize);
        FreeMem(tm,sizeof(struct TMData));
        }

    PlayerPrefsBase->ppb_TMData = NULL;
    ReleaseSemaphore(&PlayerPrefsBase->ppb_LibLock);
    }


__asm CheckData(register __a0 UBYTE *,register __a1 UBYTE *, register __d0 LONG);



/****i* playerprefs.library/PrepareTM **************************************
*
*   NAME
*       PrepareTM -- Prepare CDTV trademark image for display.
*
*   SYNOPSIS
*       success = PrepareTM (disktm, size);
*         D0                   A0     D0
*
*       LONG    success;
*       UBYTE   *disktm;
*       LONG    size;
*
*   FUNCTION
*       This routine decompresses the CDTV trademark image and prepares an
*       internal View and ViewPort for its display.
*
*       'disktm' and 'size' are what you think the decompressed image and
*       size are.  PrepareTM() compares the ROM copy to your copy, and
*       refuses to proceed if they are different.
*
*       If any internal operation fails, zero is returned and nothing is
*       allocated.
*
*   INPUTS
*       disktm  - pointer to what you think the decompressed CDTV trademark
*                 image is
*       size    - what you think the decompressed CDTV trademark image size
*                 is, in bytes
*
*   RESULT
*       success - non-zero if all went well; zero otherwise
*
*   EXAMPLE
*
*   NOTES
*       Your copy of the CDTV trademark image (cdtvtm) is used for
*       comparison purposes only, and may be freed after this call.
*
*   BUGS
*
*   SEE ALSO
*       DisplayTM(), FreeTM()
*
*****************************************************************************
*/

    /* 
     * This will get the screen up there if someone turns off the copper
     * display.  This is Commadore's Trademark and it WILL be seen!!!!
     * so there.  If this one doesn't work I'll put it on every scan line!
     * (this is a threat...)
     *
     * Further Devlopments -- Thankfully the think that is turning off the
     * screen is zaping the DMAF_RASTER bit.  Therefore get the copper to 
     * zap it back on.  However, I think the calling program is calling
     * { waitTOF(); custom.dmacon = DMAF_RASTER }, so makesure the 
     * move is down some.
     */

int __asm LIBPrepareTM(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

register struct TMData *tm;

    ObtainSemaphore( &PlayerPrefsBase->ppb_LibLock );
    

tryagain:
    if (PlayerPrefsBase->ppb_TMData) FreeTM();
    
    if (!(tm = AllocMem(sizeof(struct TMData), MEMF_CHIP|MEMF_CLEAR))) goto errxit;             /* we probably should leave */

    if (!DecompTradeMark(NULL, &tm->datasize)) goto allocxit;

    if (!DecompTradeMark(&tm->tmdata,NULL)) goto allocxit;

    tm->bm.BytesPerRow      = TRADEMARK_BYTESPERROW;
    tm->bm.Rows             = TRADEMARK_HEIGHT;
    tm->bm.Depth            = TRADEMARK_DEPTH;
    tm->bm.Planes[0]        = &tm->tmdata[ TRADEMARK_PLANE0 ];
    tm->bm.Planes[1]        = &tm->tmdata[ TRADEMARK_PLANE1 ];

    if (!(tm->view = (struct View *)CreateView(NULL, &tm->bm,
        TRADEMARK_WIDTH, TRADEMARK_HEIGHT, TRADEMARK_MODES))) goto allocxit;
    
    CINIT(&tm->ucop, 10);
    CWAIT(&tm->ucop, -4,0);
    CMOVE(&tm->ucop, custom.dmacon, (DMAF_SETCLR | DMAF_RASTER));
    CEND (&tm->ucop);

    tm->view->ViewPort->UCopIns = &tm->ucop;
    
    CenterCDTVView(NULL, tm->view, TRADEMARK_WIDTH, TRADEMARK_HEIGHT);

    PlayerPrefsBase->ppb_TMData = tm;

    ReleaseSemaphore(&PlayerPrefsBase->ppb_LibLock);
    return(1);

allocxit:

    PlayerPrefsBase->ppb_TMData = tm;

    goto tryagain;

errxit:

    PlayerPrefsBase->ppb_TMData = tm;
    FreeTM();
    ReleaseSemaphore(&PlayerPrefsBase->ppb_LibLock);
    return(0);
    }


/****i* playerprefs.library/DisplayTM **************************************
*
*   NAME
*       DisplayTM -- Display the CDTV trademark image in a forceful way.
*
*   SYNOPSIS
*       success = DisplayTM ();
*         D0
*
*       LONG    success;
*
*   FUNCTION
*       A CALL TO PrepareTM() MUST PRECEED THIS CALL!
*
*       This routine loads the internal View prepared by PrepareTM(), and
*       fades it in.
*
*       This routine SetFunction()s LoadView() such that no other View may
*       be loaded until this one is taken down.
*
*   INPUTS
*
*   RESULT
*       success - non-zero if all went well; zero otherwise
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*       PrepareTM(), FreeTM()
*
*****************************************************************************
*/
__asm __saveds LIBDisplayTM(register __a6 struct PlayerPrefsBase *PlayerPrefsBase) {

register struct TMData *tm;
UWORD                   cmap[TRADEMARK_NOCOLORS];
int                     lv = 0;

    ObtainSemaphore(&PlayerPrefsBase->ppb_LibLock);

    if (tm = PlayerPrefsBase->ppb_TMData) {

        if (!tm->altlv) {

            if (tm->altlv = TrapLoadView(GfxBase)) {

                OrgLoadView(tm->altlv, tm->view);

                for (lv=15; lv>=0; lv--) {

                    fadeblack((UWORD *)&tm->tmdata[TRADEMARK_COLOROFFSET], cmap, TRADEMARK_NOCOLORS, lv, PlayerPrefsBase);
                    LoadRGB4(tm->view->ViewPort,cmap,TRADEMARK_NOCOLORS);
                    WaitTOF();
                    }

                lv = 1;
                }
            }

        else lv = 1;
        }

    ReleaseSemaphore(&PlayerPrefsBase->ppb_LibLock);
    return(lv);
    }



