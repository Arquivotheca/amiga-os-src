/****i* autodoc.c/autodoc ***************************************************
*
*   NAME
*     autodoc -- extract and process autodocs to stdout.
*
*   SYNOPSIS
*     autodoc [options] infile1 [infile2...]
*
*   FUNCTION
*     Autodoc reads infile[s] looking for blocks of lines that meet the
*     requirements for autodocs. An autodoc is defined by a line that
*     starts with either a '/' '*' or ';' followed by '******', or
*     '****i*' (if the autodoc is internal) followed by a space, a header,
*     and at least one more space.
*     The end of an autodoc is defined by a line that begins with any
*     character followed by three asterics.
*     The bodys of the autodocs are written to a tempfile as they are read
*     in and are then output to stdout in sorted order.
*
*   INPUTS
*     options  -  -tnum -  converts tabs into num spaces
*                 -i    -  process internal autodocs only
*                 -a    -  Does not Allow asterics to start autodocs
*                 -s    -  Allow semicolons to start autodocs
*                 -C    -  Allow the slash to start autodocs (Default)
*                 -lnum -  line length = num characters
*                 -w    -  trun off word wrap, characters past line length
*                          will be discarded.
*                 -c    -  Do not convert C comments
*                 -f    -  Disable form feeds between autodoc entries
*                 -r    -  Output in troff format
*                 -Ffile-  Use file as the temp file instead of tmpfile
*                 -I    -  Do not Include a table of contents.
*
*     infile[s]         -  files to be processed for autodocs
*
*   RESULT
*     A sorted listing of autodocs is printed to stdout.
*
*   EXAMPLE
*     autodoc >file.adoc file  ; will perform as the old autodoc did!
*
*   NOTES
*     Created by Bill Koester around August 1989.
*
*   BUGS
*     What, in one of MY programs!
*
*   SEE ALSO
*
*****************************************************************************
*
*/
#include <stdio.h>
#ifdef  AMIGA
#include <stdlib.h>
#endif
#ifdef  MSDOS
#include <stdlib.h>
#endif

#ifndef TRUE
#define TRUE   (-1)
#endif
#ifndef FALSE
#define FALSE  (0)
#endif

#define HEADER_LENGTH      80
#define MAX_LINE_LENGTH    512
#define LINE_WRAP_DEFAULT  78

#define ADOC_TAB_TO_SPACE  (1<<0)
#define ADOC_INTERNAL      (1<<1)
#define ADOC_ASTERIC       (1<<2)
#define ADOC_SEMI_COLON    (1<<3)
#define ADOC_C             (1<<4)
#define ADOC_NO_WRAP       (1<<6)
#define ADOC_CONVERT_COM   (1<<7)
#define ADOC_NO_FORM_FEED  (1<<8)
#define ADOC_TROFF         (1<<9)
#define ADOC_TEMP_FILE     (1<<10)
#define ADOC_CONTENTS      (1<<11)


int spaces = 8;
int line_length = LINE_WRAP_DEFAULT;
char tempfiledef[]="tmpfile";
char *tempfile = tempfiledef;

#ifdef MSDOS
char readmode[]="rt";
char writemode[]="wt+";
#else
char readmode[]="r";
char writemode[]="w+";
#endif

struct List {
      struct List *Next;
      struct List *Prev;
      char name[HEADER_LENGTH];
      unsigned long Start;        /* byte offset into tempfile where     */
      unsigned long Length;       /* Length bytes of autodoc are strored */
      };

#ifdef AMIGA
char mytolower(char);
#endif
#ifdef MSDOS
char mytolower(char);
#endif

void main(argc,argv)
int argc;
char *argv[];
{
   int x;
   int firstinfile;
   int lastinfile;
   unsigned long flags = 0;
   struct List *list = 0;
   FILE *tfile;

   if(argc==1)
   {
      printf("Autodoc Version 1.01 by BK\n");
      printf("Options:\n");
      printf("    -tnum  - Turn tabs into num spaces\n");
      printf("    -i     - Process only INTERNAL autodocs\n");
      printf("    -a     - Do not process autodocs starting with an asteric\n");
      printf("    -s     - Process autodocs starting with a semicolon\n");
      printf("    -C     - Process autodocs starting with /* (Default)\n");
      printf("    -lnum  - Set line length to num (default = %d)\n",line_length);
      printf("    -w     - Turn off word wrap\n");
      printf("    -c     - Do not convert \\* to /* and *\\ to */\n");
      printf("    -f     - No form feeds between entries\n");
      printf("    -r     - troff source output\n");
      printf("    -Ffile - Use file as temporary file (default = %s)\n",tempfile);
      printf("    -I     - Do not Output Table of Contents before entries\n");
      printf("\nUsage: autodoc [options] infile [infile ...]\n");
      exit(20);
   }

   if((firstinfile = GetOptions(argc,argv,&flags))<0)
   {
      exit(20);
   }
   lastinfile  = argc-1;

   if((tfile=fopen(tempfile,writemode))==NULL)
   {
      fprintf(stderr,"Can't open temp file %s\n",tempfile);
      exit(20);
   }

   for(x=firstinfile;x<=lastinfile;x++)
   {
      if(DoFile(&list,argv[x],flags,tfile))
      {
         fclose(tfile);
         unlink(tempfile);
         exit(20);
      }
   }
   if(flags&ADOC_CONTENTS)
      PrintTOC(list);
   if(!(flags&ADOC_NO_FORM_FEED))
      fputc('\f',stdout);

   PrintADOC(list,tfile,flags);

   FreeNodes(&list);

   fclose(tfile);
   unlink(tempfile);
}
/****i* autodoc.c/GetOptions ************************************************
*
*   NAME
*     GetOptions -- Get the command line options.
*
*   SYNOPSIS
*     GetOptions(argc,argv,flags)
*
*     result = GetOptions(int,char **,unsigned long *);
*
*   FUNCTION
*     Parse the command line extracting all options. For each option set
*     the appropriate flags and global variables.
*
*   INPUTS
*     argc  -  Begining index to use is search fr options.
*
*     argv  -  The command line arguments.
*
*     flags -  A pointer to the flags variable to be updated.
*
*   RESULT
*     Returns the index of the first NON-option command line argument
*     ie first input file or (-1) on error.
*
*     Set the flags variable according to the options found.
*
*     The following globals may be changed.
*
*     int spaces      - Number of spaces to turn a tab into.
*
*     int Line_length - Max line length for word wrap.
*
*     char *tempfile  - Pointer to the name of the temp file to use.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
GetOptions(argc,argv,flags)
int argc;
char *argv[];
unsigned long *flags;
{
   int x;
   unsigned long tflags=0;

   *flags = ADOC_C|ADOC_CONTENTS|ADOC_CONVERT_COM|ADOC_ASTERIC;

   if(argc==2)
   {
      return(1);
   }

   for(x=1;x<(argc);x++)
   {
      if(argv[x][0]=='-')
      {
         switch(argv[x][1])
         {
            case 't':
                     *flags |= ADOC_TAB_TO_SPACE;
                     spaces=atoi(&argv[x][2]);
                     break;
            case 'i':
                     *flags |= ADOC_INTERNAL;
                     break;
            case 'a':
                     *flags &= ~ADOC_ASTERIC;
                     break;
            case 's':
                     *flags |= ADOC_SEMI_COLON;
                     *flags &= ~ADOC_C;
                     break;
            case 'C':
                     /* *flags |= ADOC_C; */
                     tflags = ADOC_C;
                     break;
            case 'l':
                     line_length = atoi(&argv[x][2]);
                     break;
            case 'w':
                     *flags |= ADOC_NO_WRAP;
                     break;
            case 'c':
                     *flags &= ~ADOC_CONVERT_COM;
                     break;
            case 'f':
                     *flags |= ADOC_NO_FORM_FEED;
                     break;
            case 'r':
                     *flags |= ADOC_TROFF;
                     break;
            case 'F':
                     *flags |= ADOC_TEMP_FILE;
                     tempfile = &argv[x][2];
                     break;
            case 'I':
                     *flags &= ~ADOC_CONTENTS;
                     break;
            default:
                     fprintf(stderr,"Invalid option %s\n",argv[x]);
                     return(-1);
         }
      } else {
         *flags |= tflags;
         return(x);
      }
   }
   return(x);
}
/****i* autodoc.c/ConvertComments *******************************************
*
*   NAME
*     ConvertComments -- Convert backwards C comments in to normal comments
*
*   SYNOPSIS
*     ConvertComments(line)
*
*     ConvertComments(char *);
*
*   FUNCTION
*     Converts the '\*' and '*\' character sequences into normal C comments
*     This allows you to include example code in the autodocs even if your
*      compiler will not support nested comments.
*
*   INPUTS
*     line  -  Input line.
*
*   RESULT
*     All backwards commants in line are replaced with normal comments.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
ConvertComments(line)
char *line;
{
   int x = 0;

   while(line[x]!='\0')
   {
      if(line[x]=='\\')
         if((x!=0)&&(line[x-1]=='*'))
            line[x]='/';
         else
            if((x<line_length-1)&&(line[x+1]=='*'))
               line[x]='/';
      x++;
   }
   return(0);
}
/****i* autodoc.c/GetLine ***************************************************
*
*   NAME
*     GetLine -- Get the next line of input.
*
*   SYNOPSIS
*     length = GetLine(line,fp,flags)
*
*     int GetLine(char *, FILE *, unsigned long);
*
*   FUNCTION
*     Get the next input line from file. Based on flags optionally perform
*     the following conversions.
*
*     1) Tabs to spaces.
*     2) Convert C comments
*
*   INPUTS
*     line  -  pointer to a buffer to recieve the input line.
*
*     fp    -  file to read the line from.
*
*     flags -  flags to determine conversions performed.
*
*   RESULT
*     Buffer pointed to by line contains the next input line from file.
*     The line has been converted according to flags.
*     Returns the length of the converted line.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
GetLine(line,fp,flags)
char *line;
FILE *fp;
unsigned long flags;
{
   register int c;
   register char *cs;
   register int n;
   int x;

   n=MAX_LINE_LENGTH;
   cs=line;
   while(--n > 0&& (c=fgetc(fp))!=EOF)
   {
      if(c=='\n')
      {
         *cs++=c;
         goto byebye;
      }
      if((c=='\t')&&(flags&ADOC_TAB_TO_SPACE))
      {
         for(x=0;x<spaces;x++)
            *cs++=' ';
      } else
         *cs++=c;
   }
byebye:
   *cs='\0';

   if(c==EOF && cs==line)
      return(0);

   if(flags&ADOC_CONVERT_COM)
      ConvertComments(line);

   return(strlen(line));
}
/****i* autodoc.c/PutLine ***************************************************
*
*   NAME   
*     PutLine -- Puts a line to the output file.
*
*   SYNOPSIS
*     length = PutLine(line,fp,flags)
*
*     int PutLine(char *,FILE *,unsigned long);
*
*   FUNCTION
*     Strips the first charachter of line then puts line_length characters
*     to the file. If ADOC_NO_WORD_WRAP is set any characters after
*     line_length will be discarded.
*
*   INPUTS
*     line  -  pointer to the NULL terminated line to be put.
*
*     fp    -  output file.
*
*     flags -  flags variable.
*
*     line_length - GLOBAL - line_length for word wrap.
*
*   RESULT
*     Puts the line to the output file and returns the number of bytes
*     written.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
PutLine(line,fp,flags)
char *line;
FILE *fp;
unsigned long flags;
{
   int x;
   int i = 0;

   for(x=1;x<(strlen(line)-1);x++)
   {
      if(x==line_length)
      {
         fputc('\n',fp);
         i++;
         if(flags&ADOC_NO_WRAP)
            return(i);
      }
      fputc((int)line[x],fp);
      i++;
   }
   fputc('\n',fp);
   i++;

   return(i);
}
/****i* autodoc.c/BeginAutodoc **********************************************
*
*   NAME
*     BeginAutodoc -- Determine if the given line signals the begining of an
*                     autodoc.
*
*   SYNOPSIS
*     result = BeginAutodoc(line)
*
*     BeginAutodoc(char *);
*
*   FUNCTION
*     Determines if the given line signals the begining of an autodoc.
*     The begining of an autodoc is signaled by of the following patterns.
*
*        '/****** '   '******* '   ';****** '
*        '/****i* '   '*****i* '   ';****i* '
*
*     Any or all combinations of these patterns can signal an autodoc based
*     on the flags given.
*
*   INPUTS
*     line  -  Pointer to the NULL terminated line to be tested.
*
*     flags -  Flags to determine the type of autodoc we are working with.
*
*   RESULT
*     Returns TRUE if the line is the begining of an autodoc, FALSE
*     otherwise.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
BeginAutodoc(line,flags)
char *line;
unsigned long flags;
{
   int x;


   if((flags & ADOC_C) && (line[0]=='/'))
      goto cont;
   if((flags & ADOC_ASTERIC) && (line[0]=='*'))
      goto cont;
   if((flags & ADOC_SEMI_COLON) && (line[0]==';'))
      goto cont;
   return(FALSE);

cont:

   for(x=1;x<=4;x++)
      if(line[x]!='*')
         return(FALSE);

   if(flags & ADOC_INTERNAL)
   {
      if(line[5]!='i')
         return(FALSE);
   } else {
      if(line[5]!='*')
         return(FALSE);
   }


   if((line[6]!='*')||(line[7]!=' '))
      return(FALSE);

   return(TRUE);
}
/****i* autodoc.c/EndAutodoc ************************************************
*
*   NAME
*     EndAutodoc -- Determine if the line given signals the end of the
*                   autodoc.
*
*   SYNOPSIS
*     result = EndAutodoc(line)
*
*     EndAutodoc(char *);
*
*   FUNCTION
*     This function determines if the given line is the END of an autodoc.
*     The END of an autodoc is defined as 'the first line after the BEGINING
*     of an autodoc that has asterics in positions line[1]-line[3].
*
*   INPUTS
*     line  -  pointer to the NULL terminated line to be tested.
*
*   RESULT
*     Returns TRUE if the line is the end of an autodoc, FALSE otherwise.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
EndAutodoc(line)
char *line;
{
   if((line[1]=='*')&&(line[2]=='*')&&(line[3]=='*'))
      return(TRUE);
   else
      return(FALSE);
}
/****i* autodoc.c/DoHeader **************************************************
*
*   NAME
*     DoHeader -- process the autodoc header.
*
*   SYNOPSIS
*     DoHeader(line,dest)
*
*     DoHeader(char *,char *);
*
*   FUNCTION
*     This function expects line to point to a line that IS the begining
*     of an autodoc. DoHeader() strips out the header field and copies it
*     to dest. Then line is converted to the line containing the header
*     at both ends. This converted line is one character longer the
*     line_length since we know PutLine() will kill the first character.
*
*   INPUTS
*     line  -  Pointer to the inout line that is the BEGINING of an autodoc
*
*     dest  -  Pointer to a buffer to recieve just the trimmed header.
*
*     line_length - GLOBAL - max length for output line.
*
*   RESULT
*     dest contains a copy of the trimmed header. line contains a new line
*     with the header at both ends.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
DoHeader(line,dest)
char *line;
char *dest;
{
   char *p;
   char *d;
   char *d1;
   int x;
   int l;

   p=(char *)&line[8];
   d=dest;

   for(x=0;x<HEADER_LENGTH-1;x++)
   {
      if(*p==' ')
         break;
      *(d++)=(char)*(p++);
   }
   *d='\0';

   for(x=0;x<line_length;x++)
      line[x]=' ';
   line[line_length]='\n';
   line[line_length+1]='\0';

   l=strlen(dest);
   p=dest;
   d=(char *)&line[1];
   d1=(char *)&line[line_length-l];
   for(x=0;x<l;x++)
   {
      *(d++)=(char)*p;
      *(d1++)=(char)*(p++);
   }

   return(0);
}
/****i* autodoc.c/mytolower ***************************************************
*
*   NAME
*     mytolower -- convert to lower case
*
*   SYNOPSIS
*     result = mytolower(c)
*
*     char mytolower(char);
*
*   FUNCTION
*     converts c to lowercase
*
*   INPUTS
*     c  -  character to convert
*
*   RESULT
*     Returns the lowercase version of c.
*
*   EXAMPLE
*
*   NOTES
*     Another piece of original autodoc program.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
char mytolower(b)
char b;
{
   if( (b >= 'A') && (b<= 'Z') )
      return((char)(b-'A'+'a'));
   else
      return(b);
}
/****i* autodoc.c/compst ******************************************
*
*   NAME
*     compst -- case insensitive string compare
*
*   SYNOPSIS
*     result = compst(s,t)
*
*     int compst(char *,char *);
*
*   FUNCTION
*     Compare NULL terminated string s to t.
*     Result -1 [s<t], 0 [s=t], 1 [s>t]
*
*   INPUTS
*     s  -  string 1
*
*     t  -  string 2
*
*   RESULT
*     Result -1 [s<t], 0 [s=t], 1 [s>t]
*
*   EXAMPLE
*
*   NOTES
*     This is the only function retained from the original autodoc
*     program.
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
compst(s,t)
char *s, *t;
{
   char cs, ct;

   while(1)
   {
      cs = mytolower(*s);
      ct = mytolower(*t);

      if (ct > cs)   /* first is lesser  or shorter */
         return(-1);
      else
         if(ct < cs)/* first is greater or longer  */
            return(1);
         else      /* first equals second */
         {
            if( cs == '\0')   /* if nulls match, strings are equal */
               return(0);
            t++;
            s++;
         }
   }      /* end of while statement */
}
/****i* autodoc.c/AddANode **************************************************
*
*   NAME
*     AddANode -- Adds and index node the master tree in alphabetical order.
*
*   SYNOPSIS
*     AddANode(list,node)
*
*     AddANode(char *,struct List **);
*
*   FUNCTION
*     Adds the node to the List list in alphabetical order case insensitive.
*
*   INPUTS
*     node  -  pointer to the node to add.
*
*     list  -  pointer to the list pointer to add this node to.
*
*   RESULT
*     node is added to list in alphabetical case insensitive order.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
AddANode(list,node)
struct List **list;
struct List *node;
{
   struct List *p,*pl;

   if(*list==NULL)                           /* empty list */
   {
      *list=node;
      node->Next=NULL;
   } else {
      if(compst(node->name,(*list)->name)<0) /* Add to head */
      {
         node->Next=(struct List *)*list;
         *list=node;
      } else{                                /* add to middle */
         pl=(struct List *)*list;
         while((p=pl->Next)!=NULL)
         {
            if((compst(node->name,pl->name)>0)&&(compst(node->name,p->name)<0))
            {
               pl->Next=node;
               node->Next=p;
               return(0);
            }
            pl=p;
         }
         pl->Next=node;                      /* add to end */
         node->Next=NULL;
      }
   }
   return(0);
}
/****i* autodoc.c/FreeNodes *************************************************
*
*   NAME
*     FreeNodes -- Frees up all the nodes associated with list.
*
*   SYNOPSIS
*     FreeNodes(list)
*
*     FreeNodes(struct List **);
*
*   FUNCTION
*     Frees all the memory associated with the List list.
*
*   INPUTS
*     list  -  Pointer to a List pointer.
*
*   RESULT
*     All the node memory is freed and the list pointer is reset to NULL.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
FreeNodes(list)
struct List **list;
{
   struct List *p,*pn;

   p=(struct List *)*list;

   while(p!=NULL)
   {
      pn=p;                   /* save current   */
      p=pn->Next;             /* get next       */
      free(pn);               /* free current   */
   }
   *list=NULL;                /* reset list ptr */
   return(0);
}
/****i* autodoc.c/PrintTOC **************************************************
*
*   NAME
*     PrintTOC -- Print out the table of contents.
*
*   SYNOPSIS
*     PrintTOC(list)
*
*     PrintTOC(struct List *);
*
*   FUNCTION
*     Prints out the table of contents by scanning list from head to tail
*     and printing each header. 
*
*   INPUTS
*     list - Pointer to a List.
*
*   RESULT
*     Table of contents is printed to stdout.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
PrintTOC(list)
struct List *list;
{
   struct List *p;

   printf("TABLE OF CONTENTS\n\n");

   for(p=list;p!=NULL;p=p->Next)
   {
      printf("%s\n",p->name);
   }
   return(0);
}
/****i* autodoc.c/PrintADOC *************************************************
*
*   NAME   
*     PrintADOC -- Prints out the bodys of the autodocs.
*
*   SYNOPSIS
*     PrintADOC(list,tfile,flags)
*
*     PrintADOC(struct List *,FILE *,unsigned long);
*
*   FUNCTION
*     Prints the bodys of the autodocs optionally separated by form feeds.
*
*   INPUTS
*     list  -  Pointer to the list of headers.
*
*     tfile -  tempfile containing the unsorted bodys.
*
*     flags -  flags variable for ADOC_NO_FORM_FEED.
*
*   RESULT
*     Autodoc bodys are printed to stdout.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
PrintADOC(list,tfile,flags)
struct List *list;
FILE *tfile;
unsigned long flags;
{
   struct List *p;
   int x,c;

   for(p=list;p!=NULL;p=p->Next)
   {
      if(fseek(tfile,p->Start,0)!=0)
         fprintf(stderr,"FSEEK ERROR\n");

      for(x=0;x<p->Length;x++)
      {
         if((c=fgetc(tfile))==EOF)
            fprintf(stderr,"fgetc ERROR\n");
         fputc((char)c,stdout);
      }
      if(!(flags&ADOC_NO_FORM_FEED))
         fputc('\f',stdout);
   }
   return(0);
}
/****i* autodoc.c/DoFile ****************************************************
*
*   NAME
*     DoFile -- process one file for autodocs.
*
*   SYNOPSIS
*     result = DoFile(list,filename,flags,tfile)
*
*     int DoFile(struct List **,char *,unsigned long,FILE *);
*
*   FUNCTION
*     Opens file with filename and scans for autodocs according to flags.
*     for each autodoc found a node is created and added to list and the
*     actual lines of autodoc are written to the temporary tfile. The
*     file is then closed.
*
*   INPUTS
*     list     -  pointer to the master list pointer
*
*     filename -  ASCII name of file to be opened
*
*     flags    -  flags varaible set by GetOptions()
*
*     tfile    -  temp file to write to (already opened)
*
*   RESULT
*     Returns FASLE if all OK TRUE otherwise.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
DoFile(list,filename,flags,tfile)
struct List **list;
char *filename;
unsigned long flags;
FILE *tfile;
{
   FILE *fp;
   char line[MAX_LINE_LENGTH];
   int  cur_ll;

   struct List *nodep;

   if((fp=fopen(filename,readmode))==NULL)
   {
      fprintf(stderr,"\nERROR opening file %s\n",filename);
      return(TRUE);
   }

   while((cur_ll=GetLine(line,fp,flags))!=0)
   {
      if(BeginAutodoc(line,flags))
      {
         if((nodep=(struct List *)calloc(1,sizeof(struct List)))==0)
         {
            fprintf(stderr,"Out of memory!\n");
            fclose(fp);
            return(TRUE);
         }

         nodep->Start=ftell(tfile);
         DoHeader(line,nodep->name);
         nodep->Length += PutLine(line,tfile,flags);
         while((cur_ll=GetLine(line,fp,flags))!=0)
         {
            if(EndAutodoc(line))
               break;
            nodep->Length += PutLine(line,tfile,flags);
         }
         AddANode(list,nodep);
      }
   }
   fclose(fp);
   return(FALSE);
}
