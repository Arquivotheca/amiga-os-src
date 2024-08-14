
#ifndef DEMO

#include "Toolmaker.h"
#include "Externs.h"

#include <exec/memory.h>
#include <libraries/asl.h>
#include <dos/dostags.h>
#include <utility/date.h>
#include <workbench/workbench.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include <clib/dos_protos.h>
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <clib/alib_protos.h>
#include <clib/utility_protos.h>
#include <clib/icon_protos.h>

/****************************************************************************/

void GenerateCode(void)
  {
  int    i;
  BOOL   status;
  BOOL   abortgen=FALSE;
  BOOL   failed;
  char   **stringpointer;
  char   *laberr;
  struct WindowNode    *windownode;
  struct NewGadgetNode *newgadgetnode;
  struct MenuNode      *menunode;
  struct ItemNode      *itemnode;
  struct SubNode       *subnode;
  struct TagNode       *tagnode;
  struct StringNode    *stringnode;
  struct StringNode    *labelnode;

  DEBUGENTER("GenerateCode", NULL);

  MainWindowSleep();

  if(filename[0] == '\0')
    {
    abortgen = TRUE;
    if(!PutError("Unable to generate code until\nproject is saved\n(generated files use project filename)", "Continue|Abort")) goto abort;
    }

  /************** Check labels **************/

  if(!screennode.Workbench && (laberr=InvalidLabel(screennode.SourceLabel)))
    {
    abortgen = TRUE;
    sprintf(string, "%s\nfor screen '%s'", laberr, screennode.Title);
    if(!PutError(string, "Continue|Abort")) goto abort;
    }

  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    if(laberr=InvalidLabel(windownode->SourceLabel))
      {
      abortgen = TRUE;
      sprintf(string, "%s\nfor window '%s'", laberr, windownode->Title);
      if(!PutError(string, "Continue|Abort")) goto abort;
      }

    for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
        newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
      {
      if(laberr=InvalidLabel(newgadgetnode->SourceLabel))
        {
        abortgen = TRUE;
        sprintf(string, "%s\nfor gadget '%s'", laberr, newgadgetnode->GadgetText);
        if(!PutError(string, "Continue|Abort")) goto abort;
        }

      for(tagnode = (struct TagNode *) newgadgetnode->TagList.lh_Head; tagnode->Node.ln_Succ;
          tagnode = (struct TagNode *) tagnode->Node.ln_Succ)
        {
        switch(tagnode->Node.ln_Type)
          {
          case TYPE_STRINGLIST:
            stringpointer = (char **) tagnode->Data;
            labelnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head;
            for(i=0; i<tagnode->DataCount; i++)
              {
              if(laberr=InvalidLabel(labelnode->String))
                {
                sprintf(string, "%s\nfor gadget '%s',\ntext '%s'", laberr, newgadgetnode->GadgetText, *stringpointer);
                if(!PutError(string, "Continue|Abort"))
                  {
                  abortgen = TRUE;
                  goto abort;
                  }
                }
              stringpointer++;
              labelnode = (struct StringNode *) labelnode->Node.ln_Succ;
              }
            break;

          case TYPE_LINKEDLIST:
            labelnode = (struct StringNode *) tagnode->SourceLabelList.lh_Head;
            for(stringnode = (struct StringNode *) ((struct List *) tagnode->Data)->lh_Head; stringnode->Node.ln_Succ;
                stringnode = (struct StringNode *) stringnode->Node.ln_Succ)
              {
              if(laberr=InvalidLabel(labelnode->String))
                {
                sprintf(string, "%s\nfor gadget '%s',\ntext '%s'", laberr, newgadgetnode->GadgetText);
                if(!PutError(string, "Continue|Abort"))
                  {
                  abortgen = TRUE;
                  goto abort;
                  }
                }
              labelnode = (struct StringNode *) labelnode->Node.ln_Succ;
              }
            break;
          }
        }
      }

    for(menunode = (struct MenuNode *) windownode->MenuList.lh_Head; menunode->Node.ln_Succ;
        menunode = (struct MenuNode *) menunode->Node.ln_Succ)
      {
      if(laberr=InvalidLabel(menunode->SourceLabel))
        {
        abortgen = TRUE;
        sprintf(string, "%s\nfor menu '%s'", laberr, menunode->MenuText);
        if(!PutError(string, "Continue|Abort")) goto abort;
        }

      for(itemnode = (struct ItemNode *) menunode->ItemList.lh_Head; itemnode->Node.ln_Succ;
          itemnode = (struct ItemNode *) itemnode->Node.ln_Succ)
        {
        if(itemnode->ItemText[0]!='-' && (laberr=InvalidLabel(itemnode->SourceLabel)))
          {
          abortgen = TRUE;
          sprintf(string, "%s\nfor menu item '%s'", laberr, itemnode->ItemText);
          if(!PutError(string, "Continue|Abort")) goto abort;
          }

        for(subnode = (struct SubNode *) itemnode->SubList.lh_Head; subnode->Node.ln_Succ;
            subnode = (struct SubNode *) subnode->Node.ln_Succ)
          {
          if(subnode->SubText[0]!='-' && (laberr=InvalidLabel(subnode->SourceLabel)))
            {
            abortgen = TRUE;
            sprintf(string, "%s\nfor menu subitem '%s'", laberr, subnode->SubText);
            if(!PutError(string, "Continue|Abort")) goto abort;
            }
          }
        }
      }
    }

  abort:  /* Jump to here if Abort gadget selected */

  failed = TRUE;

  if(abortgen)
    {
    PutError("Code generation aborted", "OK");
    }
  else
    {
    status = TRUE;
    while(!GenerateRevision() && status)
      status = PutError("Error bumping revision", "Retry|Abort");

    if(status)
      {
      status = TRUE;
      while(!BuildTextAttrList() && status)
        status = PutError("Out of memory", "Retry|Abort");

      if(status)
        {
        status = TRUE;
        while(!GenerateSource() && status)
          status = PutError("Error creating\nToolmaker source file (_tm.c)", "Retry|Abort");

        if(status)
          {
          status = TRUE;
          while(!GenerateHeader() && status)
            status = PutError("Error creating\nToolmaker header file (_tm.h)", "Retry|Abort");

          if(status)
            {
            status = TRUE;
            while(!GenerateTemplate() && status)
              status = PutError("Error creating\nuser source file (.c.new)", "Retry|Abort");

            if(status)
              {
              status = TRUE;
              while(!GenerateText() && status)
                status = PutError("Error creating\nToolmaker text file (_tm_text.h)", "Retry|Abort");

              if(status)
                {
                status = TRUE;
                while(!GenerateUserHeader() && status)
                  status = PutError("Error creating\nuser header file (.h)", "Retry|Abort");

                if(status)
                  {
                  status = TRUE;
                  while(!GenerateUserTextHeader() && status)
                    status = PutError("Error creating\nuser text header file (_text.h)", "Retry|Abort");

                  if(status)
                    {
                    status = TRUE;
                    while(!GenerateMakefile() && status)
                      status = PutError("Error creating\nMakefile", "Retry|Abort");

                    if(status)
                      {
                      status = TRUE;
                      while(!GenerateMakefileScript() && status)
                        status = PutError("Error creating\nDoMake script", "Retry|Abort");

                      if(status)
                        {
                        status = TRUE;
                        while(arexxflag && !CopyDefaultFile("SimpleRexx_tm.h") && status)
                          status = PutError("Error copying\nSimpleRexx_tm.h\nfrom Defaults drawer", "Retry|Abort");

                        if(status)
                          {
                          status = TRUE;
                          while(arexxflag && !CopyDefaultFile("SimpleRexx_tm.c") && status)
                            status = PutError("Error copying\nSimpleRexx_tm.c\nfrom Defaults drawer", "Retry|Abort");

                          if(status)
                            {
                            status = TRUE;
                            while(userdataflag && !GenerateEventsFile() && status)
                              status = PutError("Error creating\nevents file (_events.c.new)", "Retry|Abort");

                            if(status)
                              {
                              WriteExeIcon();
                              failed = FALSE;
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }

        KillTextAttrList(&TextAttrList);
        }
      }

    if(failed) PutError("Code generation failed", "OK");
    }

  MainWindowWakeUp();

  DEBUGEXIT(FALSE, NULL);
  }

/****************************************************************************/

BOOL GenerateSource(void)
  {
  FILE *fp;
  BOOL st=TRUE;

  DEBUGENTER("GenerateSource", NULL);

  sprintf(fullname, "%s_tm.c", filename);

  if(fp = fopen(fullname, "wb"))
    {
    st = GenerateString("", fp, st, NULL);
    sprintf(string, "/* Code generated by %s V%s */", title, version);
    st = GenerateString(string, fp, st, NULL);
    st = GenerateString("", fp, st, NULL);
    st = GeneratePrecode(fp, st);
    st = GenerateRequest(fp, st);
    st = GenerateOpen(fp, st);
    st = GenerateClose(fp, st);
    st = GenerateEventLoop(fp, st);
    st = GenerateScreen(fp, st);
    st = GenerateWindow(fp, st);

    st = GenerateString(NULL, fp, st, "***********************************************************");
    st = GenerateString(NULL, fp, st, "Static functions (available only to functions in this file)");
    st = GenerateString(NULL, fp, st, "***********************************************************");
    st = GenerateString(NULL, fp, st, "");
    st = GenerateWindowSignal(fp, st);
    st = GenerateWindowIDCMP(fp, st);
    st = GenerateARexxSignal(fp, st);
    st = GenerateRemoveWindow(fp, st);

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

  DEBUGEXIT(TRUE, st);
  return(st);
  }

/****************************************************************************/

BOOL GenerateUserHeader(void)
  {
  BOOL st=TRUE;
  FILE *fp;

  DEBUGENTER("GenerateUserHeader", NULL);

  sprintf(fullname, "%s.h", filename);

  if(fp = fopen(fullname, "r"))
    {
    fclose(fp);
    }
  else
    {
    if(fp = fopen(fullname, "w"))
      {
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("/* User header file */", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      st = GenerateString(NULL, fp, st, "NewGadget/NewMenu UserData structure");
      st = GenerateString(NULL, fp, st, "");
      st = GenerateString("struct TMObjectData", fp, st, NULL);
      st = GenerateString("  {", fp, st, NULL);
      st = GenerateString("  BOOL (* EventFunc)();", fp, st, "Function to call");
      st = GenerateString("  };", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("typedef struct TMObjectData TMOBJECTDATA;", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      sprintf(string,     "#include \"%s_tm.h\"", savefilename);
      st = GenerateString(string, fp, st, "Toolmaker header file");
      sprintf(string,     "#include \"%s_text.h\"", savefilename);
      st = GenerateString(string, fp, st, "User text header file");
      st = GenerateString("", fp, st, NULL);

      st = GenerateString(NULL, fp, st, "Function prototypes");
      st = GenerateString(NULL, fp, st, "");
      st = GenerateString("VOID  wbmain(VOID *);", fp, st, "for DICE compatibility");
      st = GenerateString("VOID  main(int, char **);", fp, st, NULL);
      st = GenerateString("VOID  cleanexit(struct TMData *, int);", fp, st, NULL);
      st = GenerateString("UBYTE *getfilename(struct TMData *, UBYTE *, UBYTE *, ULONG, struct Window *, BOOL);", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      fclose(fp);

      st = UpdateIcon(fullname, "h", st);
      }
    else
      {
      st = FALSE;
      }
    }

  DEBUGEXIT(TRUE, st);
  return(st);
  }

/****************************************************************************/

BOOL GenerateUserTextHeader(void)
  {
  BOOL st=TRUE;
  FILE *fp;

  DEBUGENTER("GenerateUserTextHeader", NULL);

  sprintf(fullname, "%s_text.h", filename);

  if(fp = fopen(fullname, "r"))
    {
    fclose(fp);
    }
  else
    {
    if(fp = fopen(fullname, "w"))
      {
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("/* User text header file */", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);
      st = GenerateString("#define TEXT_OK\t\"OK\"", fp, st, NULL);
      st = GenerateString("#define TEXT_ERROR\t\"Error\"", fp, st, NULL);
      st = GenerateString("#define TEXT_ABORT\t\"Abort\"", fp, st, NULL);
/*
      st = GenerateString("#define TEXT_CONTQUIT\t\"Continue|Quit\"", fp, st, "Contains | to separate gadget strings");
*/
      st = GenerateString("#define TEXT_NORELEASE2\t\"Requires 2.0 or higher\"", fp, st, NULL);
      st = GenerateString("#define TEXT_NOLIBRARY\t\"Error opening library\\n%s\"", fp, st, "Contains %s for library name");
      st = GenerateString("#define TEXT_NOMEMORY\t\"Out of memory\"", fp, st, NULL);
      st = GenerateString("#define TEXT_NOMSGPORT\t\"Error creating\\nmessage port\"", fp, st, NULL);
      st = GenerateString("#define TEXT_NOFONT\t\"Error opening font\\n%s %ld\"", fp, st, "Contains %s and %d for font name and size");
      st = GenerateString("#define TEXT_NOFILEREQ\t\"Error opening\\nasl file requester\"", fp, st, NULL);
      st = GenerateString("#define TEXT_NOSCREEN\t\"Error opening\\nscreen\"", fp, st, NULL);
      st = GenerateString("#define TEXT_NOWINDOW\t\"Error opening\\nwindow\"", fp, st, NULL);
      st = GenerateString("", fp, st, NULL);

      fclose(fp);

      st = UpdateIcon(fullname, "h", st);
      }
    else
      {
      st = FALSE;
      }
    }

  DEBUGEXIT(TRUE, st);
  return(st);
  }

/****************************************************************************/

BOOL GenerateRevision(void)
  {
  ULONG currentver=1;
  ULONG currentrev=0;
  BOOL success=TRUE;
  FILE *fp;

  DEBUGENTER("GenerateRevision", NULL);

  sprintf(fullname, "%s_rev.h", filename);
  if(fp = fopen(fullname, "r"))
    {
    fclose(fp);

    sprintf(fullname, "%s_rev.ver", filename);
    if(fp = fopen(fullname, "r"))
      {
      fscanf(fp, "%d", &currentver);
      fclose(fp);
      }

    sprintf(fullname, "%s_rev.rev", filename);
    if(fp = fopen(fullname, "r"))
      {
      fscanf(fp, "%d", &currentrev);
      fclose(fp);
      }

    sprintf(string, "Current version: %d.%d\nChange to %d.%d?", currentver,currentrev, currentver, currentrev+1);
    if(PutError(string, "Yes|No"))
      {
      if(!CallBumpRev()) success = FALSE;
      }
    }
  else
    {
    if(!CallBumpRev()) success = FALSE;
    }

  DEBUGEXIT(TRUE, success);
  return(success);
  }

/****************************************************************************/

BOOL CallBumpRev(void)
  {
  struct ClockData clockdata;
  ULONG seconds, micros;
  LONG previousrev=-1;
  LONG currentver=1;
  LONG currentrev=0;
  BOOL success=TRUE;
  FILE *fp;

  DEBUGENTER("CallBumpRev", NULL);

  CurrentTime(&seconds, &micros);
  Amiga2Date(seconds, &clockdata);

  /* Version */

  sprintf(fullname, "%s_rev.ver", filename);
  if(fp = fopen(fullname, "r"))
    {
    fscanf(fp, "%d", &currentver);
    fclose(fp);
    }

  if(fp = fopen(fullname, "w"))
    {
    fprintf(fp, "%d", currentver);
    fclose(fp);

    success = UpdateIcon(fullname, "h", success);
    }
  else
    {
    success = FALSE;
    }

  /* Revision */

  sprintf(fullname, "%s_rev.rev", filename);
  if(fp = fopen(fullname, "r"))
    {
    fscanf(fp, "%d", &previousrev);
    fclose(fp);
    }

  currentrev = previousrev+1;
  if(fp = fopen(fullname, "w"))
    {
    fprintf(fp, "%d", currentrev);
    fclose(fp);

    success = UpdateIcon(fullname, "h", success);
    }
  else
    {
    success = FALSE;
    }

  /* Header */

  sprintf(fullname, "%s_rev.h", filename);
  if(fp = fopen(fullname, "w"))
    {
    sprintf(string, "%d.%d.%d", clockdata.mday, clockdata.month, clockdata.year%100);
    fprintf(fp, "#define VERSION \t%d\n", currentver);
    fprintf(fp, "#define REVISION\t%d\n", currentrev);
    fprintf(fp, "#define DATE    \"%s\"\n", string);
    fprintf(fp, "#define VERS    \"%s %d.%d\"\n", savefilename, currentver, currentrev);
    fprintf(fp, "#define VSTRING \"%s %d.%d (%s)\\n\\r\"\n", savefilename, currentver, currentrev, string);
    fprintf(fp, "#define VERSTAG \"\\0$");
    fprintf(fp, "VER: %s %d.%d (%s)\"\n", savefilename, currentver, currentrev, string);
    fclose(fp);

    success = UpdateIcon(fullname, "h", success);
    }
  else
    {
    success = FALSE;
    }

  DEBUGEXIT(TRUE, success);
  return(success);
  }

/****************************************************************************/

BOOL GenerateMakefile(void)
  {
  int  ch;
  BOOL retval=FALSE;
  FILE *fpsource;
  FILE *fpdest;
  struct DiskObject *diskobject;

  DEBUGENTER("GenerateMakefile", NULL);

  strcpy(string, toolpath);
  AddPart(string, "Defaults/Makefile", 1024);

  strcpy(fullname, savepathname);
  AddPart(fullname, "MakeFile", PATHSIZE+FILESIZE);

  if(fpdest = fopen(fullname, "r"))
    {
    fclose(fpdest);
    retval = TRUE;  /* Makefile already there; don't change it */
    }
  else
    {
    if(fpsource = fopen(string, "r"))
      {
      if(fpdest = fopen(fullname, "w"))
        {
        fprintf(fpdest, "\n");
        fprintf(fpdest, "# APP, CFILES, and OFILES added by %s V%s\n", title, version);
        fprintf(fpdest, "#\n");
        fprintf(fpdest, "# APP    = the application name\n");
        fprintf(fpdest, "# CFILES = source files (may include SimpleRexx_tm.c and $(APP)_events.c)\n");
        fprintf(fpdest, "# OFILES = object files (may include SimpleRexx_tm.o and $(APP)_events.o)\n");
        fprintf(fpdest, "# HFILES = header files (may include SimpleRexx_tm.h)\n");
        fprintf(fpdest, "\n");

        fprintf(fpdest, "APP\t= %s\n", savefilename);

        fprintf(fpdest, "CFILES\t= $(APP).c $(APP)_tm.c");
        if(userdataflag) fprintf(fpdest, " $(APP)_events.c");
        if(arexxflag)    fprintf(fpdest, " SimpleRexx_tm.c");
        fprintf(fpdest, "\n");

        fprintf(fpdest, "OFILES\t= $(APP).o $(APP)_tm.o");
        if(userdataflag) fprintf(fpdest, " $(APP)_events.o");
        if(arexxflag)    fprintf(fpdest, " SimpleRexx_tm.o");
        fprintf(fpdest, "\n");

        fprintf(fpdest, "HFILES\t= $(APP).h $(APP)_text.h $(APP)_tm.h $(APP)_tm_text.h");
        if(arexxflag)    fprintf(fpdest, " SimpleRexx_tm.h");
        fprintf(fpdest, "\n");

        while((ch = getc(fpsource)) != EOF) putc(ch, fpdest);

        if(iconflag)
          {
          if(diskobject = GetDiskObject(string))
            {
            PutDiskObject(fullname, diskobject);
            FreeDiskObject(diskobject);
            }
          }

        retval = TRUE;  /* No problems; new Makefile created */

        fclose(fpdest);
        }
      fclose(fpsource);
      }
    else
      {
      retval = TRUE;  /* No makefile.def found; user doesn't want makefile */
      }
    }

  DEBUGEXIT(TRUE, retval);
  return(retval);
  }

/****************************************************************************/

BOOL GenerateMakefileScript(void)
  {
  int  ch;
  BOOL retval=FALSE;
  FILE *fpsource;
  FILE *fpdest;
  struct DiskObject *diskobject;

  DEBUGENTER("GenerateMakefileScript", NULL);

  strcpy(string, toolpath);
  AddPart(string, "Defaults/DoMake", 1024);

  strcpy(fullname, savepathname);
  AddPart(fullname, "DoMake", PATHSIZE+FILESIZE);

  if(fpdest = fopen(fullname, "r"))
    {
    fclose(fpdest);
    retval = TRUE;  /* Makefile script already there; don't change it */
    }
  else
    {
    if(fpsource = fopen(string, "r"))
      {
      if(fpdest = fopen(fullname, "w"))
        {
        fprintf(fpdest, "\n");
        fprintf(fpdest, "; APP added by %s V%s\n", title, version);
        fprintf(fpdest, "\n");
        fprintf(fpdest, "Set APP \"%s\"\n", savefilename);
        fprintf(fpdest, "\n");

        while((ch = getc(fpsource)) != EOF) putc(ch, fpdest); /* copy file */

        if(iconflag)
          {
          if(diskobject = GetDiskObject(string))
            {
            PutDiskObject(fullname, diskobject);
            FreeDiskObject(diskobject);
            }
          }

        retval = TRUE;  /* No problems; new Makefile script created */

        fclose(fpdest);
        }
      fclose(fpsource);
      }
    else
      {
      retval = TRUE;  /* No makefile script found; user doesn't want one */
      }
    }

  DEBUGEXIT(TRUE, retval);
  return(retval);
  }

/****************************************************************************/

BOOL CopyDefaultFile(char *file)
  {
  int  ch;
  BOOL retval=FALSE;
  FILE *fpsource;
  FILE *fpdest;
  struct DiskObject *diskobject;

  DEBUGENTER("CopyDefaultFile", NULL);

  strcpy(string, toolpath);
  AddPart(string, "Defaults/", 1024);
  strcat(string, file);

  strcpy(fullname, savepathname);
  AddPart(fullname, file, PATHSIZE+FILESIZE);

  if(fpdest = fopen(fullname, "r"))
    {
    fclose(fpdest);
    retval = TRUE;  /* file already there; don't change it */
    }
  else
    {
    if(fpsource = fopen(string, "rb"))
      {
      if(fpdest = fopen(fullname, "wb"))
        {
        while((ch = getc(fpsource)) != EOF) putc(ch, fpdest); /* copy file */

        if(iconflag)
          {
          if(diskobject = GetDiskObject(string))
            {
            PutDiskObject(fullname, diskobject);
            FreeDiskObject(diskobject);
            }
          }

        retval = TRUE;  /* No problems; file copied */

        fclose(fpdest);
        }
      fclose(fpsource);
      }
    }

  return(retval);
  }

/****************************************************************************/

BOOL BuildTextAttrList(void)
  {
  BOOL   success=TRUE;
  BOOL   found;
  struct WindowNode    *windownode;
  struct NewGadgetNode *newgadgetnode;
  struct TextAttrNode  *textattrnode;
  struct TextAttrNode  *newtextattrnode;

  DEBUGENTER("BuildTextAttrList", NULL);

  diskfontcount = 0;
  for(windownode = (struct WindowNode *) WindowList.lh_Head; windownode->Node.ln_Succ;
      windownode = (struct WindowNode *) windownode->Node.ln_Succ)
    {
    for(newgadgetnode = (struct NewGadgetNode *) windownode->NewGadgetList.lh_Head; newgadgetnode->Node.ln_Succ;
        newgadgetnode = (struct NewGadgetNode *) newgadgetnode->Node.ln_Succ)
      {
      found = FALSE;
      for(textattrnode = (struct TextAttrNode *) TextAttrList.lh_Head; textattrnode->Node.ln_Succ;
          textattrnode = (struct TextAttrNode *) textattrnode->Node.ln_Succ)
        {
        if(!(strcmp(TextAttrName(&newgadgetnode->TextAttr), textattrnode->Name))) found = TRUE;
        }

      if(!found && (newgadgetnode->FontName[0] != '\0') &&
                   (newgadgetnode->FontName[0] != '.'))
        {
        if(newtextattrnode = AllocMem(sizeof(struct TextAttrNode), MEMF_CLEAR))
          {
          DEBUGALLOC(sizeof (struct TextAttrNode));

          strcpy(newtextattrnode->Name, TextAttrName(&newgadgetnode->TextAttr));
          strcpy(newtextattrnode->FontName, newgadgetnode->FontName);
          newtextattrnode->TextAttr.ta_Name  = newtextattrnode->FontName;
          newtextattrnode->TextAttr.ta_YSize = newgadgetnode->TextAttr.ta_YSize;
          newtextattrnode->TextAttr.ta_Style = newgadgetnode->TextAttr.ta_Style;
          newtextattrnode->TextAttr.ta_Flags = newgadgetnode->TextAttr.ta_Flags;
          AddTail(&TextAttrList, (struct Node *) newtextattrnode);
          if(!((strcmp(newtextattrnode->FontName, "topaz.font") == 0) &&
              ((newtextattrnode->TextAttr.ta_YSize == 8) ||
               (newtextattrnode->TextAttr.ta_YSize == 9))))
            {
            diskfontcount++;
            }
          }
        else
          {
          success = FALSE;
          }
        }
      }

    found = FALSE;
    for(textattrnode = (struct TextAttrNode *) TextAttrList.lh_Head; textattrnode->Node.ln_Succ;
        textattrnode = (struct TextAttrNode *) textattrnode->Node.ln_Succ)
      {
      if(!(strcmp(TextAttrName(&screennode.TextAttr), textattrnode->Name))) found = TRUE;
      }

    if(!found && (screennode.FontName[0] != '\0') &&
                 (screennode.FontName[0] != '.'))
      {
      if(newtextattrnode = AllocMem(sizeof(struct TextAttrNode), MEMF_CLEAR))
        {
        DEBUGALLOC(sizeof (struct TextAttrNode));

        strcpy(newtextattrnode->Name, TextAttrName(&screennode.TextAttr));
        strcpy(newtextattrnode->FontName, screennode.FontName);
        newtextattrnode->TextAttr.ta_Name  = newtextattrnode->FontName;
        newtextattrnode->TextAttr.ta_YSize = screennode.TextAttr.ta_YSize;
        newtextattrnode->TextAttr.ta_Style = screennode.TextAttr.ta_Style;
        newtextattrnode->TextAttr.ta_Flags = screennode.TextAttr.ta_Flags;
        AddTail(&TextAttrList, (struct Node *) newtextattrnode);
        if(!((strcmp(newtextattrnode->FontName, "topaz.font") == 0) &&
            ((newtextattrnode->TextAttr.ta_YSize == 8) ||
             (newtextattrnode->TextAttr.ta_YSize == 9))))
          {
          diskfontcount++;
          }
        }
      else
        {
        success = FALSE;
        }
      }
    }

  DEBUGEXIT(TRUE, success);
  return(success);
  }

/****************************************************************************/

char *InvalidLabel(char *label)
  {
  static char *errstring[4] =
    {
    "Label missing",
    "Label contains space(s)",
    "Label contains bad character(s)",
    "Label contains too many characters"
    };
  char *character;
  char *retval=NULL;

  DEBUGENTER("InvalidLabel", NULL);

  if(label[0] == '\0') retval = errstring[0];
  for(character=label; *character!='\0'; character++)
    {
    if(*character == ' ') retval = errstring[1];
    if(!isalnum(*character)) retval = errstring[2];
    }
  if(strlen(label) > LABELSIZE) retval = errstring[3];

  DEBUGEXIT(TRUE, retval);
  return(retval);
  }

/****************************************************************************/

BOOL GenerateString(char *line, FILE *fp, BOOL st, char *comment)
  {
  DEBUGENTER("GenerateString", st);

  if(fp && st)
    {
    if(line)
      strcpy(string2, line);
    else
      string2[0] = '\0';

    if(comment && commentflag)
      {
      if(line) strcat(string2, "\t");
      if(comment[0] != '\0')
        {
        strcat(string2, "/* ");
        strcat(string2, comment);
        strcat(string2, " */");
        }
      else
        strcat(string2, " ");
      }

    strcat(string2, "\n");

    if(string2[0] != '\n' || line != NULL)
      {
      if(fputs(string2, fp)) st = FALSE;
      }
    }

  DEBUGEXIT(TRUE, st);
  return(st);
  }

#endif /* ndef DEMO */
