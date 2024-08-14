
#include "Toolmaker.h"
#include "Externs.h"

#include <stdlib.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

void EditColors(void)
  {
  int    i;
  BOOL   done=FALSE;
  BOOL   error=FALSE;
  struct NewGadget NG_Temp;
  struct Gadget    *G_List, *G_Context;
  struct Gadget    *G_OK, *G_Cancel;

  DEBUGENTER("EditColors", NULL);

  MainWindowSleep();

  for(i=0; i<1<<screennode.Depth; i++) newscreennode.Color[i] = screennode.Color[i];

  currentcolor = 0;
  currentred   = COLOR2RED(newscreennode.Color[0]);
  currentgreen = COLOR2GREEN(newscreennode.Color[0]);
  currentblue  = COLOR2BLUE(newscreennode.Color[0]);

  G_List = NULL;
  G_Context = CreateContext(&G_List);

  NG_Temp.ng_TextAttr = &TOPAZ80;
  NG_Temp.ng_VisualInfo = VisualInfo;
  NG_Temp.ng_UserData = NULL;

  NG_Temp.ng_LeftEdge = 10;
  NG_Temp.ng_TopEdge = currenttopborder+10;
  NG_Temp.ng_Width = 210;
  NG_Temp.ng_Height = 20;
  NG_Temp.ng_GadgetID = ID_SCREENPALETTE;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "";
  G_ScreenPalette = CreateGadget(PALETTE_KIND, G_Context, &NG_Temp, GTPA_Depth, screennode.Depth, GTPA_IndicatorWidth, 40, GTPA_Color, 0, TAG_DONE);

  /* R G B */

  NG_Temp.ng_LeftEdge = 98;
  NG_Temp.ng_TopEdge = currenttopborder+35;
  NG_Temp.ng_Width = 120;
  NG_Temp.ng_Height = 12;
  NG_Temp.ng_GadgetID = ID_SCREENRED;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "Red:   ";
  G_ScreenRed = CreateGadget(SLIDER_KIND, G_ScreenPalette, &NG_Temp,
                                  GTSL_Min, 0,
                                  GTSL_Max, 15,
                                  GTSL_Level, currentred,
                                  GTSL_MaxLevelLen, 3,
                                  GTSL_LevelFormat, (ULONG) "%2ld",
                                  GTSL_LevelPlace, PLACETEXT_LEFT,
                                  TAG_DONE);

  NG_Temp.ng_LeftEdge = 98;
  NG_Temp.ng_TopEdge = currenttopborder+50;
  NG_Temp.ng_Width = 120;
  NG_Temp.ng_Height = 12;
  NG_Temp.ng_GadgetID = ID_SCREENGREEN;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "Green:   ";
  G_ScreenGreen = CreateGadget(SLIDER_KIND, G_ScreenRed, &NG_Temp,
                                    GTSL_Min, 0,
                                    GTSL_Max, 15,
                                    GTSL_Level, currentgreen,
                                    GTSL_MaxLevelLen, 3,
                                    GTSL_LevelFormat, (ULONG) "%2ld",
                                    GTSL_LevelPlace, PLACETEXT_LEFT,
                                    TAG_DONE);

  NG_Temp.ng_LeftEdge = 98;
  NG_Temp.ng_TopEdge = currenttopborder+65;
  NG_Temp.ng_Width = 120;
  NG_Temp.ng_Height = 12;
  NG_Temp.ng_GadgetID = ID_SCREENBLUE;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "Blue:   ";
  G_ScreenBlue = CreateGadget(SLIDER_KIND, G_ScreenGreen, &NG_Temp,
                                  GTSL_Min, 0,
                                  GTSL_Max, 15,
                                  GTSL_Level, currentblue,
                                  GTSL_MaxLevelLen, 3,
                                  GTSL_LevelFormat, (ULONG) "%2ld",
                                  GTSL_LevelPlace, PLACETEXT_LEFT,
                                  TAG_DONE);

  /* OK and Cancel */

  NG_Temp.ng_TopEdge = currenttopborder+88;
  NG_Temp.ng_Width = 80;
  NG_Temp.ng_Height = 15;
  NG_Temp.ng_Flags = PLACETEXT_IN;

  NG_Temp.ng_LeftEdge = 10;
  NG_Temp.ng_GadgetText = "_OK";
  NG_Temp.ng_GadgetID = ID_OK;
  G_OK = CreateGadget(BUTTON_KIND, G_ScreenBlue, &NG_Temp, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 138;
  NG_Temp.ng_GadgetText = "_Cancel";
  NG_Temp.ng_GadgetID = ID_CANCEL;
  G_Cancel = CreateGadget(BUTTON_KIND, G_OK, &NG_Temp, GT_Underscore, '_', TAG_DONE);

  if(G_Cancel)
    {
    if(W_Colors = OpenWindowTags(NULL,
                                 WA_PubScreen, screennode.Screen,
                                 WA_Left, CenterHoriz(222),
                                 WA_Top,  CenterVert(108),
                                 WA_IDCMP, colorsidcmp,
                                 WA_Gadgets, G_List,
                                 WA_Title, "Palette",
                                 WA_Activate, TRUE,
                                 WA_DragBar, TRUE,
                                 WA_DepthGadget, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 WA_InnerWidth, 222,
                                 WA_InnerHeight, 108,
                                 TAG_DONE))
      {
      GT_RefreshWindow(W_Colors, NULL);

      colorssignal = 1L << W_Colors->UserPort->mp_SigBit;

      while(!done)
        {
        Wait(colorssignal);
        done = HandleRequesterIDCMP(W_Colors);
        }

      ModifyIDCMP(W_Colors, NULL);
      colorssignal = NULL;

      switch(done)
        {
        case DONE_CANCEL:
          for(i=0; i<1<<screennode.Depth; i++)
            {
            SetRGB4(&screennode.Screen->ViewPort, i, COLOR2RED(screennode.Color[i]),
                                                     COLOR2GREEN(screennode.Color[i]),
                                                     COLOR2BLUE(screennode.Color[i]));
            }
          break;

        case DONE_OK:
          for(i=0; i<1<<screennode.Depth; i++) screennode.Color[i] = newscreennode.Color[i];
          modified = TRUE;
          break;
        }

      CloseWindow(W_Colors);
      }
    else
      {
      error = TRUE;  /* Error opening window */
      }

    FreeGadgets(G_List);
    }
  else
    {
    error = TRUE;  /* Error creating gadgets */
    }

  if(error)
    {
    PutError("Out of memory\nopening palette window", "OK");
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenPaletteGadget(int code)
  {
  DEBUGENTER("HandleScreenPaletteGadget", NULL);

  currentcolor = code;
  currentred =   COLOR2RED(newscreennode.Color[currentcolor]);
  currentgreen = COLOR2GREEN(newscreennode.Color[currentcolor]);
  currentblue =  COLOR2BLUE(newscreennode.Color[currentcolor]);
  GT_SetGadgetAttrs(G_ScreenRed,   W_Colors, NULL, GTSL_Level, currentred, TAG_DONE);
  GT_SetGadgetAttrs(G_ScreenGreen, W_Colors, NULL, GTSL_Level, currentgreen, TAG_DONE);
  GT_SetGadgetAttrs(G_ScreenBlue,  W_Colors, NULL, GTSL_Level, currentblue, TAG_DONE);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenRedGadget(int code)
  {
  DEBUGENTER("HandleScreenRedGadget", NULL);

  currentred = code;
  newscreennode.Color[currentcolor] = RGB2COLOR(currentred, currentgreen, currentblue);
  SetRGB4(&screennode.Screen->ViewPort, currentcolor, currentred, currentgreen, currentblue);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenGreenGadget(int code)
  {
  DEBUGENTER("HandleScreenGreenGadget", NULL);

  currentgreen = code;
  newscreennode.Color[currentcolor] = RGB2COLOR(currentred, currentgreen, currentblue);
  SetRGB4(&screennode.Screen->ViewPort, currentcolor, currentred, currentgreen, currentblue);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenBlueGadget(int code)
  {
  DEBUGENTER("HandleScreenBlueGadget", NULL);

  currentblue = code;
  newscreennode.Color[currentcolor] = RGB2COLOR(currentred, currentgreen, currentblue);
  SetRGB4(&screennode.Screen->ViewPort, currentcolor, currentred, currentgreen, currentblue);

  DEBUGEXIT(FALSE, NULL);
  }

