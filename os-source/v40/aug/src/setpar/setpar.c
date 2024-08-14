/*
lc -d -j73 -O -o/obj/cpu.o -i/include -v -csf Cpu
blink /obj/Cpu.o to /bin/Cpu sc sd nd
protect /bin/Cpu +p
quit
*/

#include <exec/types.h>
#include <internal/commands.h>
#include "setpar_rev.h"

#include <exec/types.h>
#include <dh1:parallel/parallel.h>
#include <stdio.h>

#define DEVICE_NAME "parallel.device"
#define UNIT_NUMBER 0

#define TEMPLATE "SLOW/S,ACK/S,FAST/S" CMDREV

#define OPT_SLOW	0
#define OPT_ACK		1
#define OPT_FAST	2
#define OPT_COUNT	3

/*----------------------------------------------------------------------*/
/* Structure used by the pattern matching routines                      */
/*----------------------------------------------------------------------*/

struct uAnchor
{
   struct AnchorPath uA_AP;
   BYTE   uA_Path[256];
};

int main(void)
{
   long               opts[OPT_COUNT];
   struct DosLibrary *DOSBase;
   struct RDargs     *rdargs;
   struct uAnchor    *ua;
   int                rc;
   int                code;
   char              *curarg;    /* Current /M argument being processed */
   char             **argptr;    /* Pointer to the list of /M arguments */

   int i;
   struct MsgPort	*ParallelMP;
   struct IOExtPar	*ParallelIO;

   rc = RETURN_OK;

   if (DOSBase = (struct DosLibrary *)OpenLibrary(DOSLIB,DOSVER))
   {
      memset(opts, 0, sizeof(opts));
      rdargs = ReadArgs(TEMPLATE, opts, NULL);

      if (rdargs == NULL)
      {
         PrintFault(IoErr(), NULL);
      }
      else
      {
         if( ParallelMP=CreatePort("setpar.parport",0) )
            {
               if(ParallelIO=(struct IOExtPar *)
                  CreateExtIO(ParallelMP,sizeof(struct IOExtPar)) )
                     {
                        if (opts[OPT_SLOW]) ParallelIO->io_ParFlags = PARF_SLOWMODE;
                        if (opts[OPT_ACK]) ParallelIO->io_ParFlags = PARF_ACKMODE;
                        if (opts[OPT_FAST]) ParallelIO->io_ParFlags = PARF_FASTMODE;
                        if ( OpenDevice(DEVICE_NAME,UNIT_NUMBER,ParallelIO,0) )
                           {
                              PutStr("Warning: parallel.device did not open\n");
                              rc = RETURN_WARN;
                           }
                        else
                           {
                              ParallelIO->IOPar.io_Command = PDCMD_SETPARAMS;
                              DoIO(ParallelIO);
                              CloseDevice(ParallelIO);
                           }
                        DeleteExtIO(ParallelIO);
                     }
                   DeletePort(ParallelMP);
            }
      }
      CloseLibrary((struct Library *)DOSBase);
   }
   else
   {
      rc = RETURN_FAIL;
      OPENFAIL;
   }
   return(rc);
}

