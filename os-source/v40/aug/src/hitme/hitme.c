/*****************************************/
/*                                       */
/* COPYRIGHT (c) 1991 SAS Institute, Inc */
/*                    SAS Campus Drive   */
/*                    Cary, NC 27513     */
/*                    USA                */
/*                                       */
/* Written by Doug Walker                */
/* bix: djwalker  portal: djwalker       */
/* usenet: walker@unx.sas.com            */
/*                                       */
/* Please do not redistribute any        */
/* modified versions of this source      */
/* without permission from the copyright */
/* holder!                               */
/*                                       */
/*****************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libraries/dosextens.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/dos_protos.h>
#include <pragmas/dos_pragmas.h>

#include "hitme_rev.h"

extern struct Library *DOSBase;

#define HUNKHEAD " num hunk         size filename         punit    type         base  alv"

void gethead(char *buf, int size, FILE *fp);
int doomd(char *oname, char *cname, long targoff);

char buf[300];

#define WORKFILE "ram:hitme.tmp" VERSTAG

char *omdfile = WORKFILE;
char *cfiles = "";
char *ofiles = "";

int keepfile;

#define EMSG(x) fprintf(stderr, x);

void main(int argc, char **argv)
{
   FILE *fp;
   char *p;
   char *cmdname;
   char fname[2][50], punit[2][50], type[2][10];
   int hunk, cur, base, lbase, offset, toggle;
   int rc = 69;

   if(argc > 0) cmdname = argv[0];
   else cmdname = NULL;

   if(argc < 3)
   {
      usage:
      if(cmdname)
      {
         fprintf(stderr, "USAGE: %s <options> <mapfilename> <offset> [<hunk>]\n",
            cmdname);
         EMSG("   <options> is one or more of\n");
         EMSG("      -n         Do not run OMD\n");
         EMSG("      -d         Keep OMD output\n");
         EMSG("      -d<file>   Keep OMD output in <file>\n");
         EMSG("      -t         Delete OMD output (default)\n");
         EMSG("      -t<file>   Use <file> for OMD output, then delete\n");
         EMSG("      -o<dir>    Directory for object files (default current directory)\n");
         EMSG("      -c<dir>    Directory for C source files (default current directory)\n");
         EMSG("   <mapfilename> is a BLINK-produced map of your executable\n");
         EMSG("   <offset>      is the offset reported by ENFORCER\n");
         EMSG("   <hunk>        is the hunk reported by ENFORCER (default 0)\n");
      }
      exit(99);
   }

   while(argv[1][0] == '-')
   {
      switch(argv[1][1])
      {
         case 'c': case 'C':  /* C source file path */
            cfiles = argv[1]+2;
            break;

         case 'o': case 'O':  /* Object file path */
            ofiles = argv[1]+2;
            break;

         case 'd': case 'D':  /* Disassembly output */
            keepfile = 1;
            if(argv[1][2]) omdfile = argv[1]+2;
            break;

         case 't': case 'T':  /* Temp file name */
            keepfile = 0;
            if(argv[1][2]) omdfile = argv[1]+2;
            break;

         case 'n': case 'N':
            keepfile = 0;
            omdfile = NULL;
            break;
      }
      argv++, argc--;
   }

   if(argc < 2) goto usage;

   if(!(fp = fopen(argv[1], "r")))
   {
      fprintf(stderr, "ERROR: Can't open file '%s'\n", argv[1]);
      exit(89);
   }

   stch_i(argv[2], &offset);

   if(argc >= 4)
      stcd_i(argv[3], &hunk);
   else
      hunk = 0;

   buf[0] = 0x0c;  /* Force a gethead call */
   do
   {
      if(buf[0] == 0x0c) gethead(buf, sizeof(buf)-1, fp);

      for(p=buf; *p == ' '; p++);

      if(stcd_i(p, &cur) && cur == hunk)
      {
         /* Bingo - found the right hunk */
         base = 0;
         toggle = 0;
         do
         {
            if(buf[0] == 0x0c) gethead(buf, sizeof(buf)-1, fp);

            for(p=buf; *p == ' '; p++);

            if(stcd_i(buf, &cur) && cur != hunk) break;  /* New hunk */
            lbase = base;
            sscanf(buf+23, "%s %s %s %lx",
               fname[toggle], punit[toggle], type[toggle], &base);
            toggle = !toggle;
         }
         while(base < offset && fgets(buf, sizeof(buf)-1, fp));

         if(base < offset)
         {
            fprintf(stderr, "ERROR: Can't find offset 0x%08lx in hunk %d\n",
               offset, hunk);
            rc = 79;
            goto endit;
         }
         if(base == offset)
         {
            toggle = !toggle;
            lbase = base;
         }

         offset -= lbase;

         if(strcmp(type[toggle], "code") ||
            doomd(fname[toggle], punit[toggle], offset))
         {
            printf("Offset 0x%08lx file '%s' punit '%s' type %s\n",
               offset, fname[toggle], punit[toggle], type[toggle]);
            if(keepfile) printf("No OMD output available\n");
         }
         rc = 0;
         goto endit;
      }
   }
   while(fgets(buf, sizeof(buf)-1, fp));

   fprintf(stderr, "Can't find hunk %d\n", hunk);

endit:
   if(fp) fclose(fp);

   exit(rc);
}

void gethead(char *buf, int size, FILE *fp)
{
   while(fgets(buf, size, fp))
   {
      if(!memcmp(buf, HUNKHEAD, strlen(HUNKHEAD)))
         break;
   }
   fgets(buf, size, fp);
}


int doomd(char *oname, char *cname, long targoff)
{
   BPTR ofp, ifp;
   FILE *fp;
   char *p;
   int lno, offset;
   int rc;

   lno = -1;

   if(!omdfile) return(1);

   if(!(ofp = Open(omdfile, MODE_NEWFILE)))
   {
      errfile:
      fprintf(stderr, "ERROR: Can't open file '%s'\n", omdfile);
      return(1);
   }

   ifp = Open("NIL:", MODE_NEWFILE);

   sprintf(buf, "LC:OMD %s%s%s %s%s%s", ofiles, (ofiles[0] ? "/" : ""), oname,
                                        cfiles, (cfiles[0] ? "/" : ""), cname);
   Execute(buf, ifp, ofp);

   Close(ofp);
   if(ifp) Close(ifp);

   if(!(fp = fopen(omdfile, "r"))) goto errfile;

   rc = 1;
   while(fgets(buf, sizeof(buf)-1, fp))
   {
      if(buf[0] == ';')
      {
         for(p=buf+1; *p == ' '; p++);
         stcd_i(p, &lno);
      }
      else if(buf[7] == '|')
      {
         if(lno < 0) break;   /* No debug */

         stch_i(buf+9, &offset);
         if(offset >= targoff)
         {
            printf("Line %d file '%s'\n", lno, cname);
            rc = 0;
            break;
         }
      }
   }

   fclose(fp);

   if(!keepfile) DeleteFile(omdfile);
   else if(!rc) printf("OMD output is in '%s'\n", omdfile);

   return(rc);
}
