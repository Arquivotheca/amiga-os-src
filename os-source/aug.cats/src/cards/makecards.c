/*
 * makecards.c --- Convert text file into card file
 */

#include <exec/types.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

extern LONG stdout;

/* Default template lines */

char *template[] = {
   "@TEMPLATE\n",
   "@@@@@@@|@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n",
   " ---------------------- Description ----------------------- \n",
   "@ENDTEMPLATE\n"
   };


#define MAXLINE 80

char lbuf[MAXLINE];

LONG filea, fileb, ccnt;


main(argc, argv)
int argc;
char **argv;
   {
   LONG            rLen, wLen, lpc, k;
   char            *fnamea, *fnameb;
   UBYTE  buf[4];

   if (argc==0) cleanexit("");
   if ((argc<4)||(*argv[1]=='?'))
      cleanexit("Usage: makecards infile outfile linespercard");

   fnamea = argv[1];
   fnameb = argv[2];
   lpc = atoi(argv[3]);
   if(lpc < 1)  cleanexit("Cards must have at least one line\n");

   if(!(filea = Open(fnamea, MODE_OLDFILE)))
      cleanexit("Infile not found");

   if(!(fileb = Open(fnameb, MODE_NEWFILE)))
      cleanexit("Can't open outfile");

   rLen = wLen = 1;
   *buf = NULL;

   /* First 3 lines of template */
   for(k=0; (k<3)&&(wLen>0); k++)
      {
      wLen = Write(fileb,template[k],strlen(template[k]));
      }
   /* LineN lines */
   for(k=1;( k<=lpc)&&(wLen>0); k++)
      {
      sprintf(lbuf," Line%ld\n",k);
      wLen = Write(fileb,lbuf,strlen(lbuf));
      }
   /* End of template */
   if(wLen>0)
      {
      wLen = Write(fileb,template[3],strlen(template[3]));
      wLen = Write(filea,"\n",1);
      }
   if(wLen<=0)  cleanexit("Error writing template\n");


   while((rLen > 0)&&(wLen > 0)&&(!(SetSignal(0,0)&SIGBREAKF_CTRL_C)))
      {
      /* Get to a line of text, do @ and first line */
      while((rLen = getLine(filea,lbuf))==1);
      if(rLen) wLen = Write(fileb,"\n@\n",3);
      if(wLen) wLen = putLine(fileb,lbuf,rLen);

      /* do rest of card lines */
      for(k=1; (k<lpc)&&(rLen>0)&&(wLen>0); k++)
         {
         rLen = getLine(filea,lbuf);
         if(rLen) wLen = putLine(fileb,lbuf,rLen);
         }
      if(wLen>0)
         {
         ccnt++;
         printf("\033[F\033[B%ld   ",ccnt);
         }
      }

   if(wLen < 0)  printf("Write error\n");
   if(rLen < 0)  printf("Read error\n");
   else printf("%ld cards written\n",ccnt);
   cleanup();
   }


putLine(fileb,lbuf,len)
LONG  fileb;
UBYTE *lbuf;
LONG  len;
   {
   int k;

   if(len>1)
      {
      k = len - 2;

      while((k>=0)&&((lbuf[k]==',')||(lbuf[k]==' ')))  k--;
      k++;
      lbuf[k] = '\n';
      len = k + 1;
      }
   return(Write(fileb,lbuf,len));
   }

getLine(file,buf)
LONG file;
UBYTE *buf;
   {
   int l;

   for(l=0; (Read(file,buf,1) > 0)&&(l < MAXLINE); buf++, l++)
      {
      if(*buf == '\n') return(++l);
      if((*buf==0x0d)||(*buf==0x0c)||((*buf==' ')&&(l==0))) buf--, l--;
      }
   if(l == MAXLINE)  cleanexit("Input line too long\n");
   return(0);
   }

strlen(s)
char *s;
   {
   int i = 0;
   while(*s++) i++;
   return(i);
   }

cleanexit(s)
   char  *s;
   {
   printf("Output %ld cards\n",ccnt);
   if(*s)
      {
      printf("%s \n",s);
      }
   cleanup();
   exit();
   }

cleanup()
   {
   if(filea)  Close(filea);
   if(fileb)  Close(fileb);
   }
