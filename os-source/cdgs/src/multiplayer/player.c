
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <devices/inputevent.h>
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
#include "/playerlibrary/playerlibrary.h"

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/debox_protos.h>
#include <clib/lowlevel_protos.h>
#include "/playerlibrary/player_protos.h"

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/debox_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include "/playerlibrary/player_pragmas.h"

#include "animation.h"
#include "player.h"
#include "keydefs.h"

#define SysBase       (*((LONG *)4))
#define LowLevelBase  V->LowLevelBase
#define DeBoxBase     V->DeBoxBase
#define IntuitionBase V->IntuitionBase
#define GfxBase       V->GfxBase
#define PlayerBase    V->PlayerBase

extern VOID         FadeIn(struct V *V);
extern VOID         FadeOut(struct V *V);

#define HEADTOP     94
#define HEADLEFT    37
#define HEADWIDTH   88
#define HEADHEIGHT  63
#define HEADRANGE   55

struct ButtonInfo ButtonInfo[] = {

    {   KEY_TIMEMODE,   300, 128, 48,  27,      0,   27,      48,  27,      96,  27,    144, 27  },             // TIMEMODE

    {   KEY_FREV,       36,  174, 30,  27,      60,  0,       90,  0   },                                       // FREV
    {   KEY_PLAYPAUSE,  69,  174, 41,  27,      180, 0,       221, 0,       262, 0   },                         // PLAY/PAUSE
    {   KEY_FFWD,       113, 174, 30,  27,      120, 0,       150, 0   },                                       // FFWD
    {   KEY_STOP,       146, 174, 30,  27,      0,   0,       30,  0   },                                       // STOP

    {   KEY_LOOP,       187, 174, 48,  27,      0,   81,      48,  81  },                                       // LOOP
    {   KEY_INTRO,      244, 174, 48,  27,      0,   54,      48,  54,      96,  54,    144, 54,    192, 54  }, // INTRO
    {   KEY_SHUFFLE,    300, 174, 48,  27,      96,  81,      144, 81  },                                       // SHUFFLE

    {   0,              183, 20,  33,  23,      188, 191      },                                                // Grid
    {   0,              217, 20,  33,  23,      188, 191      },
    {   0,              251, 20,  33,  23,      188, 191      },
    {   0,              285, 20,  33,  23,      188, 191      },
    {   0,              319, 20,  33,  23,      188, 191      },

    {   0,              183, 43,  33,  23,      188, 191      },
    {   0,              217, 43,  33,  23,      188, 191      },
    {   0,              251, 43,  33,  23,      188, 191      },
    {   0,              285, 43,  33,  23,      188, 191      },
    {   0,              319, 43,  33,  23,      188, 191      },

    {   0,              183, 66,  33,  23,      188, 191      },
    {   0,              217, 66,  33,  23,      188, 191      },
    {   0,              251, 66,  33,  23,      188, 191      },
    {   0,              285, 66,  33,  23,      188, 191      },
    {   0,              319, 66,  33,  23,      188, 191      },

    {   0,              183, 89,  33,  23,      188, 191      },
    {   0,              217, 89,  33,  23,      188, 191      },
    {   0,              251, 89,  33,  23,      188, 191      },
    {   0,              285, 89,  33,  23,      188, 191      },
    {   0,              319, 89,  33,  23,      188, 191      },

    };
    
struct DirectionInfo {

    BYTE Left, Right, Up, Down;
    };

struct DirectionInfo DirInfo[] = {

    { 27,  1,   27,  7  },          // TIMEMODE  
                                                 
    { 6,   1,   0,   0  },          // FREV      
    { -1,  1,   0,   0  },          // PLAY/PAUSE
    { -1,  1,   0,   0  },          // FFWD      
    { -1,  1,   0,   0  },          // STOP      
                                                 
    { -1,  1,   18,  3  },          // LOOP      
    { -1,  1,   19,  4  },          // INTRO     
    { -1, -6,   -7,  5  },          // SHUFFLE   
                                                 
    { -1,  1,   -3,  5  },          // Grid      
    { -1,  1,   -4,  5  },
    { -1,  1,   -4,  5  },
    { -1,  1,   -4,  5  },
    { -1,  1,   -5,  5  },

    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },

    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },
    { -1,  1,   -5,  5  },

    { -1,  1,   -5, -18  },
    { -1,  1,   -5, -19  },
    { -1,  1,   -5, -19  },
    { -1,  1,   -5, -26  },
    { -1,  -27, -5, -27  },
    };

ULONG CKeys[7] = {

    KEY_PLAYPAUSE,
    KEY_FREV,
    KEY_FFWD,
    KEY_SHUFFLE,
    KEY_LOOP,
    0,
    KEY_STOP,
    };

ULONG CDirs[] = {

    RAWKEY_ARROWRT,
    RAWKEY_ARROWLT,
    RAWKEY_ARROWDN,
    RAWKEY_ARROWUP,
    };

                                                                                
//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      //
// GetController - Get joystick/mouse/controller movement                               //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

ULONG GetController(ULONG Port, struct V *V) {                                          // GetController()

ULONG Result;
ULONG JoyState = 0;
BYTE  CX, CY, DX, DY;

    Result = ReadJoyPort(Port);                                                         // Get port movement

    switch (Result & JP_TYPE_MASK) {                                                    // Type of input?

        case JP_TYPE_GAMECTLR:                                                          // Game controller

            if (!V->ControllerLock || V->ControllerLock == JP_TYPE_GAMECTLR) {          // Are we locked into something else?

                V->ControllerLock = JP_TYPE_GAMECTLR;                                   // Lock in controller movements

                return(Result);                                                         // Return result
                }

            break;                                                                      // Must be locked

        case JP_TYPE_JOYSTK:                                                            // Joystick

            if (!V->ControllerLock) return(Result);                                     // If not locked, return joystick

            break;                                                                      // Must be locked

        case JP_TYPE_MOUSE:

            if (!V->ControllerLock || V->ControllerLock == JP_TYPE_MOUSE) {             // Are we locked into something else?

                V->ControllerLock = JP_TYPE_MOUSE;                                      // Lock in mouse movements

                CX = (BYTE)(Result & 0xFF);                                             // Calculate controller X
                CY = (BYTE)((Result >> 8) & 0xFF);

                DX = V->LastMouseX - CX;                                                // Calculate delta
                DY = V->LastMouseY - CY;

                if      (DX >= 80)  { JoyState |= JPF_JOY_LEFT;  V->LastMouseX = CX; }  // Mimic joystick movement
                else if (DX <= -80) { JoyState |= JPF_JOY_RIGHT; V->LastMouseX = CX; }
                if      (DY >= 80)  { JoyState |= JPF_JOY_UP;    V->LastMouseY = CY; }
                else if (DY <= -80) { JoyState |= JPF_JOY_DOWN;  V->LastMouseY = CY; }

                return((Result & 0xFFFF0000) | JoyState);                               // Return result
                }

            break;                                                                      // Must be locked
        }

    return(0);                                                                          // Unknown movement (or no movement)
    }


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

    Y += 20;                                                    // It takes a few lines before the beam gets to our RastPort

    while (1) {                                                 // Loop until we find it.

        y = VBeamPos();                                         // Get beam position

        if ((y > Y) && (y < Y + 100)) return;                   // Return if it is within range.
        }
    }


//////////////////////////////////////////////////////////////////
//                                                              //
// Blit - Blit a bit map (simplified parameters)                //
//                                                              //
//////////////////////////////////////////////////////////////////

void Blit(WORD SrcX,  WORD SrcY,                                // Blit()
          WORD DstX,  WORD DstY,
          WORD Width, WORD Height, struct V *V) {

    BltBitMapRastPort(V->ButtonBM,                              // Blit the data
                      SrcX,
                      SrcY,
                      &V->Screen->RastPort,
                      DstX,
                      DstY,
                      Width,
                      Height,
                      0x0c0);
    }


//////////////////////////////////////////////////////////////////
//                                                              //
// Blit - Blit a bit map with a mask (simplified parameters)    //
//                                                              //
//////////////////////////////////////////////////////////////////

void BlitMask(WORD SrcX,     WORD SrcY,                         // BlitMask()
              WORD DstX,     WORD DstY,
              WORD Width,    WORD Height,
              PLANEPTR Mask, struct V *V) {

    BltMaskBitMapRastPort(V->ButtonBM,                          // Blit with a mask
                          SrcX,
                          SrcY,
                          &V->Screen->RastPort,
                          DstX,
                          DstY,
                          Width,
                          Height,
                          (ABC|ABNC|ANBC),
                          Mask);
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// BlitBox - Blit a box from buttondata screen to our visible screen.                   //
//           If a mask pointer is provided, it will be used.                            //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void BlitBox(UWORD X, UWORD Y, UWORD Box, PLANEPTR Mask, struct V *V) {                 // BlitBox()

    WaitBeam(ButtonInfo[Box].ScreenY + ButtonInfo[Box].Height, V);                      // Wait for beam to just pass box

    if (!Mask) {                                                                        // Blit with a mask?

        Blit(X,                                                                         // Blit original button
             Y,
             ButtonInfo[Box].ScreenX,
             ButtonInfo[Box].ScreenY,
             ButtonInfo[Box].Width,
             ButtonInfo[Box].Height,
             V);
        }

    else {                                                                              // Use a mask.

        BlitMask(X,                                                                     // Blit (with mask) the non-highlight
                 Y,
                 ButtonInfo[Box].ScreenX,
                 ButtonInfo[Box].ScreenY,
                 ButtonInfo[Box].Width,
                 ButtonInfo[Box].Height,
                 Mask,
                 V);
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// Highlight - Highlight or unhighlight a button/track grid box.                        //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void Highlight(UWORD Box, UWORD Enable, struct V *V) {                                  // Highlight()

UWORD X, Y;
UWORD Offset;

    if (Enable) {                                                                       // Enable highlight

        if       (Box == B_PLAYPAUSE)                { X = 270; Y = 81;  }              // Play/Pause highlight
        else if ((Box >= B_FREV) && (Box <= B_STOP)) { X = 240; Y = 81;  }              // Any other square button
        else if ((Box <= B_SHUFFLE))                 { X = 192; Y = 81;  }              // Any other rectangle button
        else                                         { X = 188; Y = 214; }              // Grid highlight

        BlitBox(X, Y, Box, V->ButtonBM->Planes[4], V);                                  // Blit the highlight
        }

    else {                                                                              // Else, highlight disable

        if (Box < B_NUMBUTTONS) {                                                       // Blit image or original border?

            BlitBox(ButtonInfo[Box].SourcePos[V->ButtonSelect[Box]].X,                  // Blit the hightlight
                    ButtonInfo[Box].SourcePos[V->ButtonSelect[Box]].Y,
                    Box,
                    NULL,
                    V);
            }

        else {                                                                          // Blit a grid highlight

            if (Box >= B_GRID) Offset = ButtonInfo[Box].Height;                         // Find appropriate mask
            else               Offset = 0;

            BlitBox(ButtonInfo[Box].SourcePos[0].X,                                     // Blit the highlight
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

            BlitBox(ButtonInfo[i].SourcePos[V->ButtonSelect[i]].X,                      // Blit the button
                    ButtonInfo[i].SourcePos[V->ButtonSelect[i]].Y,
                    i,
                    NULL,
                    V);

            V->LastButtonSelect[i] = V->ButtonSelect[i];                                // State has now been updated

            if (i == V->Highlighted) Highlight(i, TRUE, V);                             // Highlight the button if highlighted
            }
        }

    if (V->Highlighted != V->LastHighlighted) {                                         // Highlight change?

        Highlight(V->LastHighlighted, FALSE, V);                                        // Move highlight
        Highlight(V->Highlighted,     TRUE,  V);

        V->LastHighlighted = V->Highlighted;                                            // Remember new highlighted button
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// BlitDisk - Blit the disk to a specific X position (clearing the trail)               //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void BlitDisk(WORD Position, UWORD Direction, struct V *V) {                            // BlitDisk()

WORD XPos, OPos, CPos, BlitWidth;

    if (Position < 0) Position = 0;                                                     // No positions less than 0

    if ((OPos      = 207 - Position) < 0) OPos      = 0;                                // Souce object position
    if ((XPos      = Position - 207) < 0) XPos      = 0;                                // Destination X position
    if ((BlitWidth = Position) > 207)     BlitWidth = 207;                              // Width to blit
    if ((CPos      = XPos - 24) < 0)      CPos      = 0;                                // Position of clear blit

    if (!Direction) if ((CPos  = Position)  > 275-24) CPos = 275-24;                    // Don't clear too far

    WaitBeam(112, V);                                                                   // Wait for beam to pass disk position

    if ((Direction && XPos) || !Direction) Blit(272, 82, CPos, 94, 24, 18, V);          // Clear where disk was

    Blit(94, 191, 183, 94, 94, 18, V);                                                  // Blit the grid

    if (BlitWidth) Blit(OPos, 173, XPos, 94, BlitWidth, 18, V);                         // Blit disk

    BlitMask(0, 191, 183, 94, 94, 18, V->ButtonBM->Planes[0]                            // Blit (with mask) the grid
                                    + V->ButtonBM->BytesPerRow * 18, V);
    }



//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// BlitHead - Blit the head to a specific position (clearing the trail)                 //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void BlitHead(WORD XPosition, WORD YPosition, struct V *V) {                            // BlitHead()

WORD YPos, OPos, BlitHeight;

    if (YPosition < 0) YPosition = 0;                                                   // No positions less than 0

    if ((OPos = 108 + HEADHEIGHT - YPosition) < 108) OPos = 108;                        // Source object position

    BlitHeight = HEADHEIGHT - (OPos - 108);                                             // Height of blit
    YPos       = YPosition  - BlitHeight;                                               // Destination Y position

    WaitBeam(YPosition, V);                                                             // Wait for beam to pass head position

    if (BlitHeight) Blit(176, OPos, XPosition, YPos, HEADWIDTH, BlitHeight, V);         // Blit the head
    }


//////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                          // 
// Display2Dig - Display a 2-digit value on the screen by blitting the digits               //
//                                                                                          //
//////////////////////////////////////////////////////////////////////////////////////////////

void Display2Dig(BYTE Value,                                                                // Display2Dig()
                 WORD SrcX,  WORD SrcY,
                 WORD DstX,  WORD DstY,
                 WORD Width, WORD Height,
                 WORD Space, struct V *V) {

    if (Value == 100) {                                                                     // Display blank?

        Blit(SrcX + 10 * Width, SrcY, DstX,             DstY, Width, Height, V);            // Blit the blank
        Blit(SrcX + 10 * Width, SrcY, DstX+Width+Space, DstY, Width, Height, V);
        }

    else {                                                                                  // Else, actual number

        Blit(SrcX + (Value / 10) * Width, SrcY, DstX,             DstY, Width, Height, V);  // Blit 2 digits
        Blit(SrcX + (Value % 10) * Width, SrcY, DstX+Width+Space, DstY, Width, Height, V);
        }
    }




//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// UpdateTrack - Update the track display                                               //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void UpdateTrack(struct V *V) {                                                         // UpdateTrack()

    if (   (V->PlayerState.Track     != V->LastPlayerState.Track)                       // Track display change?
        || (V->PlayerState.PlayState != V->LastPlayerState.PlayState)
        || (V->HeadX                 != V->LastHeadX)) {

        if (V->PlayerState.PlayState >= PLS_SELECTED) {                                 // Should track be displayed?

            Display2Dig(V->PlayerState.Track, 0, 150,                                   // Display new track
                        V->CurrentHeadX + 24, 45, 16, 23, 8, V);
            }

        else Display2Dig(100, 0, 150, V->CurrentHeadX + 24, 45, 16, 23, 8, V);          // Display blank
        }
    }



//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              // 
// UpdateHead - Blit laser image if necessary                                                   //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateHead(struct V *V) {

UWORD Laser;

    if (V->PlayerDisk && V->SABlank) {                                                          // Audio disk present?

        if (V->PlayerState.PlayState >= PLS_PLAYING) Laser = 1;                                 // Should the laser be on?
        else                                         Laser = 0;

        if ((  (V->PlayerState.PlayState     >= PLS_PLAYING)                                    // Has state of laser
            != (V->LastPlayerState.PlayState >= PLS_PLAYING))                                   //   or head position changed?
            || (V->HeadX != V->LastHeadX)) {

            if      (V->HeadX < V->LastHeadX) V->CurrentHeadX = V->LastHeadX - 1;               // Move head a pixel?
            else if (V->HeadX > V->LastHeadX) V->CurrentHeadX = V->LastHeadX + 1;
            else                              V->CurrentHeadX = V->LastHeadX;

            if (!Laser) {                                                                       // Laser off?

                BlitHead(V->CurrentHeadX, HEADTOP, V);                                          // Blit head
                UpdateTrack(V);                                                                 // - Blit the track number too
                Blit(0, 173, 69, 94, 100, 18, V);                                               // - Blit half of disk
                }

            else {                                                                              // Laser on

                BlitHead(V->CurrentHeadX, HEADTOP, V);                                          // Blit head and laser
                UpdateTrack(V);                                                                 // - Blit the track number too
                Blit(0, 173, 69, 94, 100, 18, V);                                               // - Blit half of disk
                BlitMask(264, 109, V->CurrentHeadX + 16, 74, 52, 33, V->ButtonBM->Planes[0]     // - Blit on laser
                         + V->ButtonBM->BytesPerRow * 33, V);
                }
            }

        else UpdateTrack(V);                                                                    // Blit the track number too

        V->LastHeadX = V->CurrentHeadX;                                                         // Remember where we are
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// DiskAnimation - Do disk/head animation                                               //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void DiskAnimation(UWORD Direction, struct V *V) {                                      // DiskAnimation()

WORD DPos, DVelocity, DAccel = -160;
WORD HPos, HVelocity, HAccel = -200;
WORD i;

    if (Direction) { DPos = 0;   DVelocity = 10000; HPos = -329+HEADTOP; HVelocity = 12000; }  // Set up movement stuff for disk-in
    else           { DPos = 276; DVelocity = 0;     HPos = HEADTOP;      HVelocity = 0;     }  // ... for disk-out

    WaitTOF();                                                                          // For consistency, wait for sync.

    if (!Direction) for (i=V->CurrentHeadX; i!=HEADLEFT; i--) {

        BlitHead(i, HPos, V);
        }

    while ((DPos < (276 + (1 - Direction))) && (DPos > -1)) {                           // Base termination on disk movement.

        HVelocity += HAccel;                                                            // Calculate new head position
        HPos      += (HVelocity / 1000);

        BlitHead(HEADLEFT, HPos, V);                                                    // Blit head

        DVelocity += DAccel;                                                            // Calculate new disk position
        DPos      += (DVelocity / 1000);

        BlitDisk(DPos, Direction, V);                                                   // Blit disk
        }

    if (Direction) for (i=HEADLEFT; i!=HEADLEFT+HEADRANGE+1; i++) {                     // Move the head

        BlitHead(i, HPos, V);                                                           // Do the blit
        }

    V->HeadX = V->CurrentHeadX = V->LastHeadX = i;                                      // Current head position
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// UpdateTime - Update the time display                                                 //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void UpdateTime(struct V *V) {                                                          // UpdateTime()

    if (V->PlayerState.Minus != V->LastPlayerState.Minus) {                             // Minus change?

        Blit((V->PlayerState.Minus ? 140:132), 131, 191, 135, 8, 3, V);                 // Display new minus
        }

    if (V->PlayerState.Second != V->LastPlayerState.Second) {                           // Second change?

        Display2Dig(V->PlayerState.Second, 0, 131, 242, 127, 12, 19, 6, V);             // Display new second
        }

    if (V->PlayerState.Minute != V->LastPlayerState.Minute) {                           // Minute change?

        Display2Dig(V->PlayerState.Minute, 0, 131, 202, 127, 12, 19, 6, V);             // Display new minute

        Blit(132 + ((V->PlayerState.Minute != 100) ? 0:2), 134, 236, 131, 2, 13, V);    // Display colon
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// UpdateGrid - Update grid display                                                     //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void UpdateGrid(struct V *V) {                                                          // UpdateGrid()

UWORD i, j, Flag = 0, EntryCount = V->PlayerState.Tracks;

    for (i=0; i!=20; i++) {                                                             // Check all 20 boxes

        if (   (V->LastEntryCount != EntryCount)                                        // Has element changed?
            || (V->LastGrid[i]    != V->PlayList->Entry[V->GridDisplayOffset + i])) {

            Flag = 1;                                                                   // Grid had changed
            }
        }

    if (Flag) {                                                                         // Has grid changed?

        WaitBeam(112, V);                                                               // Wait for beam to pass grid

        for (i=0; i!=20; i++) {                                                         // Check all 20 boxes

            j = V->GridDisplayOffset + i;                                               // Calculate playlist index

            if (   (V->LastEntryCount != EntryCount)                                    // Has element changed?
                || (V->LastGrid[i]    != V->PlayList->Entry[j])) {

                V->LastGrid[i] = V->PlayList->Entry[j];                                 // Update change

                if (((V->LastGrid[i] & PLEF_TRACK) == 100) || (j >= EntryCount)         // Display emptyness?
                                                           || !V->PlayerDisk) {

                    Display2Dig(100, 0, 122, 191 + 34 * (i % 5),                        // Element is empty
                                             27  + 23 * (i / 5), 7, 9, 3, V);
                    }

                else {                                                                  // Element is not empty

                    Display2Dig( V->LastGrid[i] & PLEF_TRACK,                           // Display digit
                               ((V->LastGrid[i] & PLEF_ENABLE) ? 0:77),
                               122, 191 + 34 * (i % 5), 27 + 23 * (i / 5), 7, 9, 3, V);
                    }
                }
            }
        }

    V->LastEntryCount = EntryCount;                                                     // Update change
    }

//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// UpdateSA - Update spectrum analyzer                                                  //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void UpdateSA(struct V *V) {                                                            // UpdateSA()

UWORD  x, y;
WORD   i;
UBYTE *Analysis, Analyzed = 0;

    if ((V->HeadX == V->LastHeadX) && (V->PlayerState.PlayState >= PLS_PLAYING)) {      // When do we analyze?

        Analysis = Analyze();                                                           // Get spectrum analysis
        Analyzed = 1;
        }

    for (i=10,x=41,V->SABlank=TRUE; i>=0; --i,x+=12) {                                  // Check all frequencies

        if (Analyzed) y = Analysis[i];                                                  // If we analyzed, get data
        else          y = 0;                                                            // ... else make bar drop

        if (y > V->SALevels[i]) {                                                       // Level increasing?

            V->SALevels[i] = y;                                                         // Blit new bar
            Blit(224, V->SALevels[i] * 3 + 184, x, 128, 10, 27, V);
            }

        else if (y < V->SALevels[i]) {                                                  // Level is decreasing

            if (--V->SALevels[i] < 0) V->SALevels[i] = 0;                               // Only allow decrease by 1
            Blit(224, 184, x, 152 - V->SALevels[i] * 3, 10, 3, V);
            }

        if (y) V->SABlank = FALSE;                                                      // Is spectrum analyzer blank?
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// InvertGridEntry - Add/Remove a track from the track list and recalculate play time   //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void InvertGridEntry(UWORD Entry, struct V *V) {                                        // InvertGridEntry()

UWORD j = V->Highlighted - B_GRID + V->GridDisplayOffset;                               // Calculate PlayList index

    if (V->PlayerState.PlayState) {                                                     // Are we stopped?

        SubmitKeyStroke(PKS_STOP|PKSF_PRESS);                                           // Tell player to stop
        SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);

        while (V->PlayerState.PlayState) { WaitTOF(); GetPlayerState(&V->PlayerState); }// Wait for stop to take effect
        }

    while (!ModifyPlayList(TRUE)) WaitTOF();                                            // Obtain semaphore on PlayList

    V->PlayList->Entry[j] =   V->PlayList->Entry[j] & PLEF_TRACK                        // Invert the track enable bit
                          | ((V->PlayList->Entry[j] & PLEF_ENABLE) ? 0:PLEF_ENABLE);

    while (!ModifyPlayList(FALSE)) WaitTOF();                                           // Free the semaphore

    SubmitKeyStroke(PKS_STOP|PKSF_PRESS);                                               // Recalculate play time
    SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// ReadTOC - Read table-of-contents of disk                                             //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void ReadTOC(struct V *V) {                                                             // ReadTOC()

    V->CDIO->io_Command = CD_TOCLSN;                                                    // Set up request
    V->CDIO->io_Offset  = 0;
    V->CDIO->io_Length  = 100;
    V->CDIO->io_Data    = &V->TOC[0];

    DoIO(V->CDIO);                                                                      // Do it
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// UpdateHeadPos - Update horizontal position of head                                   //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

void UpdateHeadPos(struct V *V) {

    if (!V->PlayerState.PlayState) V->HeadX = HEADLEFT + HEADRANGE;                     // Nothing selected.

    else if (   (V->PlayerState.PlayState < PLS_PLAYING)                                // Track is selected
             || (V->PlayerState.PlayMode == PLM_SKIPFWD)
             || (V->PlayerState.PlayMode == PLM_SKIPREV)) {

        V->HeadX = HEADLEFT + HEADRANGE                                                 // Calculate position based on TOC
                 - V->TOC[V->PlayerState.Track].Entry.Position.LSN * HEADRANGE
                 / V->TOC[0].Summary.LeadOut.LSN;
        }        

    else {                                                                              // We are playing

        if (++V->QCount >= 60) V->QCount = 0;                                           // Get head position once a second

        V->CDIO->io_Command = CD_QCODELSN;                                              // Get position we are located
        V->CDIO->io_Offset  =
        V->CDIO->io_Length  = 0;
        V->CDIO->io_Data    = &V->QCode;
        DoIO(V->CDIO);

        if (!V->CDIO->io_Error) {                                                       // Was there an error?

            V->HeadX = HEADLEFT + HEADRANGE                                             // Calculate new position
                     - V->QCode.DiskPosition.LSN * HEADRANGE
                     / V->TOC[0].Summary.LeadOut.LSN;
            }
        }
    }


//////////////////////////////////////////////////////////////////////////////////////////
//                                                                                      // 
// DoButton - Translate button keystrokes                                               //
//                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////

LONG DoButton(ULONG Code, struct V *V) {

    switch (Code) {

        case RAWKEY_ESC:

            SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
            SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);

            while (V->PlayerState.PlayState) { WaitTOF(); GetPlayerState(&V->PlayerState); }    // Wait for stop to take effect

            UpdateHead(V);

            V->PlayerState.Tracks    =
            V->PlayerState.AudioDisk =
            V->PlayerDisk            =
            V->PlayerState.Minus     = 0;
            V->PlayerState.Track     =
            V->PlayerState.Minute    =
            V->PlayerState.Second    = 100;

            return(1);

        case KEY_PLAYPAUSE:

            SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_PRESS);
            SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_RELEASE);
            break;

        case KEY_STOP:

            SubmitKeyStroke(PKS_STOP|PKSF_PRESS);
            SubmitKeyStroke(PKS_STOP|PKSF_RELEASE);
            break;

        case KEY_FREV:

            V->ButtonSelect[B_FREV] = TRUE;
            SubmitKeyStroke(PKS_REVERSE|PKSF_PRESS);
            break;

        case KEY_FFWD:

            V->ButtonSelect[B_FFWD] = TRUE;
            SubmitKeyStroke(PKS_FORWARD|PKSF_PRESS);
            break;

        case (KEY_FREV|IECODE_UP_PREFIX):

            V->ButtonSelect[B_FREV] = FALSE;
            SubmitKeyStroke(PKS_REVERSE|PKSF_RELEASE);
            break;

        case (KEY_FFWD|IECODE_UP_PREFIX):

            V->ButtonSelect[B_FFWD] = FALSE;
            SubmitKeyStroke(PKS_FORWARD|PKSF_RELEASE);
            break;

        case KEY_TIMEMODE:

            V->ButtonSelect[B_TIMEMODE] =
            V->PlayerOptions.TimeMode   = (V->PlayerOptions.TimeMode + 1) & 0x03;

            V->PlayerOptions.Loop = V->PlayerOptions.Intro = V->PlayerOptions.Subcode = -1;
            SetOptions(&V->PlayerOptions);
            GetOptions(&V->PlayerOptions);
            break;

        case KEY_SHUFFLE:

            V->ButtonSelect[B_SHUFFLE] = (V->ButtonSelect[B_SHUFFLE] ? 0:1);
            break;

        case KEY_LOOP:

            V->ButtonSelect[B_LOOP] =
            V->PlayerOptions.Loop   = (V->PlayerOptions.Loop ? 0:1);

            V->PlayerOptions.TimeMode = V->PlayerOptions.Intro = V->PlayerOptions.Subcode = -1;
            SetOptions(&V->PlayerOptions);
            GetOptions(&V->PlayerOptions);
            break;

        case KEY_INTRO:

            V->PlayerOptions.Intro  = (V->PlayerOptions.Intro ? 0:1);

            V->PlayerOptions.Loop = V->PlayerOptions.TimeMode = V->PlayerOptions.Subcode = -1;
            SetOptions(&V->PlayerOptions);
            GetOptions(&V->PlayerOptions);

            if (V->PlayerOptions.Intro && V->PlayerState.PlayState == PLS_STOPPED) {

                SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_PRESS);
                SubmitKeyStroke(PKS_PLAYPAUSE|PKSF_RELEASE);
                }

            while (V->PlayerState.PlayState < PLS_PLAYING) { WaitTOF(); GetPlayerState(&V->PlayerState); }    // Wait for stop to take effect

            break;

        case RAWKEY_ARROWLT:
        case RAWKEY_ARROWRT:
        case RAWKEY_ARROWUP:
        case RAWKEY_ARROWDN:

            if      (Code == RAWKEY_ARROWLT) V->Highlighted = V->Highlighted + DirInfo[V->Highlighted].Left;
            else if (Code == RAWKEY_ARROWRT) V->Highlighted = V->Highlighted + DirInfo[V->Highlighted].Right;
            else if (Code == RAWKEY_ARROWUP) V->Highlighted = V->Highlighted + DirInfo[V->Highlighted].Up;
            else if (Code == RAWKEY_ARROWDN) V->Highlighted = V->Highlighted + DirInfo[V->Highlighted].Down;

            if (V->KeyRepeat) V->RepeatCount = 15;
            else              V->RepeatCount = 60;

            V->KeyRepeat = Code;

            break;

        case (RAWKEY_ARROWLT|IECODE_UP_PREFIX):
        case (RAWKEY_ARROWRT|IECODE_UP_PREFIX):
        case (RAWKEY_ARROWUP|IECODE_UP_PREFIX):
        case (RAWKEY_ARROWDN|IECODE_UP_PREFIX): V->KeyRepeat = 0; break;

        case RAWKEY_ENTER:
        case RAWKEY_RETURN:

            if (ButtonInfo[V->Highlighted].Button) DoButton(ButtonInfo[V->Highlighted].Button, V);
            else                                   InvertGridEntry(V->Highlighted, V);

            break;

        case (RAWKEY_ENTER|IECODE_UP_PREFIX):
        case (RAWKEY_RETURN|IECODE_UP_PREFIX):

            if (ButtonInfo[V->Highlighted].Button) {

                DoButton(ButtonInfo[V->Highlighted].Button|IECODE_UP_PREFIX, V);
                }

            break;
        }

    return(0);
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                      // 
// DoPlayer - Player program event loop.                                                                //
//                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

void DoPlayer(struct V *V) {                                                                            // DoPlayer()

LONG                  i, Controller;
UWORD                 Exit = 0;
struct IntuiMessage  *Msg;

    Forbid();
    FadeIn(V);

    V->SABlank = TRUE;

    while (!Exit) {

        if (V->KeyRepeat) if (!--V->RepeatCount) DoButton(V->KeyRepeat, V);

        if (Msg = (struct IntuiMessage *)GetMsg(V->Window->UserPort)) {         
                                                                                
            if (Msg->Class == IDCMP_RAWKEY) Exit = DoButton(Msg->Code, V);
            }                                                                   

        if (!Exit) {

            GetPlayerState(&V->PlayerState);
            GetOptions(&V->PlayerOptions);
            }

        if      (V->PlayerState.PlayState == PLS_PAUSED)  V->ButtonSelect[B_PLAYPAUSE] = 2;
        else if (V->PlayerState.PlayState == PLS_PLAYING) V->ButtonSelect[B_PLAYPAUSE] = 1;
        else                                              V->ButtonSelect[B_PLAYPAUSE] = 0;

        V->ButtonSelect[B_STOP]  = (V->ButtonSelect[B_PLAYPAUSE] ? 0:1);

        if (Controller = GetController(1, V)) {

            if ((V->DirectionState = Controller & JP_DIRECTION_MASK) != V->LastDirectionState) {

                for (i=0; i!=4; i++) {

                    if (V->CDirection[i] != (Controller & (1 << i))) {

                        V->CDirection[i]  =  Controller & (1 << i);

                        DoButton(CDirs[i] | (V->CDirection[i] ? 0:IECODE_UP_PREFIX), V);
                        }

                    V->LastDirectionState = V->DirectionState;
                    }
                }

            if ((V->ButtonState = Controller & JP_BUTTON_MASK) != V->LastButtonState) {

                for (i=0; i!=7; i++) {

                    if (V->CButtons[i] != (Controller & (1 << (JPB_BUTTON_PLAY + i)))) {

                        V->CButtons[i]  =  Controller & (1 << (JPB_BUTTON_PLAY + i));

                        if (CKeys[i]) DoButton(CKeys[i] | (V->CButtons[i] ? 0:IECODE_UP_PREFIX), V);

                        else {

                            if (ButtonInfo[V->Highlighted].Button) {

                                DoButton(ButtonInfo[V->Highlighted].Button | (V->CButtons[i] ? 0:IECODE_UP_PREFIX), V);
                                }

                            else if (V->CButtons[i]) InvertGridEntry(V->Highlighted, V);
                            }
                        }
                    }
                                                                                
                V->LastButtonState = V->ButtonState;                            
                }                                                               
            }                                                                   

//**************

        if (V->PlayerState.AudioDisk == -1) V->PlayerState.AudioDisk = 1;

//**************

        if ((V->LastAudioDisk != V->PlayerState.AudioDisk) && (V->PlayerState.AudioDisk == 1)) {

            DiskAnimation(TRUE, V);
            Highlight(V->Highlighted, TRUE, V);
            ReadTOC(V);
            V->PlayerDisk = 1;
            }

        WaitTOF();

        if (V->PlayerOptions.Intro && (V->LastPlayerState.Track != V->PlayerState.Track)) {

            if (++V->ButtonSelect[B_INTRO] >= 5) V->ButtonSelect[B_INTRO] = 1;
            }

        if      ( V->PlayerOptions.Intro && !V->ButtonSelect[B_INTRO]) V->ButtonSelect[B_INTRO] = 1;
        else if (!V->PlayerOptions.Intro &&  V->ButtonSelect[B_INTRO]) V->ButtonSelect[B_INTRO] = 0;

        UpdateHead(V);
        UpdateGrid(V);
        UpdateTime(V);
        UpdateSA(V);
        UpdateButtons(V);
        if (V->HeadX == V->LastHeadX) UpdateHeadPos(V);

        if ((V->LastAudioDisk != V->PlayerState.AudioDisk) && !V->PlayerState.AudioDisk) {

            V->PlayerDisk = 0;
            Highlight(V->Highlighted, FALSE, V);
            DiskAnimation(FALSE, V);
            }

        else if ((V->LastAudioDisk != V->PlayerState.AudioDisk) && (V->PlayerState.AudioDisk == -1)) Exit = 1;

        V->LastPlayerState = V->PlayerState;
        V->LastAudioDisk   = V->PlayerState.AudioDisk;
        }                                                                       

    if (V->PlayerDisk == 1) {
                                                                                
        Highlight(V->Highlighted, FALSE, V);
        DiskAnimation(FALSE, V);
        }
                                                                                
    FadeOut(V);
    Permit();
    }                                                                           
                                                                                
