; /*
lc -v -j73 -cus -M -O avail
blink avail.o nd
protect avail +p
quit
*/

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* |_o_o|\\ Copyright (c) 1988 The Software Distillery.  All Rights Reserved */
/* |. o.| || This program may not be distributed without the permission of   */
/* | .  | || the authors:                                          BBS:      */
/* | o  | ||   Gordon Keener    John Toebes                  (919)-471-6436  */
/* |  . |//                                                                  */
/* ======                                                                    */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/* Avail.c - Remotely related to the original version found on 1.2 Gamma     */
/*           Toolkit disk.                                                   */
/*                                                                           */
/*  To compile, simply execute the C program as an execute script            */
/*    SHAR eat your heart out...                                             */
/*                                                                           */
/*  Requires Lattice 5.0 and does not use any assembler startup...           */
/*   Runs ONLY from CLI, accepts command line arguments EXACTLY like the 1.3   */
/*   version.                                                                */
/*                                                                           */
/*  In doing this we might have just broken every rule in the book as well   */
/*   as made up several new tricks along the way.  But then again, what can  */
/*   you say for a 604 byte RESIDENT module COMPLETELY written in C?         */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/execbase.h>
#include <libraries/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>

void RawDoFmt(char *, long *, void (*)(), char *);
#pragma syscall RawDoFmt 20a ba9804

void __regargs prbuf(char c);
#define R_A3 (8+3)

#define ABSEXECBASE ((struct ExecBase **)4L)

void __asm avail(register __a0 char *cmd)
{
   struct MemHeader  *mem;
   long              *pptr;
   ULONG              max1, max2;
   ULONG              avail1, avail2;
   ULONG              largest1, largest2;
   char               obuf[200];
   long               parray[13];
   char               c;
   struct DosLibrary *DOSBase;
   char              *txt;

   while((c = (*cmd++|0x20)) == ' ');
   max1 = max2 = 0;

   if (!(DOSBase = (struct DosLibrary *)OpenLibrary("dos.library",0)))
      return;

   Forbid ();
   for (mem = (struct MemHeader *)(*ABSEXECBASE)->MemList.lh_Head;
        mem->mh_Node.ln_Succ;
        mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
      {
      if (mem -> mh_Attributes & MEMF_CHIP)
         max1 += ((ULONG) mem -> mh_Upper - (ULONG) mem -> mh_Lower);
      /* Just in case we have something that is both chip and fast... */
      if (mem -> mh_Attributes & MEMF_FAST)
         max2 += ((ULONG) mem -> mh_Upper - (ULONG) mem -> mh_Lower);
      }

   avail1   = AvailMem (MEMF_CHIP);
   largest1 = AvailMem (MEMF_CHIP | MEMF_LARGEST);

   avail2   = AvailMem (MEMF_FAST);
   largest2 = AvailMem (MEMF_FAST | MEMF_LARGEST);
   Permit ();

   pptr = parray;
   *pptr++ = avail1;

   txt = "%ld\n";

   if (c == '?')
      txt = "Usage: avail [CHIP|FAST|TOTAL]\n";
   else if (c == 't')
      parray[0] += avail2;
   else if (c == 'f')
      parray[0] = avail2;
   else if (c != 'c')
      {
      /*        ssss   dddddddd  dddddddd  dddddddd  dddddddd  */
      txt =  "Type  Available    In-Use   Maximum   Largest\n"
             "chip%11ld %9ld %9ld %9ld\n"
	     "fast%11ld %9ld %9ld %9ld\n"
             "total%10ld %9ld %9ld %9ld\n";
      *pptr++ = max1-avail1;
      *pptr++ = max1;
      *pptr++ = largest1;

      *pptr++ = avail2;
      *pptr++ = max2-avail2;
      *pptr++ = max2;
      *pptr++ = largest2;
      
      *pptr++ = avail1+avail2;
      *pptr++ = (max1+max2)-(avail1+avail2);
      *pptr++ = max1+max2;
      max1 = largest1;
      if (largest2 > max1) max1 = largest2;
      *pptr++ = max1;
      }

   RawDoFmt(txt, parray, prbuf, obuf);
   Write(Output(), obuf, strlen(obuf));
   CloseLibrary((struct Library *)DOSBase);
}

/*----------------------------------------------------------------*/
/* This stub routine is called from the RawDoFmt routine for each */
/* character in the string.  At invocation, we have:              */
/*   D0 - next character to be formatted                          */
/*   A3 - pointer to data buffer                                  */
/*----------------------------------------------------------------*/
void __regargs prbuf(char c)
{
char *p = (char *)__builtin_getreg(R_A3);
*p++ = c;
__builtin_putreg(R_A3, p);
}
