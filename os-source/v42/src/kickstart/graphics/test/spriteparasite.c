/****** AAHard.application/SpriteParasite *************************************

    NAME
        SpriteParasite -- Sprite Test

    VERSION
        1.00    04 Mar 1992 - Inception
        1.01    20 May 1992 - Reworked for new graphics calls

    SYNOPSIS
        Screen,BW=Bandwidth/N,Mode/NIFF=Picture/K,
        W=Width/N,H=Height/N,OH=OutputHeight/N/K,
        NS=NumberOfSprites/N,SP=StartSprite/N,ASPR=Attached/S,
        XR=XReplication/N,YR=YReplication/N,
        Test/N,Stress/N,Diag/K,Time/N/K

    FUNCTION
        Allocates sprites on given public screen. The screen
        is locked for the duration of the test, and is unlocked
        only after all of the sprites have been deallocated. If
        a picture is specified, it is read in and used;
        otherwise a default image will be generated. Appropiate
        colors are allocated for the sprites if available. The
        Sprites are then animated and collisions between them
        are checked for, and the results  of these are printed out
        if Diag is specified.

        SpriteParasite tests for: (1) proper hardware sprite function,
        (2) Proper operation of AllocSpriteData(), FreeSpriteData(),
        and GetExtSprite(), ChangeExtSprite().

    INPUTS
        Screen                  - name of Public Screen to perform test on
        BW=Bandwidth/N          - number representing the sprite bandwidth:
                                    0 - ECS (normal ECS-mode sprite)
                                    1 - 140 ns
                                    2 - 70 ns
                                    3 - 35 ns

        Mode/N                  - Data mode of sprite
                                    0 - Normal (1X)
                                    1 - Page (2X)
                                    2 - Wide (2X)
                                    3 - Page+Wide (4X)

        IFF=Picture/K           - Picture or anim file to use as sprite
        W=Width/N               - Width to make sprite (overrided by IFF)
        H=Height/N              - Height to make sprite (overrided by IFF)

        OH=OutputHeight/N/K     - Supply the output height to
                                  AllocSpriteData(), if given. This may be
                                  different from Height if supplied BitMap
                                  is too "tall".

        NS=NumberOfSprites/N    - Number of sprites to allocate
        SP=StartSprite/N        - Sprite to start allocations with
        ASPR=Attached/S         - Create attached sprites. SP Must be even
                                  if this is true.

        XR=XReplication/N       - Controls the horizontal pixel replication
                                  used when converting the BitMap data to
                                  a sprite. The possible values are:
                                    0   - Perform a 1 to 1 conversion.
                                    1   - Each pixel from the source is
                                          replicated twice.
                                    2   - Each pixel is replicated 4 times.
                                    -1  - Skip every other pixel.
                                    -2  - Only include every fourth pixel.

        YR=YReplication/N       - Controls the vertical pixel replication
                                  used when converting the BitMap data to
                                  a sprite. The possible values are:
                                    0   - Perform a 1 to 1 conversion.
                                    1   - Each pixel from the source is
                                          replicated twice.
                                    2   - Each pixel is replicated 4 times.
                                    -1  - Skip every other pixel.
                                    -2  - Only include every fourth pixel.

        Test/N                  - Test to perform
        Stress/N                - Stress test level
        Diag/K                  - Diagnostic output file
        Time/N/K                - Time, in milliseconds, to run the test

    NOTES
        There is support for sprites now. In spite of this, we will
        have to do this partly by hand. We shall avoid banging on
        metal as much as possible, but obviously this test,
        as a consequence, will be limited to AA machines.

        Internal details of a sprites on AA:

            NORMAL (1X) MODE

                RGA     Addr    Data
                ------------------------
                144     x       A
                146     x+2     B
                140     x+4     POS
                142     x+6     CTL
                144     x+8     A
                146     x+A     B

            PAGE (2X) MODE

                RGA     Addr    Data    Addr    Data
                -------------------------------------
                144     x       A       x+2     A'
                146     x+2     B       x+6     B'
                140     x+4     POS     x+A     no cycles
                142     x+6     CTL     x+E     no cycles
                144     x+8     A       x+12    A'
                146     x+A     B       x+16    B'

            WIDE (2X) MODE

                RGA     Addr    Data
                ----------------------------
                144     x       A       A'
                146     x+4     B       B'
                140     x+5     POS     X
                142     x+C     CTL     X
                144     x+10    A       A'
                146     x+14    B       B'

            PAGE + WIDE (4X) MODE

                RGA     Addr    Data            Addr    Data
                -----------------------------------------------------
                144     x       A       A'      x+4     A''     A'''
                146     x+8     B       B'      x+C     B''     B'''
                140     x+10    POS     X       x+14    no cycles
                142     x+18    CTL     X       x+1C    no cycles
                144     x+20    A       A'      x+24    A''     A'''
                146     x+??    B       B'      x+??    B''     B'''


******************************************************************************/

// Includes
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/videocontrol.h>
#include <graphics/view.h>
#include <graphics/sprite.h>

#include <libraries/iffparse.h>

#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/iffparse.h>

#include <stdlib.h>
#include "MWLib_protos.h"
#include "WBCliArgs.h"
#include "CALib_protos.h"
#include "mo/SpriteParasite.i"
#include "protos.h"
#include "rev.auto.h"

#ifdef FAST
#include <m68881.h>
#endif

// Defines
#define PROGRAM_NAME "SpriteParasite"
#define TEMPLATE    "Screen,BW=Bandwidth/N,Mode/N,IFF=Picture/K,W=Width/N,H=Height/N,OH=OutputHeight/N/K,NS=NumberOfSprites/N,SP=StartSprite/N,ASPR=Attached/S,XR=XReplication/N,YR=YReplication/N,Test/N,Stress/N,Diag/K,Time/N/K"
#define LBOOL       ULONG
#define MAX_VECTORS 100
#define MAX_SPRITES 8

#ifndef DEBUG
#define ECS_LIB_VERSION     37
#define AA_LIB_VERSION      39
#else // DEBUG
#define ECS_LIB_VERSION     37
#define AA_LIB_VERSION      39
#endif

// Synonyms
#define sWidth          (*env->args.Width)
#define sHeight         (*env->args.Height)
#define sNumberOfSprites (*env->args.NumberOfSprites)
#define sMode           (*env->args.Mode)
#define sXReplication   (*env->args.XReplication)
#define sYReplication   (*env->args.YReplication)

// Structs
struct SPEnv {
    struct args {
        char *Screen;
        long *Bandwidth;
        long *Mode;
        char *Picture;
        long *Width;
        long *Height;
        long *OutputHeight;
        long *NumberOfSprites;
        long *StartSprite;
        LBOOL Attached;
        long *XReplication;
        long *YReplication;
        long *Test;
        long *Stress;
        char *Diag;
        long *Time;
        } args;

    // Defaults
    long def_Bandwidth;
    long def_Mode;
    long def_Width;
    long def_Height;
    long def_NumberOfSprites;
    long def_StartSprite;
    long def_XReplication;
    long def_YReplication;
    long def_Test;
    long def_Stress;
    long def_Time;

    // Misc
    BPTR fd_diag;           // Diagnostic filehandle
    struct Screen *scr;     // Screen to display sprites on
    struct Window *win;     // Window to display sprite patterns in
    struct MsgPort *reply;  // port to receive replys
    };

enum error_codes {
    err_ok,
    err_out_of_memory,
    err_library_failure,
    err_screen,             // could not get public screen
    err_parameter,
    err_cannot_open_window,
    err_cnt
    };

// Globals
struct Library *IffparseBase, *UtilityBase, *TimerBase;

int main(int ac, char **av)
{
    long ret = 0;

    if (GfxBase = OpenLibrary("graphics.library", AA_LIB_VERSION))
    {
        if (IntuitionBase = OpenLibrary("intuition.library", ECS_LIB_VERSION))
        {
            if (IffparseBase = OpenLibrary("iffparse.library", ECS_LIB_VERSION))
            {
                struct SPEnv *env;

                if (env = AllocVec(sizeof(*env), MEMF_CLEAR | MEMF_PUBLIC))
                {
                    struct WBCli *wbc;

                    // Init environment
                    env->args.Screen = "Workbench";
                    env->args.Bandwidth       = &env->def_Bandwidth;
                    env->args.Mode            = &env->def_Mode;
                    env->args.Width           = &env->def_Width;
                    env->args.Height          = &env->def_Height;
                    env->args.NumberOfSprites = &env->def_NumberOfSprites;
                    env->args.StartSprite     = &env->def_StartSprite;
                    env->args.XReplication    = &env->def_XReplication;
                    env->args.YReplication    = &env->def_YReplication;
                    env->args.Test            = &env->def_Test;
                    env->args.Stress          = &env->def_Stress;
                    env->args.Time            = &env->def_Time;

                    env->def_Bandwidth = SPRITERESN_ECS;
                    env->def_Mode = 0;
                    env->def_Width = 16; // &&& blew up when set to 1
                    env->def_Height = 32;
                    env->def_NumberOfSprites = 4;
                    env->def_StartSprite = 1;
                    env->def_Test = 1;
                    env->def_Stress = 1;
                    env->def_Time = 10000;

                    if (wbc = WBCliArgsTags(WC_Startup, av,
                                            WC_TrustStartup, !ac,
                                            WC_DiskObjectName, PROGRAM_NAME,
                                            WC_Template, TEMPLATE,
                                            WC_Args, &env->args,
                                            WC_Project, TRUE,
                                            TAG_END))
                    {
                        // Diagnostic Output
                        if (env->args.Diag && (env->fd_diag = Open(env->args.Diag, MODE_NEWFILE)))
                        {
                            char *bwidth[] = {"ECS", "140 ns", "70 ns", "35 ns", "INVALID"};
                            char *mode[] = {"Normal (1X)", "Page (2X)", "Wide (2X)", "Page+Widew (4X)"};

                            FPrintf(env->fd_diag, "******* %s %s Sprite Test\n", PROGRAM_NAME, VERS);
                            FPrintf(env->fd_diag, "    Screen = %s\n", (ULONG) env->args.Screen);
                            FPrintf(env->fd_diag, "    Bandwidth = %ld [%s]\n", *env->args.Bandwidth, bwidth[*env->args.Bandwidth]);
                            FPrintf(env->fd_diag, "    Mode = %ld [%s]\n", *env->args.Mode, mode[*env->args.Mode]);
                            FPrintf(env->fd_diag, "    Picture = %s\n", (ULONG) ((env->args.Picture) ? env->args.Picture : "(none)"));
                            FPrintf(env->fd_diag, "    Width = %ld\n", *env->args.Width);
                            FPrintf(env->fd_diag, "    Height = %ld\n", *env->args.Height);
                            FPrintf(env->fd_diag, "    NumberOfSprites = %ld\n", *env->args.NumberOfSprites);
                            FPrintf(env->fd_diag, "    StartSprite = %ld\n", *env->args.StartSprite);
                            FPrintf(env->fd_diag, "    Attached Sprites = %s\n", (ULONG) (env->args.Attached) ? "TRUE" : "FALSE");
                            FPrintf(env->fd_diag, "    XReplication = %ld\n", *env->args.XReplication);
                            FPrintf(env->fd_diag, "    YReplication = %ld\n", *env->args.YReplication);
                            FPrintf(env->fd_diag, "    Test = %ld\n", *env->args.Test);
                            FPrintf(env->fd_diag, "    Stress = %ld\n", *env->args.Stress);
                            FPrintf(env->fd_diag, "    Time = %ld ms\n", *env->args.Time);
                            FPrintf(env->fd_diag, "    Diag = %s\n\n", (ULONG) env->args.Diag);
                        }

                        ret = SpriteParasite(env);

                        FreeWBCli(wbc);
                    }
                    else ret = err_parameter;

                    if (env->fd_diag) // Close diagnostic channel
                    {
                        FPrintf(env->fd_diag, "******* %s %s - Session Terminated\n", (ULONG) PROGRAM_NAME, VERS);
                        Close(env->fd_diag);
                    }

                    FreeVec(env);
                }
                else ret = err_out_of_memory;

                CloseLibrary(IffparseBase);
            }
            else ret = err_library_failure;

            CloseLibrary(IntuitionBase);
        }
        else ret = err_library_failure;

        CloseLibrary(GfxBase);
    }
    else ret = err_library_failure;

    exit( (ret) ? ret+20 : 0 );
    return (ret) ? ret+20 : 0;
}

long SpriteParasite(struct SPEnv *env)
{
    long ret = 0;

    if (env->scr = LockPubScreen(env->args.Screen))
    {
        Point org;

        VPOrigin(ViewAddress(), &env->scr->ViewPort, &org, NULL); // Where to display window

        if (env->win = OpenWindowTags(NULL, WA_CustomScreen, env->scr,
                                            WA_Left, org.x,
                                            WA_Top, org.y,
                                            WA_InnerWidth, sWidth * sNumberOfSprites,
                                            WA_InnerHeight, sHeight,
                                            //WA_SizeGadget, TRUE,
                                            WA_DragBar, TRUE,
                                            WA_DepthGadget, TRUE,
                                            WA_CloseGadget, TRUE,
                                            WA_NoCareRefresh, TRUE,
                                            WA_SmartRefresh, TRUE,
                                            WA_Activate, TRUE,
                                            WA_GimmeZeroZero, TRUE,
                                            WA_AutoAdjust, TRUE,
                                            WA_ScreenTitle, VERS,
                                            WA_Title, "Sprite Parasite -- Sprite BitMap Dump",
                                            WA_MaxWidth, -1,
                                            WA_MaxHeight, -1,
                                            WA_MinWidth, sWidth * sNumberOfSprites,
                                            WA_MinHeight, sHeight,
                                            WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE,
                                            TAG_DONE))
        {
            if (env->reply = CreateMsgPort())
            {
                switch(*env->args.Test)
                {
                case 1: ret = TestCase1(env); break;

                default: break;
                }

                DeleteMsgPort(env->reply);
            }
            else ret = err_out_of_memory;

            CloseWindow(env->win);
        }
        else ret = err_cannot_open_window;

        UnlockPubScreen(NULL, env->scr);
    }
    else ret = err_screen;

    return ret;
}

long TestCase1(struct SPEnv *env)
{
    long ret = 0;
    struct RastPort **arp;

    if (arp = AllocVec(sizeof(*arp) * MAX_SPRITES, MEMF_CLEAR))
    {
        long i, j;
        long qx, qy, bug; // bug to eliminate an internal error in the compiler

        // Create rastports and draw patterns in them
        for (i = *env->args.StartSprite, qx=qy=0; i < sNumberOfSprites + *env->args.StartSprite; ++i)
            if (arp[i] = CreateRastPortBitMap(*env->args.Width, *env->args.Height, 2, MAX_VECTORS, FALSE))
            {
                SetDrMd(arp[i], JAM2);
                for (j = 1; j < 4; ++j)
                {
                    SetAPen(arp[i], j);
                    DrawEllipse(arp[i], sWidth / 2, sHeight / 2, sWidth / (1+j) - 2, sHeight / (1+j) - 2);
                }

                BltBitMapRastPort(arp[i]->BitMap, 0, 0, env->win->RPort, qx, qy, sWidth, bug=sHeight, 0x0c0);
                if ((qx += sWidth) + sWidth >= env->win->Width)
                {
                    qx = 0;
                    qy += sHeight;
                }
            }
            else
            {
                ret = err_out_of_memory;
                break;
            }

        if (!ret)
        {
            struct ExtSprite **ass;

            if (ass = AllocVec(sizeof(*ass) * MAX_SPRITES, MEMF_CLEAR))
            {
                BOOL *spused = NULL;

                // Create Sprites
                if (env->fd_diag) FPrintf(env->fd_diag, "Sprite Creation...\n", 0);
                for (i = *env->args.StartSprite; i < sNumberOfSprites + *env->args.StartSprite; ++i)
                {
                    if (ass[i] = AllocSpriteData(arp[i]->BitMap, SPRITEA_Width, sWidth,
                                                                 SPRITEA_Attached, env->args.Attached,
                                                                 SPRITEA_XReplication, sXReplication,
                                                                 SPRITEA_YReplication, sYReplication,
                                                                 (env->args.OutputHeight) ? SPRITEA_OutputHeight : TAG_IGNORE,
                                                                        (env->args.OutputHeight) ? *env->args.OutputHeight : 0,
                                                                 TAG_DONE))
                    {
                        ass[i]->es_SimpleSprite.num = i;
                        if (env->fd_diag) FPrintf(env->fd_diag, "\tSprite #%ld created\n", i);
                    }
                    else
                    {
                        if (env->fd_diag) FPrintf(env->fd_diag, "\tSprite #%ld failed -- aborting\n", i);
                        ret = err_out_of_memory;
                        break;
                    }
                }

                // allocate sprite used array
                if (!(spused = AllocVec(sizeof(*spused) * MAX_SPRITES, MEMF_CLEAR)))
                    ret = err_out_of_memory;

                if (!ret)
                {
                    long x, y;
                    BOOL done;

                    // Set up sprites!
                    if (env->fd_diag) FPrintf(env->fd_diag, "Sprite Set Up...\n", 0);
                    for (i = *env->args.StartSprite; i < sNumberOfSprites + *env->args.StartSprite; i += (env->args.Attached) ? 2 : 1)
                    {
                        if (spused[i] = GetExtSprite(ass[i], GSTAG_SPRITE_NUM, i,
                                                             (env->args.Attached) ? GSTAG_ATTACHED : TAG_IGNORE, ass[i+1],
                                                             TAG_DONE) != -1)
                        {
                            if (env->fd_diag) FPrintf(env->fd_diag, "\tSprite %ld being set up\n", i);
                            if (!ChangeExtSprite(&env->scr->ViewPort, ass[i], ass[i], TAG_DONE))
                            {
                                Printf("?? ChangeExtSprite() failed for sprite #%ld\n", i);
                                if (env->fd_diag) FPrintf(env->fd_diag, "?? ChangeExtSprite() failed for sprite #%ld\n", i);
                            }
                        }
                    }

                    // Do Sprites
                    if (env->fd_diag) FPrintf(env->fd_diag, "Do Sprites...\n", 0);
                    for (done = FALSE, x = 0; !done && x < env->scr->Width; ++x)
                        for (y = 0; !done && y < env->scr->Height; ++y)
                        {
                            struct IntuiMessage *mess, im;

                            WaitTOF();
                            for (i = *env->args.StartSprite; i < sNumberOfSprites + *env->args.StartSprite; ++i)
                                if (spused[i])
                                    MoveSprite(&env->scr->ViewPort, (struct SimpleSprite *) ass[i], x * (i + 1), y + i);

                            if (mess = GetMsg(env->win->UserPort))
                            {
                                im = *mess;
                                ReplyMsg((struct Message *) mess);
                                switch(im.Class)
                                {
                                case IDCMP_CLOSEWINDOW: done = TRUE; break;
                                case IDCMP_NEWSIZE:     break;
                                }
                            }
                        }

                    // Cleanup Sprite Allocations
                    for (i = *env->args.StartSprite; i < sNumberOfSprites + *env->args.StartSprite; ++i)
                        if (spused[i])
                        {
                            FreeSprite(i);
                            if (env->fd_diag) FPrintf(env->fd_diag, "Sprite #%ld freed\n", i);
                        }
                }

                // Cleanup Sprites
                if (spused) FreeVec(spused);

                for (i = *env->args.StartSprite; i < sNumberOfSprites + *env->args.StartSprite; ++i)
                    if (ass[i])
                    {
                        FreeSpriteData(ass[i]);
                        if (env->fd_diag) FPrintf(env->fd_diag, "Sprite #%ld memory returned\n", i);
                    }
                FreeVec(ass);
            }
            else
                ret = err_out_of_memory;
        }

        // Cleanup RastPorts
        for (i = *env->args.StartSprite; i < *env->args.NumberOfSprites + *env->args.StartSprite; ++i)
            if (arp[i])
                FreeRastPortBitMap(arp[i]);
        FreeVec(arp);
    }
    else
        ret = err_out_of_memory;

    return ret;
}

