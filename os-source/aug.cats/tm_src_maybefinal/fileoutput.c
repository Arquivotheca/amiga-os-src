
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <libraries/asl.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <dos/dos.h>
#include <string.h>
#include <ctype.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/icon_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/iffparse_protos.h>

/****************************************************************************/

BOOL SaveFile(void)
  {
  int    i;
  ULONG  reserved=0;
  ULONG  menucount;
  ULONG  itemcount;
  ULONG  subcount;
  ULONG  gadgetcount;
  ULONG  stringid;
  BOOL   st=TRUE;
  UBYTE  zerobyte=0;
  UBYTE  error=TMFILEERR_NONE;
  UBYTE  color;
  UBYTE  temp;
  char   reservedstring[] = "";
  struct WindowNode    *windownode;
  struct TagNode       *tagnode;
  struct NewGadgetNode *newgadgetnode;
  struct MenuNode      *menunode;
  struct ItemNode      *itemnode;
  struct SubNode       *subnode;
  struct AvailableTags *availabletags;
  struct IFFHandle     *iff;
  struct NewGadget     newgadget;

  DEBUGENTER("SaveFile", NULL);

  if(filename[0] == '\0')
    {
    if(!FileRequest("Save Project", savepathname, savefilename, TRUE)) return(FALSE);
    }

  MainWindowSleep();

  strcpy(fullname, filename);
  strcat(fullname, extension);

  if(iff = AllocIFF())
    {
    if(iff->iff_Stream = (ULONG) fopen(fullname, "wb"))
      {
      InitIFFasStdIO(iff);

      if(OpenIFF(iff, IFFF_WRITE) == 0)
        {
        /****************************************************************/

        st = PutBlock(ID_TMUI, ID_FORM, iff, st); /* FORM: Toolmaker GUI */

        /****************************************************************/

        st = PutBlock(0, ID_THDR, iff, st); /* Chunk: Header */
        strcpy(string, "$VE");
        strcat(string, "R: ");
        strcat(string, title);
        strcat(string, "Project ");
        strcat(string, version);
        strcat(string, " ");
        strcat(string, date);
        st = PutBytes(&zerobyte, sizeof(UBYTE), 1, iff, st); /* For version  */
        st = PutString(string, iff, st);
        st = PutBytes(&reserved, sizeof(ULONG), 1, iff, st); /* Reserved 1 */
        st = PutBytes(&reserved, sizeof(ULONG), 1, iff, st); /* Reserved 2 */
        st = PopBlock(iff, st); /* Chunk THDR */

        /****************************************************************/

        st = PutBlock(0, ID_ANNO, iff, st); /* Chunk: Annotation */
        sprintf(string, "Project file created by Toolmaker (c) %s Commodore Amiga Inc. (c) %s Michael J. Erwin", cbmdate, merdate);
        st = PutString(string, iff, st);
        st = PopBlock(iff, st); /* Chunk ANNO */

        /****************************************************************/

        st = PutBlock(ID_TMSE, ID_FORM, iff, st); /* FORM: Settings */

        st = PutBlock(0, ID_SATB, iff, st);
        st = PutBytes(&autotopborderflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SCOM, iff, st);
        st = PutBytes(&commentflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SPRA, iff, st);
        st = PutBytes(&pragmaflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SICN, iff, st);
        st = PutBytes(&iconflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SGSZ, iff, st);
        st = PutBytes(&gridsize, sizeof(UBYTE), 1, iff, st);
        st = PutBytes(&reserved, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SUSG, iff, st);
        st = PutBytes(&usersignalflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SARX, iff, st);
        st = PutBytes(&arexxflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SCHP, iff, st);
        st = PutBytes(&chipflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SUDA, iff, st);
        st = PutBytes(&userdataflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SFRQ, iff, st);
        st = PutBytes(&aslflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PopBlock(iff, st); /* FORM TSET */

        /****************************************************************/

        st = PutBlock(ID_TSCR, ID_FORM, iff, st); /* FORM: Screen */

        st = PutBlock(0, ID_TSDA, iff, st);
        st = PutBytes(&screennode.DisplayID, sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.Width,     sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.Height,    sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.Depth,     sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.Overscan,  sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.TMSFlags,  sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.Mode,      sizeof(ULONG),            1, iff, st);
        st = PutBytes(&screennode.DClip,     sizeof(struct Rectangle), 1, iff, st);
        st = PutBytes(&stringid,             sizeof(ULONG),            1, iff, st);
        st = PutBytes(&reserved,             sizeof(ULONG),            1, iff, st);
        st = PutString(screennode.Title, iff, st);
        st = PutString(screennode.SourceLabel, iff, st);
        st = PutString(reservedstring, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_CMAP, iff, st);
        for(i=0; i<(1<<screennode.Depth); i++)
          {
          temp = COLOR2RED(screennode.Color[i]);
          color = temp<<4 | temp;
          PutBytes(&color, sizeof(UBYTE), 1, iff, st);

          temp = COLOR2GREEN(screennode.Color[i]);
          color = temp<<4 | temp;
          PutBytes(&color, sizeof(UBYTE), 1, iff, st);

          temp = COLOR2BLUE(screennode.Color[i]);
          color = temp<<4 | temp;
          PutBytes(&color, sizeof(UBYTE), 1, iff, st);
          }
        st = PopBlock(iff, st);

        for(tagnode = (struct TagNode *) screennode.TagList.lh_Head; tagnode->Node.ln_Succ;
            tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
          {
          st = PutTag(AvailableScreenTags[tagnode->Node.ln_Pri].Type, tagnode, iff, st);
          }

        st = PutBlock(ID_TFON, ID_FORM, iff, st); /* FORM: Screen Font */

        st = PutBlock(0, ID_TTAT, iff, st);
        st = PutBytes(&screennode.TextAttr, sizeof(struct TextAttr), 1, iff, st);
        st = PutString(screennode.FontName, iff, st);
        st = PopBlock(iff, st);

        /* FORM TFON will also eventually contain tag chunks */

        st = PopBlock(iff, st); /* FORM: TFON */

        /****************************************************************/

        for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
            windownode = (struct WindowNode *) windownode->Node.ln_Succ)
          {
          st = PutBlock(ID_TWIN, ID_FORM, iff, st); /* FORM: Window */

          st = PutBlock(0, ID_TWDA, iff, st);
          st = PutBytes(&windownode->IDCMP,    sizeof(ULONG), 1, iff, st);
          st = PutBytes(&windownode->TMWFlags, sizeof(ULONG), 1, iff, st);
          st = PutBytes(&stringid,             sizeof(ULONG), 1, iff, st);
          st = PutBytes(&reserved,             sizeof(ULONG), 1, iff, st);
          st = PutString(windownode->Title, iff, st);
          st = PutString(windownode->SourceLabel, iff, st);
          st = PutString(reservedstring, iff, st);
          st = PopBlock(iff, st);

          for(tagnode = (struct TagNode *) windownode->TagList.lh_Head; tagnode->Node.ln_Succ;
              tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
            {
            st = PutTag(AvailableWindowTags[tagnode->Node.ln_Pri].Type, tagnode, iff, st);
            }

          /****************************************************************/

          menucount = 0;
          for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
              menunode = (struct MenuNode *) menunode->Node.ln_Succ)
            {
            st = PutBlock(ID_TMEN, ID_FORM, iff, st); /* FORM: Menu */

            st = PutBlock(0, ID_TMDA, iff, st);
            st = PutBytes(&menucount,           sizeof(ULONG), 1, iff, st);
            st = PutBytes(&menunode->Flags,     sizeof(UWORD), 1, iff, st);
            st = PutBytes(&menunode->TMMFlags,  sizeof(ULONG), 1, iff, st);
            st = PutBytes(&stringid,            sizeof(ULONG), 1, iff, st);
            st = PutBytes(&reserved,            sizeof(ULONG), 1, iff, st);
            st = PutString(menunode->MenuText, iff, st);
            st = PutString(menunode->SourceLabel, iff, st);
            st = PutString(reservedstring, iff, st);
            st = PopBlock(iff, st);

            itemcount = 0;
            for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
                itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
              {
              st = PutBlock(ID_TITE, ID_FORM, iff, st); /* FORM: Item */

              st = PutBlock(0, ID_TIDA, iff, st);
              st = PutBytes(&itemcount,             sizeof(ULONG), 1, iff, st);
              st = PutBytes(&itemnode->Flags,       sizeof(UWORD), 1, iff, st);
              st = PutBytes(&itemnode->TMMFlags,    sizeof(ULONG), 1, iff, st);
              st = PutBytes(&stringid,              sizeof(ULONG), 1, iff, st);
              st = PutBytes(&stringid,              sizeof(ULONG), 1, iff, st);
              st = PutBytes(&reserved,              sizeof(ULONG), 1, iff, st);
              st = PutString(itemnode->ItemText, iff, st);
              st = PutString(itemnode->CommKey, iff, st);
              st = PutString(itemnode->SourceLabel, iff, st);
              st = PutString(reservedstring, iff, st);
              st = PopBlock(iff, st);

              subcount = 0;
              for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                  subnode = (struct SubNode *) subnode->Node.ln_Succ)
                {
                st = PutBlock(0, ID_TSID, iff, st);
                st = PutBytes(&subcount,             sizeof(ULONG), 1, iff, st);
                st = PutBytes(&subnode->Flags,       sizeof(UWORD), 1, iff, st);
                st = PutBytes(&subnode->TMMFlags,    sizeof(ULONG), 1, iff, st);
                st = PutBytes(&stringid,             sizeof(ULONG), 1, iff, st);
                st = PutBytes(&stringid,             sizeof(ULONG), 1, iff, st);
                st = PutBytes(&reserved,             sizeof(ULONG), 1, iff, st);
                st = PutString(subnode->SubText, iff, st);
                st = PutString(subnode->CommKey, iff, st);
                st = PutString(subnode->SourceLabel, iff, st);
                st = PutString(reservedstring, iff, st);
                st = PopBlock(iff, st);

                subcount++;
                }

              st = PopBlock(iff, st); /* FORM TITE */
              itemcount++;
              }

            st = PopBlock(iff, st); /* FORM TMEN */
            menucount++;
            }

          /****************************************************************/

          gadgetcount = 0;
          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            CopyMem(&newgadgetnode->NewGadget, &newgadget, sizeof(struct NewGadget));

            if(autotopborderflag)
              {
              newgadget.ng_TopEdge -= windownode->Window->BorderTop;
              newgadget.ng_LeftEdge -= windownode->Window->BorderLeft;
              }

            st = PutBlock(ID_TGAD, ID_FORM, iff, st); /* FORM: Gadtools gadget */

            st = PutBlock(0, ID_TGDA, iff, st);
            st = PutBytes(&gadgetcount,               sizeof(ULONG), 1, iff, st);
            st = PutBytes(&newgadgetnode->Kind,       sizeof(ULONG), 1, iff, st);
            st = PutBytes(&newgadgetnode->TMGFlags,   sizeof(ULONG), 1, iff, st);
            st = PutBytes(&newgadget,                 sizeof(struct NewGadget), 1, iff, st);
            st = PutBytes(&stringid,                  sizeof(ULONG), 1, iff, st);
            st = PutBytes(&reserved,                  sizeof(ULONG), 1, iff, st);
            st = PutString(newgadgetnode->GadgetText, iff, st);
            st = PutString(newgadgetnode->SourceLabel, iff, st);
            st = PutString(reservedstring, iff, st);
            st = PopBlock(iff, st);

            for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
                tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
              {
              switch(newgadgetnode->Kind)
                {
                case BUTTON_KIND:
                  availabletags = &AvailableBUGadgetTags[0];
                  break;
                case CHECKBOX_KIND:
                  availabletags = &AvailableCBGadgetTags[0];
                  break;
                case CYCLE_KIND:
                  availabletags = &AvailableCYGadgetTags[0];
                  break;
                case INTEGER_KIND:
                  availabletags = &AvailableINGadgetTags[0];
                  break;
                case LISTVIEW_KIND:
                  availabletags = &AvailableLVGadgetTags[0];
                  break;
                case MX_KIND:
                  availabletags = &AvailableMXGadgetTags[0];
                  break;
                case NUMBER_KIND:
                  availabletags = &AvailableNMGadgetTags[0];
                  break;
                case PALETTE_KIND:
                  availabletags = &AvailablePAGadgetTags[0];
                  break;
                case SCROLLER_KIND:
                  availabletags = &AvailableSCGadgetTags[0];
                  break;
                case SLIDER_KIND:
                  availabletags = &AvailableSLGadgetTags[0];
                  break;
                case STRING_KIND:
                  availabletags = &AvailableSTGadgetTags[0];
                  break;
                case TEXT_KIND:
                  availabletags = &AvailableTXGadgetTags[0];
                  break;
                }

              st = PutTag(availabletags[tagnode->Node.ln_Pri].Type, tagnode, iff, st);
              }

            st = PutBlock(ID_TFON, ID_FORM, iff, st); /* FORM: Gadget Font */

            st = PutBlock(0, ID_TTAT, iff, st);
            st = PutBytes(&newgadgetnode->TextAttr, sizeof(struct TextAttr), 1, iff, st);
            st = PutString(newgadgetnode->FontName, iff, st);
            st = PopBlock(iff, st);

            /* FORM TFON will also eventually contain tag chunks */

            st = PopBlock(iff, st);  /* FORM TFON */

            st = PopBlock(iff, st);  /* FORM TGAD */
            gadgetcount++;
            }

          st = PopBlock(iff, st); /* FORM TWND */
          }

        st = PopBlock(iff, st); /* FORM TSCR */

        st = PopBlock(iff, st); /* FORM TMUI */

        CloseIFF(iff);

        st = UpdateIcon(fullname, "tm", st);
        }
      else
        {
        error = TMFILEERR_OPEN;
        }

      fclose((FILE *) iff->iff_Stream);
      }
    else
      {
      error = TMFILEERR_OPEN;
      }

    FreeIFF(iff);

    MainWindowWakeUp();
    }
  else
    {
    error = TMFILEERR_OUTOFMEM;
    }

  if(st == FALSE) error = TMFILEERR_WRITE;

  if(error)
    {
    DeleteFile(fullname);
    DeleteIcon(fullname);

    switch(error)
      {
      case TMFILEERR_OPEN:
        PutError("Error opening\nproject file\nfor output", "OK");
        break;
      case TMFILEERR_OUTOFMEM:
        PutError("Out of memory", "OK");
        break;
      case TMFILEERR_WRITE:
        PutError("Error writing\nproject file", "OK");
        break;
      }
    }
  else
    {
    modified = FALSE;
    }

  DEBUGEXIT(TRUE,!error);
  return(!error);
  }

/****************************************************************************/

BOOL PutTag(ULONG type, struct TagNode *tagnode, struct IFFHandle *iff, BOOL st)
  {
  ULONG  stringid;
  ULONG  numlabels;
  ULONG  reserved=0;
  WORD   *wordpointer;
  char   **stringpointer;
  struct StringNode *stringnode;
  int    i;

  DEBUGENTER("PutTag", NULL);

  switch(type)
    {
    /****************************************************************/

    case TYPE_IGNORE:
      st = PutBlock(0, ID_TNIN, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);
      st = PutBytes(&reserved,                 sizeof(ULONG), 1, iff, st);
      st = PutString(tagnode->Data, iff, st);
      st = PopBlock(iff, st);
      break;

    /****************************************************************/

    case TYPE_CHARACTER:
      st = PutBlock(0, ID_TCHA, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Data, sizeof(ULONG), 1, iff, st);
      st = PutBytes(&reserved,                 sizeof(ULONG), 1, iff, st);
      st = PopBlock(iff, st);
      break;

    /****************************************************************/

    case TYPE_INTEGER:
      st = PutBlock(0, ID_TINT, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Data, sizeof(ULONG), 1, iff, st);
      st = PutBytes(&reserved,                 sizeof(ULONG), 1, iff, st);
      st = PutString(tagnode->Data, iff, st);
      st = PopBlock(iff, st);
      break;

    /****************************************************************/

    case TYPE_WORDLIST:
      st = PutBlock(0, ID_TWLS, iff, st);
      st = PutBytes(&tagnode->DataCount,       sizeof(SHORT), 1, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);
      st = PutBytes(&reserved,                 sizeof(ULONG), 1, iff, st);

      wordpointer = (WORD *) tagnode->Data;
      for(i=0; i<tagnode->DataCount; i++)
        {
        st = PutBytes(wordpointer, sizeof(WORD), 1, iff, st);
        wordpointer++;
        }

      st = PopBlock(iff, st);
      break;

    /****************************************************************/

    case TYPE_STRING:
      st = PutBlock(0, ID_TSTR, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);
      st = PutBytes(&stringid,                 sizeof(ULONG), 1, iff, st);
      st = PutBytes(&reserved,                 sizeof(ULONG), 1, iff, st);
      st = PutString(tagnode->Data, iff, st);
      st = PopBlock(iff, st);
      break;

    /****************************************************************/

    case TYPE_STRINGLIST:
      st = PutBlock(0, ID_TSLS, iff, st);
      st = PutBytes(&tagnode->DataCount,       sizeof(SHORT), 1, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);

      for(i=0; i<tagnode->DataCount; i++)
        {
        st = PutBytes(&stringid, sizeof(ULONG), 1, iff, st);
        }

      for(i=0; i<tagnode->DataCount; i++)
        {
        st = PutBytes(&reserved, sizeof(ULONG), 1, iff, st);
        }

      stringpointer = (char **) tagnode->Data;
      for(i=0; i<tagnode->DataCount; i++)
        {
        st = PutString(*stringpointer, iff, st);
        stringpointer++;
        }

      numlabels = 0;
      for(stringnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head; stringnode->Node.ln_Succ;
          stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
        {
        st = PutString(stringnode->String, iff, st);
        numlabels++;
        }

      for(i=0; i<tagnode->DataCount-numlabels; i++)
        {
        st = PutString("", iff, st);
        }

      st = PopBlock(iff, st);
      break;

    /****************************************************************/

    case TYPE_LINKEDLIST:
      st = PutBlock(0, ID_TLLS, iff, st);
      st = PutBytes(&tagnode->DataCount,       sizeof(SHORT), 1, iff, st);
      st = PutBytes(&tagnode->TagItem.ti_Tag,  sizeof(ULONG), 1, iff, st);

      for(i=0; i<tagnode->DataCount; i++)
        {
        st = PutBytes(&stringid, sizeof(ULONG), 1, iff, st);
        }

      for(i=0; i<tagnode->DataCount; i++)
        {
        st = PutBytes(&reserved, sizeof(ULONG), 1, iff, st);
        }

      for(stringnode = (struct StringNode *) ((struct List *) tagnode->Data)->lh_Head; stringnode->Node.ln_Succ;
          stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
        {
        st = PutString(stringnode->String, iff, st);
        }

      numlabels = 0;
      for(stringnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head; stringnode->Node.ln_Succ;
          stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
        {
        st = PutString(stringnode->String, iff, st);
        numlabels++;
        }

      for(i=0; i<tagnode->DataCount-numlabels; i++)
        {
        st = PutString("", iff, st);
        }

      st = PopBlock(iff, st);
      break;
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

BOOL SaveSettingsFile(void)
  {
  ULONG  reserved=0;
  BOOL st=TRUE;
  UBYTE error=TMFILEERR_NONE;
  struct IFFHandle *iff;

  DEBUGENTER("SaveSettingsFile", NULL);

  MainWindowSleep();

  if(iff = AllocIFF())
    {
    if(iff->iff_Stream = (ULONG) fopen(settingsfilename, "wb"))
      {
      InitIFFasStdIO(iff);

      if(OpenIFF(iff, IFFF_WRITE) == 0)
        {
        st = PutBlock(ID_TMSE, ID_FORM, iff, st); /* FORM: Settings */

        st = PutBlock(0, ID_THDR, iff, st); /* Chunk: Header */
        strcpy(string, "$VE");
        strcat(string, "R: ");
        strcat(string, title);
        strcat(string, "Settings ");
        strcat(string, version);
        strcat(string, " ");
        strcat(string, date);
        st = PutBytes(&reserved, sizeof(UBYTE), 1, iff, st); /* For version */
        st = PutString(string, iff, st);
        st = PutBytes(&reserved, sizeof(ULONG), 1, iff, st); /* Reserved 1 */
        st = PutBytes(&reserved, sizeof(ULONG), 1, iff, st); /* Reserved 2 */
        st = PopBlock(iff, st); /* Chunk: THDR */

        st = PutBlock(0, ID_ANNO, iff, st); /* Chunk: Annotation */
        sprintf(string, "Settings file created by Toolmaker (c) %s Commodore Amiga Inc. (c) %s Michael J. Erwin", cbmdate, merdate);
        st = PutString(string, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SATB, iff, st);
        st = PutBytes(&autotopborderflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SCOM, iff, st);
        st = PutBytes(&commentflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SPRA, iff, st);
        st = PutBytes(&pragmaflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SICN, iff, st);
        st = PutBytes(&iconflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SGSZ, iff, st);
        st = PutBytes(&gridsize, sizeof(UBYTE), 1, iff, st);
        st = PutBytes(&reserved, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SUSG, iff, st);
        st = PutBytes(&usersignalflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SARX, iff, st);
        st = PutBytes(&arexxflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SCHP, iff, st);
        st = PutBytes(&chipflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SUDA, iff, st);
        st = PutBytes(&userdataflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PutBlock(0, ID_SFRQ, iff, st);
        st = PutBytes(&aslflag, sizeof(UBYTE), 1, iff, st);
        st = PopBlock(iff, st);

        st = PopBlock(iff, st); /* FORM TMSE */

        CloseIFF(iff);
        }
      else
        {
        error = TMFILEERR_OPEN;
        }

      fclose((FILE *) iff->iff_Stream);
      }
    else
      {
      error = TMFILEERR_OPEN;
      }

    FreeIFF(iff);

    MainWindowWakeUp();
    }
  else
    {
    error = TMFILEERR_OUTOFMEM;
    }

  if(st == FALSE) error = TMFILEERR_WRITE;

  if(error)
    {
    switch(error)
      {
      case TMFILEERR_OPEN:
        PutError("Error opening\nsettings file\nfor output", "OK");
        break;
      case TMFILEERR_OUTOFMEM:
        PutError("Out of memory", "OK");
        break;
      case TMFILEERR_WRITE:
        PutError("Error writing\nsettings file", "OK");
        break;
      }
    }

  DEBUGEXIT(TRUE,!error);
  return(!error);
  }

/****************************************************************************/

BOOL UpdateIcon(char *name, char *type, BOOL st)
  {
  struct DiskObject *diskobject;
  struct DiskObject diskobject2;
  int               freedo = FALSE;

  DEBUGENTER("UpdateIcon", NULL);

  if(iconflag && st)
    {
    if(diskobject = GetDiskObject(name))
      {
      freedo = TRUE;
      }
    else
      {
      sprintf(string, "Env:%s/def_%s", title, type);
      if(diskobject = GetDiskObject(string))
        {
        diskobject->do_CurrentX = NO_ICON_POSITION;
        diskobject->do_CurrentY = NO_ICON_POSITION;
        st = PutDiskObject(name, diskobject);
        freedo = TRUE;
        }
      else
        {
        sprintf(string, "Env:Sys/def_%s", type);
        if(diskobject = GetDiskObject(string))
          {
          diskobject->do_CurrentX = NO_ICON_POSITION;
          diskobject->do_CurrentY = NO_ICON_POSITION;
          st = PutDiskObject(name, diskobject);
          freedo = TRUE;
          }
        else
          {
          diskobject = GetDefDiskObject(WBPROJECT);

          if(!strcmp(type, "tm"))               /* if project */
            sprintf(defaulttool, "%s", title);
          else
            strcpy(defaulttool, "Ed");

          CopyMem(diskobject, &diskobject2, sizeof(struct DiskObject));
          diskobject2.do_DefaultTool = defaulttool;
          st = PutDiskObject(name, &diskobject2);
          }
        }
      }

    if(freedo) FreeDiskObject(diskobject);
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

BOOL WriteExeIcon(void)
  {
  struct DiskObject *diskobject;
  BOOL st = TRUE;

  DEBUGENTER("WriteExeIcon", NULL);

  if(iconflag && st)
    {
    if(diskobject = GetDiskObject(filename))
      {
      FreeDiskObject(diskobject);
      }
    else
      {
      if(diskobject = GetDefDiskObject(WBTOOL))
        {
        st = PutDiskObject(filename, diskobject);
        }
      }
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

void DeleteIcon(char *name)
  {
  DEBUGENTER("DeleteIcon", NULL);

  if(iconflag) DeleteDiskObject(name);

  DEBUGEXIT(FALSE,NULL);
  }

/****************************************************************************/

BOOL PutBlock(ULONG type, ULONG id, struct IFFHandle *iff, BOOL st)
  {
  DEBUGENTER("PutBlock", NULL);

  if(st)
    {
    if(PushChunk(iff, type, id, IFFSIZE_UNKNOWN) != 0) st = FALSE;
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

BOOL PopBlock(struct IFFHandle *iff, BOOL st)
  {
  DEBUGENTER("PopBlock", NULL);

  if(st)
    {
    if(PopChunk(iff) != 0) st = FALSE;
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

BOOL PutBytes(char *address, int size, int count, struct IFFHandle *iff, BOOL st)
  {
  LONG totalsize;

  DEBUGENTER("PutBytes", size);

  if(st)
    {
    totalsize = size*count;
    if(WriteChunkBytes(iff, address, totalsize) != totalsize) st = FALSE;
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

BOOL PutString(char *str, struct IFFHandle *iff, BOOL st)
  {
  LONG length;

  DEBUGENTER("PutString", NULL);

  if(st)
    {
    length = strlen(str);
    if(WriteChunkBytes(iff, str, length) != length) st = FALSE;
    if(st && WriteChunkBytes(iff, "\n", 1) != 1) st = FALSE;
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

#endif

