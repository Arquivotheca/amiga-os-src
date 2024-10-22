
/*******************************************************/
/*	  BLIB support for the Assembler/Linker        */
/*   (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.  */
/*						       */
/*	    Last modified : 11-APR-85	(PJF)	       */
/*******************************************************/


#ifdef SUN
#include <sys/file.h>
typedef unsigned char* string;
#define bytesperword 4
#define MAXINT 0x7FFFFFFF
#define MININT 0x80000000
#endif


#ifdef IBM
#include "\lc\fcntl.h"
int _stack = 8000;
typedef char* string;
#define bytesperword 2
#define MAXINT 0x7FFF
#define MININT 0x8000
#endif

typedef int *SCBPTR;

#define true 1
#define false 0
#define bytesper68000word 4
#define endstreamch (-1)
#define TRUE true
#define FALSE false
#define BYTESPERWORD bytesperword
#define ENDSTREAMCH endstreamch
#define BYTESPER68000WORD bytesper68000word
#define NULL 0

#define c_esc 0x1B
#define max_scb 16
#define buffsize 512 			    /* 512 byte scb buffer    */
#define scbsize buffsize+3*bytesperword     /* Plus 3 integer fields */
#define scb_pos 0
#define scb_max 1
#define scb_id	2

#define rdflag O_RDONLY
#ifdef SUN
#define wrflag (O_WRONLY | O_CREAT | O_TRUNC | O_NDELAY)
#endif
#ifdef IBM
#define wrflag (O_WRONLY | O_CREAT | O_TRUNC)
#endif

/**********************************************************/
/* Standard C functions used which do not return integers */
/* and must therefore be declared as before they are used */
/**********************************************************/

extern long lseek();
extern char* malloc();

/* sysout only used for Debugging */
extern SCBPTR sysout;

/*********************/
/* Global varaiables */
/*********************/

long	  result2;
SCBPTR	  cis;
SCBPTR	  cos;
int	  cargc;
string	  cargv[50];

/* Replaceable functions */

int  (*wrch_fn)();
int  (*rdch_fn)();
int  (*unrdch_fn)();


/*****************************************************/
/*	Local functions (not called outside BLIB     */
/*****************************************************/

SCBPTR getscb( ch_id )
int ch_id;
{
   SCBPTR res = (SCBPTR)( malloc(scbsize) );

   if ( res != (SCBPTR)0 )
   {
     res[scb_pos] = 0;
     res[scb_max] = 0;
     res[scb_id]  = ch_id;
   }

   return res;
}


bflush()
{
  if (cos != (SCBPTR)0)
  {
    SCBPTR scb = cos;
    int pos    = scb[scb_pos];

    if (pos != 0)
    {
      scb[scb_pos] = 0;
      write( scb[scb_id], scb+3, pos );
    }
  }
}


SCBPTR bopen(name,mode)
string name;
int mode;
{
  char uname[128];

  SCBPTR scb = getscb();
  int	 res = -1;

  if ( scb != (SCBPTR)0 )
  {
    bst2cst(name,uname);

#ifdef SUN
    /* creation mode changed to 666 from 644, 17 sep 85, Neil */
    res = open(uname,mode,0666);
#endif
#ifdef IBM
    res = open(uname,mode);
#endif
  }

  if ( res == -1 )
  {
    free(scb);
    return (SCBPTR)0;
  }
  else
  {
    scb[scb_id] = res;
    return scb;
  }
}


bclose(scb)
SCBPTR scb;
{
  close( scb[scb_id] );
  free( scb );
}


/**********************************************************/
/*   'Global' library functions start here                */
/**********************************************************/


int capitalch ( ch )
int ch;
{
    return (('a' <= ch) && (ch <= 'z')) ? (ch + ('A' - 'a')) : ch;
}


/****************************************************************/
/* Default RDCH, UNRDCH, WRCH which can be replaced by the user */
/****************************************************************/

int default_rdch()
{
   SCBPTR scb = cis;
   int pos = scb[scb_pos];
   int mx  = scb[scb_max];
   int c;

   if ( pos >= mx )
   {
     int nbytes = read(scb[scb_id], scb+3, buffsize);

     if ( nbytes <= 0 )
       return endstreamch;

     scb[scb_max] = nbytes;
     pos = 0;
   }

   c = ((string)(scb+3))[pos++];
   scb[scb_pos] = pos;

   return c;
}


int default_unrdch()
{
   SCBPTR scb = cis;
   int pos    = scb[scb_pos];

   if ( pos == 0 )
     return false;
   else
   {
     scb[scb_pos] = pos - 1;
     return true;
   }
}


int default_wrch(c)
int c;
{
   SCBPTR scb = cos;
   int pos    = scb[scb_pos];

   /* First install the byte in SCB */

   ((string)(scb+3))[pos] = c;
   scb[scb_pos] = ++pos;

   /* Decide whether to flush */

   if ( (pos == buffsize) ||
	(c == '\n')       ||
	(c == '\033')  )
     bflush();
}



stop( code )
int code;
{
   bflush();
   exit( code );
}


wrch(c)
int c;
{
   (*wrch_fn)(c);
}


int rdch()
{
   return (*rdch_fn)();
}


int unrdch()
{
   return (*unrdch_fn)();
}

#ifndef ASM
long readbytes( buff, n )
string buff;
int n;
{
  int *scb = cis;
  return (long)read( scb[scb_id], buff, n );
}
#endif


long wbytes( buff, n )
string buff;
int n;
{
  int *scb = cos;

/*  return (long)write( scb[scb_id], buff, n ); */

  int status = write( scb[scb_id], buff, n );

  if (status == -1)
  {
    printf("\n*** Error while writing - Disk full ?\n");
    exit(1);
  }

  return (long)status;
}


selectinput(scb)
SCBPTR scb;
{
  cis = scb;
}


selectoutput(scb)
SCBPTR scb;
{
  if ( cos != (SCBPTR)0 )
    bflush();

  cos = scb;
}


SCBPTR input()
{
  return cis;
}



SCBPTR output()
{
  return cos;
}



SCBPTR findinput( name, binary )
string name;
int binary;
{
  int mode = rdflag;
#ifdef IBM
  if (binary)
    mode |= 0x8000;
#endif
  return bopen(name, mode);
}


SCBPTR findoutput( name, binary )
string name;
int binary;
{
  int mode = wrflag;
#ifdef IBM
  if (binary)
    mode |= 0x8000;
#endif
  return bopen(name, mode);
}


endread( )
{
  if ( cis != (SCBPTR)0 )
    bclose(cis);
  cis = (SCBPTR)0;
}


endwrite( )
{
  if ( cos != (SCBPTR)0 )
  {
    bflush();
    bclose(cos);
  }

  cos = (SCBPTR)0;
}

#ifndef ASM
long readwords( buf , n )
char *buf;
int n;
{
  return (readbytes(buf, n*bytesper68000word) / bytesper68000word);
}
#endif

long writewords( buf, n )
char *buf;
int n;
{
  return (wbytes(buf, n*bytesper68000word) / bytesper68000word);
}

#ifndef ASM
long pointword( n )
int n;
{
  cis[scb_pos] = 0;
  cis[scb_max] = 0;

  return lseek( cis[scb_id], n*bytesper68000word, 0 );
}
#endif

newline ( )
{
   wrch ( '\n' );
}


writed ( n, d )
long n;
int d;
{
   int	j;
   int	i = 0;
   int	t[10];

   long k = -n;

   if ( n < 0 )
   {
      d--;
      k = n;
   }

   do
   {
      t[i++] = - (int)((k % (long)(10)));
      k /= 10;
   } while ( k != 0 );

   for ( j = i+1; j <= d; j++ )
      wrch ( ' ' );

   if ( n < 0 )
      wrch ( '-' );

   for ( j = i-1; j >= 0; j-- )
      wrch ( t[j] + '0' );
}


writen ( n )
long n;
{
   writed( n, 0 );
}


writehex ( n, d )
long n;
int  d;
{
   static int digits[] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

   if ( d > 1 )
      writehex ( n >> 4, d - 1 );

   wrch ( digits[(n & (long)(15))] );
}


writeoct ( n, d )
long n;
int d;
{
  if ( d > 1 )
     writeoct ( n >> 3, d - 1 );
  wrch ( (int)((n & (long)(7)) + '0') );
}


writes ( s )
string s;
{
   int i;
   for ( i = 1; i <= s[0]; i++ )
      wrch( s[i] );
}


writet ( s, n )
string s;
int n;
{
   int i;

   writes ( s );

   for ( i = 1; i <= (n - s[0]) ; i++)
      wrch( ' ' );
}


writeu ( n, d )
long n;
int d;
{
   long m = (n >> 1)/5;

   if ( m != 0 )
   {
      writed ( m, d - 1 );
      d = 1;
   }

   writed ( n - ( m * (long)(10) ), d );
}


writef ( format, a, b, c, d, e, f, g, h, i, j, k )
string format;
long a,b,c,d,e,f,g,h,i,j,k;
{
   int p;
   long *t =  & a;

   for ( p = 1; p <= format[0]; p++ )
   {
      int k = format[p];

      if ( k == '%' )
      {
	 int (*f)();
	 long arg = t[ 0 ];
	 int n = 0;

	 p++;
	 {
	    int type = capitalch(format[p]);
	    switch ( type )
	    {
	      default:
		 wrch ( type );
		 break;
	      case 'S':
		 f = writes;
		 goto l;
	      case 'T':
		 n = format[++p];
		 n = (('0' <= n) && (n <= '9')) ? (n - '0') :
						(n + (10 - 'A'));
		 writet( (string)arg, n );
		 break;
	      case 'C':
		 f = wrch;
		 goto l;
	      case 'O':
		 f = writeoct;
		 goto m;
	      case 'X':
		 f = writehex;
		 goto m;
	      case 'I':
		 f = writed;
		 goto m;
	      case 'N':
		 f = writen;
		 goto l;
	      case 'U':
		 f = writeu;
		 goto m;
	    m:
	       n = format[ ++p ];
	       n = (('0' <= n) && (n <= '9')) ? (n - '0') :
						(n + (10 - 'A'));
	    l:
	       (*f) ( arg, n );

	    case '$':
	       t++;
	       break;
	    }
	 }
      }
      else
	 wrch (k);
   }
}


int compch( ch1, ch2 )
int ch1, ch2;
{
   return (capitalch(ch1) - capitalch(ch2));
}


int compstring( s1, s2 )
string s1, s2;
{
   int i;
   int lens1 = s1[ 0 ];
   int lens2 = s2[ 0 ];

   string smaller = lens1 < lens2 ? s1 : s2;

   for ( i = 1; i <= smaller[0]; i++ )
   {
      int res = compch( (int)s1[i], (int)s2[i] );
      if ( res != 0 ) return res;
   }

   if ( lens1 == lens2 ) return 0;

   return (smaller == s1) ? -1 : 1;
}

#ifndef ASM
long readn ( )
{
   long sum = 0;
   int ch   = 0;
   int neg  = false;
l:
   ch = rdch();

   if ( ! ( ('0' <= ch) && (ch <= '9') ) )
      switch ( ch )
      {
	default:
	   unrdch();
	   result2 = -1;
	   return (long)0;
	case ' ':
	case '\t':
	case '\n':
	   goto l;
	case '-':
	   neg = true;
	case '+':
	   ch = rdch();
      }

   while ( ('0' <= ch) && (ch <= '9') )
   {
      sum = sum*(long)(10) + (ch - '0');
      ch = rdch();
   }

   if ( neg )
      sum =  - sum;

   unrdch ();
   result2 = 0;

   return sum;
}
#endif

int *getvec( upb )
#ifdef SUN
int upb;
#endif
#ifdef IBM
unsigned upb;
#endif
{
   return (int*) malloc( (upb+1)*bytesperword );
}


freevec( ptr )
string ptr;
{
   free( ptr );
}

cst2bst( s1, s2 )
string s1, s2;
{
   int i = 0;

   while ( i < 255 )
   {
      int c = s1[i];
      if (c == 0) break;
      s2[++i] = c;
   }

   s2[0] = i;
}


bst2cst( s1, s2 )
string s1, s2;
{
   int length = s1[0];
   int i;

   for (i=1; i <= length; i++)
      s2[i-1] = s1[i];

   s2[length] = 0;
}

#ifndef ASM
int getbyte(v,offset)
string v;
long offset;
{
  return (int)(v[offset]);
}
#endif

#ifndef ASM
putbyte(v,offset,b)
string v;
long offset;
int b;
{
  v[offset] = b;
}
#endif

#ifndef ASM
long gbytes(v,size)
string v;
int size;
{
  long c = 0;
  int  p = 0;

  while (size>p)
    c = (c<<8) + v[p++];

  return c;
}
#endif

#ifndef ASM
pbytes(v,size,w)
string v;
int size;
long w;
{
  while( size>0 )
  {
    v[--size] = (w & 0xff);
    w = w>>8;
  }
}
#endif

#ifndef ASM
long note( scb, p )
SCBPTR scb;
long *p;
{
  long bytepos;

  if ( scb = cos )
    bflush();

  bytepos = lseek(scb[scb_id], (long)0, 1);

  if ( scb = cis )
    bytepos += scb[scb_max] - scb[scb_pos];

  *p = bytepos;

  return bytepos;
}
#endif

#ifndef ASM
long point(scb,p)
SCBPTR scb;
long *p;
{
  if ( scb = cos )
    bflush();
  else
  {
    scb[scb_pos] = 0;
    scb[scb_max] = 0;
  }

  return lseek( scb[scb_id], (long)*p, 0 );
}
#endif

#ifdef DBUG
mydebug(s1,s2,a,b,c,d,e,f,g,h,i,j,k,l)
string s1,s2;
long a,b,c,d,e,f,g,h,i,j,k,l;
{
   char ustring[256];
   SCBPTR o = output();

   cst2bst(s1,ustring);
   selectoutput ( sysout );
   writes( ustring );
   writes( "\002: " );
   cst2bst(s2,ustring);
   writef( ustring,a,b,c,d,e,f,g,h,i,j,k,l );
   newline();
   selectoutput(o);
}
#endif

/*********************************************************/
/*  Define 'main' to do initialisation, then call START  */
/*********************************************************/

main(argc, argv)
int argc;
string argv[];
{
   int i;

   cargc = argc;
   for ( i=0; i<=argc; i++ )
     cargv[i] = argv[i];

   wrch_fn   = default_wrch;
   rdch_fn   = default_rdch;
   unrdch_fn = default_unrdch;

/* Set up default Input/Output SCBs */

   cis = getscb( 0 );
   cos = getscb( 1 );

   if  ( (cis == (SCBPTR)0) || (cos == (SCBPTR)0) )
   {
      printf( "\n** Initialisation failed **" );
      exit(1);
   }

   start();

   stop(0);
}

/*************************** RDARGS **********************************/

/*  Special Implementation of RDARGS for UNIX			     */
/*    Essentially a direct crib from TRIPOS BLIB		     */
/*    with RDCH/UNRDCH redefined as rdgetc/rdungetc		     */
/*    N.B. UNIX provides us with an 'argvector' (here it's 'cargv' - */
/*    a BCPL pointer) and an 'argcount' (here it's 'cargc')          */
/*    Example:- 						     */
/*	   asm fred to bill					     */
/*		results in the following:			     */
/*	   cargc = 4						     */
/*					 All pointers are MC ptrs    */
/*					 to C strings		     */
/*	   cargv ----->   ----------				     */
/*		     !0   |    ----|-------> a s m 0		     */
/*			  ----------				     */
/*		     !1   |    ----|-------> f r e d 0		     */
/*			  ----------				     */
/*		     !2   |    ----|-------> t o 0		     */
/*			  ----------				     */
/*		     !3   |    ----|-------> b i l l 0		     */
/*			  ----------				     */
/*			  |    0   |				     */
/*			  ----------				     */
/*								     */
/*********************************************************************/

int argword;
int argchptr;

/*  This routine undoes argv/argc to give a stream */
/*  of chars to RDARGS */

int rdgetc ( )
{
   int ch;

   if ( argword == cargc )
      return '\n';

   ch = (cargv[argword])[argchptr];

   if ( ch == 0 )
   {
      /*  We have reached the end of one of the */
      /*  'words' in the argument list          */

      argword++;	       /*  Move to next word	  */
      argchptr = 0;	       /*  Beginning of next word */

      /* writes("\020Getchar : space\n" ); */

      return ' ';
   }
   else
   {
      argchptr++;
      /* writef("\017Getchar : %c :\n",ch); */
      return ch;
   }
}


int rdungetc()
{
   if ( argchptr > 0 )
      argchptr--;
   else
      if (argword != 1)
      {
	 argword--;
	 argchptr = 0;

	 while ( (cargv[argword])[argchptr] != 0 )
	    argchptr++;
      }
}


/*  FINDARG - Cribbed from TRIPOS BLIB (no changes)	*/
/*  Routine which searches the KEYS supplied		*/
/*  to RDARGS to find whether an input WORD (w) 	*/
/*  is part of the specified argument list in KEYS.	*/
/*  If it is, it returns the argument number, otherwise */
/*  -1 is returned.					*/

#define matching 0
#define skipping 1

int findarg ( keys, w )
string keys;
string w;
{
   int i;

   int state = matching;
   int wp    = 0;
   int argno = 0;

   for ( i = 1; i <= keys[ 0 ]; i++ )
   {
      int kch = keys[i];

      if ( state == matching )
      {
	 if ( ( (kch == '=') ||
		(kch == '/') ||
		(kch == ',') ) && (wp == w[0]) )
	    return argno;

	 if ( compch(kch, w[++wp]) != 0 )
	    state = skipping;
      }

      if ( (kch == ',') || (kch == '=') )
      {
	 state = matching;
	 wp = 0;
      }

      if ( kch == ',' )
	 argno++;
   }

   if ( (state == matching) && (wp == w[0]) )
      return argno;

   return  - 1 ;
}


/*  RDITEM - Cribbed directly from TRIPOS BLIB (not changed) */
/*  Read an item from command line */
/*  returns -2	  "=" Symbol */
/*	    -1	  error */
/*	     0	  *N, *E, ;, endstreamch */
/*	     1	  unquoted item */
/*	     2	  quoted item */

int rditem ( vword, size )
string *vword;
int size;
{
   int i,ch;

   int p       = 0;
   int quoted  = false;
   string vstr = (string)vword;

   int pmax = ( (size+1)*bytesper68000word ) - 1;


   /* Fill space with zeros first */

   for ( i = 0 ; i < size ; i++ )
      vword[i] = 0;

   /* Now read some chars into it */

   do
      ; while ( (ch = rdch()) == ' ');

   if ( ch == '\"' )
   {
      quoted = true;
      ch     = rdch();
   }

   while ( ! ( (ch == '\033') || (ch == '\n') || (ch == endstreamch) ) )
   {
      if ( quoted )
      {
	 if ( ch == '\"' )
	    return 2;

	 if ( ch == '*' )
	 {
	    ch = rdch();
	    if ( capitalch(ch) == 'E' )
	       ch = '\033';
	    if ( capitalch(ch) == 'N' )
	       ch = '\n';
	 }
      }
      else
	 if ( (ch == ';') || (ch == ' ') || ( ch == '=' ) )
	    break;

      if ( ++p > pmax )
	 return  -1 ;

      vstr[p] = ch;
      vstr[0] = p;

      ch = rdch();
   }

   unrdch();

   if ( quoted )
      return  -1;

   if ( p == 0 )
   {
      if ( ch == '=' )
      {
	 rdch();
	 return  -2;
      }
      return 0;
   }
   else
      return 1;
}


string rdargs ( keys, argv, size )
string keys;
string *argv;
int size;
{
   int	p,numbargs;
   string *w = argv;

   int (*saferdch)()   = rdch_fn;
   int (*safeunrdch)() = unrdch_fn;

#ifdef DBUG
   mydebug("rdargs","keys = %s\n    argv = %x8; size = %n",(long)keys,(long)argv,(long)size);
#endif

   /*  Temporarily alter RDCH/UNRDCH */

   rdch_fn   = rdgetc;
   unrdch_fn = rdungetc;

   /*  Set pointer to first char on the command line */

   argword  = 1;
   argchptr = 0;

   (*w) = 0;

   for ( p = 1 ; p <= keys[ 0 ] ; p++ )
   {
      int kch = keys[p];

      if ( kch == '/' )
      {
	 int c = capitalch( keys[p+1] );

	 if ( c == 'A' )
	    (*w) = (string)((int)(*w) | 1);

	 if ( c == 'K' )
	    (*w) = (string)((int)(*w) | 2);

	 if ( c == 'S' )
	    (*w) = (string)((int)(*w) | 4);

	 continue;
      }

      if ( kch == ',' )
      {
	 w++;
	 (*w) = NULL;
      }

   }

   w++;
   numbargs = (int)(w - argv);	     /* Long words !! */

   do
   {
      /*  At this stage, the argument elements of argv have been */
      /*  initialised to  0    - */
      /*		  1   /A */
      /*		  2   /K */
      /*		  3   /A/K */
      /*		  4   /S */
      /*		  5   /S/A */
      /*		  6   /S/K */
      /*		  7   /S/A/K */

      int  i;
      int  argno =  - 1 ;
/*    Code doesn't work with GCC even though assembly looks OK !! */
/*    int  wsize = size + (int)(argv - w); */
      int  wsize = size - (int)(w - argv);

      switch ( rditem(w, wsize) )
      {
	 default:
	 err:
	    {
	       int ch;
	       do
		  ch = rdch();
		  while ( ! (  (ch == '\033')     ||
			       (ch == '\n')       ||
			       (ch == ';' )       ||
			       (ch == endstreamch) ) );
	       result2	 = 120;
	       rdch_fn	 = saferdch;
	       unrdch_fn = safeunrdch;
	       return 0;
	    }


	 case 0:   /*  *N, *E, ;, endstreamch */

	    for ( i = 0 ; i <= (numbargs - 1) ; i++ )
	    {
	       int a = (int)(argv[ i ]);

	       if ( (0 <= a) && (a <= 7) )
		  if ( (a & 1) == 0 )
		  {
		     argv[i] = NULL;
		  }
		  else
		     goto err;
	    }

	    rdch();

	    rdch_fn   = saferdch;
	    unrdch_fn = safeunrdch;

	    return (string)w;


	 case 1:   /*  ordinary item */

	    argno = findarg(keys, w);

	    if ( argno >= 0 )	/*  get and check argument */
	    {
	       if ( (4 <= (int)(argv[argno])) && ((int)(argv[argno]) <= 7) )
	       {
		  /*  no value for key. */
		  argv[argno] = (string)(-1);
		  continue;
	       }
	       else
	       {
		  int item = rditem(w, wsize);
		  if ( item ==	-2 )
		     item = rditem(w, wsize);
		  if ( item <= 0 )
		     goto err;
	       }
	    }
	    else
	       if ( ( rdch() == '\n' ) && ( compstring("\001?", w) == 0 ) )
	       {
		  /*  help facility */
		  writef ( "\005%S: \033", keys );
		  rdch_fn   = saferdch;
		  unrdch_fn = safeunrdch;
		  break;
	       }
	       else
		  /*  *** For testing under TRIPOS only *** */
		  /*  $( LET ch=RDCH(); UNTIL ch='*N' DO ch:=RDCH() $) */
		  unrdch();

	 case 2:   /*  quoted item (i.e. arg value) */

	    if ( argno < 0 )
	    {
	       int i;

	       for ( i = 0 ; i <= (numbargs - 1) ; i++ )
		  switch ( (int)(argv[i]) )
		  {
		     case 0:
		     case 1:
			argno = i;
			goto label1;
		     case 2:
		     case 3:
			goto err;
		  }
	    }
label1:
	    if ( ! ( argno >= 0 ) )
	       goto err;

	    {
	      string s = (string)w;
	      argv[argno] = s;
	      w += (s[0]/bytesperword) + 1;

#ifdef DBUG
	      mydebug("rdargs","arg%n = %s @s = %x8",(long)argno,(long)s,(long)s);
#endif
	    }
      }

   }
       while ( true );

}



