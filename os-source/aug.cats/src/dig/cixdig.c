
/* cixdig92.c --- version of protodig.c
 * Modified version of bixdig to accomodate CIX header style.
 * Based on version modified 10/87 to accomodate removed line in BIX headers
 *  with no comments
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>

#define EMPTY_FILE (RETURN_WARN+1)

#define BUFSZ	1024

UBYTE *vers="$VER: cixdig 37.1";

UBYTE *rbuf = NULL;
char *outtitle = NULL;
LONG  rbe, rbi = BUFSZ, flen = 0;

char *clearline = "\033[F\033[B\033[J";
char *cursoff = "\033[0 p";
char *curson  = "\033[1 p";
char *color1 = "\033[31m";
char *color3 = "\033[33m";

UBYTE *fromformat = "\n%s %s (CIX) %s";
UBYTE *dateformat = "Date: %s";
UBYTE *subjformat = "Subject: %s\n";
UBYTE *subsformat = "Subsystem: %s\n\n";

UBYTE *mesgformat = "CIX %s msg %ld";
UBYTE *commformat = "  (Comment to %ld)";

struct name_list
        {
        struct name_list *next;
        char *name;
        };
typedef struct name_list Names;

struct bix_record
        {
        struct bix_record *next;        /* linear link */
        struct bix_record *brother;     /* next record at same level */
        struct bix_record *child;       /* first child */
        int id;         /*the number of this message */
        int ref;        /* message this is a comment to, 0 if not comment */
        char *who;      /* bixname of who left this message */
	char *date;     /* date posted */
        Names *text;    /* the body of the message */
        };
typedef struct bix_record Bixing;

BPTR in, out;
char *title;
Bixing *bixes = NULL;
char line_buf[512];    /* 256 is supposed to be enough but I'm paranoid */
int line_count = 0;
char separator[] = "==========================\n";
char dashes[] =    "--------------------------\n";

LONG   IntuitionBase = NULL;
struct Remember *RememberKey = NULL;
char *hilite  = "====";
char *abbrev = " ";
char confName[40];
BOOL OneLine  = FALSE;
BOOL ChopLine = FALSE;
BOOL First = TRUE;
int  lastNum, firstNum = 0, abbrevLen = 0;
#define CHOPLEN  76L

char *
get_line()
{
int i;
char c;
int rLen;
char *pt;

rLen= myRead(in,&c,1);
if (rLen < 1)
        return(NULL);
line_count++;
pt = line_buf;
i = sizeof(line_buf)-2;
while (--i >= 0)
        {
        if (c != '\r')  /* strip carriage returns */
                *pt++ = c;
        if (c == '\n')
                break;
        rLen = myRead(in,&c,1);
        if (rLen < 1) 
		break;
        }

/* drop trailing spaces */
while(((*(pt-2))==' ')&&((pt-1)>line_buf)) pt--;
*(pt-1)='\n';
*pt = 0;
return(line_buf);
}


/* chop word ... put the next word (separated by white space) in line
   into the buffer word points to, return what's left of the line, or
   NULL if at end of line */
char *
chop_word(line, word)
register char *line, *word;
{
register char c;

while (isspace(*line) )
        line++;
if (*line == 0)
        return(NULL);
for (;;)
        {
        c = *line++;
        if (isspace(c))
                {
                *word = 0;
                return(line);
                }
        if (c == 0)
                {
                *word = 0;
                return(--line);  /*so catch end of string next call...*/
                }
        *word++ = c;
        }
}
        
Names *
reverse_list(l)
Names *l;
{
Names *next;
Names *nl = NULL;

while (l)
        {
        next = l->next;
        l->next = nl;
        nl = l;
        l = next;
        }
return(nl);
}

Bixing *
add_tail(list, tail)
Bixing *list, *tail;
{
Bixing *pt;

if (!list)
        return(tail);
pt = list;
while (pt->brother)
        {
        pt = pt->brother;
        }
pt->brother = tail;
return(list);
}


/* pferror - print fatal error */
pferror(err)
char *err;
{
printf("Fatal error %s line %ld: error %s\n", title, line_count, err);
cleanexit("",RETURN_FAIL);
}

header_screwed(n)
{
printf("## %d  ##\n", n);
pferror("header screwed");
}

truncated()
{
pferror("truncated");
}

outta_mem()
{
cleanexit("Out of memory\n",RETURN_FAIL);
}

char *
smalloc(amount)
{
char *pt;

if ((pt=(char *)AllocRemember(&RememberKey,
       (amount),MEMF_PUBLIC|MEMF_CLEAR))==NULL)
          outta_mem();
return(pt);
}

#define Alloc1(type)  ((type *)smalloc(sizeof(type)))

char *
clone_string(s)
char *s;
{
char *d;

if (s == NULL)
        return(NULL);   /* sometimes this is useful ... */
d = smalloc(strlen(s)+1);
strcpy(d, s);
return(d);
}


/* skip past diverts the input stream into the bit bucket until
        it comes to "what".  Returns 1 if found, 0 if EOF */
skip_past(what)
char *what;
{
for (;;)
        {
        if (get_line() == NULL)
                return(0);
        if (strcmp(line_buf, what) == 0)
                return(1);
        }
}

Bixing *
one_bix()
{
Bixing *one;
char *line;
char word_buf[256];     
Names *text, *next;

if ( get_line() == NULL)
        return(NULL);
one = Alloc1(Bixing);
one->next = one->brother = one->child = NULL;   /* not on list yet */
line = line_buf;
if ( (line = chop_word(line, word_buf)) == NULL)        /* skip name of conf */
        header_screwed(0);
if (First) strcpy(confName,word_buf);
if ( (line = chop_word(line, word_buf)) == NULL)    /* get message # */
        header_screwed(1);
one->id = atoi(word_buf+1);     /* asciicise it but skip the '#' */
if (First) firstNum = one->id, First = FALSE;
lastNum = one->id;

printf("%s%ld",clearline,one->id);

if ( (line = chop_word(line, word_buf)) == NULL)        /* skip "from" */
        header_screwed(2);
if ( (line = chop_word(line, word_buf)) == NULL)    /* get  who done it*/
        header_screwed(3);
word_buf[ strlen(word_buf) - 1] = 0;    /* cut off the comma */
one->who = clone_string(word_buf);

/* skip "n chars," */
if ( (line = chop_word(line, word_buf)) == NULL) header_screwed(3);
if ( (line = chop_word(line, word_buf)) == NULL) header_screwed(3);
one->date = clone_string(line);

one->ref = 0;   /*start out with reference to nothing */
if ( get_line() == NULL)
        truncated();
line = line_buf;

/* perhaps no comment line */
if ( strcmp(line_buf, dashes) != 0)
   {
   if ( (line = chop_word(line, word_buf)) != NULL)
      {
      if (strcmp(word_buf, "Comment") == 0)
         {
         if ( (line = chop_word(line, word_buf)) == NULL) /* skip "to" */
                 header_screwed(5);
         if ( (line = chop_word(line, word_buf)) == NULL) /* get  ref*/
                 header_screwed(6);
         one->ref = atoi(word_buf);
         }
      }
   if ( get_line() == NULL) /* get the line "---------" and ignore */
           truncated();
   }

/* now that the header is parsed, just stuff the rest of the message
   a line at a time onto one->text */
text = NULL;
for (;;)
        {
        if ( get_line() == NULL)
                break;
        if ( strcmp(line_buf, separator) == 0)
                break;
        next = Alloc1(Names);
        next->name = clone_string(line_buf);
        next->next = text;
        text = next;
        }
one->text = reverse_list(text);
return(one);
}

Bixing *
find_parent(c)
Bixing *c;
{
Bixing *l;
int ref;

ref = c->ref;
l = bixes;
while (l)
        {
        if ( ref == l->id)
                return(l);
        l = l->next;
        }
return(NULL);
}

Bixing *
make_tree()
{
Bixing *t;
Bixing *parent;

t = bixes->next;        /*first message is root of tree *always* */
while (t)
        {
        if ( parent = find_parent(t) )
                {
                parent->child = add_tail(parent->child, t);
                }
        else
                {
                add_tail(bixes, t);     /*if no parents its a brother of the root */
                }
        t = t->next;
        }
}

print_spaces(count)
int count;
{
while (--count >= 0)
        Write(out," ",1);
}

output_line(s,indented)
char *s;
int  indented;
   {
   char *es;
   if (ChopLine)
      {
      if((strlen(s) + indented) > CHOPLEN)
         {
         es = (char *)(s + CHOPLEN - indented);
         *(es - 1) = '\n';
         *es = NULL;
         }
      }

   if (Write(out,s,strlen(s)) == -1)
      cleanexit("Write error\n",RETURN_FAIL);
   }

print_tree(t, level)
Bixing *t;
int level;      /* recursive depth, how many <spaces> before message # */
{
Names *n;
LONG tagLen;

while (t)
        {
        if (level == 0) fprintf(out,fromformat,"From",t->who,t->date);
        else		fprintf(out,fromformat,"Comment_From",t->who,t->date);
	fprintf(out,dateformat,t->date);

	if (level == 0)	fprintf(out,subjformat,"????");
	else		fprintf(out,subjformat,"Re:");
        fprintf(out,subsformat,"??");

	fprintf(out,mesgformat,confName,t->id);
	if(t->ref)	fprintf(out,commformat,t->ref);
	fprintf(out,"\n\n");

        printf("%s%ld",clearline,t->id);

        tagLen = 0;

        n = t->text;

        /* Skip initial blank lines */
        while((n)&&(*(n->name)=='\n'))  n = n->next;

        output_line(n->name,level+tagLen+abbrevLen);
        n = n->next;
        if(!OneLine)
           {
           while (n)
              {
              output_line(n->name, level+abbrevLen);
              n = n->next;
              }
	   fprintf(out,"\n%s\n",hilite);
           }
        print_tree(t->child, level+1);
        t = t->brother;
        }
}

main(argc, argv)
int argc;
char *argv[];
{
Bixing *b;

LONG l;

if (argc < 3)
        {
        printf("usage: bugdig sourcefile destfile\n");
        exit(0);
        }

if(!(IntuitionBase = OpenLibrary("intuition.library",0)))
        cleanexit("Can't open Intuition\n",RETURN_FAIL);

if(!(rbuf = (UBYTE *)AllocMem(BUFSZ,MEMF_PUBLIC|MEMF_CLEAR)))
        cleanexit("Can't alloc buffers\n",RETURN_FAIL);

title = argv[1];
if ((in = Open(title, MODE_OLDFILE)) == NULL)
        cleanexit("Can't read input file\n", RETURN_WARN);

outtitle = argv[2];
if ((out = Open(outtitle, MODE_NEWFILE)) == NULL)
	cleanexit("Can't open output file\n", RETURN_WARN);

if (argc > 3)
        {
        for(l=3; l<argc; l++)
           {
           if(*argv[l]=='+') abbrev = argv[l] + 1L, abbrevLen=strlen(abbrev);
           else if(*argv[l]=='1')  OneLine = TRUE;
           else if(*argv[l] | 0x20 =='c')  ChopLine = TRUE; 
           else if(*argv[l]=='h')  hilite = argv[l] + 1L;
           }
        }

if (!skip_past(separator) )
        cleanexit("Input file doesn't seem to have any CIX messages... \n",
                          EMPTY_FILE);

printf(cursoff);
printf("Scanning...\n");

while (b = one_bix())
        {

        b->next = bixes;
        bixes = b;
        }
bixes = (Bixing *)reverse_list(bixes);
make_tree();

printf("\n%sWriting...\n",color3);

print_tree(bixes, 0);
fprintf(out,"%s%s%s\n\n",hilite,hilite,hilite);

cleanexit("",RETURN_OK);
}

myRead(file,buf,n)
LONG file;
UBYTE *buf;
LONG n;
   {
   LONG rLen, charsinbuf, rval=0;
	
   if(rbi == BUFSZ)
	{
        rbe = Read(file,rbuf,BUFSZ);
	filter(rbuf,BUFSZ);
	rbi = 0; 
	
	if(rbe > 0) flen += rbe; /* for debugging virtual read stuff */

	if(SetSignal(0,0) & SIGBREAKF_CTRL_C) cleanexit("BREAK\n",RETURN_OK);
	}

   if(rbe < 1)       
	{
	*buf = 0xFF;
        return(rbe);
	}

   charsinbuf = rbe + 1 - rbi;

   if(n <= (charsinbuf))
      {
      if(n==1) *buf = rbuf[rbi];
      else CopyMem(&rbuf[rbi],buf,n);
      rbi += n;
      rval=n;
      }
   else
      {
      CopyMem(&rbuf[rbi],buf,charsinbuf);
      rbi = BUFSZ;
      rLen = Read(file,&buf[charsinbuf],n-charsinbuf);
      filter(&buf[charsinbuf],n-charsinbuf);
      if(rLen >= 0) rval=(charsinbuf + rLen);
      else rval=charsinbuf;
      }
   return(rval);
   }

filter(buf, len)
UBYTE *buf;
LONG len;
   {
   UBYTE ch;
   while((--len) >= 0)
	{
 	ch=buf[len];
        if(((ch>0x00)&&(ch<0x08))||
           ((ch>0x0d)&&(ch<0x1b))||
           ((ch>0x1b)&&(ch<0x20))||
           ((ch>0x7f)&&(ch<0xa0)))  buf[len]=0xde; /* amiga para, vax ^ */
        }
   }

cleanexit(s,e)
UBYTE *s;
LONG e;
   {
   printf("%s%s\n",color1,curson);
   if(*s) printf(s);
   cleanup(e);
   exit(e);
   }

cleanup(e)
LONG e;
   {
   if(RememberKey)  FreeRemember(&RememberKey,TRUE);
   if(IntuitionBase) CloseLibrary(IntuitionBase);
   if(in)  Close(in);
   if(out)
	{
	Close(out);
	if(e==EMPTY_FILE)	DeleteFile(outtitle);
	}
   if(rbuf)   FreeMem(rbuf,BUFSZ);
   }

