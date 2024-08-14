
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
#include <intuition/pointerclass.h>
#include <utility/tagitem.h>
#include <libraries/debox.h>
#include <libraries/lowlevel.h>
#include <hardware/blit.h>
#include <internal/playerlibrary.h>

#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/alib_protos.h>
#include <clib/debox_protos.h>
#include <clib/lowlevel_protos.h>
#include <internal/player_protos.h>

#include <pragmas/intuition_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/debox_pragmas.h>
#include <pragmas/lowlevel_pragmas.h>
#include <internal/player_pragmas.h>

#include "animation.h"
#include "player.h"

#define SysBase       (*((LONG *)4))
#define LowLevelBase  V->LowLevelBase
#define DeBoxBase     V->DeBoxBase
#define IntuitionBase V->IntuitionBase
#define GfxBase       V->GfxBase
#define PlayerBase    V->PlayerBase

extern UBYTE * __far screendata;
extern UBYTE * __far buttondata;
extern UBYTE * __far cdgdata;

extern LONG    __asm FakeOWB(void);

extern struct Task *CreateColorChanger(struct V *V);
extern void         DeleteColorChanger(struct V *V);
extern void         DoPlayer(struct V *V);
extern void         UpdateButtons(struct V *V);
extern void         Highlight(UWORD Box, UWORD Enable, struct V *V);

extern struct ButtonInfo ButtonInfo[];


//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              // 
// DoAlert - Create an alert box and display the supplied string                                //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

void DoAlert(char *String, struct V *V) {                                                       // DoAlert()

UWORD i;
UBYTE AlertString[100] = { 0, 50, 20 };

    for (i=0; String[i]; i++) AlertString[3+i] = String[i];                                     // Copy the string

    AlertString[3+i] = AlertString[4+i] = 0;                                                    // Terminate the alert

    DisplayAlert(RECOVERY_ALERT, AlertString, 37);                                              // Display the alert
    }


//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              // 
// InitPlayer - Prepares player program to be started.                                          //
//                                                                                              //
//              This function initializes global data including color tables and gradient       //
//              background.  The screen is then openned and DoPlayer() is called which is       //
//              the actual player program event loop.                                           //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

void InitPlayer(struct V *V) {                                                                  // BeginPlayer()

struct Rectangle    textoscan, maxoscan, dclip;
WORD                XOFF, YOFF;

APTR                OWB;
struct ColorSpec    ColorSpec[NUM_COLORS+1];

LONG                Result, i;

    V->ColorPack.cp_NumColors  = NUM_COLORS;                                                    // Initialize fade color pack
    V->ColorPack.cp_FirstColor = 0;
    V->ColorPack.cp_EndMarker  = 0;

    for (i=0; i!=NUM_COLORS; i++) {                                                             // Build fade color table

        V->ColorPack.cp_Colors[i].Red   = ((V->ScreenBMI->bmi_ColorMap[i] >> 8) & 0x0F)
                                        * 0x11111111;
        V->ColorPack.cp_Colors[i].Green = ((V->ScreenBMI->bmi_ColorMap[i] >> 4) & 0x0F)
                                        * 0x11111111;
        V->ColorPack.cp_Colors[i].Blue  =  (V->ScreenBMI->bmi_ColorMap[i]       & 0x0F)
                                        * 0x11111111;

        ColorSpec[i].ColorIndex = i;                                                            // Initial colors (all black)
        ColorSpec[i].Red        =
        ColorSpec[i].Green      =
        ColorSpec[i].Blue       = 0;
        }

    ColorSpec[i].ColorIndex = -1;                                                               // Terminate color spec.

    QueryOverscan(LORES_KEY, &textoscan, OSCAN_TEXT);                                           // Get overscan information
    QueryOverscan(LORES_KEY, &maxoscan,  OSCAN_MAX);

    dclip.MinX = maxoscan.MinX;                                                                 // Create a centered DClip
    dclip.MinY = maxoscan.MinY;
    dclip.MaxX = textoscan.MaxX;
    dclip.MaxY = textoscan.MaxY;

    XOFF = ((dclip.MaxX + 1) - V->ScreenBMI->bmi_Width ) / 2;
    YOFF = ((dclip.MaxY + 1) - V->ScreenBMI->bmi_Height) / 2;

    if (V->Screen = OpenScreenTags(NULL, SA_Colors,      (ULONG)&ColorSpec[0],                  // Open screen (faded black)
                                         SA_DClip,       (ULONG)&dclip,
                                         SA_Left,        XOFF,
                                         SA_Top,         YOFF,
                                         SA_Width,       V->ScreenBMI->bmi_Width,
                                         SA_Height,      V->ScreenBMI->bmi_Height,
                                         SA_Depth,       V->ScreenBMI->bmi_Depth,
                                         SA_BitMap,      (ULONG)V->ScreenBM,
                                         SA_Type,        CUSTOMSCREEN|CUSTOMBITMAP|SCREENQUIET,
                                         SA_DisplayID,   LORES_KEY,
                                         SA_Quiet,       TRUE,
                                         SA_Exclusive,   TRUE,
                                         SA_Draggable,   FALSE,
                                         SA_Interleaved, TRUE,
                                         TAG_DONE)) {

        if (V->ColorRange = AllocVec(sizeof(struct ColorRange) *                                // Create range info for fade
            ((V->Screen->Height/COPPER_RATE) + 1), MEMF_CLEAR)) {

            for (i=0; i!=B_NUMBUTTONS; i++) {                                                   // Touch all buttons

                V->ButtonSelect[i]     = 0;                                                     // Touch
                V->LastButtonSelect[i] = (UWORD)-1;
                }

            V->ButtonSelect[B_STOP] = 1;                                                        // Player is stopped

            GetOptions(&V->PlayerOptions);                                                      // Set current player options
            V->ButtonSelect[B_TIMEMODE] = V->PlayerOptions.TimeMode;
            V->ButtonSelect[B_LOOP]     = V->PlayerOptions.Loop;
            V->ButtonSelect[B_INTRO]    = V->PlayerOptions.Intro;

            V->LastHighlighted = V->Highlighted = B_PLAYPAUSE;                                  // Highlight play/pause

            UpdateButtons(V);                                                                   // Blit all buttons to screen

            Highlight(V->LastHighlighted, FALSE, V);                                            // Don't show highlight yet

            Result = ReadJoyPort(0);
            V->LastMouseX = (BYTE)(Result & 0xFF);
            V->LastMouseY = (BYTE)((Result >> 8) & 0xFF);

            if (CreateColorChanger(V)) {

                if (V->PointerBM = AllocBitMap(64,1,2,BMF_CLEAR,NULL)) {

                    if (V->Pointer = NewObject(NULL,
                                              "pointerclass",
                                               POINTERA_BitMap,
                                               V->PointerBM,
                                               TAG_DONE)) {

                        if (V->Window = OpenWindowTags(NULL,
                                        WA_CustomScreen,  V->Screen,
                                        WA_Borderless,    TRUE,
                                        WA_Backdrop,      TRUE,
                                        WA_RMBTrap,       TRUE,
                                        WA_Activate,      TRUE,
                                        WA_IDCMP,         IDCMP_RAWKEY,
                                        WA_RptQueue,      0,
                                        WA_NoCareRefresh, TRUE,
                                        WA_SimpleRefresh, TRUE,
                                        WA_BackFill,      LAYERS_NOBACKFILL,
                                        WA_Pointer,       V->Pointer,
                                        TAG_DONE)) {

                            if (V->PlayList = ObtainPlayList()) {

                                DoPlayer(V);                                                    // Run the player
                                }

                            else DoAlert("Could not obtain PlayList", V);

                            CloseWindow(V->Window);
                            }

                        else DoAlert("Could not open window", V);

                        DisposeObject(V->Pointer);
                        }

                    else DoAlert("Could not create object", V);

                    FreeBitMap(V->PointerBM);
                    }

                else DoAlert("Could not allocate bit map", V);

                DeleteColorChanger(V);
                }

            else DoAlert("Could not create color changer", V);                                  // Error

            FreeVec(V->ColorRange);                                                             // Free color range info
            }

        else DoAlert("Could not allocate color range", V);                                      // Error

        OWB = SetFunction(IntuitionBase, -0xd2, (APTR)&FakeOWB);                                // Done showing screen
        CloseScreen(V->Screen);
        SetFunction(IntuitionBase, -0xd2, OWB);
        }

    else DoAlert("Could not open screen", V);                                                   // Error
    }


//////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                              // 
// DoDebox - Allocate and decompress bit map information                                        //
//                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////

void DoDebox(struct V *V) {

    if (V->ScreenBMI = DecompBMInfo(NULL, NULL, &screendata)) {                                 // Get Info about pic

        if (V->ScreenBM = AllocBitMap(V->ScreenBMI->bmi_Width,                                  // Allocate bit map
                                      V->ScreenBMI->bmi_Height,
                                      V->ScreenBMI->bmi_Depth,
                                      BMF_DISPLAYABLE, NULL)) {

            if (!DecompBitMap(NULL, &screendata, V->ScreenBM, NULL)) {                          // Decompress bit map


                if (V->ButtonBMI = DecompBMInfo(NULL, NULL, &buttondata)) {                     // Get Info about pic
    
                    if (V->ButtonBM = AllocBitMap(V->ButtonBMI->bmi_Width,                      // Allocate bit map
                                                  V->ButtonBMI->bmi_Height,
                                                  V->ButtonBMI->bmi_Depth,
                                                  BMF_DISPLAYABLE, NULL)) {

                        if (!DecompBitMap(NULL, &buttondata, V->ButtonBM, NULL)) {              // Decompress bit map


                            if (V->cdgBMI = DecompBMInfo(NULL, NULL, &cdgdata)) {               // Get Info about pic

                                if (V->cdgBM = AllocBitMap(V->cdgBMI->bmi_Width,                // Allocate bit map
                                                              V->cdgBMI->bmi_Height,
                                                              V->cdgBMI->bmi_Depth,
                                                              BMF_DISPLAYABLE, NULL)) {

                                    if (!DecompBitMap(NULL, &cdgdata, V->cdgBM, NULL)) {        // Decompress bit map

                                        InitPlayer(V);                                          // Initialize player
                                        }

                                    else DoAlert("Could not decompress bit map", V);            // Error

                                    FreeBitMap(V->cdgBM);                                       // Free bit map
                                    }

                                else DoAlert("Could not allocate bit map", V);                  // Error

                                FreeBMInfo(V->cdgBMI);                                          // Free bit map info
                                }

                            else DoAlert("Could not decompress bit map info", V);               // Error
                            }

                        else DoAlert("Could not decompress bit map", V);                        // Error

                        FreeBitMap(V->ButtonBM);                                                // Free bit map
                        }

                    else DoAlert("Could not allocate bit map", V);                              // Error

                    FreeBMInfo(V->ButtonBMI);                                                   // Free bit map info
                    }

                else DoAlert("Could not decompress bit map", V);                                // Error
                }

            else DoAlert("Could not decompress bit map info", V);                               // Error

            FreeBitMap(V->ScreenBM);                                                            // Free bit map
            }

        else DoAlert("Could not allocate bit map", V);                                          // Error

        FreeBMInfo(V->ScreenBMI);                                                               // Free bit map info
        }

    else DoAlert("Could not decompress bit map info", V);                                       // Error
    }


//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                      // 
// MultiPlayer - Entry point into MultiPlayer program.                                                  //
//                                                                                                      //
//               This function opens libraries and allocates memory, and calls DoDebox()                //
//               to continue with the player program, and when finished closes the libraries.           //
//                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(void) {                                                                                        // MultiPlayer()

struct V *V;

    Forbid();                                                                                           // Bump our priority      
    SetTaskPri(FindTask(NULL), 50);
    Permit();

    if (V = (struct V *)AllocVec(sizeof(struct V), MEMF_PUBLIC | MEMF_CLEAR)) {                         // Allocate variables

        if (IntuitionBase = OpenLibrary("intuition.library", 0)) {                                      // For RGB stuff

            if (LowLevelBase = OpenLibrary("lowlevel.library", 0)) {                                    // For controller

                if (DeBoxBase = OpenLibrary("debox.library", 0)) {                                      // For compression

#undef              GfxBase
                    if (V->GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 0)) {            // For graphics stuff
#define GfxBase V->GfxBase

                        if (PlayerBase = OpenLibrary("player.library", 0)) {                            // For compression

                            if (PlayerBase->lib_Revision >= 13) V->SAAvailable = TRUE;                  // does lib support SA?

                            if (V->CDReply = (struct MsgPort *)CreateMsgPort()) {                       // Create reply port

                                if (V->CDIO = (struct IOStdReq *)CreateStdIO(V->CDReply)) {             // Create IO request

                                    if (!OpenDevice("cd.device", 0, V->CDIO, 0)) {                      // Open cd.device

                                        SetChipRev(SETCHIPREV_BEST);                                    // Go into AA mode

                                        DoDebox(V);                                                     // Create screens

                                        CloseDevice(V->CDIO);                                           // Close cd.device
                                        }

                                    else DoAlert("Could not open cd.device", V);                        // Could not open

                                    DeleteStdIO(V->CDIO);                                               // Delete IORequest
                                    }

                                else DoAlert("Could not create IORequest", V);                          // Could not create

                                DeleteMsgPort(V->CDReply);                                              // Delete MsgPort
                                }

                            else DoAlert("Could not create MsgPort", V);                                // Could not create

                            CloseLibrary(PlayerBase);                                                   // Close player lib
                            }

                        else DoAlert("Could not open player.library", V);                               // Could not open

                        CloseLibrary(GfxBase);                                                          // Close graphics lib
                        }

                    else DoAlert("Could not open graphics.library", V);                                 // Could not open

                    CloseLibrary((struct Library *)DeBoxBase);                                          // Close debox library
                    }

                else DoAlert("Could not open debox.library", V);                                        // Could not open

                CloseLibrary(LowLevelBase);                                                             // Close lowlevel
                }

            else DoAlert("Could not open lowlevel.library", V);                                         // Could not open        

            CloseLibrary(IntuitionBase);                                                                // Close intuition
            }

        FreeVec(V);                                                                                     // Free variable base
        }

    exit(0);
    }


