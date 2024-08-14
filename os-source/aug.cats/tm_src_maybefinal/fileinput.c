
#include "Toolmaker.h"
#include "Externs.h"

#include <exec/memory.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <dos/dosasl.h>
#include <dos/dos.h>
#include <string.h>
#include <ctype.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/asl_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/diskfont_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/utility_protos.h>

/****************************************************************************/

BOOL OpenFile(void)
  {
  BOOL retval = FALSE;

  DEBUGENTER("OpenFile",NULL);

  if(RequestKill())
    {
    if(FileRequest("Open Project", savepathname, savefilename, FALSE))
      {
      openflag = TRUE;

      TagWindows(COMMAND_KILL);
      TagMainWindow(COMMAND_REMOVE);
      TagScreen(COMMAND_CHANGE);

      if(W_Main) ActivateWindow(W_Main);

      retval = TRUE;
      }
    }

  DEBUGEXIT(TRUE,retval);

  return(retval);
  }

/****************************************************************************/

BOOL ActuallyOpenFile(void)
  {
  static char dummystring[STRINGSIZE];
  static char fontname[STRINGSIZE];
  static struct TextAttr textattr;
  BOOL   retval=TRUE;
  BOOL   st=TRUE;
  BOOL   found;
  ULONG  dummy;
  ULONG  reserved=0;
  ULONG  screencount=0;
  ULONG  tag;
  ULONG  data;
  LONG   ifferror;
  LONG   i;
  SHORT  datacount;
  WORD   *wordarray;
  WORD   *wordpointer;
  UBYTE  tagtype;
  UBYTE  error=TMFILEERR_NONE;
  UBYTE  red;
  UBYTE  green;
  UBYTE  blue;
  char   **stringpointer;
  struct StringData    stringdata = {0, NULL};
  struct StringNode    *stringnode;
  struct WindowNode    *windownode;
  struct WindowNode    tempwindownode;
  struct NewGadgetNode *newgadgetnode;
  struct NewGadgetNode tempnewgadgetnode;
  struct MenuNode      *menunode;
  struct MenuNode      tempmenunode;
  struct ItemNode      *itemnode;
  struct ItemNode      tempitemnode;
  struct SubNode       *subnode;
  struct SubNode       tempsubnode;
  struct TagNode       *tagnode;
  struct FontNode      *fontnode;
  struct Node          *node;
  struct IFFHandle     *iff;
  struct ContextNode   *chunk;

  DEBUGENTER("ActuallyOpenFile",NULL);

  sprintf(fullname, "%s%s", filename, extension);

  if(iff = AllocIFF())
    {
    if(iff->iff_Stream = (ULONG) fopen(fullname, "rb"))
      {
      InitIFFasStdIO(iff);

      if(OpenIFF(iff, IFFF_READ) == 0)
        {
        realgadgets = FALSE;
        realmenus = FALSE;
        windowcount = 0;

        if(StopChunk(iff, ID_TMUI, ID_FORM) == 0)
          {
          if(ifferror = ParseIFF(iff, IFFPARSE_SCAN) == 0)
            {
            while((((ifferror = ParseIFF(iff, IFFPARSE_RAWSTEP)) == 0) || (ifferror == IFFERR_EOC)) && (ifferror != IFFERR_EOF) && st && screencount<2)
              {
              if((chunk = CurrentChunk(iff)) && (ifferror != IFFERR_EOC))
                {
                switch(chunk->cn_ID)
                  {
                  /**************************************************************/

                  case ID_SCOM:
                    st = GetBytes(&commentflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SATB:
                    st = GetBytes(&autotopborderflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SPRA:
                    st = GetBytes(&pragmaflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SGSZ:
                    st = GetBytes(&gridsize,   sizeof(UBYTE), 1, iff, st);
                    st = GetBytes(&reserved,   sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SICN:
                    st = GetBytes(&iconflag,   sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SUSG:
                    st = GetBytes(&usersignalflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SARX:
                    st = GetBytes(&arexxflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SCHP:
                    st = GetBytes(&chipflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SUDA:
                    st = GetBytes(&userdataflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SFRQ:
                    st = GetBytes(&aslflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_TSDA:
                    if(screencount++ == 0)
                      {
                      st = GetBytes(&screennode.DisplayID, sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.Width,     sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.Height,    sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.Depth,     sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.Overscan,  sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.TMSFlags,  sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.Mode,      sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&screennode.DClip,     sizeof(struct Rectangle), 1, iff, st);
                      st = GetBytes(&dummy,                sizeof(ULONG),            1, iff, st);
                      st = GetBytes(&reserved,             sizeof(ULONG),            1, iff, st);
                      st = GetString(screennode.Title,       STRINGSIZE, iff, st);
                      st = GetString(screennode.SourceLabel, STRINGSIZE, iff, st);
                      st = GetString(dummystring,            STRINGSIZE, iff, st);

                      if(st)
                        {
                        if(screennode.DisplayID == INVALID_ID)
                          screennode.Workbench = TRUE;
                        else
                          screennode.Workbench = FALSE;

                        screennode.TextAttr.ta_Name = screennode.FontName;
                        strcpy(screennode.FontName, "");

                        KillTagList(&screennode.TagList);
                        screennode.TagCount = 0;

                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Title, (ULONG) screennode.Title);
                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_DisplayID, screennode.DisplayID);
                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Width, screennode.Width);
                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Height, screennode.Height);
                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Depth, screennode.Depth);
                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Overscan, screennode.Overscan);
                        AddTag(TYPE_SCREEN, (struct Node *) &screennode, SA_Font, (ULONG) &screennode.TextAttr);

                        node = (struct Node *) &screennode;  /* For next tags */

                        TagScreen(STATUS_REMOVED);
                        }
                      else
                        {
                        node = NULL;
                        error = TMFILEERR_READ;
                        }
                      }

                    tagtype = TYPE_SCREEN;
                    break;

                  /**************************************************************/

                  case ID_CMAP:
                    dummy = MIN(32, chunk->cn_Size/3);
                    for(i=0; i<dummy; i++)
                      {
                      st = GetBytes(&red,   sizeof(UBYTE), 1, iff, st);
                      st = GetBytes(&green, sizeof(UBYTE), 1, iff, st);
                      st = GetBytes(&blue,  sizeof(UBYTE), 1, iff, st);
                      screennode.Color[i] = RGB2COLOR(red>>4, green>>4, blue>>4);
                      }
                    break;

                  /**************************************************************/

                  case ID_TWDA:
                    st = GetBytes(&tempwindownode.IDCMP,    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempwindownode.TMWFlags, sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,                   sizeof(ULONG),            1, iff, st);
                    st = GetBytes(&reserved,                sizeof(ULONG),            1, iff, st);
                    st = GetString(tempwindownode.Title,       STRINGSIZE, iff, st);
                    st = GetString(tempwindownode.SourceLabel, STRINGSIZE, iff, st);
                    st = GetString(dummystring,                STRINGSIZE, iff, st);

                    if(st)
                      {
                      if(windownode = (struct WindowNode *) AllocMem(sizeof(struct WindowNode), MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(struct WindowNode));

                        NewList(&windownode->NewGadgetList);
                        NewList(&windownode->GadgetList);
                        NewList(&windownode->MenuList);
                        NewList(&windownode->TagList);
                        windownode->TagCount = 0;

                        windownode->IDCMP = tempwindownode.IDCMP;
                        windownode->TMWFlags = tempwindownode.TMWFlags | WINDOWFLAG_DISABLED;
                        strcpy(windownode->Title, tempwindownode.Title);
                        strcpy(windownode->SourceLabel, tempwindownode.SourceLabel);

                        if(windownode->Title[0] != '\0')
                          {
                          AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_Title, (ULONG) windownode->Title);
                          }
                        AddTag(TYPE_WINDOW, (struct Node *) windownode, WA_IDCMP, arrayidcmp);

                        node = (struct Node *) windownode;  /* For next tags */

                        AddTail(&WindowList, node);
                        windowcount++;
                        TagWindow(windownode, STATUS_REMOVED);
                        }
                      else
                        {
                        node = NULL;
                        error = TMFILEERR_OUTOFMEM;  /* Allocate windownode memory fail */
                        }
                      }

                    tagtype = TYPE_WINDOW;
                    break;

                  /**************************************************************/

                  case ID_TMDA:
                    st = GetBytes(&dummy,                    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempmenunode.Flags,       sizeof(UWORD), 1, iff, st);
                    st = GetBytes(&tempmenunode.TMMFlags,    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,                    sizeof(ULONG),            1, iff, st);
                    st = GetBytes(&reserved,                 sizeof(ULONG),            1, iff, st);
                    st = GetString(tempmenunode.MenuText,    STRINGSIZE, iff, st);
                    st = GetString(tempmenunode.SourceLabel, STRINGSIZE, iff, st);
                    st = GetString(dummystring,              STRINGSIZE, iff, st);

                    if(st && windownode)
                      {
                      if(menunode = (struct MenuNode *) AllocMem(sizeof(struct MenuNode), MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(struct MenuNode));

                        menunode->Node.ln_Name = menunode->MenuText;
                        NewList(&menunode->ItemList);
                        menunode->Flags = tempmenunode.Flags;
                        menunode->TMMFlags = tempmenunode.TMMFlags;
                        menunode->ItemCount = 0;
                        strcpy(menunode->MenuText, tempmenunode.MenuText);
                        strcpy(menunode->SourceLabel, tempmenunode.SourceLabel);

                        AddTail(&windownode->MenuList, (struct Node *) menunode);
                        }
                      else
                        {
                        error = TMFILEERR_OUTOFMEM;  /* Allocate menunode memory fail */
                        }
                      }
                    break;

                  /**************************************************************/

                  case ID_TIDA:
                    st = GetBytes(&dummy,                    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempitemnode.Flags,       sizeof(UWORD), 1, iff, st);
                    st = GetBytes(&tempitemnode.TMMFlags,    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,                    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,                    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved,                 sizeof(ULONG), 1, iff, st);
                    st = GetString(tempitemnode.ItemText,    STRINGSIZE, iff, st);
                    st = GetString(tempitemnode.CommKey,     2,          iff, st);
                    st = GetString(tempitemnode.SourceLabel, STRINGSIZE, iff, st);
                    st = GetString(dummystring,              STRINGSIZE, iff, st);

                    if(st && windownode)
                      {
                      if(itemnode = (struct ItemNode *) AllocMem(sizeof(struct ItemNode), MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(struct ItemNode));

                        itemnode->Node.ln_Name = itemnode->ItemText;
                        NewList(&itemnode->SubList);
                        itemnode->Flags       = tempitemnode.Flags;
                        itemnode->TMMFlags    = tempitemnode.TMMFlags;
                        itemnode->SubCount    = 0;
                        strcpy(itemnode->ItemText,    tempitemnode.ItemText);
                        strcpy(itemnode->CommKey,     tempitemnode.CommKey);
                        strcpy(itemnode->SourceLabel, tempitemnode.SourceLabel);

                        AddTail(&menunode->ItemList, (struct Node *) itemnode);

                        if(menunode) menunode->ItemCount++;
                        }
                      else
                        {
                        error = TMFILEERR_OUTOFMEM;  /* Allocate itemnode memory fail */
                        }
                      }
                    break;

                  /**************************************************************/

                  case ID_TSID:
                    st = GetBytes(&dummy,                   sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempsubnode.Flags,       sizeof(UWORD), 1, iff, st);
                    st = GetBytes(&tempsubnode.TMMFlags,    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,                   sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,                   sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved,                sizeof(ULONG), 1, iff, st);
                    st = GetString(tempsubnode.SubText,     STRINGSIZE, iff, st);
                    st = GetString(tempsubnode.CommKey,     2,          iff, st);
                    st = GetString(tempsubnode.SourceLabel, STRINGSIZE, iff, st);
                    st = GetString(dummystring,             STRINGSIZE, iff, st);

                    if(st && windownode)
                      {
                      if(subnode = (struct SubNode *) AllocMem(sizeof(struct SubNode), MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(struct SubNode));

                        subnode->Node.ln_Name = subnode->SubText;
                        subnode->Flags       = tempsubnode.Flags;
                        subnode->TMMFlags    = tempsubnode.TMMFlags;
                        strcpy(subnode->SubText,     tempsubnode.SubText);
                        strcpy(subnode->CommKey,     tempsubnode.CommKey);
                        strcpy(subnode->SourceLabel, tempsubnode.SourceLabel);

                        AddTail(&itemnode->SubList, (struct Node *) subnode);

                        if(itemnode) itemnode->SubCount++;
                        }
                      else
                        {
                        error = TMFILEERR_OUTOFMEM;  /* Allocate subnode memory fail */
                        }
                      }
                    break;

                  /**************************************************************/

                  case ID_TGDA:
                    st = GetBytes(&dummy,                       sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempnewgadgetnode.Kind,      sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempnewgadgetnode.TMGFlags,  sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&tempnewgadgetnode.NewGadget, sizeof(struct NewGadget), 1, iff, st);
                    st = GetBytes(&dummy,                       sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved,                    sizeof(ULONG), 1, iff, st);
                    st = GetString(tempnewgadgetnode.GadgetText,  STRINGSIZE, iff, st);
                    st = GetString(tempnewgadgetnode.SourceLabel, STRINGSIZE, iff, st);
                    st = GetString(dummystring,                   STRINGSIZE, iff, st);

                    if(st && windownode)
                      {
                      if(newgadgetnode = (struct NewGadgetNode *) AllocMem(sizeof(struct NewGadgetNode), MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(struct NewGadgetNode));

                        NewList(&newgadgetnode->TagList);
                        newgadgetnode->TagCount = 0;
                        newgadgetnode->Kind = tempnewgadgetnode.Kind;

                        CopyMem(&tempnewgadgetnode.NewGadget, &newgadgetnode->NewGadget, sizeof(struct NewGadget));

                        strcpy(newgadgetnode->GadgetText,  tempnewgadgetnode.GadgetText);
                        strcpy(newgadgetnode->SourceLabel, tempnewgadgetnode.SourceLabel);
                        strcpy(newgadgetnode->FontName, "");
                        newgadgetnode->TextAttr.ta_Name = newgadgetnode->FontName;
                        newgadgetnode->NewGadget.ng_TextAttr = &newgadgetnode->TextAttr;
                        newgadgetnode->NewGadget.ng_GadgetText = newgadgetnode->GadgetText;
                        newgadgetnode->NewGadget.ng_VisualInfo = VisualInfo;

                        node = (struct Node *) newgadgetnode;  /* For next tags */

                        AddTail(&windownode->NewGadgetList, node);
                        }
                      else
                        {
                        node = NULL;
                        error = TMFILEERR_OUTOFMEM;  /* Allocate newgadgetnode memory fail */
                        }
                      }

                    tagtype = TYPE_GADGET;
                    break;

                  /**************************************************************/

                  case ID_TTAT:
                    st = GetBytes(&textattr, sizeof(struct TextAttr),  1, iff, st);
                    st = GetString(fontname, STRINGSIZE, iff, st);

                    textattr.ta_Name = fontname;

                    if(fontname[0] == '\0' ||
                       fontname[0] == '.')
                      {
                      fontname[0] = '\0';
                      textattr.ta_YSize = 8;
                      textattr.ta_Style = FS_NORMAL;
                      textattr.ta_Flags = FPF_ROMFONT;
                      }
                    else
                      {
                      /* Check if font already loaded */

                      found = FALSE;
                      for(fontnode = (struct FontNode *) FontList.lh_Head; fontnode->Node.ln_Succ;
                          fontnode = (struct FontNode *) fontnode->Node.ln_Succ)
                        {
                        sprintf(string, "%s%d", fontname, textattr.ta_YSize);
                        if(!strcmp(fontnode->FontName, string))
                          {
                          found = TRUE;
                          }
                        }

                      if(!found)
                        {
                        if(fontnode = (struct FontNode *) AllocMem(sizeof(struct FontNode), MEMF_CLEAR))
                          {

                          DEBUGALLOC(sizeof(struct FontNode));

                          sprintf(fontnode->FontName, "%s%d", fontname, textattr.ta_YSize);

                          if(!(fontnode->TextFont = OpenDiskFont(&textattr)))
                            {
                            sprintf(string, "Font not found:\n%s %d", fontname, textattr.ta_YSize);
                            PutError(string, "OK");
                            }

                          AddTail(&FontList, (struct Node *) fontnode);
                          }
                        else
                          {
                          error = TMFILEERR_OUTOFMEM;  /* Allocate fontnode memory fail */
                          }
                        }
                      }

                    if(st && node)
                      {
                      if(tagtype == TYPE_SCREEN)
                        {
                        screennode.TextAttr.ta_YSize = textattr.ta_YSize;
                        screennode.TextAttr.ta_Style = textattr.ta_Style;
                        screennode.TextAttr.ta_Flags = textattr.ta_Flags;
                        strcpy(screennode.FontName, fontname);
                        }
                      else if(tagtype == TYPE_GADGET)
                        {
                        newgadgetnode->TextAttr.ta_YSize = textattr.ta_YSize;
                        newgadgetnode->TextAttr.ta_Style = textattr.ta_Style;
                        newgadgetnode->TextAttr.ta_Flags = textattr.ta_Flags;
                        strcpy(newgadgetnode->FontName, fontname);
                        }
                      }

                    break;

                  /**************************************************************/

                  case ID_TCHA:
                    st = GetBytes(&tag,      sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&data,     sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved, sizeof(ULONG), 1, iff, st);

                    if(st && node)
                      {
                      AddTag(tagtype, node, tag, data);
                      }
                    break;

                  /**************************************************************/

                  case ID_TINT:
                    st = GetBytes(&tag,      sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&data,     sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved, sizeof(ULONG), 1, iff, st);
                    st = GetString(string, STRINGSIZE, iff, st);

                    if(st && node)
                      {
                      if(string[0] == '\0') sprintf(string, "%d", data);
                      AddTag(tagtype, node, tag, (ULONG) string);
                      }
                    break;

                  /**************************************************************/

                  case ID_TNIN:
                    st = GetBytes(&tag,      sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved, sizeof(ULONG), 1, iff, st);
                    st = GetString(string, STRINGSIZE, iff, st);

                    if(st && node)
                      {
                      AddTag(tagtype, node, tag, (ULONG) string);
                      }
                    break;

                  /**************************************************************/

                  case ID_TSTR:
                    st = GetBytes(&tag,      sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&dummy,    sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved, sizeof(ULONG), 1, iff, st);
                    st = GetString(string, STRINGSIZE, iff, st);

                    if(st && node)
                      {
                      AddTag(tagtype, node, tag, (ULONG) string);
                      }
                    break;

                  /**************************************************************/

                  case ID_TSLS:
                  case ID_TLLS:
                    st = GetBytes(&datacount, sizeof(SHORT), 1, iff, st);
                    st = GetBytes(&tag,       sizeof(ULONG), 1, iff, st);

                    if(st && node)
                      {
                      stringdata.StringCount = datacount;

                      for(i=0; i<datacount; i++) /* Read string ids */
                        {
                        st = GetBytes(&dummy, sizeof(ULONG), 1, iff, st);
                        }

                      for(i=0; i<datacount; i++) /* Read string ids */
                        {
                        st = GetBytes(&reserved, sizeof(ULONG), 1, iff, st);
                        }

                      /* Allocate pointer memory */

                      if(stringdata.StringArray = (char **) AllocMem(sizeof(char *) * stringdata.StringCount, MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(char *)*stringdata.StringCount);

                        /* Allocate string memory and get strings */

                        stringpointer = stringdata.StringArray;
                        for(i=0; i<stringdata.StringCount; i++)
                          {
                          if(*stringpointer = (char *) AllocMem(STRINGSIZE, MEMF_CLEAR))
                            {
                            DEBUGALLOC(STRINGSIZE);

                            st = GetString(*stringpointer, STRINGSIZE, iff, st);
                            stringpointer++;
                            }
                          else
                            {
                            st = GetString(dummystring, STRINGSIZE, iff, st);
                            error = TMFILEERR_OUTOFMEM;  /* Allocate string memory fail */
                            }
                          }

                        tagnode = AddTag(tagtype, node, tag, (ULONG) &stringdata);

                        /* Free string memory */

                        stringpointer = stringdata.StringArray;
                        for(i=0; i<stringdata.StringCount; i++)
                          {
                          if(*stringpointer)
                            {
                            DEBUGFREE(STRINGSIZE);
                            FreeMem(*stringpointer, STRINGSIZE);
                            }
                          stringpointer++;
                          }

                        /* Free pointer memory */

                        DEBUGFREE(sizeof(char *)*stringdata.StringCount);
                        FreeMem(stringdata.StringArray, sizeof(char *) * stringdata.StringCount);

                        /* Get label strings */

                        if(tagnode)
                          {
                          for(i=0; i<stringdata.StringCount; i++)
                            {
                            if(stringnode = (struct StringNode *) AllocMem(sizeof(struct StringNode), MEMF_CLEAR))
                              {
                              DEBUGALLOC(sizeof(struct StringNode));

                              st = GetString(stringnode->String, STRINGSIZE, iff, st);
                              AddTail(&tagnode->SourceLabelList, (struct Node *) stringnode);
                              }
                            else
                              {
                              st = GetString(dummystring, STRINGSIZE, iff, st);
                              error = TMFILEERR_OUTOFMEM;  /* Allocate label string memory fail */
                              }
                            }
                          }
                        }
                      else
                        {
                        error = TMFILEERR_OUTOFMEM;  /* Allocate pointer memory fail */
                        }

                      stringdata.StringCount = 0;
                      stringdata.StringArray = NULL;
                      }
                    break;

                  /**************************************************************/

                  case ID_TWLS:
                    st = GetBytes(&datacount, sizeof(SHORT), 1, iff, st);
                    st = GetBytes(&tag,       sizeof(ULONG), 1, iff, st);
                    st = GetBytes(&reserved,  sizeof(ULONG), 1, iff, st);

                    if(st && node)
                      {
                      if(wordarray = (WORD *) AllocMem(sizeof(WORD) * (datacount+1), MEMF_CLEAR))
                        {
                        DEBUGALLOC(sizeof(WORD)*(datacount+1));

                        wordpointer = wordarray;
                        *wordpointer = datacount;
                        wordpointer++;
                        for(i=0; i<datacount; i++)
                          {
                          st = GetBytes(wordpointer, sizeof(WORD),  1, iff, st);
                          wordpointer++;
                          }

                        AddTag(tagtype, node, tag, (ULONG) wordarray);

                        DEBUGFREE(sizeof(WORD)*(datacount+1));
                        FreeMem(wordarray, sizeof(WORD) * (datacount+1));
                        }
                      else
                        {
                        for(i=0; i<datacount; i++)
                          {
                          st = GetBytes(&dummy, sizeof(WORD), 1, iff, st);
                          }
                        error = TMFILEERR_OUTOFMEM;  /* Allocate word array memory fail */
                        }
                      }
                    break;
                  }
                }
              }
            }
          else
            {
            error = TMFILEERR_WRONGTYPE;
            }
          }
        else
          {
          error = TMFILEERR_OUTOFMEM;
          }

        PutIFFError(ifferror);

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

    modified = FALSE;
    }
  else
    {
    error = TMFILEERR_OUTOFMEM;
    }

  if(st==FALSE) error = TMFILEERR_READ;

  if(error)
    {
    switch(error)
      {
      case TMFILEERR_OPEN:
        PutError("Error opening\nproject file\nfor input", "OK");
        break;
      case TMFILEERR_WRONGTYPE:
        PutError("Improper project\nfile format", "OK");
        break;
      case TMFILEERR_OUTOFMEM:
        PutError("Out of memory", "OK");
        break;
      case TMFILEERR_READ:
        PutError("Error reading\nproject file", "OK");
        break;
      }

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      TagWindow(windownode, COMMAND_KILL);
      }
    ProcessWindows();

    TagScreen(COMMAND_KILL);
    ProcessScreen();

    newprojectflag = TRUE;
    retval = FALSE;
    }
  else
    {
    newprojectflag = FALSE;
    }

  DEBUGEXIT(TRUE,retval);

  return(retval);
  }

/****************************************************************************/

void OpenSettingsFile(void)
  {
  ULONG  reserved=0;
  BOOL   st=TRUE;
  UBYTE  error=TMFILEERR_NONE;
  LONG   ifferror;
  struct IFFHandle *iff;
  struct ContextNode   *chunk;

  DEBUGENTER("OpenSettingsFile",NULL);

  MainWindowSleep();

  if(iff = AllocIFF())
    {
    if(iff->iff_Stream = (ULONG) fopen(settingsfilename, "rb"))
      {
      InitIFFasStdIO(iff);

      if(OpenIFF(iff, IFFF_READ) == 0)
        {
        realgadgets = FALSE;
        realmenus = FALSE;
        screennode.Node.ln_Pri = STATUS_NORMAL;

        if(StopChunk(iff, ID_TMSE, ID_FORM) == 0)
          {
          if(ifferror = ParseIFF(iff, IFFPARSE_SCAN) == 0)
            {
            while((((ifferror = ParseIFF(iff, IFFPARSE_RAWSTEP)) == 0) || (ifferror == IFFERR_EOC)) && (ifferror != IFFERR_EOF))
              {
              if((chunk = CurrentChunk(iff)) && (ifferror != IFFERR_EOC))
                {
                switch(chunk->cn_ID)
                  {
                  /**************************************************************/

                  case ID_SCOM:
                    st = GetBytes(&commentflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SATB:
                    st = GetBytes(&autotopborderflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SPRA:
                    st = GetBytes(&pragmaflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SGSZ:
                    st = GetBytes(&gridsize,   sizeof(UBYTE), 1, iff, st);
                    st = GetBytes(&reserved,   sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SICN:
                    st = GetBytes(&iconflag,   sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SUSG:
                    st = GetBytes(&usersignalflag, sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SARX:
                    st = GetBytes(&arexxflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SCHP:
                    st = GetBytes(&chipflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SUDA:
                    st = GetBytes(&userdataflag,  sizeof(UBYTE), 1, iff, st);
                    break;

                  /**************************************************************/

                  case ID_SFRQ:
                    st = GetBytes(&aslflag,  sizeof(UBYTE), 1, iff, st);
                    break;
                  }
                }
              }
            }
          else
            {
            error = TMFILEERR_WRONGTYPE;
            }
          }
        else
          {
          error = TMFILEERR_OUTOFMEM;
          }

        PutIFFError(ifferror);

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
    }
  else
    {
    error = TMFILEERR_OUTOFMEM;
    }

  if(st==FALSE) error = TMFILEERR_READ;

  if(error)
    {
    switch(error)
      {
      case TMFILEERR_OPEN:
        PutError("Error opening\nsettings file\nfor input", "OK");
        break;
      case TMFILEERR_WRONGTYPE:
        PutError("Improper settings\nfile format", "OK");
        break;
      case TMFILEERR_OUTOFMEM:
        PutError("Out of memory", "OK");
        break;
      case TMFILEERR_READ:
        PutError("Error reading\nsettings file", "OK");
        break;
      }
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE,NULL);
  }

/****************************************************************************/

BOOL GetBytes(void *address, int size, int count, struct IFFHandle *iff, BOOL st)
  {
  char *firstchar;
  LONG totalsize;

  DEBUGENTER("GetBytes",st);

  if(st)
    {
    totalsize = size*count;
    if(ReadChunkBytes(iff, address, totalsize) != totalsize)
      {
      firstchar = address;
      *firstchar = 0;
      st = FALSE;
      }
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

BOOL GetString(char *string, int size, struct IFFHandle *iff, BOOL st)
  {
  UBYTE *chptr;
  UBYTE ch;

  DEBUGENTER("GetString",st);

  if(st)
    {
    chptr = string;

    while(((st = ReadChunkBytes(iff, &ch, 1)) == 1) && (ch != '\n') && st)
      {
      *chptr++ = ch;
      }
    *chptr = '\0';
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

/****************************************************************************/

VOID PutIFFError(LONG ifferror)
  {
  char *ok="OK";

  switch(ifferror)
    {
    case IFFERR_NOSCOPE:
      PutError("No valid scope for property", ok);
      break;
    case IFFERR_NOMEM:
      PutError("Internal memory alloc failed", ok);
      break;
    case IFFERR_READ:
      PutError("Stream read error", ok);
      break;
    case IFFERR_WRITE:
      PutError("Stream write error", ok);
      break;
    case IFFERR_SEEK:
      PutError("Stream seek error", ok);
      break;
    case IFFERR_MANGLED:
      PutError("Data in file is corrupt", ok);
      break;
    case IFFERR_SYNTAX:
      PutError("IFF syntax error", ok);
      break;
    case IFFERR_NOTIFF:
      PutError("Not an IFF file", ok);
      break;
    case IFFERR_NOHOOK:
      PutError("No call-back hook provided", ok);
      break;
/*
    case IFFERR_EOF:
      PutError("Reached logical end of file", ok);
      break;
    case IFFERR_EOC:
      PutError("About to leave context", ok);
      break;
    case IFF_RETURN2CLIENT:
      PutError("Client handler normal return", ok);
      break;
*/
    }
  }

/****************************************************************************/

LONG __saveds __asm HandleStdIOStream(register __a0 struct Hook *hook,
                                      register __a2 struct IFFHandle *iff,
                                      register __a1 struct IFFStreamCmd *actionpkt)
  {
  register FILE *stream;
  register LONG nbytes;
  register int actual;
  register UBYTE *buf;
  LONG len;

  stream = (FILE *) iff->iff_Stream;
  if(!stream) return(1);

  nbytes = actionpkt->sc_NBytes;
  buf = (UBYTE *) actionpkt->sc_Buf;

  switch (actionpkt->sc_Command)
    {
    case IFFSCC_READ:
      do
        {
        actual = nbytes > 32767 ? 32767 : nbytes;
	if ((len=fread (buf, 1, actual, stream)) != actual) break;
        nbytes -= actual;
        buf += actual;
        } while (nbytes > 0);
      return (nbytes ? IFFERR_READ : 0 );

    case IFFSCC_WRITE:
      do
        {
        actual = nbytes > 32767 ? 32767 : nbytes;
        if ((len=fwrite (buf, 1, actual, stream)) != actual) break;
        nbytes -= actual;
        buf += actual;
        } while (nbytes > 0);
      return (nbytes ? IFFERR_WRITE : 0);

    case IFFSCC_SEEK:
      return ((fseek (stream, nbytes, 1) == -1) ? IFFERR_SEEK : 0);

    default:
      return (0);
    }
  }

/****************************************************************************/

void InitIFFasStdIO(struct IFFHandle *iff)
  {
  static struct Hook stdiohook =
    {
      { NULL },
      (ULONG (*)()) HandleStdIOStream,
      NULL,
      NULL
    };

  InitIFF (iff, IFFF_FSEEK | IFFF_RSEEK, &stdiohook);
  }

