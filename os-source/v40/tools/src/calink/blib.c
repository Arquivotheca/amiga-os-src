/*****************************************************/
/*	  BLIB support for the Linker		     */
/*						     */
/* Last modified : 23-March-85			 PJF */
/*****************************************************/

/* Type declarations */

#ifdef SUN
typedef long word;
typedef unsigned char* string;
#endif
#ifdef AMIGA
typedef long word;
typedef unsigned char* string;
#endif
#ifdef IBM
typedef long word;
typedef char* string;
#endif


/* Include files */

#ifdef SUN
#include <sys/file.h>
#endif

#ifdef AMIGA
#include "lattice/fcntl.h"  /* was c: */
int _stack = 8000;
#endif

#ifdef IBM
#include "\lc\fcntl.h"  /* was c: */
int _stack = 8000;
#endif


/* Local DEFINES */

#define bytesperword 4
#define bytesper68000word 4
#define c_esc 0x1B
#define max_scb 16
#define endstreamch -1
#define true 1
#define false 0

#define buffsize 80			    /* 80 byte scb buffer    */
#define scbsize buffsize+3*bytesperword     /* Plus 3 integer fields */
#define scb_pos 0
#define scb_max 1
#define scb_id	2

#define rdflag O_RDONLY
#ifdef SUN
#define wrflag (O_WRONLY | O_CREAT | O_TRUNC | O_NDELAY)
#endif

#ifdef AMIGA
#define wrflag (O_WRONLY | O_CREAT | O_TRUNC)
#endif

#ifdef IBM
#define wrflag (O_WRONLY | O_CREAT | O_TRUNC)
#endif


#define NULLPTR (word)(0)


/* Standard C functions used which do not return integers and must therefore
   be declared as externals */

extern char *malloc();
extern long lseek();


/* Global varaiables */
word   result2;
int    *cis;
int    *cos;
int    cargc;
char   *cargv[50];

/* Replaceable functions */

int  (*wrch_fn)();
int  (*rdch_fn)();
int  (*unrdch_fn)();


/*****************************************************/
/*	Local functions (not called outside BLIB     */
/*****************************************************/

int *getscb( ch_id )
int ch_id;
{
   int *res = (int*)malloc( scbsize );

   if ( (res != (int*)0) )
   {
     res[scb_pos] = 0;
     res[scb_max] = 0;
     res[scb_id]  = ch_id;
   }

   return res;
}


bflush()
{
  if (cos != (int*)0)
  {
    int *scb = cos;
    int pos  = scb[scb_pos];

    if (pos != 0)
    {
      scb[scb_pos] = 0;
      write( scb[scb_id], scb+3, pos );
    }
  }
}


word bopen(name,mode,rawflag)
string name;
int mode;
word rawflag;
{
  char uname[128];
  int  *scb = getscb();
  int  res  = -1;

  if ( scb != (int*)0 )
  {
    bst2cst(name,uname);

#ifdef SUN
    res = open(uname,mode,0644);
#endif
#ifdef AMIGA
    if (rawflag == true) { mode = (mode | 0x8000) ;}
    res = open(uname,mode);
#endif
#ifdef IBM
    if (rawflag==true) { mode = ( mode | 0x8000 ) ;}
    res = open(uname,mode);
#endif
  }

  if ( res == -1 )
  {
    free(scb);
    return (word)0;
  }
  else
  {
    scb[scb_id] = res;
    return (word)scb;
  }
}


bclose(scb)
int *scb;
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
   int *scb = cis;
   int pos = scb[scb_pos];
   int mx  = scb[scb_max];
   int c;

   if ( pos >= mx )
   {
     int nbytes = read(scb[scb_id], scb+3, buffsize);

     if ( nbytes <= 0 )
     {
       scb[scb_max] = 0;
       scb[scb_pos] = 0;
       return endstreamch;
     }

     scb[scb_max] = nbytes;
     pos = 0;
   }

   c = ((string)(scb+3))[pos++];
   scb[scb_pos] = pos;

   return c;
}


int default_unrdch()
{
   int *scb = cis;
   int pos  = scb[scb_pos];

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
   int *scb = cos;
   int pos  = scb[scb_pos];

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


word readbytes( buff, n )
string buff;
long n;
{
  int *scb = cis;
  int nbytes;
  int status = 0;
  word bytesread = 0;

  while (n>0)
  {
    nbytes = n > 32767 ? 32767 : n;
    status = read( scb[scb_id], buff, nbytes );
    if (status == -1) break;
    bytesread += status;
    n -= nbytes;
    buff += 32767;
  }

  return bytesread;
}


word wbytes( buff, n )
string buff;
word n;
{
  int *scb = cos;
  int nbytes;
  int status = 0;

  while( (status != -1) && (n>0))
  {
   nbytes = n > 32767 ? 32767 : n;
   status = write( scb[scb_id],buff, nbytes);
   n -= nbytes;
   buff += 32767;
  }
  return (long)status;
}


selectinput(scb)
word scb;
{
  cis = (int*)scb;
}


selectoutput(scb)
word scb;
{
  if ( cos != NULLPTR )
    bflush();

  cos = (int*)scb;
}


word input()
{
  return (word)cis;
}



word output()
{
  return (word)cos;
}



word findinput( name,rawflag )
string name;
word rawflag;
{
  return bopen(name, rdflag, rawflag);
}


word findoutput( name ,rawflag)
string name;
word rawflag;
{
  return bopen(name, wrflag, rawflag);
}


endread( )
{
  if ( cis != NULLPTR )
    bclose(cis);
  cis = NULLPTR;
}


endwrite( )
{
  if ( cos != NULLPTR )
  {
    bflush();
    bclose(cos);
  }

  cos = NULLPTR;
}


word readwords( buf , n )
word *buf, n;
{
  return (readbytes(buf, n*bytesper68000word) / bytesper68000word);
}


word writewords( buf, n )
word *buf, n;
{
  return (wbytes(buf, n*bytesper68000word) / bytesper68000word);
}


word pointword( n )
word n;
{
  int* scb = cis;
  scb[scb_pos] = 0;
  scb[scb_max] = 0;

  return (word)(lseek( scb[scb_id], n*bytesper68000word, 0 ));
}


newline ( )
{
   wrch ( '\n' );
}


writed ( n, d )
word n;
int d;
{
   int	j;
   int	i = 0;
   int	t[10];

   word k = -n;

   if ( n < (word)0 )
   {
      d--;
      k = n;
   }

   do
   {
      t[i++] = - (int)((k % (word)(10)));
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
word n;
{
   writed( n, 0 );
}


writehex ( n, d )
word n;
int  d;
{
   static int digits[] = {
      '0', '1', '2', '3', '4', '5', '6', '7',
      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

   if ( d > 1 )
      writehex ( n >> 4, d - 1 );

   wrch ( digits[(n & (word)(15))] );
}


/*********************************************/
/*  Linker redefines WRITEOCT, hence removal */
/*********************************************/
/* writeoct ( n, d )			     */
/* word n;				     */
/* int d;				     */
/* {					     */
/*    if ( d > 1 )			     */
/*	 writeoct ( n >> 3, d - 1 );	     */
/*    wrch ( (int)((n & (word)(7)) + '0') ); */
/* }					     */
/*********************************************/

extern int writeoct();



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
word n;
int d;
{
   word m = ( n >> 1 ) / 5;

   if ( m != 0 )
   {
      writed ( m, d - 1 );
      d = 1;
   }

   writed ( n - ( m * (word)(10) ), d );
}


writef ( format, a, b, c, d, e, f, g, h, i, j, k )
string format;
word a,b,c,d,e,f,g,h,i,j,k;
{
   int p;
   word *t =  & a;

   for ( p = 1; p <= format[0]; p++ )
   {
      int k = format[p];

      if ( k == '%' )
      {
	 int (*f)();
	 word arg = t[ 0 ];
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
		 f = writet;
		 goto m;
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

   for ( i = 1 ; i <= smaller[ 0 ] ; i++ )
   {
      int res = compch( (int)s1[i], (int)s2[i] );
      if ( res != 0 ) return res;
   }

   if ( lens1 == lens2 ) return 0;

   return (smaller == s1) ? -1 : 1;
}


word readn ( )
{
   word sum = 0;
   int ch   = 0;
   int neg  = false;
l:
   ch = rdch();

   if ( ! ( ('0' <= ch) && (ch <= '9') ) )
      switch ( ch )
      {
	default:
	   unrdch();
	   result2 = (word)-1;
	   return 0;
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
      sum = sum*(word)(10) + (ch - '0');
      ch = rdch();
   }

   if ( neg )
      sum =  - sum;

   unrdch ();
   result2 = (word)0;

   return sum;
}


word *getvec( upb )
word upb;
{
   return (word*) malloc( (upb+1)*bytesper68000word );
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


int getbyte(v,offset)
string v;
word offset;
{
  return (int)(v[offset]);
}


putbyte(v,offset,b)
string v;
word offset;
int b;
{
  v[offset] = b;
}
word gbytes(v,size)
string v;
int size;
{
  word c = 0;
  int  p = 0;

  while (size>p)
    c = (c<<8) + (((word)(v[p++])) & 0xFF);

  return c;
}

pbytes(v,size,w)
string v;
int    size;
word   w;
{
  while( size>0 )
  {
    v[--size] = (w & 0xff);
    w = w>>8;
  }
}

word note( scb, p )
int  *scb;
word *p;
{
  word bytepos;

  if ( scb = cos )
    bflush();

  bytepos = (word)(lseek(scb[scb_id], (word)0, 1));

  if ( scb = cis )
    bytepos += scb[scb_max] - scb[scb_pos];

  *p = bytepos;

  return bytepos;
}


word point(scb,p)
int  *scb;
word *p;
{
  if ( scb = cos )
    bflush();
  else
  {
    scb[scb_pos] = 0;
    scb[scb_max] = 0;
  }

  return (word)(lseek( scb[scb_id], *p, 0 ));
}

/*********************************************************/
/*  Define 'main' to do initialisation, then call START  */
/*********************************************************/

main(argc, argv)
int argc;
char *argv[];
{
   int i;
   int *scb;

   cargc = argc;
   for ( i=0; i<=argc; i++ )
     cargv[i] = argv[i];

   wrch_fn   = default_wrch;
   rdch_fn   = default_rdch;
   unrdch_fn = default_unrdch;

/* Set up default Input/Output SCBs */

   cis = getscb( 0 );
   cos = getscb( 1 );

   if  ( (cis == NULLPTR) || (cos == NULLPTR) )
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

word argword;
word argchptr;

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
word *vword;
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


word rdargs ( keys, argv, size )
string keys;
word *argv;
int size;
{
   int	p,numbargs;
   word *w = argv;

   int (*saferdch)()   = rdch_fn;
   int (*safeunrdch)() = unrdch_fn;

#ifdef DBUG
   mydebug("rdargs","keys = %s\n    argv = %x8; size = %n",keys,argv,(word)size);
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
	    (*w) = (*w) | (word)1;

	 if ( c == 'K' )
	    (*w) = (*w) | (word)2;

	 if ( c == 'S' )
	    (*w) = (*w) | (word)4;

	 continue;
      }

      if ( kch == ',' )
      {
	 w++;
	 (*w) = 0;
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
      int  wsize = size + (int)(argv - w);

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
		     argv[i] = (word)0;
		  }
		  else
		     goto err;
	    }

	    rdch();

	    rdch_fn   = saferdch;
	    unrdch_fn = safeunrdch;

	    return (word)w;


	 case 1:   /*  ordinary item */

	    argno = findarg(keys, w);

	    if ( argno >= 0 )	/*  get and check argument */
	    {
	       if ( (4 <= argv[ argno ]) && (argv[ argno ] <= 7) )
	       {
		  /*  no value for key. */
		  argv[ argno ] =  -1;
		  continue;
	       }
	       else
	       {
		  word item = rditem(w, wsize);
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
	       word i;

	       for ( i = 0 ; i <= (numbargs - 1) ; i++ )
		  switch ( argv[ i ] )
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
	      argv[argno] = (word)(s);
	      w += ( s[0]/bytesperword ) + 1;

#ifdef DBUG
	      mydebug("rdargs","arg%n = %s @s = %x8",(word)argno,s,s);
#endif
	    }
      }

   }
       while ( true );

}


