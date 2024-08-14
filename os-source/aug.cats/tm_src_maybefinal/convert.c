
/****************************************************************************/
/*                                                                          */
/*  Convert.c - Conversion functions                                        */
/*                                                                          */
/****************************************************************************/

#include "Toolmaker.h"
#include "Externs.h"

#include <exec/memory.h>
#include <graphics/text.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>

/****************************************************************************/

SHORT ConvertGridX(SHORT val)
  {
  register SHORT borderleft;
  register SHORT retval;

  DEBUGENTER("ConvertGridX", NULL);

  borderleft = currentwindow->Window->BorderLeft;
  retval = ((val-borderleft) /gridsize) *gridsize + borderleft;

  DEBUGEXIT(TRUE, retval);

  return(retval);
  }

/****************************************************************************/

SHORT ConvertGridY(SHORT val)
  {
  register SHORT bordertop;
  register SHORT retval;

  DEBUGENTER("ConvertGridY", NULL);

  bordertop = currentwindow->Window->BorderTop;
  retval = ((val-bordertop) /gridsize) *gridsize + bordertop;

  DEBUGEXIT(TRUE, retval);

  return(retval);
  }

/****************************************************************************/

char *IDCMP2String(ULONG idcmp)
  {
  static char returnstring[650]; /* 520 */
  struct AvailableIDCMP *thisidcmp;

  DEBUGENTER("IDCMP2String", NULL);

  returnstring[0] = '\0';

  for(thisidcmp = &AvailableIDCMP[0]; thisidcmp->Name!=NULL; thisidcmp++)
    {
    if(thisidcmp->IDCMP & idcmp && thisidcmp->Type == TYPE_USER)
      {
/*
      strcat(returnstring, " | ");
*/
      strcat(returnstring, " |\n\t\t\t");
      strcat(returnstring, thisidcmp->Name);
      }
    }

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *GadgetIDCMP(struct WindowNode *windownode)
  {
  static char returnstring[280]; /* 200 */
  struct NewGadgetNode *newgadgetnode;
  UBYTE buflag = FALSE,
        cbflag = FALSE,
        cyflag = FALSE,
        inflag = FALSE,
        lvflag = FALSE,
        mxflag = FALSE,
        nuflag = FALSE,
        paflag = FALSE,
        scflag = FALSE,
        slflag = FALSE,
        stflag = FALSE,
        txflag = FALSE;

  DEBUGENTER("GadgetIDCMP", NULL);

  returnstring[0] = '\0';

  for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
      newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
    {
    switch(newgadgetnode->Kind)
      {
      case BUTTON_KIND:
        if(!buflag) { strcat(returnstring, " |\n\t\t\tBUTTONIDCMP");   buflag = TRUE; }
        break;

      case CHECKBOX_KIND:
        if(!cbflag) { strcat(returnstring, " |\n\t\t\tCHECKBOXIDCMP"); cbflag = TRUE; }
        break;

      case CYCLE_KIND:
        if(!cyflag) { strcat(returnstring, " |\n\t\t\tCYCLEIDCMP");    cyflag = TRUE; }
        break;

      case INTEGER_KIND:
        if(!inflag) { strcat(returnstring, " |\n\t\t\tINTEGERIDCMP");  inflag = TRUE; }
        break;

      case LISTVIEW_KIND:
        if(!lvflag) { strcat(returnstring, " |\n\t\t\tLISTVIEWIDCMP"); lvflag = TRUE; }
        break;

      case MX_KIND:
        if(!mxflag) { strcat(returnstring, " |\n\t\t\tMXIDCMP");       mxflag = TRUE; }
        break;

/*
      case NUMBER_KIND:
        if(!nuflag) { strcat(returnstring, " |\n\t\t\tNUMBERIDCMP");   nuflag = TRUE; }
        break;
*/

      case PALETTE_KIND:
        if(!paflag) { strcat(returnstring, " |\n\t\t\tPALETTEIDCMP");  paflag = TRUE; }
        break;

      case SCROLLER_KIND:
        if(!scflag) { strcat(returnstring, " |\n\t\t\tSCROLLERIDCMP"); scflag = TRUE; }
        break;

      case SLIDER_KIND:
        if(!slflag) { strcat(returnstring, " |\n\t\t\tSLIDERIDCMP");   slflag = TRUE; }
        break;

      case STRING_KIND:
        if(!stflag) { strcat(returnstring, " |\n\t\t\tSTRINGIDCMP");   stflag = TRUE; }
        break;

/*
      case TEXT_KIND:
        if(!txflag) { strcat(returnstring, " |\n\t\t\tTEXTIDCMP");     txflag = TRUE; }
        break;
*/
      }
    }

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *MenuFlags2String(ULONG flags)
  {
  static char returnstring[20];

  DEBUGENTER("MenuFlags2String", NULL);

  returnstring[0] = '\0';

  if(flags & NM_MENUDISABLED) strcat(returnstring, "NM_MENUDISABLED");
  if(flags == 0) strcat(returnstring, "0");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *ItemFlags2String(ULONG flags)
  {
  static char returnstring[60];

  DEBUGENTER("ItemFlags2String", NULL);

  returnstring[0] = '\0';

  if(flags & NM_ITEMDISABLED) strcat(returnstring, "NM_ITEMDISABLED | ");
  if(flags & CHECKIT)         strcat(returnstring, "CHECKIT | ");
  if(flags & CHECKED)         strcat(returnstring, "CHECKED | ");
  if(flags & MENUTOGGLE)      strcat(returnstring, "MENUTOGGLE | ");

  if(strlen(returnstring) > 3) returnstring[strlen(returnstring)-3] = '\0';

  if(flags == 0) strcpy(returnstring, "0");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *NGFlags2String(ULONG flags)
  {
  static char returnstring[16];

  DEBUGENTER("NGFlags2String", NULL);

  returnstring[0] = '\0';

  if(flags & PLACETEXT_LEFT)  strcat(returnstring, "PLACETEXT_LEFT");
  if(flags & PLACETEXT_RIGHT) strcat(returnstring, "PLACETEXT_RIGHT");
  if(flags & PLACETEXT_ABOVE) strcat(returnstring, "PLACETEXT_ABOVE");
  if(flags & PLACETEXT_BELOW) strcat(returnstring, "PLACETEXT_BELOW");
  if(flags & PLACETEXT_IN)    strcat(returnstring, "PLACETEXT_IN");

  if(flags & NG_HIGHLABEL)    strcat(returnstring, " | NG_HIGHLABEL");

  if(flags == 0) strcpy(returnstring, "0");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *Mode2String(ULONG mode)
  {
  static char returnstring[40];

  DEBUGENTER("Mode2String", NULL);

  returnstring[0] = '\0';

  if(mode & MODE_HIRES)
    {
    if(mode & MODE_INTERLACE)
      strcat(returnstring, "HIRESLACE_KEY");
    else
      strcat(returnstring, "HIRES_KEY");
    }
  else if(mode & MODE_SUPERHIRES)
    {
    if(mode & MODE_INTERLACE)
      strcat(returnstring, "SUPERLACE_KEY");
    else
      strcat(returnstring, "SUPER_KEY");
    }
  else if(mode & MODE_PRODUCTIVITY)
    {
    if(mode & MODE_INTERLACE)
      strcat(returnstring, "VGAPRODUCTLACE_KEY");
    else
      strcat(returnstring, "VGAPRODUCT_KEY");
    }
  else if(mode & MODE_A202410HZ)
    strcat(returnstring, "A2024TENHERTZ_KEY");
  else if(mode & MODE_A202415HZ)
    strcat(returnstring, "A2024FIFTEENHERTZ_KEY");

  if(mode & MODE_PAL)
    strcat(returnstring, " | PAL_MONITOR_ID");
  else if(mode & MODE_NTSC)
    strcat(returnstring, " | NTSC_MONITOR_ID");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *Overscan2String(ULONG overscan)
  {
  static char returnstring[20];

  DEBUGENTER("Overscan2String", NULL);

  returnstring[0] = '\0';

  switch(overscan)
    {
    case OSCAN_TEXT:
      strcpy(returnstring, "OSCAN_TEXT");
      break;

    case OSCAN_STANDARD:
      strcpy(returnstring, "OSCAN_STANDARD");
      break;

    case OSCAN_MAX:
      strcpy(returnstring, "OSCAN_MAX");
      break;

    case OSCAN_VIDEO:
      strcpy(returnstring, "OSCAN_VIDEO");
      break;
    }

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

ULONG StringList2Data(struct List *stringlist, int type)
  {
  static char *defaultstring[] = {"0"};
  static struct StringData defaultstringdata = {1, defaultstring};
  static WORD defaultwordarray[] = {1, 0};
  static struct StringData stringdata;
  int status;
  ULONG data;
  SHORT datacount;
  WORD *wordarray;
  WORD *wordpointer;
  char **stringpointer;
  struct StringNode *stringnode;

  DEBUGENTER("StringList2Data", NULL);

  switch(type)
    {
    case TYPE_INTEGER:
    case TYPE_IGNORE:
    case TYPE_STRING:
      stringnode = (struct StringNode *) stringlist->lh_Head;
      data = (ULONG) &stringnode->String[0];
      break;

    case TYPE_CHARACTER:
      stringnode = (struct StringNode *) stringlist->lh_Head;
      data = (ULONG) stringnode->String[0];
      break;

    case TYPE_WORDLIST:
      datacount = 0;
      for(stringnode = (struct StringNode *) stringlist->lh_Head; stringnode->Node.ln_Succ;
          stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
        {
        datacount++;
        }

      status = TRUE;
      while(!(wordarray = (WORD *) AllocMem(sizeof(WORD) * (datacount+1), MEMF_CLEAR)) && status)
        status = PutError("Out of memory\ncreating tag data", "Retry|Abort");
      if(status)
        {
        DEBUGALLOC(sizeof(WORD) * (datacount+1));

        wordpointer = wordarray;
        *wordpointer = datacount;
        wordpointer++;

        for(stringnode = (struct StringNode *) stringlist->lh_Head; stringnode->Node.ln_Succ;
            stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
          {
          *wordpointer = (WORD) atoi(stringnode->String);
          wordpointer++;
          }
        data = (ULONG) wordarray;
        }
      else
        {
        data = (ULONG) defaultwordarray;
        }
      break;

    case TYPE_LINKEDLIST:
    case TYPE_STRINGLIST:
      datacount = 0;
      for(stringnode = (struct StringNode *) stringlist->lh_Head; stringnode->Node.ln_Succ;
          stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
        {
        datacount++;
        }
      stringdata.StringCount = datacount;

      status = TRUE;
      while(!(stringdata.StringArray = (char **) AllocMem(sizeof(char *) * stringdata.StringCount, MEMF_CLEAR)) && status)
        status = PutError("Out of memory\ncreating tag data", "Retry|Abort");
      if(status)
        {
        DEBUGALLOC(sizeof(char *) * stringdata.StringCount);

        stringpointer = stringdata.StringArray;
        for(stringnode = (struct StringNode *) stringlist->lh_Head; stringnode->Node.ln_Succ;
            stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
          {
          status = TRUE;
          while(!(*stringpointer = (char *) AllocMem(STRINGSIZE, MEMF_CLEAR)) && status)
            status = PutError("Out of memory\ncreating tag data", "Retry|Abort");
          if(status)
            {
            DEBUGALLOC(STRINGSIZE);

            strcpy(*stringpointer, stringnode->String);
            stringpointer++;
            }
          else
            {
            stringdata.StringCount--;
            }
          }
        if(stringdata.StringCount)
          data = (ULONG) &stringdata;
        else
          data = (ULONG) &defaultstringdata;
        }
      else
        {
        data = (ULONG) &defaultstringdata;
        }
      break;
    }

  DEBUGEXIT(TRUE, (ULONG) data);
  return(data);
  }

/****************************************************************************/

void FreeData(ULONG data, int type)
  {
  int i;
  SHORT datacount;
  WORD *wordpointer;
  char **stringpointer;
  struct StringData *stringdata;

  DEBUGENTER("FreeData", NULL);

  switch(type)
    {
    case TYPE_WORDLIST:
      wordpointer = (WORD *) data;
      if(wordpointer)
        {
        datacount = (WORD) *wordpointer;

        DEBUGFREE(sizeof(WORD) * (datacount+1));
        FreeMem(wordpointer, sizeof(WORD) * (datacount+1));
        }
      break;

    case TYPE_LINKEDLIST:
    case TYPE_STRINGLIST:
      stringdata = (struct StringData *) data;
      if(stringdata)
        {
        stringpointer = stringdata->StringArray;
        datacount = stringdata->StringCount;

        if(stringpointer)
          {
          for(i=0; i<datacount; i++)
            {
            if(*stringpointer)
              {
              DEBUGFREE(STRINGSIZE);
              FreeMem(*stringpointer, STRINGSIZE);
              stringpointer++;
              }
            }
          DEBUGFREE(sizeof(char *) * stringdata->StringCount);
          FreeMem(stringdata->StringArray, sizeof(char *) * stringdata->StringCount);
          }
        }
      break;
    }

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

ULONG CenterHoriz(ULONG windowwidth)
  {
  return( (viewwidth - windowwidth)/2 - screennode.Screen->ViewPort.DxOffset );
  }

/****************************************************************************/

ULONG CenterVert(ULONG windowheight)
  {
  return( (viewheight - windowheight)/2 - screennode.Screen->ViewPort.DyOffset );
  }

/****************************************************************************/

char *TextAttrName(struct TextAttr *textattr)
  {
  static char returnstring[100];
  char size[10];

  DEBUGENTER("TextAttrName", NULL);

  strcpy(returnstring, textattr->ta_Name);
  returnstring[strlen(returnstring)-5] = '\0';

  sprintf(size, "%d", textattr->ta_YSize);
  strcat(returnstring, size);

  if(textattr->ta_Style & FSF_BOLD)       strcat(returnstring, "Bold");
  if(textattr->ta_Style & FSF_ITALIC)     strcat(returnstring, "Italic");
  if(textattr->ta_Style & FSF_UNDERLINED) strcat(returnstring, "Underlined");
  if(textattr->ta_Style & FSF_EXTENDED)   strcat(returnstring, "Extended");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *TextAttrStyle(struct TextAttr *textattr)
  {
  static char returnstring[60];
  ULONG style;

  DEBUGENTER("TextAttrStyle", NULL);

  style = textattr->ta_Style;
  returnstring[0] = '\0';

  if(style & FSF_BOLD)       strcat(returnstring, "FSF_BOLD | ");
  if(style & FSF_ITALIC)     strcat(returnstring, "FSF_ITALIC | ");
  if(style & FSF_UNDERLINED) strcat(returnstring, "FSF_UNDERLINED | ");
  if(style & FSF_EXTENDED)   strcat(returnstring, "FSF_EXTENDED | ");

  if(strlen(returnstring) > 3) returnstring[strlen(returnstring)-3] = '\0';

  if(returnstring[0] == '\0') strcpy(returnstring, "FS_NORMAL");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *TextAttrFlags(struct TextAttr *textattr)
  {
  static char returnstring[150];

  DEBUGENTER("TextAttrFlags", NULL);

  returnstring[0] = '\0';

/*
  if(flags & FPF_ROMFONT)       strcat(returnstring, "FPF_ROMFONT | ");
  if(flags & FPF_DISKFONT)      strcat(returnstring, "FPF_DISKFONT | ");
  if(flags & FPF_REVPATH)       strcat(returnstring, "FPF_REVPATH | ");
  if(flags & FPF_TALLDOT)       strcat(returnstring, "FPF_TALLDOT | ");
  if(flags & FPF_WIDEDOT)       strcat(returnstring, "FPF_WIDEDOT | ");
  if(flags & FPF_PROPORTIONAL)  strcat(returnstring, "FPF_PROPORTIONAL | ");
  if(flags & FPF_DESIGNED)      strcat(returnstring, "FPF_DESIGNED | ");
  if(flags & FPF_REMOVED)       strcat(returnstring, "FPF_REMOVED | ");

  if(strlen(returnstring) > 3) returnstring[strlen(returnstring)-3] = '\0';
*/

  if(returnstring[0] == '\0') strcpy(returnstring, "0x0");

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *Text2Label(char *text)
  {
  static char returnstring[LABELSIZE+2];
  char *character;
  int count;

  DEBUGENTER("Text2Label", NULL);

  count = 0;
  for(character=text; *character!='\0' && count<LABELSIZE-1; character++)
    {
    if(isalnum(*character)) returnstring[count++] = toupper(*character);
    }
  returnstring[count] = '\0';

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

/****************************************************************************/

char *UserDataName(char *label)
  {
  static char returnstring[30];

  DEBUGENTER("UserDataName", NULL);

  if((userdataflag) && (label[0] != '\0'))
    {
    strcpy(returnstring, "(APTR) &tmobjectdata_");
    strcat(returnstring, label);
    }
  else
    {
    strcpy(returnstring, "NULL");
    }

  DEBUGEXIT(TRUE, (ULONG) returnstring);
  return(returnstring);
  }

