#include <janus/janus.h>
#include <stdio.h>

struct JanusBase *JanusBase =0;

void PrintMemHeader();
void PrintJanusBase();
void PrintJanusAmiga();
void main();
void main()
{
   struct JanusAmiga *japtr;
   char  buff[255];
   long  choice;

   if((JanusBase=(struct JanusBase *)OpenLibrary("janus.library",0))==0)
   {
      printf("Unable to open Janus.Library!\n");
      return;
   }
   japtr=(struct JanusAmiga *)JanusBase->jb_ParamMem;
   japtr=(struct JanusAmiga *)MakeWordPtr(japtr);

   for(;;)
   {

      printf("\nJPeek - Services programming utility\n");
      printf("\nOptions:\n");
      printf("1 - Print out JanusBase  structure\n");
      printf("2 - Print out JanusAmiga structure\n");
      printf("3 - Print out Parameter JanusMemHead structure\n");
      printf("4 - Print out Buffer    JanusMemHead structure\n");
      printf("5 - Quit JPeek\n");

      scanf("%d\n",&choice);
      gets(buff);

      switch(choice)
      {
         case 1:
                  PrintJanusBase(JanusBase);
                  printf("Return to continue:\n");
                  gets(buff);
                  break;
         case 2:
                  PrintJanusAmiga(japtr);
                  printf("Return to continue:\n");
                  gets(buff);
                  break;
         case 3:
                  printf("Parameter:\n");
                  PrintMemHeader(&(japtr->ja_ParamMem));
                  printf("Return to continue:\n");
                  gets(buff);
                  break;
         case 4:
                  printf("Buffer:\n");
                  PrintMemHeader(&(japtr->ja_BufferMem));
                  printf("Return to continue:\n");
                  gets(buff);
                  break;
         case 5:
                  goto cleanup;
      }
   }

cleanup:
   if(JanusBase) CloseLibrary(JanusBase);
   return;
}
void PrintMemHeader(p)
struct JanusMemHead *p;
{
   struct JanusMemHead *pb,*pw;

   pb=(struct JanusMemHead *)MakeBytePtr(p);
   pw=(struct JanusMemHead *)MakeWordPtr(p);

   printf("jmh_Lock        = %x\n",pb->jmh_Lock);
   printf("jmh_pad0        = %x\n",pb->jmh_pad0);
   printf("jmh_68000Base   = %x\n",pw->jmh_68000Base);
   printf("jmh_8088Segment = %x\n",pw->jmh_8088Segment);
   printf("jmh_First       = %x\n",pw->jmh_First);
   printf("jmh_Max         = %d\n",pw->jmh_Max);
   printf("jmh_Free        = %d\n",pw->jmh_Free);
}
void PrintJanusBase(p)
struct JanusBase *p;
{
   printf("JanusBase->jb_IntReq      = 0x%08lX\n",p->jb_IntReq);
   printf("JanusBase->jb_IntEna      = 0x%08lX\n",p->jb_IntEna);
   printf("JanusBase->jb_ParamMem    = 0x%08lX\n",p->jb_ParamMem);
   printf("JanusBase->jb_IoBase      = 0x%08lX\n",p->jb_IoBase);
   printf("JanusBase->jb_ExpanBase   = 0x%08lX\n",p->jb_ExpanBase);
   printf("JanusBase->jb_ExecBase    = 0x%08lX\n",p->jb_ExecBase);
   printf("JanusBase->jb_DosBase     = 0x%08lX\n",p->jb_DOSBase);
   printf("JanusBase->jb_SegList     = 0x%08lX\n",p->jb_SegList);

   printf("JanusBase->jb_KeyboardRegisterOffset = 0x%04lX\n",p->jb_KeyboardRegisterOffset);
   printf("JanusBase->jb_ATFlag      = 0x%04lX\n",p->jb_ATFlag);
   printf("JanusBase->jb_ATOffset    = 0x%04lX\n",p->jb_ATOffset);
   printf("JanusBase->jb_ServiceBase = 0x%08lX\n",p->jb_ServiceBase);
}
void PrintJanusAmiga(p)
struct JanusAmiga *p;
{
   struct JanusAmiga *pb, *pw;

   pb=(struct JanusAmiga *)MakeBytePtr(p);
   pw=(struct JanusAmiga *)MakeWordPtr(p);

   printf("JanusAmiga->ja_Lock          = 0x%02lX\n",pb->ja_Lock);
   printf("JanusAmiga->ja_8088Go        = 0x%02lX\n",pb->ja_8088Go);
   printf("JanusAmiga->ja_Interrupts    = 0x%04lX\n",pw->ja_Interrupts);
   printf("JanusAmiga->ja_Parameters    = 0x%04lX\n",pw->ja_Parameters);
   printf("JanusAmiga->ja_NumInterrupts = 0x%04lX\n",pw->ja_NumInterrupts);
   printf("JanusAmiga->ja_AmigaState    = 0x%04lX\n",pw->ja_AmigaState);
   printf("JanusAmiga->ja_PCState       = 0x%04lX\n",pw->ja_PCState);
   printf("JanusAmiga->ja_JLibRev       = 0x%04lX\n",pw->ja_JLibRev);
   printf("JanusAmiga->ja_JLibVer       = 0x%04lX\n",pw->ja_JLibVer);
   printf("JanusAmiga->ja_JHandlerRev   = 0x%04lX\n",pw->ja_JHandlerRev);
   printf("JanusAmiga->ja_JHandlerVer   = 0x%04lX\n",pw->ja_JHandlerVer);
   printf("JanusAmiga->ja_HandlerLoaded = 0x%04lX\n",pw->ja_HandlerLoaded);
}
