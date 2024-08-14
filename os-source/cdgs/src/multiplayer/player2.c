
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/videocontrol.h>
#include <intuition/intuitionbase.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <utility/tagitem.h>
#include <libraries/debox.h>
#include <libraries/lowlevel.h>
#include <hardware/blit.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/debox_protos.h>
#include <clib/lowlevel_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/debox_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>

#include "animation.h"
#include "player.h"

#define SysBase       (*((LONG *)4))
#define LowLevelBase  V->LowLevelBase
#define DeBoxBase     V->DeBoxBase
#define IntuitionBase V->IntuitionBase
#define GfxBase       V->GfxBase

extern VOID         FadeIn(struct V *V);
extern VOID         FadeOut(struct V *V);


struct ButtonInfo ButtonInfo[] = {

    {   188, 129, 48,  27,      0,   27,      48,  27,      96,  27,    144, 27  },             // TIMEMODE
    {   269, 129, 48,  27,      96,  81,      144, 81  },                                       // SHUFFLE

    {   36,  174, 30,  27,      60,  0,       90,  0   },                                       // FREV
    {   69,  174, 41,  27,      180, 0,       121, 0,       162, 0   },                         // PLAY/PAUSE
    {   113, 174, 30,  27,      120, 0,       150, 0   },                                       // FFWD
    {   146, 174, 30,  27,      0,   0,       30,  0   },                                       // STOP

    {   188, 174, 48,  27,      0,   81,      48,  81  },                                       // LOOP
    {   240, 174, 48,  27,      0,   54,      48,  54,      96,  54,    144, 54,    192, 54  }, // INTRO
    {   300, 174, 48,  27,      192, 27,      240, 27  },                                       // CD+G

    {   55,  121, 102, 42,      207, 153      },                                                // Time display

    {   183, 20,  33,  23,      309, 191      },                                                // Grid
    {   217, 20,  33,  23,      309, 191      },
    {   251, 20,  33,  23,      309, 191      },
    {   285, 20,  33,  23,      309, 191      },
    {   319, 20,  33,  23,      309, 191      },

    {   183, 43,  33,  23,      309, 191      },
    {   217, 43,  33,  23,      309, 191      },
    {   251, 43,  33,  23,      309, 191      },
    {   285, 43,  33,  23,      309, 191      },
    {   319, 43,  33,  23,      309, 191      },

    {   183, 66,  33,  23,      309, 191      },
    {   217, 66,  33,  23,      309, 191      },
    {   251, 66,  33,  23,      309, 191      },
    {   285, 66,  33,  23,      309, 191      },
    {   319, 66,  33,  23,      309, 191      },

    {   183, 89,  33,  23,      309, 191      },
    {   217, 89,  33,  23,      309, 191      },
    {   251, 89,  33,  23,      309, 191      },
    {   285, 89,  33,  23,      309, 191      },
    {   319, 89,  33,  23,      309, 191      },

    };
    

//////////////////////////////////////////////////////////////////
//                                                              //
// WaitBeam -- Wait for beam to pass a certain Y position       //
//             within N lines so that we can safely complete    //
//             a blit of an image before it is displayed.       //
//                                                              //
//             NOTE:  This is pretty ugly, but it's the only    //
//                     way to do it.                            //
//                                                              //
//////////////////////////////////////////////////////////////////

void WaitBeam(UWORD Y, struct V *V) {                           // WaitBeam()

UWORD y;

    Y += 50;                                                    // It takes a few lines before the beam gets to our RastPort

    while (1) {                                                 // Loop until we find it.

        y = VBeamPos();                                         // Get beam position

        if ((y > Y) && (y < Y + 50)) return;                    // Return if it is within range.
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// BlitBox - Blit a box from buttondata screen to our visible screen.                   //
//           If a mask pointer is provided, it will be used.                            //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void BlitBox(UWORD X, UWORD Y, UWORD Box, PLANEPTR Mask, struct V *V) {                 // BlitBox()

    Forbid();                                                                           // We really need to do this.

    WaitBeam(ButtonInfo[Box].ScreenY + ButtonInfo[Box].Height, V);                      // Wait for beam to just pass box

    if (!Mask) {                                                                        // Blit with a mask?

        BltBitMapRastPort(V->ButtonBM,                                                  // Blit original button
                          X,
                          Y,
                          &V->Screen->RastPort,
                          ButtonInfo[Box].ScreenX,
                          ButtonInfo[Box].ScreenY,
                          ButtonInfo[Box].Width,
                          ButtonInfo[Box].Height,
                          0x0c0);
        }

    else {                                                                              // Use a mask.

        BltMaskBitMapRastPort(V->ButtonBM,                                              // Blit (with mask) the non-highlight
                              X,
                              Y,
                              &V->Screen->RastPort,
                              ButtonInfo[Box].ScreenX,
                              ButtonInfo[Box].ScreenY,
                              ButtonInfo[Box].Width,
                              ButtonInfo[Box].Height,
                              (ABC|ABNC|ANBC),
                              Mask);
        }

    Permit();                                                                           // Finished hogging the CPU
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// Highlight - Highlight or unhighlight a button/track grid box.                        //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void Highlight(UWORD Box, UWORD Enable, struct V *V) {                                  // Highlight()

UWORD X, Y;
UWORD Offset;

    if (Enable) {

        if       (Box == B_PLAYPAUSE)                { X = 270; Y = 81;  }              // Play/Pause highlight
        else if ((Box >= B_FREV) && (Box <= B_STOP)) { X = 240; Y = 81;  }              // Any other square button
        else if ((Box <= B_CDG))                     { X = 192; Y = 81;  }              // Any other rectangle button
        else if  (Box == B_TIMEBOX)                  { X = 207; Y = 195; }              // Time display highlight
        else                                         { X = 309; Y = 214; }              // Grid highlight

        BlitBox(X, Y, Box, V->ButtonBM->Planes[4], V);
        }

    else {

        if (Box < B_NUMBUTTONS) {                                                       // Blit image or original border?

            BlitBox(ButtonInfo[Box].SourcePos[V->ButtonSelect[Box]].X,
                    ButtonInfo[Box].SourcePos[V->ButtonSelect[Box]].Y,
                    Box,
                    NULL,
                    V);
            }

        else {

            if (Box >= B_TIMEBOX) Offset = ButtonInfo[Box].Height;                      // Find appropriate mask
            else                  Offset = 0;

            BlitBox(ButtonInfo[Box].SourcePos[0].X,
                    ButtonInfo[Box].SourcePos[0].Y,
                    Box,
                    V->ButtonBM->Planes[4] + V->ButtonBM->BytesPerRow * Offset,
                    V);
            }
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// UpdateButtons - If the state of a button has changed, blit the new button image      //
//                 to the screen.                                                       //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void UpdateButtons(struct V *V) {                                                       // UpdateButtons()

UWORD i;

    for (i=0; i!=B_NUMBUTTONS; i++) {                                                   // Blit all buttons to screen

        if (V->ButtonSelect[i] != V->LastButtonSelect[i]) {                             // Has the state of button changed?

            BlitBox(ButtonInfo[i].SourcePos[V->ButtonSelect[i]].X,
                    ButtonInfo[i].SourcePos[V->ButtonSelect[i]].Y,
                    i,
                    NULL,
                    V);

            V->LastButtonSelect[i] = V->ButtonSelect[i];                                // State has now been updated
            }
        }

    if (V->Highlighted != V->LastHighlighted) {

        Highlight(V->LastHighlighted, FALSE, V);
        Highlight(V->Highlighted,     TRUE,  V);

        V->LastHighlighted = V->Highlighted;
        }
    }


void BlitDisk(WORD Position, UWORD Direction, struct V *V) {

WORD XPos, OPos, CPos, BlitWidth;

    if (Position < 0) Position = 0;

    if ((OPos      = 207 - Position) < 0) OPos      = 0;
    if ((XPos      = Position - 207) < 0) XPos      = 0;
    if ((BlitWidth = Position) > 207)     BlitWidth = 207;
    if ((CPos      = XPos - 24) < 0)      CPos      = 0;

    if (!Direction) if ((CPos  = Position)  > 275-24) CPos = 275-24;

    WaitBeam(100, V);

    if ((Direction && XPos) || !Direction) {

        BltBitMapRastPort(V->ButtonBM,                                                  // Clear where disk was
                          272,
                          82,
                          &V->Screen->RastPort,
                          CPos,
                          94,
                          24,
                          18,
                          0x0c0);
        }

    BltBitMapRastPort(V->ButtonBM,                                                      // Blit the grid
                      94,
                      191,
                      &V->Screen->RastPort,
                      183,
                      94,
                      94,
                      18,
                      0x0c0);

    if (BlitWidth) {

        BltBitMapRastPort(V->ButtonBM,                                                  // Blit disk
                          OPos,
                          173,
                          &V->Screen->RastPort,
                          XPos,
                          94,
                          BlitWidth,
                          18,
                          0x0c0);
        }

    BltMaskBitMapRastPort(V->ButtonBM,                                                  // Blit (with mask) the grid
                          0,
                          191,
                          &V->Screen->RastPort,
                          183,
                          94,
                          94,
                          18,
                          (ABC|ABNC|ANBC),
                          V->ButtonBM->Planes[0] + V->ButtonBM->BytesPerRow * 18);
    }

void BlitHead(WORD XPosition, WORD YPosition, UWORD Direction, struct V *V) {

WORD YPos, OPos, CPos, BlitHeight;

    if (YPosition < 0) YPosition = 0;

    if ((OPos       = 50 - YPosition) < 6) OPos       = 6;
    if ((YPos       = YPosition - 44) < 0) YPos       = 0;
    if ((BlitHeight = YPosition) > 44)     BlitHeight = 44;
    if ((CPos       = YPos - 6) < 0)       CPos       = 0;

    if (!Direction) CPos  = YPosition;

    WaitBeam(YPosition + 44, V);

    BltBitMapRastPort(V->ButtonBM,                                                  // Clear where disk was
                      303,
                      0,
                      &V->Screen->RastPort,
                      XPosition,
                      CPos,
                      80,
                      6,
                      0x0c0);

    if (BlitHeight) {

        BltBitMapRastPort(V->ButtonBM,                                                  // Blit disk
                          303,
                          OPos,
                          &V->Screen->RastPort,
                          XPosition,
                          YPos,
                          80,
                          BlitHeight,
                          0x0c0);
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                      // 
// DiskAnimation - Do disk/head animation                                                               //
//                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void DiskAnimation(UWORD Direction, struct V *V) {                                                      // DiskAnimation()

WORD DPos, DVelocity, DAccel = -160;
WORD HPos, HVelocity, HAccel = -400;

    if (Direction) { DPos = 0;   DVelocity = 10000; HPos = -520; HVelocity = 22500; }
    else           { DPos = 276; DVelocity = 0;     HPos = 80;   HVelocity = 0;    }

    while ((DPos < (276 + (1 - Direction))) && (DPos > -1)) {

        HVelocity += HAccel;
        HPos      += (HVelocity / 1000);

        BlitHead(30, HPos, Direction, V);

        DVelocity += DAccel;
        DPos      += (DVelocity / 1000);

        BlitDisk(DPos, Direction, V);
        }
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                      // 
// DoPlayer - Player program event loop.                                                                //
//                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void DoPlayer(struct V *V) {                                                                            // DoPlayer()

LONG    i, j;

    FadeIn(V);
    Highlight(V->Highlighted, TRUE, V);

    DiskAnimation(TRUE, V);
#if 0
    for (j=0; j!=B_GRID+20; j++) {

        V->Highlighted = j;
        UpdateButtons(V);

        for (i=0; i!=30; i++) WaitTOF();
        }
#endif
    for (i=0; i!=300; i++) WaitTOF();

    DiskAnimation(FALSE, V);

    for (i=0; i!=300; i++) WaitTOF();

    Highlight(V->Highlighted, FALSE, V);
    FadeOut(V);
    }

