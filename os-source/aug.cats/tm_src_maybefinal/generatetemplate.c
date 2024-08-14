
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <string.h>

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/dos_protos.h>

/****************************************************************************/

BOOL GenerateTemplate(void)
  {
  struct WindowNode    *windownode;
  struct NewGadgetNode *newgadgetnode;
  struct MenuNode      *menunode;
  struct ItemNode      *itemnode;
  struct SubNode       *subnode;
  struct TagNode       *tagnode;
  struct StringNode    *stringnode;
  struct TextAttrNode  *textattrnode;
  FILE                 *fp;
  BOOL   st=TRUE;

  DEBUGENTER("GenerateTemplate", NULL);

  sprintf(fullname, "%s.c", filename);

  if(fp = fopen(fullname, "r"))
    {
    sprintf(fullname, "%s.c.new", filename);
    fclose(fp);
    }

  if(fp = fopen(fullname, "w"))
    {
    /**************** Includes ****************/

    st = GenerateString("", fp, st, NULL);
    st = GenerateString(NULL, fp, st, "Includes");
    st = GenerateString(NULL, fp, st, "");

    st = GenerateString("#include <exec/types.h>", fp, st, NULL);
    st = GenerateString("#include <intuition/intuition.h>", fp, st, NULL);
    st = GenerateString("#include <dos/dos.h>", fp, st, NULL);
    st = GenerateString("#include <string.h>", fp, st, NULL);
    st = GenerateString("#include <stdlib.h>", fp, st, NULL);

    if(aslflag)
      {
      st = GenerateString("#include <libraries/asl.h>", fp, st, NULL);
      }

    st = GenerateString("", fp, st, NULL);

    if(arexxflag)
      {
      st = GenerateString("#include \"SimpleRexx_tm.h\"", fp, st, "SimpleRexx header file");
      }

    sprintf(string,     "#include \"%s.h\"", savefilename);
    st = GenerateString(string, fp, st, "User header file");
    sprintf(string,     "#include \"%s_rev.h\"", savefilename);
    st = GenerateString(string, fp, st, "Bumprev header file");
    st = GenerateString("", fp, st, NULL);

    st = GenerateString("#include <clib/exec_protos.h>", fp, st, NULL);
    st = GenerateString("#include <clib/intuition_protos.h>", fp, st, NULL);
    st = GenerateString("#include <clib/gadtools_protos.h>", fp, st, NULL);
    st = GenerateString("#include <clib/utility_protos.h>", fp, st, NULL);

    if(diskfontcount)
      {
      st = GenerateString("#include <clib/graphics_protos.h>", fp, st, NULL);
      st = GenerateString("#include <clib/diskfont_protos.h>", fp, st, NULL);
      }

    if(aslflag)
      {
      st = GenerateString("#include <clib/asl_protos.h>", fp, st, NULL);
      st = GenerateString("#include <clib/dos_protos.h>", fp, st, NULL);
      }

    st = GenerateString("", fp, st, NULL);

    /************************** Pragma includes ******************************/

    if(pragmaflag)
      {
      st = GenerateString(NULL, fp, st, "Pragma includes for register parameters");
      st = GenerateString(NULL, fp, st, "");

      st = GenerateString("#include <pragmas/exec_pragmas.h>", fp, st, NULL);
      st = GenerateString("#include <pragmas/intuition_pragmas.h>", fp, st, NULL);
      st = GenerateString("#include <pragmas/gadtools_pragmas.h>", fp, st, NULL);
      st = GenerateString("#include <pragmas/utility_pragmas.h>", fp, st, NULL);

      if(diskfontcount)
        {
        st = GenerateString("#include <pragmas/graphics_pragmas.h>", fp, st, NULL);
        st = GenerateString("#include <pragmas/diskfont_pragmas.h>", fp, st, NULL);
        }

      if(aslflag)
        {
        st = GenerateString("#include <pragmas/asl_pragmas.h>", fp, st, NULL);
        st = GenerateString("#include <pragmas/dos_pragmas.h>", fp, st, NULL);
        }

      st = GenerateString("", fp, st, NULL);
      }

    /**************** Variables *****************/

    st = GenerateString(NULL, fp, st, "Global variables");
    st = GenerateString(NULL, fp, st, "");
    st = GenerateString("char vers[] = VERSTAG;", fp, st, "Bumprev version string");
    st = GenerateString("struct Library *IntuitionBase;", fp, st, "intuition.library pointer");
    st = GenerateString("struct Library *GadToolsBase;", fp, st, "gadtools.library pointer");
    st = GenerateString("struct Library *UtilityBase;", fp, st, "utility.library pointer");

    if(diskfontcount)
      {
      st = GenerateString("struct Library *GfxBase;", fp, st, "graphics.library pointer");
      st = GenerateString("struct Library *DiskfontBase;", fp, st, "diskfont.library pointer");
      }

    if(aslflag)
      {
      st = GenerateString("struct Library *AslBase;", fp, st, "asl.library pointer");
      }

    st = GenerateString("struct IntuiText BodyText = {0,1,JAM2,20,8,NULL,(UBYTE *)TEXT_NORELEASE2,NULL};", fp, st, NULL);
    st = GenerateString("struct IntuiText NegText  = {0,1,JAM2, 6,4,NULL,(UBYTE *)TEXT_OK,NULL};", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /**************** main() Function ****************/

    st = GenerateString(NULL, fp, st, "Entry functions");
    st = GenerateString(NULL, fp, st, "");

    st = GenerateString("#ifdef _DCC", fp, st, "for DICE compatibility");
    st = GenerateString("VOID wbmain(VOID *wbmsg)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  main(0, wbmsg);", fp, st, NULL);
    st = GenerateString("  }", fp, st, NULL);
    st = GenerateString("#endif", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    st = GenerateString("VOID main(int argc, char **argv)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);
    st = GenerateString("  struct TMData *TMData;", fp, st, "data structure pointer");
    st = GenerateString("  ULONG error;", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /* Open intuition.library and gadtools.library */

    st = GenerateString("  if(!(IntuitionBase = OpenLibrary((UBYTE *)\"intuition.library\", 37L)))", fp, st, "Open intuition.library V37");
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    if(IntuitionBase = OpenLibrary((UBYTE *)\"intuition.library\", 0L))", fp, st, NULL);
    st = GenerateString("      {", fp, st, NULL);
    st = GenerateString("      AutoRequest(NULL, &BodyText, NULL, &NegText, 0, 0, 320, 80);", fp, st, NULL);
    st = GenerateString("      CloseLibrary(IntuitionBase);", fp, st, NULL);
    st = GenerateString("      }", fp, st, NULL);
    st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    st = GenerateString("  if(!(GadToolsBase = OpenLibrary((UBYTE *)\"gadtools.library\", 37L)))", fp, st, "Open gadtools.library V37");
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, \"gadtools.library V37\");", fp, st, NULL);
    st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /* Open utility.library */

    st = GenerateString("  if(!(UtilityBase = OpenLibrary((UBYTE *)\"utility.library\", 37L)))", fp, st, "Open utility.library V37");
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, \"utility.library V37\");", fp, st, NULL);
    st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /* Open diskfont.library */

    if(diskfontcount)
      {
      st = GenerateString("  if(!(GfxBase = OpenLibrary((UBYTE *)\"graphics.library\", 37L)))", fp, st, "Open graphics.library V37");
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, \"graphics.library V37\");", fp, st, NULL);
      st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      st = GenerateString("  if(!(DiskfontBase = OpenLibrary((UBYTE *)\"diskfont.library\", 37L)))", fp, st, "Open diskfont.library V37");
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, \"diskfont.library V37\");", fp, st, NULL);
      st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    if(aslflag)
      {
      st = GenerateString("  if(!(AslBase = OpenLibrary((UBYTE *)\"asl.library\", 37L)))", fp, st, "Open asl.library V37");
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOLIBRARY, (UBYTE *)TEXT_ABORT, NULL, \"asl.library V37\");", fp, st, NULL);
      st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    /* Call TM_Open */

    st = GenerateString("  if(!(TMData = TM_Open(&error)))", fp, st, "Open Toolmaker data");
    st = GenerateString("    {", fp, st, NULL);
    st = GenerateString("    switch(error)", fp, st, NULL);
    st = GenerateString("      {", fp, st, NULL);
    st = GenerateString("      case TMERR_MEMORY:", fp, st, NULL);
    st = GenerateString("        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMEMORY, (UBYTE *)TEXT_ABORT, NULL, NULL);", fp, st, NULL);
    st = GenerateString("        break;", fp, st, NULL);

    st = GenerateString("      case TMERR_MSGPORT:", fp, st, NULL);
    st = GenerateString("        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOMSGPORT, (UBYTE *)TEXT_ABORT, NULL, NULL);", fp, st, NULL);
    st = GenerateString("        break;", fp, st, NULL);

    for(textattrnode = (struct TextAttrNode *) TextAttrList.lh_Head; textattrnode->Node.ln_Succ;
        textattrnode = (struct TextAttrNode *) textattrnode->Node.ln_Succ)
      {
      if(!((strcmp(textattrnode->FontName, "topaz.font") == 0) &&
          ((textattrnode->TextAttr.ta_YSize == 8) ||
           (textattrnode->TextAttr.ta_YSize == 9))))
        {
        sprintf(string,     "      case TMERR_%s:", textattrnode->Name);
        st = GenerateString(string, fp, st, NULL);
        sprintf(string,     "        TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOFONT, (UBYTE *)TEXT_ABORT, NULL, \"%s\", %d);", textattrnode->FontName, textattrnode->TextAttr.ta_YSize);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("        break;", fp, st, NULL);
        }
      }

    st = GenerateString("      }", fp, st, NULL);
    st = GenerateString("    cleanexit(NULL, RETURN_FAIL);", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /* Open file requester */

    if(aslflag)
      {
      st = GenerateString("  if(!(TMData->FileRequester = AllocAslRequestTags(ASL_FileRequest, TAG_DONE)))", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOFILEREQ, (UBYTE *)TEXT_ABORT, NULL, NULL);", fp, st, NULL);
      st = GenerateString("    cleanexit(TMData, RETURN_FAIL);", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    /* Open screens */

    if(screennode.TMSFlags & SCREENFLAG_OPENATSTART)
      {
      if(screennode.Workbench)
        {
        st = GenerateString("  if(!(OpenScreen_Workbench(TMData)))", fp, st, "Open default public screen");
        st = GenerateString("    {", fp, st, NULL);
        st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOSCREEN, (UBYTE *)TEXT_ABORT, NULL, NULL);", fp, st, NULL);
        st = GenerateString("    cleanexit(TMData, RETURN_FAIL);", fp, st, NULL);
        st = GenerateString("    }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }
      else
        {
        sprintf(string, "  if(!(OpenScreen_%s(TMData)))", screennode.SourceLabel);
        st = GenerateString(string, fp, st, "Open custom screen");
        st = GenerateString("    {", fp, st, NULL);
        st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOSCREEN, (UBYTE *)TEXT_ABORT, NULL, NULL);", fp, st, NULL);
        st = GenerateString("    cleanexit(TMData, RETURN_FAIL);", fp, st, NULL);
        st = GenerateString("    }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }
      }

    /* Open windows */

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      if(windownode->TMWFlags & WINDOWFLAG_OPENATSTART)
        {
        sprintf(string,     "  if(!(OpenWindow_%s(TMData)))", windownode->SourceLabel);
        st = GenerateString(string, fp, st, "Open the window");
        st = GenerateString("    {", fp, st, NULL);
        st = GenerateString("    TM_Request(NULL, (UBYTE *)TEXT_ERROR, (UBYTE *)TEXT_NOWINDOW, (UBYTE *)TEXT_ABORT, NULL, NULL);", fp, st, NULL);
        st = GenerateString("    cleanexit(TMData, RETURN_FAIL);", fp, st, NULL);
        st = GenerateString("    }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }
      }

    /* EventLoop */

    st = GenerateString("  TM_EventLoop(TMData);", fp, st, "Process events");
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  cleanexit(TMData, RETURN_OK);", fp, st, NULL);
    st = GenerateString("  }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /**************** cleanexit() Function ****************/

    st = GenerateString(NULL, fp, st, "cleanexit function");
    st = GenerateString(NULL, fp, st, "");

    st = GenerateString("VOID cleanexit(struct TMData *TMData, int returnvalue)", fp, st, NULL);
    st = GenerateString("  {", fp, st, NULL);

    st = GenerateString("  if(TMData)", fp, st, NULL);
    st = GenerateString("    {", fp, st, NULL);

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      sprintf(string, "    CloseWindow_%s(TMData);", windownode->SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      }

    if(screennode.Workbench)
      {
      st = GenerateString("    CloseScreen_Workbench(TMData);", fp, st, NULL);
      }
    else
      {
      sprintf(string,     "    CloseScreen_%s(TMData);", screennode.SourceLabel);
      st = GenerateString(string, fp, st, NULL);
      }

    /* Close file requester */

    if(aslflag)
      {
      st = GenerateString("    if(TMData->FileRequester) FreeAslRequest(TMData->FileRequester);", fp, st, NULL);
      }

    st = GenerateString("    TM_Close(TMData);", fp, st, NULL);
    st = GenerateString("    }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /* Close libraries */

    if(aslflag)
      {
      st = GenerateString("  if(AslBase)       CloseLibrary(AslBase);", fp, st, "Close asl.library");
      }

    if(diskfontcount)
      {
      st = GenerateString("  if(DiskfontBase)  CloseLibrary(DiskfontBase);", fp, st, "Close diskfont.library");
      st = GenerateString("  if(GfxBase)       CloseLibrary(GfxBase);", fp, st, "Close graphics.library");
      }

    st = GenerateString("  if(UtilityBase)   CloseLibrary(UtilityBase);", fp, st, "Close utility.library");
    st = GenerateString("  if(GadToolsBase)  CloseLibrary(GadToolsBase);", fp, st, "Close gadtools.library");
    st = GenerateString("  if(IntuitionBase) CloseLibrary(IntuitionBase);", fp, st, "Close intuition.library");
    st = GenerateString("", fp, st, NULL);
    st = GenerateString("  exit(returnvalue);", fp, st, NULL);
    st = GenerateString("  }", fp, st, NULL);
    st = GenerateString("", fp, st, NULL);

    /**************** File Requester Function ******************/

    if(aslflag)
      {
      st = GenerateString(NULL, fp, st, "ASL file requester function");
      st = GenerateString(NULL, fp, st, "");

      st = GenerateString("UBYTE *getfilename(struct TMData *TMData, UBYTE *title, UBYTE *buffer, ULONG bufsize, struct Window *window, BOOL saveflag)", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  ULONG funcflags;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  if(saveflag)", fp, st, NULL);
      st = GenerateString("    funcflags = FILF_SAVE;", fp, st, NULL);
      st = GenerateString("  else", fp, st, NULL);
      st = GenerateString("    funcflags = 0;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  if(buffer && TMData)", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    if(AslRequestTags((APTR) TMData->FileRequester,", fp, st, NULL);
      st = GenerateString("                      ASL_Hail, title,", fp, st, NULL);
      st = GenerateString("                      ASL_Window, window,", fp, st, NULL);
      st = GenerateString("                      ASL_FuncFlags, funcflags,", fp, st, NULL);
      st = GenerateString("                      TAG_DONE))", fp, st, NULL);
      st = GenerateString("      {", fp, st, NULL);
      st = GenerateString("      strcpy(buffer, TMData->FileRequester->rf_Dir);", fp, st, NULL);
      st = GenerateString("      if(AddPart(buffer, TMData->FileRequester->rf_File, bufsize))", fp, st, NULL);
      st = GenerateString("        {", fp, st, NULL);
      st = GenerateString("        return(buffer);", fp, st, NULL);
      st = GenerateString("        }", fp, st, NULL);
      st = GenerateString("      }", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  return(NULL);", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    /**************** Event Functions ******************/

    st = GenerateString(NULL, fp, st, "Window event functions");
    st = GenerateString(NULL, fp, st, "");

    for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
        windownode = (struct WindowNode *) windownode->Node.ln_Succ)
      {
      if(windownode->IDCMP & IDCMP_REFRESHWINDOW)
        {
        sprintf(string, "VOID Window_%s_REFRESHWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_CLOSEWINDOW)
        {
        sprintf(string, "BOOL Window_%s_CLOSEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(TRUE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MENUPICK)
        {
        sprintf(string, "BOOL Window_%s_MENUPICK(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);

        st = GenerateString("  UWORD menucode;", fp, st, NULL);

        if(userdataflag)
          {
          st = GenerateString("  struct MenuItem *menuitem;", fp, st, NULL);
          st = GenerateString("  TMOBJECTDATA *tmobjectdata;", fp, st, NULL);
          st = GenerateString("  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", fp, st, NULL);
          }
        st = GenerateString("", fp, st, NULL);

        st = GenerateString("  menucode = imsg->Code;", fp, st, NULL);
        st = GenerateString("  while(menucode != MENUNULL)", fp, st, NULL);
        st = GenerateString("    {", fp, st, NULL);

        if(userdataflag)
          {
          sprintf(string,     "    menuitem = ItemAddress(WindowInfo_%s.Menu, menucode);", windownode->SourceLabel);
          st = GenerateString(string, fp, st, NULL);
          st = GenerateString("    tmobjectdata = (TMOBJECTDATA *)(GTMENUITEM_USERDATA(menuitem));", fp, st, NULL);
          st = GenerateString("    eventfunc = tmobjectdata->EventFunc;", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("    if(eventfunc)", fp, st, NULL);
          st = GenerateString("      {", fp, st, NULL);
          st = GenerateString("      if((*eventfunc)(TMData, imsg, tmobjectdata)) return(TRUE);", fp, st, NULL);
          st = GenerateString("      }", fp, st, NULL);
          }
        else
          {
          st = GenerateString("    switch(MENUNUM(menucode))", fp, st, NULL);
          st = GenerateString("      {", fp, st, NULL);

          for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
              menunode = (struct MenuNode *) menunode->Node.ln_Succ)
            {
            sprintf(string, "      case MENU_%s: /* %s */", menunode->SourceLabel, menunode->MenuText);
            st = GenerateString(string, fp, st, NULL);
            if(menunode->ItemCount)
              {
              st = GenerateString("        switch(ITEMNUM(menucode))", fp, st, NULL);
              st = GenerateString("          {", fp, st, NULL);
              for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
                  itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
                {
                if(itemnode->ItemText[0] != '-')
                  {
                  sprintf(string, "          case ITEM_%s: /* %s */", itemnode->SourceLabel, itemnode->ItemText);
                  st = GenerateString(string, fp, st, NULL);
                  if(itemnode->SubCount)
                    {
                    st = GenerateString("            switch(SUBNUM(menucode))", fp, st, NULL);
                    st = GenerateString("              {", fp, st, NULL);
                    for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
                        subnode = (struct SubNode *) subnode->Node.ln_Succ)
                      {
                      if(subnode->SubText[0] != '-')
                        {
                        sprintf(string, "              case SUBITEM_%s: /* %s */", subnode->SourceLabel, subnode->SubText);
                        st = GenerateString(string, fp, st, NULL);
                        st = GenerateString("                break;", fp, st, NULL);
                        }
                      }
                    st = GenerateString("              }", fp, st, NULL);
                    }
                  st = GenerateString("            break;", fp, st, NULL);
                  }
                }
              st = GenerateString("          }", fp, st, NULL);
              }
            st = GenerateString("        break;", fp, st, NULL);
            }
          st = GenerateString("      }", fp, st, NULL);

          st = GenerateString("", fp, st, NULL);
          sprintf(string,     "    menucode = (((struct MenuItem *) ItemAddress(WindowInfo_%s.Menu, menucode))->NextSelect);", windownode->SourceLabel);
          st = GenerateString(string, fp, st, NULL);
          st = GenerateString("    }", fp, st, NULL);
          }

        if(userdataflag)
          {
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("    menucode = menuitem->NextSelect;", fp, st, NULL);
          st = GenerateString("    }", fp, st, NULL);
          }

        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_GADGETDOWN)
        {
        sprintf(string, "BOOL Window_%s_GADGETDOWN(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);

        if(userdataflag)
          {
          st = GenerateString("  struct Gadget *gadget;", fp, st, NULL);
          st = GenerateString("  TMOBJECTDATA *tmobjectdata;", fp, st, NULL);
          st = GenerateString("  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("  gadget = (struct Gadget *)imsg->IAddress;", fp, st, NULL);
          st = GenerateString("  tmobjectdata = (TMOBJECTDATA *)(gadget->UserData);", fp, st, NULL);
          st = GenerateString("  eventfunc = tmobjectdata->EventFunc;", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("  if(eventfunc)", fp, st, NULL);
          st = GenerateString("    {", fp, st, NULL);
          st = GenerateString("    return((*eventfunc)(TMData, imsg, tmobjectdata));", fp, st, NULL);
          st = GenerateString("    }", fp, st, NULL);
          }
        else
          {
          st = GenerateString("  switch(((struct Gadget *)imsg->IAddress)->GadgetID)", fp, st, NULL);
          st = GenerateString("    {", fp, st, NULL);
          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            if(newgadgetnode->Kind == LISTVIEW_KIND ||
               newgadgetnode->Kind == MX_KIND ||
               newgadgetnode->Kind == SCROLLER_KIND ||
               newgadgetnode->Kind == SLIDER_KIND)
              {
              if(newgadgetnode->GadgetText[0] != '\0')
                sprintf(string, "    case ID_%s: /* %s */", newgadgetnode->SourceLabel, newgadgetnode->GadgetText);
              else
                sprintf(string, "    case ID_%s:", newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);

              switch(newgadgetnode->Kind)
                {
                case MX_KIND:
                  st = GenerateString("      switch(imsg->Code)", fp, st, NULL);
                  st = GenerateString("        {", fp, st, NULL);
                  for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
                      tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
                    {
                    for(stringnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head; stringnode->Node.ln_Succ;
                        stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
                      {
                      sprintf(string,     "        case ID_%s_%s:", newgadgetnode->SourceLabel, stringnode->String);
                      st = GenerateString(string, fp, st, NULL);
                      st = GenerateString("          break;", fp, st, NULL);
                      }
                    }
                  st = GenerateString("        }", fp, st, NULL);
                  break;
                case SCROLLER_KIND:
                  st = GenerateString("      /* imsg->Code (if GA_Immediate tag) */", fp, st, NULL);
                  break;
                case SLIDER_KIND:
                  st = GenerateString("      /* imsg->Code (if GA_Immediate tag) */", fp, st, NULL);
                  break;
                }
              st = GenerateString("      break;", fp, st, NULL);
              }
            }
          st = GenerateString("    }", fp, st, NULL);
          }
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_GADGETUP)
        {
        sprintf(string, "BOOL Window_%s_GADGETUP(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        if(userdataflag)
          {
          st = GenerateString("  struct Gadget *gadget;", fp, st, NULL);
          st = GenerateString("  TMOBJECTDATA *tmobjectdata;", fp, st, NULL);
          st = GenerateString("  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("  gadget = (struct Gadget *)imsg->IAddress;", fp, st, NULL);
          st = GenerateString("  tmobjectdata = (TMOBJECTDATA *)(gadget->UserData);", fp, st, NULL);
          st = GenerateString("  eventfunc = tmobjectdata->EventFunc;", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("  if(eventfunc)", fp, st, NULL);
          st = GenerateString("    {", fp, st, NULL);
          st = GenerateString("    return((*eventfunc)(TMData, imsg, tmobjectdata));", fp, st, NULL);
          st = GenerateString("    }", fp, st, NULL);
          }
        else
          {
          st = GenerateString("  switch(((struct Gadget *)imsg->IAddress)->GadgetID)", fp, st, NULL);
          st = GenerateString("    {", fp, st, NULL);
          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            if(newgadgetnode->Kind == BUTTON_KIND ||
               newgadgetnode->Kind == CHECKBOX_KIND ||
               newgadgetnode->Kind == INTEGER_KIND ||
               newgadgetnode->Kind == LISTVIEW_KIND ||
               newgadgetnode->Kind == CYCLE_KIND ||
               newgadgetnode->Kind == PALETTE_KIND ||
               newgadgetnode->Kind == SCROLLER_KIND ||
               newgadgetnode->Kind == SLIDER_KIND ||
               newgadgetnode->Kind == STRING_KIND)
              {
              if(newgadgetnode->GadgetText[0] != '\0')
                sprintf(string, "    case ID_%s: /* %s */", newgadgetnode->SourceLabel, newgadgetnode->GadgetText);
              else
                sprintf(string, "    case ID_%s:", newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              switch(newgadgetnode->Kind)
                {
                case STRING_KIND:
                  sprintf(string, "      /* ((struct StringInfo *)gadget_%s->SpecialInfo)->Buffer */", newgadgetnode->SourceLabel);
                  st = GenerateString(string, fp, st, NULL);
                  break;

                case CHECKBOX_KIND:
                  sprintf(string, "      /* gadget_%s->Flags & GFLG_SELECTED */", newgadgetnode->SourceLabel);
                  st = GenerateString(string, fp, st, NULL);
                  break;

                case INTEGER_KIND:
                  sprintf(string, "      /* ((struct StringInfo *)gadget_%s->SpecialInfo)->LongInt */", newgadgetnode->SourceLabel);
                  st = GenerateString(string, fp, st, NULL);
                  break;

                case LISTVIEW_KIND:
                  st = GenerateString("      switch(imsg->Code)", fp, st, NULL);
                  st = GenerateString("        {", fp, st, NULL);
                  for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
                      tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
                    {
                    for(stringnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head; stringnode->Node.ln_Succ;
                        stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
                      {
                      sprintf(string,     "        case ID_%s_%s:", newgadgetnode->SourceLabel, stringnode->String);
                      st = GenerateString(string, fp, st, NULL);
                      st = GenerateString("          break;", fp, st, NULL);
                      }
                    }
                  st = GenerateString("        }", fp, st, NULL);
                  break;

                case CYCLE_KIND:
                  st = GenerateString("      switch(imsg->Code)", fp, st, NULL);
                  st = GenerateString("        {", fp, st, NULL);
                  for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
                      tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
                    {
                    for(stringnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head; stringnode->Node.ln_Succ;
                        stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
                      {
                      sprintf(string,     "        case ID_%s_%s:", newgadgetnode->SourceLabel, stringnode->String);
                      st = GenerateString(string, fp, st, NULL);
                      st = GenerateString("          break;", fp, st, NULL);
                      }
                    }
                  st = GenerateString("        }", fp, st, NULL);
                  break;

                case PALETTE_KIND:
                  st = GenerateString("      /* imsg->Code */", fp, st, NULL);
                  break;

                case SCROLLER_KIND:
                  st = GenerateString("      /* imsg->Code (if GA_RelVerify tag) */", fp, st, NULL);
                  break;

                case SLIDER_KIND:
                  st = GenerateString("      /* imsg->Code (if GA_RelVerify tag) */", fp, st, NULL);
                  break;
                }
              st = GenerateString("      break;", fp, st, NULL);
              }
            }
          st = GenerateString("    }", fp, st, NULL);
          }
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MOUSEMOVE)
        {
        sprintf(string, "BOOL Window_%s_MOUSEMOVE(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  /* WARNING!  WARNING!  WARNING!  WARNING!  WARNING!  */", fp, st, NULL);
        st = GenerateString("  /* The following code is to be used if the MOUSEMOVE */", fp, st, NULL);
        st = GenerateString("  /* message is only coming from Gadtools, not from    */", fp, st, NULL);
        st = GenerateString("  /* using WA_ReportMouse or calling ReportMouse()     */", fp, st, NULL);
        st = GenerateString("  /* since the IAddress only contains a gadget pointer */", fp, st, NULL);
        st = GenerateString("  /* if from Gadtools.                                 */", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        st = GenerateString("#ifdef NOREPORTMOUSE", fp, st, NULL);

        if(userdataflag)
          {
          st = GenerateString("  struct Gadget *gadget;", fp, st, NULL);
          st = GenerateString("  TMOBJECTDATA *tmobjectdata;", fp, st, NULL);
          st = GenerateString("  BOOL (*eventfunc)(struct TMData *, struct IntuiMessage *, TMOBJECTDATA *);", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("  gadget = (struct Gadget *)imsg->IAddress;", fp, st, NULL);
          st = GenerateString("  tmobjectdata = (TMOBJECTDATA *)(gadget->UserData);", fp, st, NULL);
          st = GenerateString("  eventfunc = tmobjectdata->EventFunc;", fp, st, NULL);
          st = GenerateString("", fp, st, NULL);
          st = GenerateString("  if(eventfunc)", fp, st, NULL);
          st = GenerateString("    {", fp, st, NULL);
          st = GenerateString("    return((*eventfunc)(TMData, imsg, tmobjectdata));", fp, st, NULL);
          st = GenerateString("    }", fp, st, NULL);
          }
        else
          {
          st = GenerateString("  switch(((struct Gadget *)imsg->IAddress)->GadgetID)", fp, st, NULL);
          st = GenerateString("    {", fp, st, NULL);

          for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
              newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
            {
            if(newgadgetnode->Kind == SCROLLER_KIND ||
               newgadgetnode->Kind == SLIDER_KIND)
              {
              if(newgadgetnode->GadgetText[0] != '\0')
                sprintf(string, "    case ID_%s: /* %s */", newgadgetnode->SourceLabel, newgadgetnode->GadgetText);
              else
                sprintf(string, "    case ID_%s:", newgadgetnode->SourceLabel);
              st = GenerateString(string, fp, st, NULL);
              switch(newgadgetnode->Kind)
                {
                case SCROLLER_KIND:
                  st = GenerateString("      /* imsg->Code */", fp, st, NULL);
                  break;

                case SLIDER_KIND:
                  st = GenerateString("      /* imsg->Code */", fp, st, NULL);
                  break;
                }
              st = GenerateString("      break;", fp, st, NULL);
              }
            }
          st = GenerateString("    }", fp, st, NULL);
          }

        st = GenerateString("#endif", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MOUSEBUTTONS)
        {
        sprintf(string, "BOOL Window_%s_MOUSEBUTTONS(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  switch(imsg->Code)", fp, st, NULL);
        st = GenerateString("    {", fp, st, NULL);
        st = GenerateString("    case SELECTDOWN:", fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("    case SELECTUP:", fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("    case MENUDOWN:", fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("    case MENUUP:", fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("    }", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_CHANGEWINDOW)
        {
        sprintf(string, "BOOL Window_%s_CHANGEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_SIZEVERIFY)
        {
        sprintf(string, "BOOL Window_%s_SIZEVERIFY(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_NEWSIZE)
        {
        sprintf(string, "BOOL Window_%s_NEWSIZE(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_REQSET)
        {
        sprintf(string, "BOOL Window_%s_REQSET(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_VANILLAKEY)
        {
        sprintf(string, "BOOL Window_%s_VANILLAKEY(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  switch(imsg->Code)", fp, st, NULL);
        st = GenerateString("    {", fp, st, NULL);
        st = GenerateString("    case 'A':", fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("    }", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_RAWKEY)
        {
        sprintf(string, "BOOL Window_%s_RAWKEY(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  switch(imsg->Code)", fp, st, NULL);
        st = GenerateString("    {", fp, st, NULL);
        st = GenerateString("    case 0:", fp, st, NULL);
        st = GenerateString("      break;", fp, st, NULL);
        st = GenerateString("    }", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_REQVERIFY)
        {
        sprintf(string, "BOOL Window_%s_REQVERIFY(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_REQCLEAR)
        {
        sprintf(string, "BOOL Window_%s_REQCLEAR(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MENUVERIFY)
        {
        sprintf(string, "BOOL Window_%s_MENUVERIFY(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_NEWPREFS)
        {
        sprintf(string, "BOOL Window_%s_NEWPREFS(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_DISKINSERTED)
        {
        sprintf(string, "BOOL Window_%s_DISKINSERTED(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_DISKREMOVED)
        {
        sprintf(string, "BOOL Window_%s_DISKREMOVED(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_ACTIVEWINDOW)
        {
        sprintf(string, "BOOL Window_%s_ACTIVEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_INACTIVEWINDOW)
        {
        sprintf(string, "BOOL Window_%s_INACTIVEWINDOW(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_DELTAMOVE)
        {
        sprintf(string, "BOOL Window_%s_DELTAMOVE(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_INTUITICKS)
        {
        sprintf(string, "BOOL Window_%s_INTUITICKS(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_IDCMPUPDATE)
        {
        sprintf(string, "BOOL Window_%s_IDCMPUPDATE(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }

      if(windownode->IDCMP & IDCMP_MENUHELP)
        {
        sprintf(string, "BOOL Window_%s_MENUHELP(struct TMData *TMData, struct IntuiMessage *imsg)", windownode->SourceLabel);
        st = GenerateString(string, fp, st, NULL);
        st = GenerateString("  {", fp, st, NULL);
        st = GenerateString("  return(FALSE);", fp, st, NULL);
        st = GenerateString("  }", fp, st, NULL);
        st = GenerateString("", fp, st, NULL);
        }
      }

    if(arexxflag)
      {
      st = GenerateString("BOOL ARexxMessage(struct TMData *TMData, struct RexxMsg *RexxMsg)", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  int  count;", fp, st, NULL);
      st = GenerateString("  BOOL done=FALSE;", fp, st, NULL);
      st = GenerateString("  UBYTE command[24];", fp, st, NULL);
      st = GenerateString("  UBYTE *msgchar;", fp, st, NULL);
      st = GenerateString("  UBYTE *comchar;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  msgchar = ARG0(RexxMsg);", fp, st, NULL);
      st = GenerateString("  comchar = command;", fp, st, NULL);
      st = GenerateString("  count   = 0;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  while((*msgchar!=' ')&&(*msgchar!='\\0')&&(count<23))", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    *comchar++ = *msgchar++;", fp, st, NULL);
      st = GenerateString("    count++;", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  *comchar = '\\0';", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  /* Stricmp() is a locale aware case-insensitive string compare in utility.library */", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  if(!Stricmp(\"QUIT\", command))", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    done = TRUE;", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("  else", fp, st, NULL);
      st = GenerateString("    {", fp, st, NULL);
      st = GenerateString("    TMData->ARexxError = \"Unknown command\";", fp, st, NULL);
      st = GenerateString("    TMData->ARexxErrorLevel = 20;", fp, st, NULL);
      st = GenerateString("    }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("  return(done);", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    if(usersignalflag)
      {
      st = GenerateString("BOOL UserSignal(struct TMData *TMData, ULONG signal)", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  return(FALSE);", fp, st, NULL);
      st = GenerateString("  }", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      }

    fclose(fp);

    st = UpdateIcon(fullname, "c", st);

    if(!st)
      {
      DeleteFile(fullname);
      DeleteIcon(fullname);
      }
    }
  else
    {
    st = FALSE;
    }

  DEBUGEXIT(TRUE,st);
  return(st);
  }

#endif
