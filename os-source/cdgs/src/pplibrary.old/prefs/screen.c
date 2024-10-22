
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

extern void            *SysBase;
extern struct GfxBase  *GfxBase;


/********     External Definitions     ********/

extern struct BitMap   *bm;
extern BYTE            *ViewGState;
extern UBYTE           *bmask;



/******************************** Set Activation of Screen Center Gadget *******************************/

static void changescdot(int no) {

    MasterF.D2 = no;
    WorkF->gstate[PEGAD_SCREEN] = CurrentF->gstate[PEGAD_SCREEN] = 0x7f7f;
    UpdateDisplay();
    UpdateDisplay();
    }       




/**************************** Get Screen Centering Events and Update Screen ****************************/

void AdjScreen() {

UWORD event;
WORD  x,y;
UBYTE upkey = 0;

    input_data.MouseXmove = input_data.MouseYmove = 0;
    intdata.cintd_VCountDown = 240;
    changescdot(1);

    while (1) {

        event = GetEvent(&input_data);
        if (event) intdata.cintd_VCountDown = 240;

        else if (!(intdata.cintd_VCountDown) && upkey) break;

        if ((event == (IECODE_LBUTTON|IECODE_UP_PREFIX)) || (event == (RAWKEY_RETURN | IECODE_UP_PREFIX))) {

            if (upkey) break;
            else       upkey = 1;
            }

        x = input_data.MouseXmove>>3;
        y = input_data.MouseYmove>>3;

        input_data.MouseXmove -= (x<<3);
        input_data.MouseYmove -= (y<<3);

        switch(event) {

            case RAWKEY_ARROWUP: y--; break;
            case RAWKEY_ARROWDN: y++; break;
            case RAWKEY_ARROWLT: x--; break;
            case RAWKEY_ARROWRT: x++; break;
            }

        if (x || y) upkey = 1;
        x += ViewGState[0];
        if (x < ADJMIN_X) x=ADJMIN_X;
        if (x > ADJMAX_X) x=ADJMAX_X;
        y += ViewGState[1];
        if (y < ADJMIN_Y) y=ADJMIN_Y;
        if (y > ADJMAX_Y) y=ADJMAX_Y;
        ViewGState[0] = x;
        ViewGState[1] = y;
        UpdateDisplay();                
        }

    changescdot( 0 );
    }




/******************************** Determine Where DOT Should Be Located ********************************/

#define SCRGAD_X    133
#define SCRGAD_Y    67
#define SCRGAD_W    69
#define SCRGAD_H    41

static void targetxy(BYTE xloc, BYTE yloc, WORD *x, WORD *y) {

    *x = SCRGAD_X + (((xloc-ADJMIN_X) * SCRGAD_W) / ADJSIZE_X);
    *y = SCRGAD_Y + (((yloc-ADJMIN_Y) * SCRGAD_H) / ADJSIZE_Y);
    }




/********************************** Activate Screen Positioning Gadget *********************************/

GADScreenPos(struct RastPort *rp, struct BitMap *bbm, struct GadDir *g, WORD old, WORD new) {

extern struct CDTVPrefs CDTVPrefs;
extern WORD ViewCenterX,ViewCenterY;
register BYTE *b;
WORD x,y;

    BltBitMap(bm, SCRGAD_X, SCRGAD_Y, rp->BitMap, SCRGAD_X, SCRGAD_Y, (SCRGAD_W+17), (SCRGAD_H+17), 0xc0, 0xff, NULL);

    b = (BYTE *)&new;
    targetxy(b[0], b[1], &x, &y);

    CDTVPrefs.DisplayX = ViewCenterX + b[0];
    CDTVPrefs.DisplayY = ViewCenterY + b[1];
    
    if (new != 0x7f7f) BltMaskBitMapRastPort(bbm, (MasterF.D2 ? 238:222), 9, rp, x, y, 16, 16, (ABC|ABNC|ANBC), bmask);

    DisplayFramePos(WorkF, &CDTVPrefs);
    return(0);
    }



