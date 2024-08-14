
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

void EditTags(int type, struct WindowNode *windownode, struct NewGadgetNode *newgadgetnode)
  {
  BOOL   done=FALSE;
  BOOL   error=FALSE;
  BOOL   found;
  int    i;
  int    status;
  int    count;
  int    lvtop;
  int    lvheight;
  int    arrowoffset;
  int    titlewidth;
  char   *windowtitle;
  char   *title;
  char   *label;
  char   **stringpointer;
  WORD   *wordpointer;
  ULONG  lastgadget;
  struct TagNode        *tagnode;
  struct TagNode        *tn;
  struct EditStringNode *editstringnode;
  struct StringNode     *labelpointer;
  struct StringNode     *stringnode;
  struct EditTagNode    *edittagnode;
  struct NewGadget      NG_Temp;
  struct Gadget         *G_Context, *G_List;
  struct Gadget         *G_OK, *G_Cancel;
  struct AvailableTags  *availabletags;
  struct AvailableTags  *thisavailabletag;
  struct List           *stringlist;
  struct List           *taglist;

  DEBUGENTER("EditTags", NULL);

  MainWindowSleep();

  edittagtype = type;

  switch(type)
    {
    case TYPE_SCREEN:
      windowtitle = "Screen Attributes";
      taglist = &screennode.TagList;
      title = screennode.Title;
      label = screennode.SourceLabel;
      lvtop = 44;
      lvheight = 95; /* 100; */
      arrowoffset = 5;
      titlewidth = 402; /* 380; */
      availabletags = &AvailableScreenTags[0];
      break;
    case TYPE_WINDOW:
      windowtitle = "Window Attributes";
      taglist = &windownode->TagList;
      title = windownode->Title;
      label = windownode->SourceLabel;
      lvtop = 44;
      lvheight = 95; /* 100; */
      arrowoffset = 5;
      titlewidth = 402; /* 380; */
      availabletags = &AvailableWindowTags[0];
      break;
    case TYPE_GADGET:
      taglist = &newgadgetnode->TagList;
      title = newgadgetnode->GadgetText;
      label = newgadgetnode->SourceLabel;
      lvtop = 60;
      lvheight = 80;
      arrowoffset = 15;
      titlewidth = 247; /* 225; */
      switch(newgadgetnode->Kind)
        {
        case BUTTON_KIND:
          windowtitle = "Button Gadget Attributes";
          availabletags = &AvailableBUGadgetTags[0];
          break;
        case CHECKBOX_KIND:
          windowtitle = "Checkbox Gadget Attributes";
          availabletags = &AvailableCBGadgetTags[0];
          break;
        case CYCLE_KIND:
          windowtitle = "Cycle Gadget Attributes";
          availabletags = &AvailableCYGadgetTags[0];
          break;
        case INTEGER_KIND:
          windowtitle = "Integer Gadget Attributes";
          availabletags = &AvailableINGadgetTags[0];
          break;
        case LISTVIEW_KIND:
          windowtitle = "Listview Gadget Attributes";
          availabletags = &AvailableLVGadgetTags[0];
          break;
        case MX_KIND:
          windowtitle = "Mx Gadget Attributes";
          availabletags = &AvailableMXGadgetTags[0];
          break;
        case NUMBER_KIND:
          windowtitle = "Number Gadget Attributes";
          availabletags = &AvailableNMGadgetTags[0];
          break;
        case PALETTE_KIND:
          windowtitle = "Palette Gadget Attributes";
          availabletags = &AvailablePAGadgetTags[0];
          break;
        case SCROLLER_KIND:
          windowtitle = "Scroller Gadget Attributes";
          availabletags = &AvailableSCGadgetTags[0];
          break;
        case SLIDER_KIND:
          windowtitle = "Slider Gadget Attributes";
          availabletags = &AvailableSLGadgetTags[0];
          break;
        case STRING_KIND:
          windowtitle = "String Gadget Attributes";
          availabletags = &AvailableSTGadgetTags[0];
          break;
        case TEXT_KIND:
          windowtitle = "Text Gadget Attributes";
          availabletags = &AvailableTXGadgetTags[0];
          break;
        }
      break;
    }

  NewList(&AvailableTagsList);
  NewList(&SelectedTagsList);

  selectedtag = NULL;
  availabletag = NULL;

  selectedcount = 0;
  availablecount = 0;

  count=0;
  thisavailabletag = availabletags;
  while(thisavailabletag->Tag != TAG_DONE)
    {
    if(thisavailabletag->Type != TYPE_VOID)
      {
      status = TRUE;
      while(!(edittagnode = AllocMem(sizeof(struct EditTagNode), MEMF_CLEAR)) && status)
        status = PutError("Out of memory", "Retry|Abort");

      if(status)
        {
        DEBUGALLOC(sizeof (struct EditTagNode));

        NewList(&edittagnode->EditStringList);

        found = FALSE;
        for(tn = (struct TagNode *) taglist->lh_Head; tn->Node.ln_Succ;
            tn = (struct TagNode *) tn->Node.ln_Succ)
          {
          if(tn->TagItem.ti_Tag == thisavailabletag->Tag)
            {
            found = TRUE;
            tagnode = tn;
            }
          }

        if(found == TRUE) /* get from screen, window, or gadget list */
          {
          edittagnode->TagItem.ti_Tag = tagnode->TagItem.ti_Tag;
          edittagnode->Node.ln_Type   = tagnode->Node.ln_Type;
          edittagnode->Node.ln_Pri    = tagnode->Node.ln_Pri;
          edittagnode->Node.ln_Name   = thisavailabletag->Name;

          switch(thisavailabletag->Type)
            {
            case TYPE_INTEGER:
            case TYPE_IGNORE:
            case TYPE_STRING:
              edittagnode->StringCount = 1;
              status = TRUE;
              while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                status = PutError("Out of memory", "Retry|Abort");

              if(status)
                {
                DEBUGALLOC(sizeof (struct EditStringNode));

                strcpy(editstringnode->String, (char *) tagnode->Data);
                strcpy(editstringnode->SourceLabel, "");
                editstringnode->Node.ln_Name = editstringnode->String;

                AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                }
              else
                {
                error = TRUE;    /* Couldn't allocate editstringnode */
                }
              break;

            case TYPE_CHARACTER:
              edittagnode->StringCount = 1;
              status = TRUE;
              while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                status = PutError("Out of memory", "Retry|Abort");

              if(status)
                {
                DEBUGALLOC(sizeof (struct EditStringNode));

                sprintf(editstringnode->String, "%c", tagnode->TagItem.ti_Data);
                strcpy(editstringnode->SourceLabel, "");
                editstringnode->Node.ln_Name = editstringnode->String;

                AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                }
              else
                {
                error = TRUE;   /* Couldn't allocate editstringnode */
                }
              break;

            case TYPE_WORDLIST:
              wordpointer = (WORD *) tagnode->Data;
              edittagnode->StringCount = tagnode->DataCount;
              for(i=0; i<edittagnode->StringCount; i++)
                {
                status = TRUE;
                while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                  status = PutError("Out of memory", "Retry|Abort");

                if(status)
                  {
                  DEBUGALLOC(sizeof (struct EditStringNode));

                  sprintf(editstringnode->String, "%d", *wordpointer); /* ((WORD **)tagnode->Data)[i]); */
                  strcpy(editstringnode->SourceLabel, "");
                  editstringnode->Node.ln_Name = editstringnode->String;

                  AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                  wordpointer++;
                  }
                else
                  {
                  error = TRUE;   /* Couldn't allocate editstringnode */
                  }
                }
              break;

            case TYPE_STRINGLIST:
              edittagnode->StringCount = tagnode->DataCount;
              stringpointer = (char **) tagnode->Data;
              labelpointer = (struct StringNode *) tagnode->SourceLabelList.lh_Head;
              for(i=0; i<edittagnode->StringCount; i++)
                {
                status = TRUE;
                while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                  status = PutError("Out of memory", "Retry|Abort");

                if(status)
                  {
                  DEBUGALLOC(sizeof (struct EditStringNode));

                  strcpy(editstringnode->String, *stringpointer);
                  strcpy(editstringnode->SourceLabel, labelpointer->String);
                  editstringnode->Node.ln_Name = editstringnode->String;

                  AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                  stringpointer++;
                  labelpointer = (struct StringNode *) labelpointer->Node.ln_Succ;
                  }
                else
                  {
                  error = TRUE;   /* Couldn't allocate editstringnode */
                  }
                }
              break;

            case TYPE_LINKEDLIST:
              edittagnode->StringCount = tagnode->DataCount;
              stringlist = (struct List *) tagnode->Data;
              labelpointer = (struct StringNode *) tagnode->SourceLabelList.lh_Head;
              for(stringnode = (struct StringNode *) stringlist->lh_Head; stringnode->Node.ln_Succ;
                  stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
                {
                status = TRUE;
                while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                  status = PutError("Out of memory", "Retry|Abort");

                if(status)
                  {
                  DEBUGALLOC(sizeof (struct EditStringNode));

                  strcpy(editstringnode->String, stringnode->String);
                  strcpy(editstringnode->SourceLabel, labelpointer->String);
                  editstringnode->Node.ln_Name = editstringnode->String;

                  AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                  labelpointer = (struct StringNode *) labelpointer->Node.ln_Succ;
                  }
                else
                  {
                  error = TRUE;   /* Couldn't allocate editstringnode */
                  }
                }
              break;
            }

          AddTail(&SelectedTagsList, (struct Node *) edittagnode);
          selectedcount++;
          }

        else /* get from available list */
          {
          edittagnode->TagItem.ti_Tag = thisavailabletag->Tag;
          edittagnode->Node.ln_Type   = thisavailabletag->Type;
          edittagnode->Node.ln_Pri    = count;
          edittagnode->Node.ln_Name   = thisavailabletag->Name;

          switch(thisavailabletag->Type)
            {
            case TYPE_INTEGER:
            case TYPE_IGNORE:
            case TYPE_STRING:
              edittagnode->StringCount = 1;
              status = TRUE;
              while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                status = PutError("Out of memory", "Retry|Abort");

              if(status)
                {
                DEBUGALLOC(sizeof (struct EditStringNode));

                strcpy(editstringnode->String, (char *) thisavailabletag->Data);
                strcpy(editstringnode->SourceLabel, "");
                editstringnode->Node.ln_Name = editstringnode->String;

                AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                }
              else
                {
                error = TRUE;
                }
              break;

            case TYPE_CHARACTER:
              edittagnode->StringCount = 1;
              status = TRUE;
              while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                status = PutError("Out of memory", "Retry|Abort");

              if(status)
                {
                DEBUGALLOC(sizeof (struct EditStringNode));

                sprintf(editstringnode->String, "%c", thisavailabletag->Data);
                strcpy(editstringnode->SourceLabel, "");
                editstringnode->Node.ln_Name = editstringnode->String;

                AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                }
              else
                {
                error = TRUE;
                }
              break;

            case TYPE_WORDLIST:
              wordpointer = (WORD *) thisavailabletag->Data;
              edittagnode->StringCount = *wordpointer;
              wordpointer++;
              for(i=0; i<edittagnode->StringCount; i++)
                {
                status = TRUE;
                while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                  status = PutError("Out of memory", "Retry|Abort");

                if(status)
                  {
                  DEBUGALLOC(sizeof (struct EditStringNode));

                  sprintf(editstringnode->String, "%d", *wordpointer);
                  strcpy(editstringnode->SourceLabel, "");
                  editstringnode->Node.ln_Name = editstringnode->String;

                  AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                  wordpointer++;
                  }
                else
                  {
                  error = TRUE;
                  }
                }
              break;

            case TYPE_STRINGLIST:
            case TYPE_LINKEDLIST:
              edittagnode->StringCount = ((struct StringData *) thisavailabletag->Data)->StringCount;
              stringpointer = ((struct StringData *) thisavailabletag->Data)->StringArray;
              for(i=0; i<edittagnode->StringCount; i++)
                {
                status = TRUE;
                while(!(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR)) && status)
                  status = PutError("Out of memory", "Retry|Abort");

                if(status)
                  {
                  DEBUGALLOC(sizeof (struct EditStringNode));

                  strcpy(editstringnode->String, *stringpointer);
                  strcpy(editstringnode->SourceLabel, "");
                  editstringnode->Node.ln_Name = editstringnode->String;

                  AddTail(&edittagnode->EditStringList, (struct Node *) editstringnode);
                  stringpointer++;
                  }
                else
                  {
                  error = TRUE;
                  }
                }
              break;
            }

          AddTail(&AvailableTagsList, (struct Node *) edittagnode);
          availablecount++;
          }
        }
      else
        {
        error = TRUE;    /* Couldn't allocate edittagnode */
        }
      }
    thisavailabletag++;
    count++;
    }

  if(!error)
    {
    G_List = NULL;
    G_Context = CreateContext(&G_List);

    /* Title and Label Gadgets */

    NG_Temp.ng_TextAttr = &TOPAZ80;
    NG_Temp.ng_VisualInfo = VisualInfo;
    NG_Temp.ng_UserData = NULL;

    NG_Temp.ng_TopEdge = currenttopborder+8;
    NG_Temp.ng_Flags = PLACETEXT_LEFT;
    NG_Temp.ng_Height = 14;

    NG_Temp.ng_LeftEdge = 60;
    NG_Temp.ng_Width = titlewidth;
    NG_Temp.ng_GadgetText = "T_itle";
    NG_Temp.ng_GadgetID = ID_TEXT;
    G_Title = CreateGadget(STRING_KIND, G_Context, &NG_Temp, GTST_String, title, GTST_MaxChars, STRINGSIZE-1, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 527; /* 505; */
    NG_Temp.ng_Width = 84; /* 106; */
    NG_Temp.ng_GadgetText = "La_bel";
    NG_Temp.ng_GadgetID = ID_LABEL;
    G_Label = CreateGadget(STRING_KIND, G_Title, &NG_Temp, GTST_String, label, GTST_MaxChars, LABELSIZE-1, GT_Underscore, '_', TAG_DONE);

    /* Available and Selected */

    NG_Temp.ng_TopEdge = currenttopborder+lvtop;
    NG_Temp.ng_Width = 195;
    NG_Temp.ng_Height = lvheight;
    NG_Temp.ng_Flags = PLACETEXT_ABOVE;

    NG_Temp.ng_LeftEdge = 10;
    NG_Temp.ng_GadgetText = "A_vailable Tags";
    NG_Temp.ng_GadgetID = ID_AVAILABLE;
    G_AvailableTags = CreateGadget(LISTVIEW_KIND, G_Label, &NG_Temp, GTLV_Labels, &AvailableTagsList, GTLV_ShowSelected, NULL, GTLV_Selected, 0, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 245;
    NG_Temp.ng_GadgetText = "_Selected Tags";
    NG_Temp.ng_GadgetID = ID_SELECTED;
    G_SelectedTags = CreateGadget(LISTVIEW_KIND, G_AvailableTags, &NG_Temp, GTLV_Labels, &SelectedTagsList, GTLV_ShowSelected, NULL, GTLV_Selected, 0, GT_Underscore, '_', TAG_DONE);

    /* Tag Type */

    NG_Temp.ng_LeftEdge = 245;
    NG_Temp.ng_TopEdge = currenttopborder+145;
    NG_Temp.ng_Width = 195;
    NG_Temp.ng_Height = 12;
    NG_Temp.ng_Flags = PLACETEXT_LEFT;
    NG_Temp.ng_GadgetText = "Tag Type";
    NG_Temp.ng_GadgetID = ID_TAGTYPE;
    G_TagType = CreateGadget(TEXT_KIND, G_SelectedTags, &NG_Temp, GA_Disabled, TRUE, GTTX_Text, "", GTTX_Border, TRUE, TAG_DONE);

    /* Use and Remove */

    NG_Temp.ng_LeftEdge = 210;
    NG_Temp.ng_Width = 30;
    NG_Temp.ng_Height = 20;
    NG_Temp.ng_Flags = PLACETEXT_IN;

    NG_Temp.ng_TopEdge = currenttopborder+55+arrowoffset;
    NG_Temp.ng_GadgetText = "->";
    NG_Temp.ng_GadgetID = ID_TAGUSE;
    G_Use = CreateGadget(BUTTON_KIND, G_TagType, &NG_Temp, TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+85+arrowoffset;
    NG_Temp.ng_GadgetText = "<-";
    NG_Temp.ng_GadgetID = ID_TAGREMOVE;
    G_Remove = CreateGadget(BUTTON_KIND, G_Use, &NG_Temp, TAG_DONE);

    /* Label and String */

    NG_Temp.ng_LeftEdge = 450;
    NG_Temp.ng_Width = 160;

    NG_Temp.ng_TopEdge = currenttopborder+96;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_GadgetText = "";
    NG_Temp.ng_Flags = PLACETEXT_RIGHT;
    NG_Temp.ng_GadgetID = ID_TAGSTRING;
    G_String = CreateGadget(STRING_KIND, G_Remove, &NG_Temp, GTST_MaxChars, STRINGSIZE-1, TAG_DONE);

    NG_Temp.ng_TopEdge = currenttopborder+110;
    NG_Temp.ng_GadgetText = "";
    NG_Temp.ng_GadgetID = ID_TAGLABEL;
    G_TagLabel = CreateGadget(STRING_KIND, G_String, &NG_Temp, GTST_MaxChars, LABELSIZE-1, TAG_DONE);

    /* Tag List */

    NG_Temp.ng_LeftEdge = 450;
    NG_Temp.ng_Width = 160;
    NG_Temp.ng_TopEdge = currenttopborder+60;
    NG_Temp.ng_Height = 42;
    NG_Temp.ng_GadgetText = "Ta_g Value/Label";
    NG_Temp.ng_GadgetID = ID_TAGLIST;
    NG_Temp.ng_Flags = PLACETEXT_ABOVE;
    G_ListView = CreateGadget(LISTVIEW_KIND, G_TagLabel, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    /* Add, Del, Up, and Down */

    NG_Temp.ng_TopEdge = currenttopborder+124;
    NG_Temp.ng_Width = 40;
    NG_Temp.ng_Height = 14;
    NG_Temp.ng_Flags = PLACETEXT_IN;

    NG_Temp.ng_LeftEdge = 450;
    NG_Temp.ng_GadgetText = "_Add";
    NG_Temp.ng_GadgetID = ID_TAGADD;
    G_TagAdd = CreateGadget(BUTTON_KIND, G_ListView, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 490;
    NG_Temp.ng_GadgetText = "_Del";
    NG_Temp.ng_GadgetID = ID_TAGDEL;
    G_TagDel = CreateGadget(BUTTON_KIND, G_TagAdd, &NG_Temp, GA_Disabled, TRUE, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 530;
    NG_Temp.ng_GadgetText = "/\\";
    NG_Temp.ng_GadgetID = ID_TAGUP;
    G_TagUp = CreateGadget(BUTTON_KIND, G_TagDel, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    NG_Temp.ng_LeftEdge = 570;
    NG_Temp.ng_GadgetText = "\\/";
    NG_Temp.ng_GadgetID = ID_TAGDOWN;
    G_TagDown = CreateGadget(BUTTON_KIND, G_TagUp, &NG_Temp, GA_Disabled, TRUE, TAG_DONE);

    /* OK, Kill, and Cancel */

    NG_Temp.ng_TopEdge = currenttopborder+145;
    NG_Temp.ng_Width = 80;
    NG_Temp.ng_Height = 15;
    NG_Temp.ng_Flags = PLACETEXT_IN;

    NG_Temp.ng_LeftEdge = 10;
    NG_Temp.ng_GadgetText = "_OK";
    NG_Temp.ng_GadgetID = ID_OK;
    G_OK = CreateGadget(BUTTON_KIND, G_TagDown, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    NG_Temp.ng_LeftEdge = 530;
    NG_Temp.ng_GadgetText = "_Cancel";
    NG_Temp.ng_GadgetID = ID_CANCEL;
    G_Cancel = CreateGadget(BUTTON_KIND, G_OK, &NG_Temp, GT_Underscore, '_', TAG_DONE);

    lastgadget = (ULONG) G_Cancel;

    switch(type)
      {
      case TYPE_GADGET:
        NG_Temp.ng_TopEdge = currenttopborder+27;
        NG_Temp.ng_Flags = PLACETEXT_RIGHT;
        NG_Temp.ng_LeftEdge = 10;
        NG_Temp.ng_GadgetText = "_Highlight";
        NG_Temp.ng_GadgetID = ID_HIGHLIGHT;
        G_Highlight = CreateGadget(CHECKBOX_KIND, G_Cancel, &NG_Temp, GTCB_Checked, newgadgetnode->NewGadget.ng_Flags & NG_HIGHLABEL, GT_Underscore, '_', TAG_DONE);

        NG_Temp.ng_TopEdge = currenttopborder+26;
        NG_Temp.ng_Width =  60;
        NG_Temp.ng_Height = 14;
        NG_Temp.ng_Flags = PLACETEXT_LEFT;

        NG_Temp.ng_LeftEdge = 164;
        NG_Temp.ng_GadgetText = "_Top";
        NG_Temp.ng_GadgetID = ID_TOPEDGE;
        G_TopEdge = CreateGadget(INTEGER_KIND, G_Highlight, &NG_Temp, GTIN_Number, newgadgetnode->NewGadget.ng_TopEdge, GT_Underscore, '_', TAG_DONE);

        NG_Temp.ng_LeftEdge = 280;
        NG_Temp.ng_GadgetText = "Le_ft";
        NG_Temp.ng_GadgetID = ID_LEFTEDGE;
        G_LeftEdge = CreateGadget(INTEGER_KIND, G_TopEdge, &NG_Temp, GTIN_Number, newgadgetnode->NewGadget.ng_LeftEdge, GT_Underscore, '_', TAG_DONE);

        NG_Temp.ng_LeftEdge = 414;
        NG_Temp.ng_GadgetText = "_Width";
        NG_Temp.ng_GadgetID = ID_WIDTH;
        G_Width = CreateGadget(INTEGER_KIND, G_LeftEdge, &NG_Temp, GTIN_Number, newgadgetnode->NewGadget.ng_Width, GT_Underscore, '_', TAG_DONE);

        NG_Temp.ng_LeftEdge = 551;
        NG_Temp.ng_GadgetText = "H_eight";
        NG_Temp.ng_GadgetID = ID_HEIGHT;
        G_Height = CreateGadget(INTEGER_KIND, G_Width, &NG_Temp, GTIN_Number, newgadgetnode->NewGadget.ng_Height, GT_Underscore, '_', TAG_DONE);

        for(i=0; !(positionvalues[i]&newgadgetnode->NewGadget.ng_Flags) && (positionvalues[i]!=NULL); i++);
        textpos = i;

        NG_Temp.ng_TopEdge = currenttopborder+8;
        NG_Temp.ng_LeftEdge = 372; /* 350; */
        NG_Temp.ng_Width = 90;
        NG_Temp.ng_GadgetText = "_Place";
        NG_Temp.ng_Flags = PLACETEXT_LEFT;
        NG_Temp.ng_GadgetID = ID_TEXTPOS;
        G_TextPos = CreateGadget(CYCLE_KIND, G_Height, &NG_Temp, GTCY_Labels, positionlabels, GTCY_Active, textpos, GT_Underscore, '_', TAG_DONE);

        lastgadget = (ULONG) G_TextPos;

        break;
      }

    if(lastgadget)
      {
      if(W_Edit = OpenWindowTags(NULL,
                                 WA_PubScreen, screennode.Screen,
                                 WA_Left, CenterHoriz(615),
                                 WA_Top,  CenterVert(165),
                                 WA_IDCMP, editidcmp,
                                 WA_Gadgets, G_List,
                                 WA_Title, windowtitle,
                                 WA_Activate, TRUE,
                                 WA_DragBar, TRUE,
                                 WA_DepthGadget, TRUE,
                                 WA_SimpleRefresh, TRUE,
                                 WA_InnerWidth, 615,
                                 WA_InnerHeight, 165,
                                 TAG_DONE))
        {
        GT_RefreshWindow(W_Edit, NULL);

        HandleAvailableGadget(0);
        HandleSelectedGadget(0);

        editsignal = 1L << W_Edit->UserPort->mp_SigBit;

        while(!done)
          {
          Wait(editsignal);
          done = HandleRequesterIDCMP(W_Edit);
          }

        ModifyIDCMP(W_Edit, NULL);
        editsignal = NULL;

        switch(done)
          {
          case DONE_OK:
            switch(type)
              {
              case TYPE_SCREEN: RemakeScreen();                          break;
              case TYPE_WINDOW: RemakeWindow(windownode);                break;
              case TYPE_GADGET: RemakeGadget(newgadgetnode, windownode); break;
              }
            modified = TRUE;
            break;
          }

        CloseWindow(W_Edit);
        }

      FreeGadgets(G_List);
      }
    }

  if(currentwindow && currentwindow->Window)
    {
    ActivateWindow(currentwindow->Window);
    }

  KillEditTagList(&AvailableTagsList);
  KillEditTagList(&SelectedTagsList);

  if(error)
    {
    PutError("Error opening\ntags window", "OK");
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTitleGadget(void)
  {
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleTitleGadget", NULL);

  stringinfo = G_Label->SpecialInfo;
  if(stringinfo->Buffer[0] == '\0')
    {
    stringinfo = G_Title->SpecialInfo;
    GT_SetGadgetAttrs(G_Label, W_Edit, NULL, GTST_String, Text2Label(stringinfo->Buffer), TAG_DONE);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleAvailableGadget(ULONG code)
  {
  int i;

  DEBUGENTER("HandleAvailableGadget", NULL);

  availablecode = code;

  if(AvailableTagsList.lh_TailPred == (struct Node *) &AvailableTagsList.lh_Head)
    {
    availabletag = NULL;
    GT_SetGadgetAttrs(G_Use,  W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
    }
  else
    {
    GT_SetGadgetAttrs(G_Use, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
    availabletag = (struct EditTagNode *) AvailableTagsList.lh_Head;
    for(i=0; i<availablecode; i++)
      availabletag = (struct EditTagNode *) availabletag->Node.ln_Succ;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleSelectedGadget(ULONG code)
  {
  int i;

  DEBUGENTER("HandleSelectedGadget", NULL);

  selectedcode = code;

  GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, &EmptyList, TAG_DONE);

  if(SelectedTagsList.lh_TailPred == (struct Node *) &SelectedTagsList.lh_Head)
    {
    selectedtag = NULL;
    GT_SetGadgetAttrs(G_String,  W_Edit, NULL, GA_Disabled, TRUE, GTST_String, " ", TAG_DONE);
    GT_SetGadgetAttrs(G_TagLabel,W_Edit, NULL, GA_Disabled, TRUE, GTST_String, " ", TAG_DONE);
    GT_SetGadgetAttrs(G_TagAdd,  W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_TagDel,  W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, TRUE, GTTX_Text, " ", TAG_DONE);
    GT_SetGadgetAttrs(G_Remove,  W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
    }
  else
    {
    selectedtag = (struct EditTagNode *) SelectedTagsList.lh_Head;
    for(i=0; i<code; i++)
      selectedtag = (struct EditTagNode *) selectedtag->Node.ln_Succ;

    GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, &selectedtag->EditStringList, GTLV_Selected, 0, GTLV_Top, 0, TAG_DONE);
    GT_SetGadgetAttrs(G_Remove,  W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);

    switch(selectedtag->Node.ln_Type)
      {
      case TYPE_INTEGER:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "Integer (ULONG)", TAG_DONE);
        break;

      case TYPE_IGNORE:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "Non-interactive", TAG_DONE);
        break;

      case TYPE_STRING:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "String (char *)", TAG_DONE);
        break;

      case TYPE_CHARACTER:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "Character (char)", TAG_DONE);
        break;

      case TYPE_WORDLIST:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "Integer List (WORD *)", TAG_DONE);
        break;

      case TYPE_LINKEDLIST:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "String List (Exec List)", TAG_DONE);
        break;

      case TYPE_STRINGLIST:
        GT_SetGadgetAttrs(G_TagType, W_Edit, NULL, GA_Disabled, FALSE, GTTX_Text, "String List (char **)", TAG_DONE);
        break;
      }

    switch(selectedtag->Node.ln_Type)
      {
      case TYPE_INTEGER:
      case TYPE_IGNORE:
      case TYPE_STRING:
      case TYPE_CHARACTER:
        GT_SetGadgetAttrs(G_TagAdd, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
        GT_SetGadgetAttrs(G_TagDel, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
        break;

      case TYPE_WORDLIST:
      case TYPE_STRINGLIST:
      case TYPE_LINKEDLIST:
        GT_SetGadgetAttrs(G_TagAdd, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
        if(selectedtag->StringCount > 1)
          GT_SetGadgetAttrs(G_TagDel, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
        break;
      }

    if(selectedtag->Node.ln_Type == TYPE_STRINGLIST ||
       selectedtag->Node.ln_Type == TYPE_LINKEDLIST)
      GT_SetGadgetAttrs(G_TagLabel, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_TagLabel, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);

    HandleTagListGadget(0);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagListGadget(ULONG code)
  {
  int i;
  struct EditStringNode *editstringnode;

  DEBUGENTER("HandleTagListGadget", NULL);

  selectedstringnum = code;

  editstringnode = (struct EditStringNode *) selectedtag->EditStringList.lh_Head;
  for(i=0; i<selectedstringnum; i++)
    editstringnode = (struct EditStringNode *) editstringnode->Node.ln_Succ;

  if(editstringnode)
    {
    GT_SetGadgetAttrs(G_String, W_Edit, NULL, GA_Disabled, FALSE, GTST_String, editstringnode->String, TAG_DONE);
    GT_SetGadgetAttrs(G_TagUp, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_TagDown, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);

    if(selectedstringnum == 0)
      GT_SetGadgetAttrs(G_TagUp, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);

    if(selectedstringnum == selectedtag->StringCount-1)
      GT_SetGadgetAttrs(G_TagDown, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);

    if(selectedtag->Node.ln_Type == TYPE_STRINGLIST ||
       selectedtag->Node.ln_Type == TYPE_LINKEDLIST)
      GT_SetGadgetAttrs(G_TagLabel, W_Edit, NULL, GA_Disabled, FALSE, GTST_String, editstringnode->SourceLabel, TAG_DONE);
    else
      GT_SetGadgetAttrs(G_TagLabel, W_Edit, NULL, GA_Disabled, TRUE, GTST_String, " ", TAG_DONE);
    }
  else
    {
    GT_SetGadgetAttrs(G_String, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
    GT_SetGadgetAttrs(G_TagLabel, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagRemoveGadget(void)
  {
  DEBUGENTER("HandleTagRemoveGadget", NULL);

  if(selectedtag)
    {
    GT_SetGadgetAttrs(G_AvailableTags, W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);
    GT_SetGadgetAttrs(G_SelectedTags,  W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);

    Remove((struct Node *) selectedtag);
    AddHead(&AvailableTagsList, (struct Node *) selectedtag);

    GT_SetGadgetAttrs(G_AvailableTags, W_Edit, NULL, GTLV_Labels, &AvailableTagsList, GTLV_Selected, 0, GTLV_Top, 0, TAG_DONE);
    GT_SetGadgetAttrs(G_SelectedTags,  W_Edit, NULL, GTLV_Labels, &SelectedTagsList,  GTLV_Selected, 0, GTLV_Top, 0, TAG_DONE);

    availablecount++;
    selectedcount--;

    HandleAvailableGadget(0);
    HandleSelectedGadget(0);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagUseGadget(void)
  {
  DEBUGENTER("HandleTagUseGadget", NULL);

  if(availabletag)
    {
    GT_SetGadgetAttrs(G_AvailableTags, W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);
    GT_SetGadgetAttrs(G_SelectedTags,  W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);

    Remove((struct Node *) availabletag);
    AddHead(&SelectedTagsList, (struct Node *) availabletag);

    GT_SetGadgetAttrs(G_AvailableTags, W_Edit, NULL, GTLV_Labels, &AvailableTagsList, GTLV_Selected, 0, GTLV_Top, 0, TAG_DONE);
    GT_SetGadgetAttrs(G_SelectedTags,  W_Edit, NULL, GTLV_Labels, &SelectedTagsList,  GTLV_Selected, 0, GTLV_Top, 0, TAG_DONE);

    availablecount--;
    selectedcount++;

    HandleAvailableGadget(0);
    HandleSelectedGadget(0);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagStringGadget(struct Gadget *gadget)
  {
  int i;
  struct EditStringNode *editstringnode;
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleTagStringGadget", NULL);

  stringinfo = gadget->SpecialInfo;
  if(selectedtag)
    {
    GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);

    editstringnode = (struct EditStringNode *) selectedtag->EditStringList.lh_Head;
    for(i=0; i<selectedstringnum; i++)
      editstringnode = (struct EditStringNode *) editstringnode->Node.ln_Succ;

    strcpy(editstringnode->String, stringinfo->Buffer);
    if(!(G_TagLabel->Flags&GFLG_DISABLED) && (editstringnode->SourceLabel[0]=='\0'))
      {
      strcpy(editstringnode->SourceLabel, Text2Label(stringinfo->Buffer));
      GT_SetGadgetAttrs(G_TagLabel, W_Edit, NULL, GTST_String, editstringnode->SourceLabel, TAG_DONE);
      }

    GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, &selectedtag->EditStringList, GTLV_Selected, selectedstringnum, GTLV_Top, selectedstringnum, TAG_DONE);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagLabelGadget(struct Gadget *gadget)
  {
  int i;
  struct EditStringNode *editstringnode;
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleTagLabelGadget", NULL);

  stringinfo = gadget->SpecialInfo;
  if(selectedtag)
    {
    editstringnode = (struct EditStringNode *) selectedtag->EditStringList.lh_Head;
    for(i=0; i<selectedstringnum; i++)
      editstringnode = (struct EditStringNode *) editstringnode->Node.ln_Succ;

    strcpy(editstringnode->SourceLabel, stringinfo->Buffer);
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagAddGadget(void)
  {
  struct EditStringNode *editstringnode;
  struct StringInfo *stringinfo;

  DEBUGENTER("HandleTagAddGadget", NULL);

  if(editstringnode = AllocMem(sizeof(struct EditStringNode), MEMF_CLEAR))
    {
    DEBUGALLOC(sizeof (struct EditStringNode));

    stringinfo = G_String->SpecialInfo;
    GT_SetGadgetAttrs(G_String, W_Edit, NULL, GTST_String, "(new)", TAG_DONE);
    editstringnode->Node.ln_Name = editstringnode->String;
    strcpy(editstringnode->String, stringinfo->Buffer);

    GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);
    AddTail(&selectedtag->EditStringList, (struct Node *) editstringnode);
    GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, &selectedtag->EditStringList, GTLV_Selected, selectedtag->StringCount, GTLV_Top, selectedtag->StringCount, TAG_DONE);

    HandleTagListGadget(selectedtag->StringCount);
    selectedtag->StringCount++;

    GT_SetGadgetAttrs(G_TagDel,  W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_TagUp,   W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
    GT_SetGadgetAttrs(G_TagDown, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);

    ActivateGadget(G_String, W_Edit, NULL);
    }
  else
    PutError("Out of memory", "OK");

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagDelGadget(void)
  {
  int i;
  struct EditStringNode *editstringnode;

  DEBUGENTER("HandleTagDelGadget", NULL);

  editstringnode = (struct EditStringNode *) selectedtag->EditStringList.lh_Head;
  for(i=0; i<selectedstringnum; i++)
    editstringnode = (struct EditStringNode *) editstringnode->Node.ln_Succ;

  if(selectedtag->StringCount > 1)
    {
/*
    sprintf(string, "Warning: you cannot get back\nwhat you delete!  OK to delete:\n\nTag value \"%s\"?", editstringnode->String);
    if(PutError(string, "OK|Cancel"))
      {
*/
      GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, ~0, TAG_DONE);
      Remove((struct Node *) editstringnode);
      GT_SetGadgetAttrs(G_ListView, W_Edit, NULL, GTLV_Labels, &selectedtag->EditStringList, GTLV_Selected, 0, GTLV_Top, 0, TAG_DONE);

      DEBUGFREE(sizeof(struct EditStringNode));
      FreeMem(editstringnode, sizeof(struct EditStringNode));

      HandleTagListGadget(0);
      selectedtag->StringCount--;

      GT_SetGadgetAttrs(G_TagAdd, W_Edit, NULL, GA_Disabled, FALSE, TAG_DONE);
      if(selectedtag->StringCount <= 1)
        {
        GT_SetGadgetAttrs(G_TagDel,  W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
        GT_SetGadgetAttrs(G_TagUp,   W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
        GT_SetGadgetAttrs(G_TagDown, W_Edit, NULL, GA_Disabled, TRUE, TAG_DONE);
        }
/*
      }
*/
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagUpGadget(void)
  {
  DEBUGENTER("HandleTagUpGadget", NULL);

  MoveNodeUp(&selectedtag->EditStringList, selectedstringnum, W_Edit, G_ListView);
  HandleTagListGadget(selectedstringnum-1);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTagDownGadget(void)
  {
  DEBUGENTER("HandleTagDownGadget", NULL);

  MoveNodeDown(&selectedtag->EditStringList, selectedstringnum, W_Edit, G_ListView);
  HandleTagListGadget(selectedstringnum+1);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void HandleTextPosGadget(ULONG code)
  {
  DEBUGENTER("HandleTextPosGadget", NULL);

  textpos = code;

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RemakeScreen(void)
  {
  ULONG  data;
  struct StringInfo    *stringinfo;
  struct EditTagNode   *edittagnode;

  DEBUGENTER("RemakeScreen", NULL);

  stringinfo = G_Title->SpecialInfo;
  strcpy(screennode.Title, stringinfo->Buffer);

  stringinfo = G_Label->SpecialInfo;
  strcpy(screennode.SourceLabel, stringinfo->Buffer);

  KillTagList(&screennode.TagList);
  screennode.TagCount = 0;

  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Title, (ULONG) screennode.Title);
  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_DisplayID, screennode.DisplayID);
  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Width, screennode.Width);
  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Height, screennode.Height);
  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Depth, screennode.Depth);
  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Overscan, screennode.Overscan);
  AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Font, (ULONG) &screennode.TextAttr);

  for(edittagnode = (struct EditTagNode *) SelectedTagsList.lh_Head; edittagnode->Node.ln_Succ;
      edittagnode = (struct EditTagNode *) edittagnode->Node.ln_Succ)
    {
    data = StringList2Data(&edittagnode->EditStringList, edittagnode->Node.ln_Type);

    AddTag(TYPE_SCREEN, (struct Node *) &screennode, edittagnode->TagItem.ti_Tag, data);

    FreeData(data, edittagnode->Node.ln_Type);
    }

  TagWindows(COMMAND_REMOVE);
  TagMainWindow(COMMAND_REMOVE);
  TagScreen(COMMAND_CHANGE);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RemakeWindow(struct WindowNode *windownode)
  {
  int    status;
  ULONG  data;
  struct StringInfo    *stringinfo;
  struct EditTagNode   *edittagnode;
  struct WindowNode    *newwindownode;
  struct NewGadgetNode *thisngn, *nextngn;
  struct MenuNode      *thismn,  *nextmn;

  DEBUGENTER("RemakeWindow", NULL);

  status = TRUE;
  while(!(newwindownode = (struct WindowNode *) AllocMem(sizeof(struct WindowNode), MEMF_CLEAR)) && status)
    status = PutError("Out of memory", "Retry|Abort");

  if(status)
    {
    DEBUGALLOC(sizeof (struct WindowNode));

    GetViewSize();

    newwindownode->TMWFlags = windownode->TMWFlags & ~WINDOWFLAG_GZZ;
    newwindownode->IDCMP = windownode->IDCMP;
    newwindownode->TagCount = windownode->TagCount;

    NewList(&newwindownode->TagList);
    NewList(&newwindownode->GadgetList);
    NewList(&newwindownode->NewGadgetList);
    NewList(&newwindownode->MenuList);

    HandleTagStringGadget(G_String);

    stringinfo = G_Title->SpecialInfo;
    strcpy(newwindownode->Title, stringinfo->Buffer);

    stringinfo = G_Label->SpecialInfo;
    strcpy(newwindownode->SourceLabel, stringinfo->Buffer);

    if(newwindownode->Title[0] != '\0')
      {
      AddTag(TYPE_WINDOW, (struct Node *) newwindownode, WA_Title, (ULONG) newwindownode->Title);
      }
    AddTag(TYPE_WINDOW, (struct Node *) newwindownode, WA_IDCMP, arrayidcmp);

    for(edittagnode = (struct EditTagNode *) SelectedTagsList.lh_Head; edittagnode->Node.ln_Succ;
        edittagnode = (struct EditTagNode *) edittagnode->Node.ln_Succ)
      {
      data = StringList2Data(&edittagnode->EditStringList, edittagnode->Node.ln_Type);
      AddTag(TYPE_WINDOW, (struct Node *) newwindownode, edittagnode->TagItem.ti_Tag, data);
      FreeData(data, edittagnode->Node.ln_Type);

      if(edittagnode->TagItem.ti_Tag == WA_CloseGadget) newwindownode->IDCMP |= IDCMP_CLOSEWINDOW;
      if(edittagnode->TagItem.ti_Tag == WA_GimmeZeroZero) newwindownode->TMWFlags |= WINDOWFLAG_GZZ;
      }

    thisngn = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head;
    while(nextngn = (struct NewGadgetNode *) (thisngn->Node.ln_Succ))
      {
      Remove((struct Node *) thisngn);
      AddTail(&newwindownode->NewGadgetList, (struct Node *) thisngn);
      thisngn = nextngn;
      }

    thismn = (struct MenuNode *) windownode->MenuList.lh_Head;
    while(nextmn = (struct MenuNode *) (thismn->Node.ln_Succ))
      {
      Remove((struct Node *) thismn);
      AddTail(&newwindownode->MenuList, (struct Node *) thismn);
      thismn = nextmn;
      }

    TagWindow(newwindownode, STATUS_REMOVED);

    AddTail(&WindowList, (struct Node *) newwindownode);

    status = TRUE;
    while(!(AddWindow(newwindownode)) && status)
      status = PutError("Error creating window", "Retry|Abort");

    if(status)
      {
      currentwindow = newwindownode;
      TagWindow(windownode, COMMAND_KILL);
      }
    else
      {
      thisngn = (struct NewGadgetNode *) newwindownode->NewGadgetList.lh_Head;
      while(nextngn = (struct NewGadgetNode *) (thisngn->Node.ln_Succ))
        {
        Remove((struct Node *) thisngn);
        AddTail(&windownode->NewGadgetList, (struct Node *) thisngn);
        thisngn = nextngn;
        }

      thismn = (struct MenuNode *) newwindownode->MenuList.lh_Head;
      while(nextmn = (struct MenuNode *) (thismn->Node.ln_Succ))
        {
        Remove((struct Node *) thismn);
        AddTail(&windownode->MenuList, (struct Node *) thismn);
        thismn = nextmn;
        }

      TagWindow(newwindownode, COMMAND_KILL);
      }
    }
  else
    {
    PutError("Out of memory", "OK");
    }

  if(W_Main) WindowToFront(W_Main);

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

void RemakeGadget(struct NewGadgetNode *newgadgetnode, struct WindowNode *windownode)
  {
  int    status;
  ULONG  data;
  struct StringInfo *stringinfo;
  struct EditTagNode *edittagnode;
  struct TagNode *tagnode;
  struct EditStringNode *editstringnode;
  struct StringNode *stringnode;

  DEBUGENTER("RemakeGadget", NULL);

  RemoveGadgets(windownode);
  KillTagList(&newgadgetnode->TagList);

  HandleTagStringGadget(G_String);

  stringinfo = G_Title->SpecialInfo;
  strcpy(newgadgetnode->GadgetText, stringinfo->Buffer);

  stringinfo = G_Label->SpecialInfo;
  strcpy(newgadgetnode->SourceLabel, stringinfo->Buffer);

  stringinfo = G_TopEdge->SpecialInfo;
  newgadgetnode->NewGadget.ng_TopEdge = stringinfo->LongInt;

  stringinfo = G_LeftEdge->SpecialInfo;
  newgadgetnode->NewGadget.ng_LeftEdge = stringinfo->LongInt;

  stringinfo = G_Width->SpecialInfo;
  newgadgetnode->NewGadget.ng_Width = stringinfo->LongInt;

  stringinfo = G_Height->SpecialInfo;
  newgadgetnode->NewGadget.ng_Height = stringinfo->LongInt;

  newgadgetnode->NewGadget.ng_Flags = positionvalues[textpos];

  if(G_Highlight->Flags & GFLG_SELECTED)
    newgadgetnode->NewGadget.ng_Flags |= NG_HIGHLABEL;

  for(edittagnode = (struct EditTagNode *) SelectedTagsList.lh_Head; edittagnode->Node.ln_Succ;
      edittagnode = (struct EditTagNode *) edittagnode->Node.ln_Succ)
    {
    data = StringList2Data(&edittagnode->EditStringList, edittagnode->Node.ln_Type);
    tagnode = AddTag(TYPE_GADGET, (struct Node *) newgadgetnode, edittagnode->TagItem.ti_Tag, data);
    FreeData(data, edittagnode->Node.ln_Type);

    if(tagnode)
      {
      if(edittagnode->Node.ln_Type == TYPE_LINKEDLIST ||
         edittagnode->Node.ln_Type == TYPE_STRINGLIST)
        {
        KillStringList(&tagnode->SourceLabelList);

        for(editstringnode = (struct EditStringNode *) edittagnode->EditStringList.lh_Head; editstringnode->Node.ln_Succ;
            editstringnode = (struct EditStringNode *) editstringnode->Node.ln_Succ)
          {
          status = TRUE;
          while(!(stringnode = (struct StringNode *) AllocMem(sizeof(struct StringNode), MEMF_CLEAR)) && status)
            status = PutError("Out of memory", "Retry|Abort");

          if(status)
            {
            DEBUGALLOC(sizeof(struct StringNode));

            strcpy(stringnode->String, editstringnode->SourceLabel);
            AddTail(&tagnode->SourceLabelList, (struct Node *) stringnode);
            }
          }
        }
      else
        {
        NewList(&tagnode->SourceLabelList);
        }
      }
    else
      {
      PutError("Error adding\ngadget tag", "OK");
      }
    }

  AddGadgets(windownode);

  DEBUGEXIT(FALSE, NULL);
  }

