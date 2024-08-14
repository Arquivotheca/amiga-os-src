
#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/asl_protos.h>

/****************************************************************************/

int EditGadgetFont(struct NewGadgetNode *newgadgetnode)
  {
  int fontflag=FALSE;
  struct FontRequester *fo;

  DEBUGENTER("EditGadgetFont", NULL);

  if(currentwindow)
    {
    if(!(fo = AllocAslRequestTags(ASL_FontRequest, TAG_DONE)))
      {
      PutError("Out of memory", "OK");
      }
    else
      {
      MainWindowSleep();

      fontflag = (int) AslRequestTags(fo, ASL_Hail, "Gadget Font",
                                          ASL_Window, W_Main,
                                          ASL_FontName, newgadgetnode->TextAttr.ta_Name,
                                          ASL_FontHeight, newgadgetnode->TextAttr.ta_YSize,
                                          ASL_FontStyles, newgadgetnode->TextAttr.ta_Style,
                                          ASL_FontFlags, newgadgetnode->TextAttr.ta_Flags,
                                          ASL_FuncFlags, FONF_STYLES,
                                          TAG_DONE);

      if(fontflag)
        {
        RemoveGadgets(currentwindow);

        strcpy(newgadgetnode->FontName, fo->fo_Attr.ta_Name);
        newgadgetnode->TextAttr.ta_YSize = fo->fo_Attr.ta_YSize;
        newgadgetnode->TextAttr.ta_Style = fo->fo_Attr.ta_Style;
        newgadgetnode->TextAttr.ta_Flags = fo->fo_Attr.ta_Flags;

        switch(newgadgetnode->Kind)
          {
          case CHECKBOX_KIND:
            newgadgetnode->NewGadget.ng_Height = newgadgetnode->TextAttr.ta_YSize + 3;
            break;

          case MX_KIND:
            newgadgetnode->NewGadget.ng_Height = newgadgetnode->TextAttr.ta_YSize + 1;
            break;
          }

        AddGadgets(currentwindow);

        modified = TRUE;
        }

      FreeAslRequest(fo);

      MainWindowWakeUp();
      }
    }

  DEBUGEXIT(TRUE, fontflag);
  return(fontflag);
  }

/****************************************************************************/

int EditScreenFont(struct ScreenNode *screennode)
  {
  int fontflag=FALSE;
  struct FontRequester *fo;

  DEBUGENTER("EditScreenFont", NULL);

  if(!(fo = AllocAslRequestTags(ASL_FontRequest, TAG_DONE)))
    {
    PutError("Out of memory", "OK");
    }
  else
    {
    MainWindowSleep();

    fontflag = (int) AslRequestTags(fo, ASL_Hail, "Screen Font",
                                        ASL_Window, W_Main,
                                        ASL_FontName, screennode->TextAttr.ta_Name,
                                        ASL_FontHeight, screennode->TextAttr.ta_YSize,
                                        ASL_FontStyles, screennode->TextAttr.ta_Style,
                                        ASL_FontFlags, screennode->TextAttr.ta_Flags,
                                        ASL_FuncFlags, FONF_STYLES,
                                        TAG_DONE);

    if(fontflag)
      {
      strcpy(screennode->FontName, fo->fo_Attr.ta_Name);
      screennode->TextAttr.ta_YSize = fo->fo_Attr.ta_YSize;
      screennode->TextAttr.ta_Style = fo->fo_Attr.ta_Style;
      screennode->TextAttr.ta_Flags = fo->fo_Attr.ta_Flags;

      TagWindows(COMMAND_REMOVE);
      TagMainWindow(COMMAND_REMOVE);
      TagScreen(COMMAND_CHANGE);

      modified = TRUE;
      }

    FreeAslRequest(fo);

    MainWindowWakeUp();
    }

  DEBUGEXIT(TRUE, fontflag);
  return(fontflag);
  }

