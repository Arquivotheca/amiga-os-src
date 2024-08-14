
#include "Toolmaker.h"
#include "Externs.h"

#include <exec/memory.h>
#include <stdlib.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

void EditMenus(void)
  {
  BOOL   done=FALSE;
  BOOL   error=FALSE;
  int    status;
  struct NewGadget NG_Temp;
  struct Gadget *G_List, *G_Context;
  struct Gadget *G_OK, *G_Cancel;
  struct WindowNode *windownode;
  struct MenuNode *menunode, *thismenu, *thismn, *nextmn;
  struct ItemNode *itemnode, *thisitem;
  struct SubNode  *subnode,  *thissub;

  DEBUGENTER("EditMenus", NULL);

  MainWindowSleep();

  windownode = currentwindow;
  menupart = 1;

  NewList(&EditMenuList);

  menucount = 0;
  for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
      menunode = (struct MenuNode *) menunode->Node.ln_Succ)
    {
    status = TRUE;
    while(!(thismenu = AllocMem(sizeof(struct MenuNode), MEMF_CLEAR)) && status)
      status = PutError("Out of memory", "Retry|Abort");

    if(status)
      {
      DEBUGALLOC(sizeof (struct MenuNode));

      thismenu->Node.ln_Name = thismenu->MenuText;
      NewList(&thismenu->ItemList);
      strcpy(thismenu->MenuText, menunode->MenuText);
      strcpy(thismenu->SourceLabel, menunode->SourceLabel);
      thismenu->Flags = menunode->Flags;
      thismenu->TMMFlags = menunode->TMMFlags;
      thismenu->ItemCount = menunode->ItemCount;
      AddTail(&EditMenuList, (struct Node *) thismenu);
      for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
          itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
        {
        status = TRUE;
        while(!(thisitem = AllocMem(sizeof(struct ItemNode), MEMF_CLEAR)) && status)
          status = PutError("Out of memory", "Retry|Abort");

        if(status)
          {
          DEBUGALLOC(sizeof (struct ItemNode));

          thisitem->Node.ln_Name = thisitem->ItemText;
          NewList(&thisitem->SubList);
          strcpy(thisitem->ItemText, itemnode->ItemText);
          strcpy(thisitem->CommKey, itemnode->CommKey);
          strcpy(thisitem->SourceLabel, itemnode->SourceLabel);
          thisitem->Flags = itemnode->Flags;
          thisitem->TMMFlags = itemnode->TMMFlags;
          thisitem->SubCount = itemnode->SubCount;
          AddTail(&thismenu->ItemList, (struct Node *) thisitem);
          for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
              subnode = (struct SubNode *) subnode->Node.ln_Succ)
            {
            status = TRUE;
            while(!(thissub = AllocMem(sizeof(struct SubNode), MEMF_CLEAR)) && status)
              status = PutError("Out of memory", "Retry|Abort");

            if(status)
              {
              DEBUGALLOC(sizeof (struct SubNode));

              thissub->Node.ln_Name = thissub->SubText;
              strcpy(thissub->SubText, subnode->SubText);
              strcpy(thissub->CommKey, subnode->CommKey);
              strcpy(thissub->SourceLabel, subnode->SourceLabel);
              thissub->Flags = subnode->Flags;
              thissub->TMMFlags = subnode->TMMFlags;
              AddTail(&thisitem->SubList, (struct Node *) thissub);
              }
            else
              {
              error = TRUE;
              }
            }
          }
        else
          {
          error = TRUE;
          }
        }
      }
    else
      {
      error = TRUE;
      }
    menucount++;
    }

  if(!error)
    {
    G_List = NULL;
    G_Context = CreateContext(&G_List);

    NG_Temp.ng_TextAttr = &TOPAZ80;
    NG_Temp.ng_VisualInfo = VisualInfo;
    NG_Temp.ng_UserData = NULL;

    /* Listviews */

    NG_Temp.ng_Width = 152;
    NG_Temp.ng_Height = 76;
    NG_Temp.ng_Flags = PLACETEXT_ABOVE;
    NG_Temp.ng_TopEdge = currenttopborder+18;

    NG_Temp.ng_LeftEdge = 14;
    NG_Temp.ng_GadgetText = "_Menus";
    NG_Temp.ng_GadgetID = ID_MENULIST;
    G_MenuList = CreateGadget(LISTVIEW_KIND, G_Context, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 176;
    NG_Temp.ng_GadgetText = "_Items";
    NG_Temp.ng_GadgetID = ID_ITEMLIST;
    G_ItemList = CreateGadget(LISTVIEW_KIND, G_MenuList, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 338;
    NG_Temp.ng_GadgetText = "_Subitems";
    NG_Temp.ng_GadgetID = ID_SUBLIST;
    G_SubList = CreateGadget(LISTVIEW_KIND, G_ItemList, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    /* Menu String, Add, Del, Up, and Down */

    NG_Temp.ng_LeftEdge = 14;
    NG_Temp.ng_TopEdge = currenttopborder+94;
    NG_Temp.ng_Width = 152;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "";
    NG_Temp.ng_GadgetID = ID_MENUSTRING;
    NG_Temp.ng_Flags = PLACETEXT_IN;
    G_MenuString = CreateGadget(STRING_KIND, G_SubList, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, STRINGSIZE-1, TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+108;
    NG_Temp.ng_GadgetText = "";
    NG_Temp.ng_GadgetID = ID_MENULABEL;
    G_MenuLabel = CreateGadget(STRING_KIND, G_MenuString, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, LABELSIZE-1, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+122;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_Width = 38;
    NG_Temp.ng_Flags = PLACETEXT_IN;

    NG_Temp.ng_LeftEdge = 14;
    NG_Temp.ng_GadgetText = "_Add";
    NG_Temp.ng_GadgetID = ID_MENUADD;
    G_MenuAdd = CreateGadget(BUTTON_KIND, G_MenuLabel, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 52;
    NG_Temp.ng_GadgetText = "_Del";
    NG_Temp.ng_GadgetID = ID_MENUDEL;
    G_MenuDel = CreateGadget(BUTTON_KIND, G_MenuAdd, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 90;
    NG_Temp.ng_GadgetText = "/\\";
    NG_Temp.ng_GadgetID = ID_MENUUP;
    G_MenuUp = CreateGadget(BUTTON_KIND, G_MenuDel, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_LeftEdge = 128;
    NG_Temp.ng_GadgetText = "\\/";
    NG_Temp.ng_GadgetID = ID_MENUDOWN;
    G_MenuDown = CreateGadget(BUTTON_KIND, G_MenuUp, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_Width = 0;
    NG_Temp.ng_Height = 12;
    NG_Temp.ng_LeftEdge = 14;

    NG_Temp.ng_TopEdge = currenttopborder+142;
    NG_Temp.ng_GadgetText = "DISA_BLED";
    NG_Temp.ng_GadgetID = ID_MENUDISABLED;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_MenuDisabled = CreateGadget(CHECKBOX_KIND, G_MenuDown, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    /* Item String, Add, Del, Up, and Down */

    NG_Temp.ng_LeftEdge = 176;
    NG_Temp.ng_TopEdge = currenttopborder+94;
    NG_Temp.ng_Width = 152;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "";
    NG_Temp.ng_GadgetID = ID_ITEMSTRING;
    NG_Temp.ng_Flags = PLACETEXT_IN;
    G_ItemString = CreateGadget(STRING_KIND, G_MenuDisabled, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, STRINGSIZE-1, TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+108;
    NG_Temp.ng_GadgetText = "";
    NG_Temp.ng_GadgetID = ID_ITEMLABEL;
    G_ItemLabel = CreateGadget(STRING_KIND, G_ItemString, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, LABELSIZE-1, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+122;
    NG_Temp.ng_Width = 38;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_Flags = PLACETEXT_IN;

    NG_Temp.ng_LeftEdge = 176;
    NG_Temp.ng_GadgetText = "_Add";
    NG_Temp.ng_GadgetID = ID_ITEMADD;
    G_ItemAdd = CreateGadget(BUTTON_KIND, G_ItemLabel, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 214;
    NG_Temp.ng_GadgetText = "_Del";
    NG_Temp.ng_GadgetID = ID_ITEMDEL;
    G_ItemDel = CreateGadget(BUTTON_KIND, G_ItemAdd, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 252;
    NG_Temp.ng_GadgetText = "/\\";
    NG_Temp.ng_GadgetID = ID_ITEMUP;
    G_ItemUp = CreateGadget(BUTTON_KIND, G_ItemDel, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_LeftEdge = 290;
    NG_Temp.ng_GadgetText = "\\/";
    NG_Temp.ng_GadgetID = ID_ITEMDOWN;
    G_ItemDown = CreateGadget(BUTTON_KIND, G_ItemUp, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_LeftEdge = 176;
    NG_Temp.ng_TopEdge = currenttopborder+142;
    NG_Temp.ng_Width = 0;
    NG_Temp.ng_Height = 0;
    NG_Temp.ng_GadgetText = "DISA_BLED";
    NG_Temp.ng_GadgetID = ID_ITEMDISABLED;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_ItemDisabled = CreateGadget(CHECKBOX_KIND, G_ItemDown, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    /* Sub String, Add, Del, Up, and Down */

    NG_Temp.ng_LeftEdge = 338;
    NG_Temp.ng_TopEdge = currenttopborder+94;
    NG_Temp.ng_Width = 152;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "Te_xt";
    NG_Temp.ng_GadgetID = ID_SUBSTRING;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_SubString = CreateGadget(STRING_KIND, G_ItemDisabled, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, STRINGSIZE-1, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+108;
    NG_Temp.ng_GadgetText = "_Label";
    NG_Temp.ng_GadgetID = ID_SUBLABEL;
    G_SubLabel = CreateGadget(STRING_KIND, G_SubString, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, LABELSIZE-1, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+122;
    NG_Temp.ng_Width = 38;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_Flags = PLACETEXT_IN;

    NG_Temp.ng_LeftEdge = 338;
    NG_Temp.ng_GadgetText = "_Add";
    NG_Temp.ng_GadgetID = ID_SUBADD;
    G_SubAdd = CreateGadget(BUTTON_KIND, G_SubLabel, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 376;
    NG_Temp.ng_GadgetText = "_Del";
    NG_Temp.ng_GadgetID = ID_SUBDEL;
    G_SubDel = CreateGadget(BUTTON_KIND, G_SubAdd, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 414;
    NG_Temp.ng_GadgetText = "/\\";
    NG_Temp.ng_GadgetID = ID_SUBUP;
    G_SubUp = CreateGadget(BUTTON_KIND, G_SubDel, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_LeftEdge = 452;
    NG_Temp.ng_GadgetText = "\\/";
    NG_Temp.ng_GadgetID = ID_SUBDOWN;
    G_SubDown = CreateGadget(BUTTON_KIND, G_SubUp, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_LeftEdge = 338;
    NG_Temp.ng_TopEdge = currenttopborder+142;
    NG_Temp.ng_Width = 0;
    NG_Temp.ng_Height = 0;
    NG_Temp.ng_GadgetText = "DISA_BLED";
    NG_Temp.ng_GadgetID = ID_SUBDISABLED;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    G_SubDisabled = CreateGadget(CHECKBOX_KIND, G_SubDown, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    /* CheckIt, Checked, Toggle, Exclude, CommKey */

    NG_Temp.ng_LeftEdge = 504;
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;

    NG_Temp.ng_TopEdge = currenttopborder+20;
    NG_Temp.ng_GadgetText = "C_HECKIT";
    NG_Temp.ng_GadgetID = ID_CHECKIT;
    G_CheckIt = CreateGadget(CHECKBOX_KIND, G_SubDisabled, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+34;
    NG_Temp.ng_GadgetText = "CHEC_KED";
    NG_Temp.ng_GadgetID = ID_CHECKED;
    G_Checked = CreateGadget(CHECKBOX_KIND, G_CheckIt, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+48;
    NG_Temp.ng_GadgetText = "MENU_TOGGLE";
    NG_Temp.ng_GadgetID = ID_TOGGLE;
    G_Toggle = CreateGadget(CHECKBOX_KIND, G_Checked, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+62;
    NG_Temp.ng_GadgetText = "_EXCLUDE ALL";
    NG_Temp.ng_GadgetID = ID_EXCLUDE;
    G_Exclude = CreateGadget(CHECKBOX_KIND, G_Toggle, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 502;
    NG_Temp.ng_TopEdge = currenttopborder+76;
    NG_Temp.ng_Width = 30;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "COMMKE_Y";
    NG_Temp.ng_GadgetID = ID_COMMKEY;
    G_CommKey = CreateGadget(STRING_KIND, G_Exclude, &NG_Temp, GA_Disabled, TRUE, GTST_MaxChars, 1, GT_Underscore, '_', TAG_DONE);

    /* OK and Cancel */

    NG_Temp.ng_TopEdge = currenttopborder+162;

    NG_Temp.ng_LeftEdge = 14;
    NG_Temp.ng_Width = 80;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "_OK";
    NG_Temp.ng_GadgetID = ID_OK;
    NG_Temp.ng_Flags = PLACETEXT_IN;
    G_OK = CreateGadget(BUTTON_KIND, G_CommKey, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 544;
    NG_Temp.ng_Width = 80;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "_Cancel";
    NG_Temp.ng_GadgetID = ID_CANCEL;
    NG_Temp.ng_Flags = PLACETEXT_IN;
    G_Cancel = CreateGadget(BUTTON_KIND, G_OK, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    if(G_Cancel)
      {
      if(W_Menu = OpenWindowTags(NULL,
                                 WA_PubScreen, screennode.Screen,
                                 WA_Left, CenterHoriz(640),
                                 WA_Top,  CenterVert(180),
                                 WA_IDCMP, menuidcmp,
                                 WA_Gadgets, G_List,
                                 WA_Title, "Menus",
                                 WA_Activate, TRUE,
                                 WA_DragBar, TRUE,
                                 WA_DepthGadget, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 WA_Width, 640,
                                 WA_InnerHeight, 180,
                                 TAG_DONE))
        {
        GT_RefreshWindow(W_Menu, NULL);

        GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Labels, &EditMenuList, TAG_DONE);
        HandleMenuListGadget(0);

        menusignal = 1L << W_Menu->UserPort->mp_SigBit;

        while(!done)
          {
          Wait(menusignal);
          done = HandleRequesterIDCMP(W_Menu);
          }

        ModifyIDCMP(W_Menu, NULL);
        menusignal = NULL;

        switch(done)
          {
          case DONE_CANCEL:
            KillMenuList(&EditMenuList);
            break;

          case DONE_OK:
            KillMenuList(&windownode->MenuList);

            thismn = (struct MenuNode *) EditMenuList.lh_Head;
            while(nextmn = (struct MenuNode *) (thismn->Node.ln_Succ))
              {
              Remove((struct Node *) thismn);
              AddTail(&windownode->MenuList, (struct Node *) thismn);
              thismn = nextmn;
              }

            if(windownode->Menu)
              {
              ClearMenuStrip(windownode->Window);
              FreeMenus(windownode->Menu);
              windownode->Menu = NULL;
              }

            menucount = 0;
            for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
                menunode = (struct MenuNode *) menunode->Node.ln_Succ)
              {
              for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
                  itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
                {
                for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                    subnode = (struct SubNode *) subnode->Node.ln_Succ)
                  {
                  menucount++;
                  }
                menucount++;
                }
              menucount++;
              }

            if(menucount) windownode->IDCMP |= IDCMP_MENUPICK;

            AddMenus(windownode);
            modified = TRUE;
            break;
          }

        CloseWindow(W_Menu);
        }
      else
        {
        error = TRUE;
        }

      FreeGadgets(G_List);
      }
    else
      {
      error = TRUE;
      }
    }

  if(error)
    {
    PutError("Error opening\nMenu window", "OK");
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuListGadget(ULONG code)
  {
  int i;

  DEBUGENTER("HandleMenuListGadget", NULL);

  if(EditMenuList.lh_TailPred == (struct Node *) &EditMenuList.lh_Head) /* empty */
    {
    selectedmenu = NULL;
    GT_SetGadgetAttrs(G_MenuString, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuLabel, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuDisabled, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuDel, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuUp, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuDown, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Labels, &EmptyList, TAG_DONE);
    HandleItemListGadget(0);
    GT_SetGadgetAttrs(G_ItemAdd, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    }
  else
    {
    selectedmenunum = code;
    selectedmenu = (struct MenuNode *) EditMenuList.lh_Head;
    for(i=0; i<selectedmenunum; i++)
      selectedmenu = (struct MenuNode *) selectedmenu->Node.ln_Succ;

    GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Top, selectedmenunum, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuString, W_Menu, NULL, GA_Disabled, FALSE, GTST_String, selectedmenu->MenuText, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuLabel, W_Menu, NULL, GA_Disabled, FALSE, GTST_String, selectedmenu->SourceLabel, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuDisabled, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_MenuDel, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(menucount >= MAXMENUS)
      GT_SetGadgetAttrs(G_MenuAdd, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_MenuAdd, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedmenunum == 0)
      GT_SetGadgetAttrs(G_MenuUp, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_MenuUp, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedmenunum == menucount-1)
      GT_SetGadgetAttrs(G_MenuDown, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_MenuDown, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedmenu->Flags & NM_MENUDISABLED)
      GT_SetGadgetAttrs(G_MenuDisabled, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_MenuDisabled, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);

    GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Labels, &selectedmenu->ItemList, TAG_DONE);
    HandleItemListGadget(0);
    GT_SetGadgetAttrs(G_ItemAdd, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);
    }

  ShowCurrentMenuList(1);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuStringGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleMenuStringGadget", NULL);

  stringinfo = G_MenuString->SpecialInfo;
  GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);

  if(selectedmenu) strcpy(selectedmenu->MenuText, stringinfo->Buffer);

  GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL,
                    GTLV_Labels, &EditMenuList,
                    GTLV_Selected, menucount,
                    GTLV_Top, menucount,
                    TAG_DONE);

  stringinfo = G_MenuLabel->SpecialInfo;
  if(stringinfo->Buffer[0] == '\0')
    {
    strcpy(selectedmenu->SourceLabel, Text2Label(selectedmenu->MenuText));
    GT_SetGadgetAttrs(G_MenuLabel, W_Menu, NULL, GTST_String, selectedmenu->SourceLabel, TAG_DONE);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuLabelGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleMenuLabelGadget", NULL);

  stringinfo = G_MenuLabel->SpecialInfo;
  if(selectedmenu) strcpy(selectedmenu->SourceLabel, stringinfo->Buffer);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuAddGadget(void)
  {
  struct MenuNode *newmenunode, *menunode;
  BOOL foundproject, foundedit, foundmacros, foundsettings, founduser;

  DEBUGENTER("HandleMenuAddGadget", NULL);

  if(menucount < MAXMENUS)
    {
    if(newmenunode = AllocMem(sizeof(struct MenuNode), MEMF_CLEAR))
      {
      DEBUGALLOC(sizeof (struct MenuNode));

      GT_SetGadgetAttrs(G_MenuString, W_Menu, NULL, GTST_String, "", GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_MenuLabel, W_Menu, NULL, GTST_String, "", GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);
      GT_SetGadgetAttrs(G_MenuDel, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

      foundproject = foundedit = foundmacros = foundsettings = founduser = FALSE;

      for(menunode = (struct MenuNode *) EditMenuList.lh_Head; menunode->Node.ln_Succ;
          menunode = (struct MenuNode *) menunode->Node.ln_Succ)
        {
        if(!strcmp(menunode->MenuText, "Project"))  foundproject = TRUE;
        if(!strcmp(menunode->MenuText, "Edit"))     foundedit = TRUE;
        if(!strcmp(menunode->MenuText, "Macros"))   foundmacros = TRUE;
        if(!strcmp(menunode->MenuText, "Settings")) foundsettings = TRUE;
        if(!strcmp(menunode->MenuText, "User"))     founduser = TRUE;
        }

      if(!foundproject)
        {
        strcpy(newmenunode->MenuText, "Project");
        strcpy(newmenunode->SourceLabel, "PROJECT");
        }
      else if(!foundedit)
        {
        strcpy(newmenunode->MenuText, "Edit");
        strcpy(newmenunode->SourceLabel, "EDIT");
        }
      else if(!foundmacros)
        {
        strcpy(newmenunode->MenuText, "Macros");
        strcpy(newmenunode->SourceLabel, "MACROS");
        }
      else if(!foundsettings)
        {
        strcpy(newmenunode->MenuText, "Settings");
        strcpy(newmenunode->SourceLabel, "SETTINGS");
        }
      else if(!founduser)
        {
        strcpy(newmenunode->MenuText, "User");
        strcpy(newmenunode->SourceLabel, "USER");
        }
      else
        {
        strcpy(newmenunode->MenuText, "(new)");
        }

      newmenunode->ItemCount = 0;
      newmenunode->Node.ln_Name = newmenunode->MenuText;

      NewList(&newmenunode->ItemList);
      AddTail(&EditMenuList, (struct Node *) newmenunode);
      selectedmenu = newmenunode;

      GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL,
                        GTLV_Labels, &EditMenuList,
                        GTLV_Selected, menucount,
                        GTLV_Top, menucount,
                        TAG_DONE);

      menucount++;
      HandleMenuListGadget(menucount-1);
      ActivateGadget(G_MenuString, W_Menu, NULL);
      }
    else
      PutError("Out of memory", "OK");
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuDelGadget(void)
  {
  int status = TRUE;

  DEBUGENTER("HandleMenuDelGadget", NULL);

  if(menucount > 0)
    {
    if(selectedmenu->ItemCount)
      {
      sprintf(string, "Warning: deleting this menu will\nalso delete its items!\n\nDelete menu \"%s\"?", selectedmenu->MenuText);
      status = PutError(string, "OK|Cancel");
      }

    if(status)
      {
      GT_SetGadgetAttrs(G_MenuString, W_Menu, NULL, GTST_String, "", TAG_DONE);
      GT_SetGadgetAttrs(G_MenuLabel, W_Menu, NULL, GTST_String, "", TAG_DONE);
      GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);
      GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);

      Remove((struct Node *) selectedmenu);

      GT_SetGadgetAttrs(G_MenuList, W_Menu, NULL,
                        GTLV_Labels, &EditMenuList,
                        GTLV_Selected, 0,
                        GTLV_Top, 0,
                        TAG_DONE);

      KillItemList(&selectedmenu->ItemList);

      DEBUGFREE(sizeof (struct MenuNode));
      FreeMem(selectedmenu, sizeof(struct MenuNode));

      menucount--;
      HandleMenuListGadget(0);
      }
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuUpGadget(void)
  {
  DEBUGENTER("HandleMenuUpGadget", NULL);

  if(selectedmenunum > 0)
    {
    MoveNodeUp(&EditMenuList, selectedmenunum, W_Menu, G_MenuList);
    HandleMenuListGadget(selectedmenunum-1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuDownGadget(void)
  {
  DEBUGENTER("HandleMenuDownGadget", NULL);

  if(selectedmenunum < menucount-1)
    {
    MoveNodeDown(&EditMenuList, selectedmenunum, W_Menu, G_MenuList);
    HandleMenuListGadget(selectedmenunum+1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleMenuDisabledGadget(void)
  {
  DEBUGENTER("HandleMenuDisabledGadget", NULL);

  if(G_MenuDisabled->Flags & SELECTED)
    selectedmenu->Flags |= NM_MENUDISABLED;
  else
    selectedmenu->Flags &= ~NM_MENUDISABLED;

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemListGadget(ULONG code)
  {
  int i;

  DEBUGENTER("HandleItemListGadget", NULL);

  if(selectedmenu == NULL || selectedmenu->ItemList.lh_TailPred == (struct Node *) &selectedmenu->ItemList.lh_Head) /* empty */
    {
    selecteditem = NULL;
    GT_SetGadgetAttrs(G_ItemString, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemLabel, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemDisabled, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemDel, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemUp, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemDown, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Labels, &EmptyList, TAG_DONE);
    GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_Exclude, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_CommKey, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    HandleSubListGadget(0);
    GT_SetGadgetAttrs(G_SubAdd, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    }
  else
    {
    selecteditemnum = code;
    selecteditem = (struct ItemNode *) selectedmenu->ItemList.lh_Head;
    for(i=0; i<selecteditemnum; i++)
      selecteditem = (struct ItemNode *) selecteditem->Node.ln_Succ;

    GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Top, selecteditemnum, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemString, W_Menu, NULL, GTST_String, selecteditem->ItemText, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemLabel, W_Menu, NULL, GTST_String, selecteditem->SourceLabel, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemDisabled, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_ItemDel, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedmenu->ItemCount >= MAXITEMS)
      GT_SetGadgetAttrs(G_ItemAdd, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_ItemAdd, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selecteditemnum == 0)
      GT_SetGadgetAttrs(G_ItemUp, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_ItemUp, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selecteditemnum == selectedmenu->ItemCount-1)
      GT_SetGadgetAttrs(G_ItemDown, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_ItemDown, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selecteditem->Flags & NM_ITEMDISABLED)
      GT_SetGadgetAttrs(G_ItemDisabled, W_Menu, NULL, GTCB_Checked, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_ItemDisabled, W_Menu, NULL, GTCB_Checked, FALSE, TAG_DONE);

    if(selecteditem->SubCount == 0)
      {
      if(selecteditem->Flags & CHECKIT)
        GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

      if(selecteditem->Flags & CHECKED)
        GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

      if(selecteditem->Flags & MENUTOGGLE)
        GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
      else
        GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

      GT_SetGadgetAttrs(G_CommKey, W_Menu, NULL, GA_Disabled, FALSE, GTST_String, selecteditem->CommKey, TAG_DONE);
      GT_SetGadgetAttrs(G_Exclude, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, selecteditem->TMMFlags & MENUFLAG_AUTOEXCLUDE, TAG_DONE);
      }
    else
      {
      selecteditem->Flags &= ~(CHECKIT | CHECKED | MENUTOGGLE);
      selecteditem->TMMFlags = 0;
      selecteditem->CommKey[0] = '\0';
      }

    GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Labels, &selecteditem->SubList, TAG_DONE);
    GT_SetGadgetAttrs(G_SubAdd, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    HandleSubListGadget(0);
    }

  ShowCurrentMenuList(2);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemStringGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleItemStringGadget", NULL);

  stringinfo = G_ItemString->SpecialInfo;
  GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);

  if(selecteditem) strcpy(selecteditem->ItemText, stringinfo->Buffer);

  GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL,
                    GTLV_Labels, &selectedmenu->ItemList,
                    GTLV_Selected, selectedmenu->ItemCount,
                    GTLV_Top, selectedmenu->ItemCount,
                    TAG_DONE);

  stringinfo = G_ItemLabel->SpecialInfo;
  if(stringinfo->Buffer[0] == '\0')
    {
    strcpy(selecteditem->SourceLabel, Text2Label(selecteditem->ItemText));
    GT_SetGadgetAttrs(G_ItemLabel, W_Menu, NULL, GTST_String, selecteditem->SourceLabel, TAG_DONE);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemLabelGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleItemLabelGadget", NULL);

  stringinfo = G_ItemLabel->SpecialInfo;
  if(selecteditem) strcpy(selecteditem->SourceLabel, stringinfo->Buffer);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemAddGadget(void)
  {
  struct ItemNode *newitemnode;
  struct ItemNode *nextitemnode;

  DEBUGENTER("HandleItemAddGadget", NULL);

  if(selectedmenu->ItemCount < MAXITEMS)
    {
    if(newitemnode = AllocMem(sizeof(struct ItemNode), MEMF_CLEAR))
      {
      DEBUGALLOC(sizeof (struct ItemNode));

      GT_SetGadgetAttrs(G_ItemString, W_Menu, NULL, GTST_String, "", GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ItemLabel, W_Menu, NULL, GTST_String, "", GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);
      GT_SetGadgetAttrs(G_ItemDel, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

      nextitemnode = NextItem(selectedmenu);

      strcpy(newitemnode->ItemText,    nextitemnode->ItemText);
      strcpy(newitemnode->CommKey,     nextitemnode->CommKey);
      strcpy(newitemnode->SourceLabel, nextitemnode->SourceLabel);

      newitemnode->Flags = nextitemnode->Flags;
      newitemnode->Node.ln_Name = newitemnode->ItemText;
      newitemnode->SubCount = 0;

      NewList(&newitemnode->SubList);
      AddTail(&selectedmenu->ItemList, (struct Node *) newitemnode);
      selecteditem = newitemnode;

      GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL,
                        GTLV_Labels, &selectedmenu->ItemList,
                        GTLV_Selected, selectedmenu->ItemCount,
                        GTLV_Top, selectedmenu->ItemCount,
                        TAG_DONE);

      selectedmenu->ItemCount++;
      HandleItemListGadget(selectedmenu->ItemCount-1);
      ActivateGadget(G_ItemString, W_Menu, NULL);
      }
    else
      PutError("Out of memory", "OK");
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemDelGadget(void)
  {
  int status = TRUE;

  DEBUGENTER("HandleItemDelGadget", NULL);

  if(selectedmenu->ItemCount > 0)
    {
    if(selecteditem->SubCount)
      {
      sprintf(string, "Warning: deleting this item will\nalso delete its subitems!\n\nDelete item \"%s\"?", selecteditem->ItemText);
      status = PutError(string, "OK|Cancel");
      }

    if(status)
      {
      GT_SetGadgetAttrs(G_ItemString, W_Menu, NULL, GTST_String, "", TAG_DONE);
      GT_SetGadgetAttrs(G_ItemLabel, W_Menu, NULL, GTST_String, "", TAG_DONE);
      GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);
      GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);

      Remove((struct Node *) selecteditem);

      GT_SetGadgetAttrs(G_ItemList, W_Menu, NULL,
                        GTLV_Labels, &selectedmenu->ItemList,
                        GTLV_Selected, 0,
                        GTLV_Top, 0,
                        TAG_DONE);

      KillSubList(&selecteditem->SubList);

      DEBUGFREE(sizeof (struct ItemNode));
      FreeMem(selecteditem, sizeof(struct ItemNode));

      selectedmenu->ItemCount--;
      HandleItemListGadget(0);
      }
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemUpGadget(void)
  {
  DEBUGENTER("HandleItemUpGadget", NULL);

  if(selecteditemnum > 0)
    {
    MoveNodeUp(&selectedmenu->ItemList, selecteditemnum, W_Menu, G_ItemList);
    HandleItemListGadget(selecteditemnum-1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemDownGadget(void)
  {
  DEBUGENTER("HandleItemDownGadget", NULL);

  if(selecteditemnum < selectedmenu->ItemCount-1)
    {
    MoveNodeDown(&selectedmenu->ItemList, selecteditemnum, W_Menu, G_ItemList);
    HandleItemListGadget(selecteditemnum+1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleItemDisabledGadget(void)
  {
  DEBUGENTER("HandleItemDisabledGadget", NULL);

  if(G_ItemDisabled->Flags & SELECTED)
    selecteditem->Flags |= NM_ITEMDISABLED;
  else
    selecteditem->Flags &= ~NM_ITEMDISABLED;

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubListGadget(ULONG code)
  {
  int i;

  DEBUGENTER("HandleSubListGadget", NULL);

  if(selecteditem == NULL || selecteditem->SubList.lh_TailPred == (struct Node *) &selecteditem->SubList.lh_Head) /* empty */
    {
    selectedsub = NULL;
    GT_SetGadgetAttrs(G_SubString, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubLabel, W_Menu, NULL, GTST_String, "", GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubDisabled, W_Menu, NULL, GTCB_Checked, FALSE, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubDel, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubUp, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubDown, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Labels, &EmptyList, TAG_DONE);
    }
  else
    {
    selectedsubnum = code;
    selectedsub = (struct SubNode *) selecteditem->SubList.lh_Head;
    for(i=0; i<selectedsubnum; i++)
      selectedsub = (struct SubNode *) selectedsub->Node.ln_Succ;

    GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Top, selectedsubnum, TAG_DONE);
    GT_SetGadgetAttrs(G_SubString, W_Menu, NULL, GTST_String, selectedsub->SubText, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubLabel, W_Menu, NULL, GTST_String, selectedsub->SourceLabel, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubDisabled, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_SubDel, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selecteditem->SubCount >= MAXSUBS)
      GT_SetGadgetAttrs(G_SubAdd, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_SubAdd, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedsubnum == 0)
      GT_SetGadgetAttrs(G_SubUp, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_SubUp, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedsubnum == selecteditem->SubCount-1)
      GT_SetGadgetAttrs(G_SubDown, W_Menu, NULL, GA_Disabled, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_SubDown, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedsub->Flags & NM_ITEMDISABLED)
      GT_SetGadgetAttrs(G_SubDisabled, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_SubDisabled, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

    if(selectedsub->Flags & CHECKIT)
      GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_CheckIt, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

    if(selectedsub->Flags & CHECKED)
      GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_Checked, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

    if(selectedsub->Flags & MENUTOGGLE)
      GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, TRUE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_Toggle, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, FALSE, TAG_DONE);

    GT_SetGadgetAttrs(G_Exclude, W_Menu, NULL, GA_Disabled, FALSE, GTCB_Checked, selectedsub->TMMFlags & MENUFLAG_AUTOEXCLUDE, TAG_DONE);
    GT_SetGadgetAttrs(G_CommKey, W_Menu, NULL, GA_Disabled, FALSE, GTST_String, selectedsub->CommKey, TAG_DONE);
    }

  ShowCurrentMenuList(3);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubStringGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleSubStringGadget", NULL);

  stringinfo = G_SubString->SpecialInfo;
  GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);

  if(selectedsub) strcpy(selectedsub->SubText, stringinfo->Buffer);

  GT_SetGadgetAttrs(G_SubList, W_Menu, NULL,
                    GTLV_Labels, &selecteditem->SubList,
                    GTLV_Selected, selecteditem->SubCount,
                    GTLV_Top, selecteditem->SubCount,
                    TAG_DONE);

  stringinfo = G_SubLabel->SpecialInfo;
  if(stringinfo->Buffer[0] == '\0')
    {
    strcpy(selectedsub->SourceLabel, Text2Label(selectedsub->SubText));
    GT_SetGadgetAttrs(G_SubLabel, W_Menu, NULL, GTST_String, selectedsub->SourceLabel, TAG_DONE);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubLabelGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleSubLabelGadget", NULL);

  stringinfo = G_SubLabel->SpecialInfo;
  if(selectedsub) strcpy(selectedsub->SourceLabel, stringinfo->Buffer);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubAddGadget(void)
  {
  struct SubNode *subnode;

  DEBUGENTER("HandleSubAddGadget", NULL);

  if(selecteditem->SubCount < MAXSUBS)
    {
    if(subnode = AllocMem(sizeof(struct SubNode), MEMF_CLEAR))
      {
      DEBUGALLOC(sizeof (struct SubNode));

      GT_SetGadgetAttrs(G_SubString, W_Menu, NULL, GTST_String, "", GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_SubLabel, W_Menu, NULL, GTST_String, "", GA_Disabled, FALSE, TAG_DONE);
      GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);
      GT_SetGadgetAttrs(G_SubDel, W_Menu, NULL, GA_Disabled, FALSE, TAG_DONE);

      strcpy(subnode->SubText, "(new)");
      subnode->Node.ln_Name = subnode->SubText;
      AddTail(&selecteditem->SubList, (struct Node *) subnode);
      selectedsub = subnode;

      GT_SetGadgetAttrs(G_SubList, W_Menu, NULL,
                        GTLV_Labels, &selecteditem->SubList,
                        GTLV_Selected, selecteditem->SubCount,
                        GTLV_Top, selecteditem->SubCount,
                        TAG_DONE);

      selecteditem->SubCount++;
/*
      HandleItemListGadget(selecteditemnum);
*/
      selectedsubnum = selecteditem->SubCount - 1;
      HandleSubListGadget(selectedsubnum);
      ActivateGadget(G_SubString, W_Menu, NULL);
      }
    else
      PutError("Out of memory","OK");
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubDelGadget(void)
  {
  DEBUGENTER("HandleSubDelGadget", NULL);

  if(selecteditem->SubCount > 0)
    {
    GT_SetGadgetAttrs(G_SubString, W_Menu, NULL, GTST_String, "", TAG_DONE);
    GT_SetGadgetAttrs(G_SubLabel, W_Menu, NULL, GTST_String, "", TAG_DONE);
    GT_SetGadgetAttrs(G_SubList, W_Menu, NULL, GTLV_Labels, ~0, TAG_DONE);

    Remove((struct Node *) selectedsub);

    GT_SetGadgetAttrs(G_SubList, W_Menu, NULL,
                      GTLV_Labels, &selecteditem->SubList,
                      GTLV_Selected, 0,
                      GTLV_Top, 0,
                      TAG_DONE);

    DEBUGFREE(sizeof (struct SubNode));
    FreeMem(selectedsub, sizeof(struct SubNode));

    selecteditem->SubCount--;
    HandleItemListGadget(selecteditemnum);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubUpGadget(void)
  {
  DEBUGENTER("HandleSubUpGadget", NULL);

  if(selectedsubnum > 0)
    {
    MoveNodeUp(&selecteditem->SubList, selectedsubnum, W_Menu, G_SubList);
    HandleSubListGadget(selectedsubnum-1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubDownGadget(void)
  {
  DEBUGENTER("HandleSubDownGadget", NULL);

  if(selectedsubnum < selecteditem->SubCount-1)
    {
    MoveNodeDown(&selecteditem->SubList, selectedsubnum, W_Menu, G_SubList);
    HandleSubListGadget(selectedsubnum+1);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSubDisabledGadget(void)
  {
  DEBUGENTER("HandleSubDisabledGadget", NULL);

  if(G_SubDisabled->Flags & SELECTED)
    selectedsub->Flags |= NM_ITEMDISABLED;
  else
    selectedsub->Flags &= ~NM_ITEMDISABLED;

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleCheckItGadget(void)
  {
  DEBUGENTER("HandleCheckItGadget", NULL);

  if(G_CheckIt->Flags & SELECTED)
    {
    if(selecteditem->SubCount)
      selectedsub->Flags |= CHECKIT;
    else
      selecteditem->Flags |= CHECKIT;
    }
  else
    {
    if(selecteditem->SubCount)
      selectedsub->Flags &= ~CHECKIT;
    else
      selecteditem->Flags &= ~CHECKIT;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleCheckedGadget(void)
  {
  DEBUGENTER("HandleCheckedGadget", NULL);

  if(G_Checked->Flags & SELECTED)
    {
    if(selecteditem->SubCount)
      selectedsub->Flags |= CHECKED;
    else
      selecteditem->Flags |= CHECKED;
    }
  else
    {
    if(selecteditem->SubCount)
      selectedsub->Flags &= ~CHECKED;
    else
      selecteditem->Flags &= ~CHECKED;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleToggleGadget(void)
  {
  DEBUGENTER("HandleToggleGadget", NULL);

  if(G_Toggle->Flags & SELECTED)
    {
    if(selecteditem->SubCount)
      selectedsub->Flags |= MENUTOGGLE;
    else
      selecteditem->Flags |= MENUTOGGLE;
    }
  else
    {
    if(selecteditem->SubCount)
      selectedsub->Flags &= ~MENUTOGGLE;
    else
      selecteditem->Flags &= ~MENUTOGGLE;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleExcludeGadget(void)
  {
  DEBUGENTER("HandleExcludeGadget", NULL);

  if(G_Exclude->Flags & SELECTED)
    {
    if(selecteditem->SubCount)
      selectedsub->TMMFlags |= MENUFLAG_AUTOEXCLUDE;
    else
      selecteditem->TMMFlags |= MENUFLAG_AUTOEXCLUDE;
    }
  else
    {
    if(selecteditem->SubCount)
      selectedsub->TMMFlags &= ~MENUFLAG_AUTOEXCLUDE;
    else
      selecteditem->TMMFlags &= ~MENUFLAG_AUTOEXCLUDE;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleCommKeyGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleCommKeyGadget", NULL);

  stringinfo = G_CommKey->SpecialInfo;

  if(selecteditem->SubCount)
    strcpy(selectedsub->CommKey, stringinfo->Buffer);
  else
    strcpy(selecteditem->CommKey, stringinfo->Buffer);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

struct ItemNode *NextItem(struct MenuNode *menunode)
  {
  static struct ItemNode nextitemnode;

  DEBUGENTER("NextItem", NULL);

  nextitemnode.Flags = 0;
  nextitemnode.TMMFlags = 0;
  nextitemnode.SubCount = 0;
  nextitemnode.ItemText[0] = '\0';
  nextitemnode.CommKey[0] = '\0';
  nextitemnode.SourceLabel[0] = '\0';

  if(!strcmp(menunode->MenuText, "Project"))
    {
    if(menunode->ItemCount < NUMPROJECTITEMS)
      {
      strcpy(nextitemnode.ItemText,    projectitem[menunode->ItemCount].ItemText);
      strcpy(nextitemnode.CommKey,     projectitem[menunode->ItemCount].CommKey);
      strcpy(nextitemnode.SourceLabel, projectitem[menunode->ItemCount].SourceLabel);
      nextitemnode.Flags = projectitem[menunode->ItemCount].Flags;
      }
    }
  else if(!strcmp(menunode->MenuText, "Edit"))
    {
    if(menunode->ItemCount < NUMEDITITEMS)
      {
      strcpy(nextitemnode.ItemText,    edititem[menunode->ItemCount].ItemText);
      strcpy(nextitemnode.CommKey,     edititem[menunode->ItemCount].CommKey);
      strcpy(nextitemnode.SourceLabel, edititem[menunode->ItemCount].SourceLabel);
      nextitemnode.Flags = edititem[menunode->ItemCount].Flags;
      }
    }
  else if(!strcmp(menunode->MenuText, "Macros"))
    {
    if(menunode->ItemCount < NUMMACROITEMS)
      {
      strcpy(nextitemnode.ItemText,    macroitem[menunode->ItemCount].ItemText);
      strcpy(nextitemnode.CommKey,     macroitem[menunode->ItemCount].CommKey);
      strcpy(nextitemnode.SourceLabel, macroitem[menunode->ItemCount].SourceLabel);
      nextitemnode.Flags = macroitem[menunode->ItemCount].Flags;
      }
    }
  else if(!strcmp(menunode->MenuText, "Settings"))
    {
    if(menunode->ItemCount < NUMSETTINGITEMS)
      {
      strcpy(nextitemnode.ItemText,    settingitem[menunode->ItemCount].ItemText);
      strcpy(nextitemnode.CommKey,     settingitem[menunode->ItemCount].CommKey);
      strcpy(nextitemnode.SourceLabel, settingitem[menunode->ItemCount].SourceLabel);
      nextitemnode.Flags = settingitem[menunode->ItemCount].Flags;
      }
    }
  else if(!strcmp(menunode->MenuText, "User"))
    {
    if(menunode->ItemCount < NUMUSERITEMS)
      {
      strcpy(nextitemnode.ItemText,    useritem[menunode->ItemCount].ItemText);
      strcpy(nextitemnode.CommKey,     useritem[menunode->ItemCount].CommKey);
      strcpy(nextitemnode.SourceLabel, useritem[menunode->ItemCount].SourceLabel);
      nextitemnode.Flags = useritem[menunode->ItemCount].Flags;
      }
    }

  if(nextitemnode.ItemText[0] == '\0')
    {
    strcpy(nextitemnode.ItemText, "(new)");
    }

  DEBUGEXIT(TRUE, (ULONG) &nextitemnode);
  return(&nextitemnode);
  }

/****************************************************************************/

void ShowCurrentMenuList(int part)
  {
  struct RastPort *rp;

  DEBUGENTER("ShowCurrentMenuList", NULL);

  rp = W_Menu->RPort;
  menupart = part;

  switch(part)
    {
    case 1:
      SetAPen(rp, 0);
      Move(rp, 176+152/2-24, currenttopborder+18-2);
      Draw(rp, 176+152/2+24, currenttopborder+18-2);
      Move(rp, 338+152/2-35, currenttopborder+18-2);
      Draw(rp, 338+152/2+35, currenttopborder+18-2);
      SetAPen(rp, 1);
      Move(rp,  14+152/2-24, currenttopborder+18-2);
      Draw(rp,  14+152/2+24, currenttopborder+18-2);
      break;

    case 2:
      SetAPen(rp, 0);
      Move(rp,  14+152/2-24, currenttopborder+18-2);
      Draw(rp,  14+152/2+24, currenttopborder+18-2);
      Move(rp, 338+152/2-35, currenttopborder+18-2);
      Draw(rp, 338+152/2+35, currenttopborder+18-2);
      SetAPen(rp, 1);
      Move(rp, 176+152/2-24, currenttopborder+18-2);
      Draw(rp, 176+152/2+24, currenttopborder+18-2);
      break;

    case 3:
      SetAPen(rp, 0);
      Move(rp,  14+152/2-24, currenttopborder+18-2);
      Draw(rp,  14+152/2+24, currenttopborder+18-2);
      Move(rp, 176+152/2-24, currenttopborder+18-2);
      Draw(rp, 176+152/2+24, currenttopborder+18-2);
      SetAPen(rp, 1);
      Move(rp, 338+152/2-35, currenttopborder+18-2);
      Draw(rp, 338+152/2+35, currenttopborder+18-2);
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

