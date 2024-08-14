
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

void EditScreen(void)
  {
  BOOL   done=FALSE;
  BOOL   error=FALSE;
  struct NewGadget  NG_Temp;
  struct Gadget     *G_List, *G_Context;
  struct Gadget     *G_OK, *G_Cancel;
  struct TagNode    *tagnode;

  DEBUGENTER("EditScreen", NULL);

  MainWindowSleep();

  newscreennode.DisplayID = screennode.DisplayID;
  newscreennode.Overscan  = screennode.Overscan;
  newscreennode.Width     = screennode.Width;
  newscreennode.Height    = screennode.Height;
  newscreennode.Depth     = screennode.Depth;
  newscreennode.Mode      = screennode.Mode;
  newscreennode.Workbench = screennode.Workbench;

  GetDisplayInfoData(NULL, (UBYTE *) &dimensioninfo, sizeof(struct DimensionInfo), DTAG_DIMS, screennode.DisplayID);

  if(screennode.Workbench)
    screenmodecode = 0;
  else if(screennode.Mode & MODE_HIRES)
    screenmodecode = 1;
  else if(screennode.Mode & MODE_SUPERHIRES)
    screenmodecode = 2;
  else if(screennode.Mode & MODE_PRODUCTIVITY)
    screenmodecode = 3;
  else if(screennode.Mode & MODE_A202410HZ)
    screenmodecode = 4;
  else if(screennode.Mode & MODE_A202415HZ)
    screenmodecode = 5;
  else
    screenmodecode = 0;

  if     (screennode.Mode & MODE_DEFAULT)
    ntscpalcode = 0;
  else if(screennode.Mode & MODE_NTSC)
    ntscpalcode = 1;
  else if(screennode.Mode & MODE_PAL)
    ntscpalcode = 2;
  else
    ntscpalcode = 0;

  if     (screennode.Mode & MODE_OSCANTEXT)
    oscancode = 0;
  else if(screennode.Mode & MODE_OSCANSTANDARD)
    oscancode = 1;
  else if(screennode.Mode & MODE_OSCANMAX)
    oscancode = 2;
  else if(screennode.Mode & MODE_OSCANVIDEO)
    oscancode = 3;
  else
    oscancode = 0;

  G_List = NULL;
  G_Context = CreateContext(&G_List);

  NG_Temp.ng_TextAttr = &TOPAZ80;
  NG_Temp.ng_VisualInfo = VisualInfo;
  NG_Temp.ng_UserData = NULL;

  NG_Temp.ng_LeftEdge = 64;
  NG_Temp.ng_TopEdge = currenttopborder+4;
  NG_Temp.ng_Width = 0;
  NG_Temp.ng_Height = 0;
  NG_Temp.ng_Flags = PLACETEXT_RIGHT;
  NG_Temp.ng_GadgetText = "";
  NG_Temp.ng_GadgetID = ID_SCREENMODE;
  G_ScreenMode = CreateGadget(MX_KIND, G_Context, &NG_Temp, GTMX_Labels, modelabels, GTMX_Active, screenmodecode, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+62;
  NG_Temp.ng_Width = 142;
  NG_Temp.ng_Height = 14;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "_Video";
  NG_Temp.ng_GadgetID = ID_NTSCPAL;
  G_NtscPal = CreateGadget(CYCLE_KIND, G_ScreenMode, &NG_Temp, GTCY_Labels, ntscpallabels, GTCY_Active, ntscpalcode, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+78;
  NG_Temp.ng_Width = 142;
  NG_Temp.ng_Height = 14;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "Over_scan";
  NG_Temp.ng_GadgetID = ID_OVERSCAN;
  G_Overscan = CreateGadget(CYCLE_KIND, G_NtscPal, &NG_Temp, GTCY_Labels, oscanlabels, GTCY_Active, oscancode, GT_Underscore, '_', TAG_DONE);

  /* Width & Height */

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+95;
  NG_Temp.ng_Width = 50;
  NG_Temp.ng_Height = 14;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "_Width";
  NG_Temp.ng_GadgetID = ID_SCREENWIDTH;
  G_ScreenWidth = CreateGadget(INTEGER_KIND, G_Overscan, &NG_Temp, GTIN_Number, screennode.Width, GTIN_MaxChars, 5, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+110;
  NG_Temp.ng_Width = 50;
  NG_Temp.ng_Height = 14;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "_Height";
  NG_Temp.ng_GadgetID = ID_SCREENHEIGHT;
  G_ScreenHeight = CreateGadget(INTEGER_KIND, G_ScreenWidth, &NG_Temp, GTIN_Number, screennode.Height, GTIN_MaxChars, 5, GT_Underscore, '_', TAG_DONE);

  /* Default Width, Height, & Depth */

  NG_Temp.ng_LeftEdge = 140;
  NG_Temp.ng_TopEdge = currenttopborder+97;
  NG_Temp.ng_Flags = PLACETEXT_RIGHT;
  NG_Temp.ng_GadgetText = "D_efault";
  NG_Temp.ng_GadgetID = ID_SCREENDWIDTH;
  G_ScreenDWidth = CreateGadget(CHECKBOX_KIND, G_ScreenHeight, &NG_Temp, GTCB_Checked, (screennode.Mode & MODE_DEFAULTWIDTH)!=NULL, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 140;
  NG_Temp.ng_TopEdge = currenttopborder+112;
  NG_Temp.ng_Flags = PLACETEXT_RIGHT;
  NG_Temp.ng_GadgetText = "De_fault";
  NG_Temp.ng_GadgetID = ID_SCREENDHEIGHT;
  G_ScreenDHeight = CreateGadget(CHECKBOX_KIND, G_ScreenDWidth, &NG_Temp, GTCB_Checked, (screennode.Mode & MODE_DEFAULTHEIGHT)!=NULL, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+127;
  NG_Temp.ng_Width = 80;
  NG_Temp.ng_Height = 12;
  NG_Temp.ng_Flags = PLACETEXT_LEFT;
  NG_Temp.ng_GadgetText = "_Depth";
  NG_Temp.ng_GadgetID = ID_SCREENDEPTH;
  G_ScreenDepth = CreateGadget(SLIDER_KIND, G_ScreenDHeight, &NG_Temp, GTSL_Level, 1, GTSL_LevelFormat, "%1ld", GTSL_LevelPlace, PLACETEXT_RIGHT, GTSL_Min, 1, GTSL_Max, 1, GTSL_MaxLevelLen, 1, GA_RelVerify, TRUE, GT_Underscore, '_', TAG_DONE);

  /* Interlace */

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+142;
  NG_Temp.ng_Flags = PLACETEXT_RIGHT;
  NG_Temp.ng_GadgetText = "_Interlace";
  NG_Temp.ng_GadgetID = ID_INTERLACE;
  G_Interlace = CreateGadget(CHECKBOX_KIND, G_ScreenDepth, &NG_Temp, GTCB_Checked, (screennode.Mode & MODE_INTERLACE)!=NULL, GT_Underscore, '_', TAG_DONE);

  /* Custom colors */

  NG_Temp.ng_LeftEdge = 86;
  NG_Temp.ng_TopEdge = currenttopborder+154;
  NG_Temp.ng_Flags = PLACETEXT_RIGHT;
  NG_Temp.ng_GadgetText = "_Palette";
  NG_Temp.ng_GadgetID = ID_CUSTOMCOLORS;
  G_CustomColors = CreateGadget(CHECKBOX_KIND, G_Interlace, &NG_Temp, GTCB_Checked, (screennode.Mode & MODE_CUSTOMCOLORS)!=NULL, GT_Underscore, '_', TAG_DONE);

  /* OK and Cancel */

  NG_Temp.ng_LeftEdge = 16;
  NG_Temp.ng_TopEdge = currenttopborder+170;
  NG_Temp.ng_Width = 80;
  NG_Temp.ng_Height = 15;
  NG_Temp.ng_GadgetText = "_OK";
  NG_Temp.ng_Flags = PLACETEXT_IN;
  NG_Temp.ng_GadgetID = ID_OK;
  G_OK = CreateGadget(BUTTON_KIND, G_CustomColors, &NG_Temp, GT_Underscore, '_', TAG_DONE);

  NG_Temp.ng_LeftEdge = 148;
  NG_Temp.ng_GadgetText = "_Cancel";
  NG_Temp.ng_Flags = PLACETEXT_IN;
  NG_Temp.ng_GadgetID = ID_CANCEL;
  G_Cancel = CreateGadget(BUTTON_KIND, G_OK, &NG_Temp, GT_Underscore, '_', TAG_DONE);

  if(G_Cancel)
    {
    if(W_Screen = OpenWindowTags(NULL,
                                 WA_PubScreen, screennode.Screen,
                                 WA_Left, CenterHoriz(235),
                                 WA_Top,  CenterVert(188),
                                 WA_InnerWidth, 235,
                                 WA_InnerHeight, 188,
                                 WA_IDCMP, screenidcmp,
                                 WA_Gadgets, G_List,
                                 WA_Title, "Screen Mode",
                                 WA_Activate, TRUE,
                                 WA_DragBar, TRUE,
                                 WA_DepthGadget, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 TAG_DONE))
      {
      GT_RefreshWindow(W_Screen, NULL);

      SetupNewScreen();

      screensignal = 1L << W_Screen->UserPort->mp_SigBit;

      while(!done)
        {
        Wait(screensignal);
        done = HandleRequesterIDCMP(W_Screen);
        }

      ModifyIDCMP(W_Screen, NULL);
      screensignal = NULL;

      switch(done)
        {
        case DONE_CANCEL:
          DEBUGTEXT("Cancel selected", NULL);
          break;

        case DONE_OK:
          DEBUGTEXT("OK selected", NULL);
          screennode.DisplayID = newscreennode.DisplayID;
          screennode.Overscan  = newscreennode.Overscan;
          screennode.Depth     = newscreennode.Depth;
          screennode.Width     = newscreennode.Width;
          screennode.Height    = newscreennode.Height;
          screennode.Mode      = newscreennode.Mode;

          for(tagnode = (struct TagNode *) screennode.TagList.lh_Head; tagnode->Node.ln_Succ;
              tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
            {
            if(tagnode->TagItem.ti_Tag == SA_DisplayID) tagnode->TagItem.ti_Data = screennode.DisplayID;
            if(tagnode->TagItem.ti_Tag == SA_Depth)     tagnode->TagItem.ti_Data = screennode.Depth;
            if(tagnode->TagItem.ti_Tag == SA_Width)     tagnode->TagItem.ti_Data = screennode.Width;
            if(tagnode->TagItem.ti_Tag == SA_Height)    tagnode->TagItem.ti_Data = screennode.Height;
            if(tagnode->TagItem.ti_Tag == SA_Overscan)  tagnode->TagItem.ti_Data = screennode.Overscan;
            }

          TagWindows(COMMAND_REMOVE);
          TagMainWindow(COMMAND_REMOVE);
          TagScreen(COMMAND_CHANGE);
          break;
        }

      CloseWindow(W_Screen);
      }
    else
      {
      error = TRUE;
      }

    FreeGadgets(G_List);
    }

  if(error)
    {
    PutError("Out of memory\nopening mode window", "OK");
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenModeGadget(int code)
  {
  DEBUGENTER("HandleScreenModeGadget", (ULONG) code);

  screenmodecode = code;

  GT_SetGadgetAttrs(G_ScreenDWidth,  W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
  GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
  GT_SetGadgetAttrs(G_Interlace,     W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);
  GT_SetGadgetAttrs(G_CustomColors,  W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);

  ntscpalcode = 0;
  GT_SetGadgetAttrs(G_NtscPal, W_Screen, NULL, GTCY_Active, ntscpalcode, TAG_DONE);

  oscancode = 0;
  GT_SetGadgetAttrs(G_Overscan, W_Screen, NULL, GTCY_Active, oscancode, TAG_DONE);

  switch(screenmodecode)
    {
    case 0:
    case 1:
      GT_SetGadgetAttrs(G_ScreenWidth,  W_Screen, NULL, GTIN_Number,  640, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GTIN_Number,  200, TAG_DONE);
      break;

    case 2:
      GT_SetGadgetAttrs(G_ScreenWidth,  W_Screen, NULL, GTIN_Number, 1280, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GTIN_Number,  200, TAG_DONE);
      break;

    case 3:
      GT_SetGadgetAttrs(G_ScreenWidth,  W_Screen, NULL, GTIN_Number,  640, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GTIN_Number,  480, TAG_DONE);
      break;

    case 4:
    case 5:
      GT_SetGadgetAttrs(G_ScreenWidth,  W_Screen, NULL, GTIN_Number, 1008, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GTIN_Number,  800, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDWidth,  W_Screen, NULL, GTCB_Checked, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GTCB_Checked, TRUE, TAG_DONE);
      break;
    }

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleNtscPalGadget(int code)
  {
  DEBUGENTER("HandleNtscPalGadget", NULL);

  ntscpalcode = code;

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleOverscanGadget(int code)
  {
  DEBUGENTER("HandleOverscanGadget", NULL);

  oscancode = code;

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleInterlaceGadget(void)
  {
  DEBUGENTER("HandleInterlaceGadget", NULL);

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleCustomColorsGadget(void)
  {
  DEBUGENTER("HandleCustomColorsGadget", NULL);

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenDepthGadget(ULONG code)
  {
  DEBUGENTER("HandleScreenDepthGadget", NULL);

  newscreennode.Depth = code;

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenWidthGadget(void)
  {
  DEBUGENTER("HandleScreenWidthGadget", NULL);

  GT_SetGadgetAttrs(G_ScreenDWidth, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenHeightGadget(void)
  {
  DEBUGENTER("HandleScreenHeightGadget", NULL);

  GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GTCB_Checked, FALSE, TAG_DONE);

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenDWidthGadget(void)
  {
  DEBUGENTER("HandleScreenDWidthGadget", NULL);

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleScreenDHeightGadget(void)
  {
  DEBUGENTER("HandleScreenDHeightGadget", NULL);

  SetupNewScreen();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

ULONG Depth2Colors(struct Gadget *gadget, WORD depth)
  {
  DEBUGENTER("Depth2Colors", NULL);

  DEBUGEXIT(TRUE, (ULONG)(1L << depth));
  return((ULONG)(1L << depth));
  }

/****************************************************************************/

void SetupNewScreen(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("SetupNewScreen", NULL);

  newscreennode.Mode = NULL;
  newscreennode.DisplayID = NULL;

  if(G_Interlace->Flags & SELECTED)     newscreennode.Mode |= MODE_INTERLACE;
  if(G_ScreenDWidth->Flags & SELECTED)  newscreennode.Mode |= MODE_DEFAULTWIDTH;
  if(G_ScreenDHeight->Flags & SELECTED) newscreennode.Mode |= MODE_DEFAULTHEIGHT;
  if(G_CustomColors->Flags & SELECTED)  newscreennode.Mode |= MODE_CUSTOMCOLORS;

  stringinfo = G_ScreenWidth->SpecialInfo;
  newscreennode.Width = stringinfo->LongInt;

  stringinfo = G_ScreenHeight->SpecialInfo;
  newscreennode.Height = stringinfo->LongInt;

  switch(screenmodecode)
    {
    case 0:
    case 4:
    case 5:
      GT_SetGadgetAttrs(G_NtscPal, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_Overscan, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenWidth, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDWidth, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      GT_SetGadgetAttrs(G_Interlace, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      if(screenmodecode == 4 || screenmodecode == 5)
        {
        GT_SetGadgetAttrs(G_ScreenDepth, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
        GT_SetGadgetAttrs(G_CustomColors, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
        }
      else
        {
        GT_SetGadgetAttrs(G_ScreenDepth, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
        GT_SetGadgetAttrs(G_CustomColors, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
        }
      break;

    case 1:
    case 2:
    case 3:
      GT_SetGadgetAttrs(G_Overscan, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDepth, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenWidth, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDWidth, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ScreenDHeight, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_Interlace, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_CustomColors, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      if(screenmodecode == 3)
        GT_SetGadgetAttrs(G_NtscPal, W_Screen, NULL, GA_Disabled, TRUE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_NtscPal, W_Screen, NULL, GA_Disabled, FALSE, TAG_DONE);
      break;
    }

  switch(screenmodecode)
    {
    case 0:
      maxdepth = 1;
      newscreennode.Mode |= MODE_WORKBENCH;
      newscreennode.DisplayID = INVALID_ID;
      break;

    case 1:
      maxdepth = 4;
      newscreennode.Mode |= MODE_HIRES;
      if(newscreennode.Mode & MODE_INTERLACE)
        newscreennode.DisplayID = HIRESLACE_KEY;
      else
        newscreennode.DisplayID = HIRES_KEY;
      break;

    case 2:
      maxdepth = 2;
      newscreennode.Mode |= MODE_SUPERHIRES;
      if(newscreennode.Mode & MODE_INTERLACE)
        newscreennode.DisplayID = SUPERLACE_KEY;
      else
        newscreennode.DisplayID = SUPER_KEY;
      break;

    case 3:
      maxdepth = 2;
      newscreennode.Mode |= MODE_PRODUCTIVITY;
      if(newscreennode.Mode & MODE_INTERLACE)
        newscreennode.DisplayID = VGAPRODUCTLACE_KEY;
      else
        newscreennode.DisplayID = VGAPRODUCT_KEY;
      break;

    case 4:
      maxdepth = 2;
      newscreennode.Mode |= MODE_A202410HZ;
      newscreennode.DisplayID = A2024TENHERTZ_KEY;
      break;

    case 5:
      maxdepth = 2;
      newscreennode.Mode |= MODE_A202415HZ;
      newscreennode.DisplayID = A2024FIFTEENHERTZ_KEY;
      break;
    }

  switch(ntscpalcode)
    {
    case 0:
      newscreennode.Mode |= MODE_DEFAULT;
      newscreennode.DisplayID |= DEFAULT_MONITOR_ID;
      break;

    case 1:
      newscreennode.Mode |= MODE_NTSC;
      newscreennode.DisplayID |= NTSC_MONITOR_ID;
      break;

    case 2:
      newscreennode.Mode |= MODE_PAL;
      newscreennode.DisplayID |= PAL_MONITOR_ID;
      break;
    }

  GetDisplayInfoData(NULL, (UBYTE *) &dimensioninfo, sizeof(struct DimensionInfo), DTAG_DIMS, newscreennode.DisplayID);

  switch(oscancode)
    {
    case 0:
      newscreennode.Mode |= MODE_OSCANTEXT;
      newscreennode.Overscan = OSCAN_TEXT;

      if(newscreennode.Mode & MODE_DEFAULTWIDTH)  newscreennode.Width  = dimensioninfo.TxtOScan.MaxX - dimensioninfo.TxtOScan.MinX +1;
      if(newscreennode.Mode & MODE_DEFAULTHEIGHT) newscreennode.Height = dimensioninfo.TxtOScan.MaxY - dimensioninfo.TxtOScan.MinY +1;
      break;

    case 1:
      newscreennode.Mode |= MODE_OSCANSTANDARD;
      newscreennode.Overscan = OSCAN_STANDARD;
      if(newscreennode.Mode & MODE_DEFAULTWIDTH)  newscreennode.Width  = dimensioninfo.StdOScan.MaxX - dimensioninfo.StdOScan.MinX +1;
      if(newscreennode.Mode & MODE_DEFAULTHEIGHT) newscreennode.Height = dimensioninfo.StdOScan.MaxY - dimensioninfo.StdOScan.MinY +1;
      break;

    case 2:
      newscreennode.Mode |= MODE_OSCANMAX;
      newscreennode.Overscan = OSCAN_MAX;
      if(newscreennode.Mode & MODE_DEFAULTWIDTH)  newscreennode.Width  = dimensioninfo.MaxOScan.MaxX - dimensioninfo.MaxOScan.MinX +1;
      if(newscreennode.Mode & MODE_DEFAULTHEIGHT) newscreennode.Height = dimensioninfo.MaxOScan.MaxY - dimensioninfo.MaxOScan.MinY +1;
      break;

    case 3:
      newscreennode.Mode |= MODE_OSCANVIDEO;
      newscreennode.Overscan = OSCAN_VIDEO;
      if(newscreennode.Mode & MODE_DEFAULTWIDTH)  newscreennode.Width  = dimensioninfo.VideoOScan.MaxX - dimensioninfo.VideoOScan.MinX +1;
      if(newscreennode.Mode & MODE_DEFAULTHEIGHT) newscreennode.Height = dimensioninfo.VideoOScan.MaxY - dimensioninfo.VideoOScan.MinY +1;
      break;
    }

  if(newscreennode.Width  < 640) newscreennode.Width  = 640;
  if(newscreennode.Height < 200) newscreennode.Height = 200;
  if(newscreennode.Depth  > maxdepth) newscreennode.Depth = maxdepth;

  GT_SetGadgetAttrs(G_ScreenWidth,  W_Screen, NULL, GTIN_Number, newscreennode.Width,  TAG_DONE);
  GT_SetGadgetAttrs(G_ScreenHeight, W_Screen, NULL, GTIN_Number, newscreennode.Height, TAG_DONE);
  GT_SetGadgetAttrs(G_ScreenDepth,  W_Screen, NULL, GTSL_Max, maxdepth, GTSL_Level, newscreennode.Depth, TAG_DONE);

  DEBUGEXIT(FALSE, NULL);
  }

