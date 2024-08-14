/* printa : Automatically translated from BCPL to C on 15-Apr-85 */
/*  PRINTA file [NOHUNK] */

/* Displays Amiga binary object file structure; */
/* The code contained in hunks is printed unless NOHUNK is specified. */

#include "libhdr";

#define hunk_puname   999
#define hunk_name    1000
#define hunk_code    1001
#define hunk_data    1002
#define hunk_bss     1003
#define hunk_reloc32 1004
#define hunk_reloc16 1005
#define hunk_reloc8  1006
#define hunk_ext     1007
#define hunk_symbol  1008
#define hunk_debug   1009
#define hunk_end     1010
#define hunk_header  1011
#define hunk_cont    1012
#define hunk_overlay 1013
#define hunk_break   1014

#define ext_symb	0
#define ext_defr	1
#define ext_defa	2
#define ext_ref32     129
#define ext_common    130
#define ext_ref16     131
#define ext_ref8      132

start ( )
{
   string  *argv[20];
   SCBPTR ins = NULL;

   if ( rdargs("\017FROM/A,NOHUNK/S", argv, 20) == NULL )
   {
      writes ( "\011Bad args\n" );
      stop (1);
   }

   ins = findinput(argv[0], true);
   if ( ins == NULL )
   {
      writef ( "\022Unable to open %S\n", (long)(argv[0]));
      stop (1);
   }

   selectinput (ins);
   display( (argv[1] == NULL) );
   endread();
}


display ( printhunk )
int printhunk;
{
   while (true)
   {
      long htype;
      long size;
      long *v;

      if (!getlong(&htype)) return;

      switch ( (int)htype )
      {
	case hunk_end:
	   writes ( "\006T.END\n" );
	   continue;

	case hunk_break:
	   writes ( "\010T.BREAK\n" );
	   continue;

	case hunk_name:
	case hunk_puname:
	   writes ( ((htype == hunk_name) ? "\012HUNK.NAME\n" :
					   "\014HUNK.PUNAME\n") );

	   if (!getlong(&size)) goto errorlab;

	   printc( size );
	   newline();
	   continue;

      case hunk_code:
      case hunk_data:
	 writes ( htype == hunk_code ? "\012HUNK.CODE\n" :
				      "\012HUNK.DATA\n" );

	 if (!getlong(&size)) goto errorlab;

	 v = (long*)getvec( (int)(size*wordsper68000word) );
	 if ( v == NULL )
	 {
	    writes ( "\021Run out of space\n" );
	    return;
	 }

	 if (!getwords(v, size)) goto errorlab;

	 if ( printhunk || ( htype == hunk_name ) )
	 {
	    long i;
	    int j=0;

	    for ( i = 0 ; i <= ( size - 1 ) ; i++ )
	    {
	      writelong( v[i] );
	      if ( (++j % 4) == 0)
		 wrch('\n');
	    }
	 }
	 newline();
	 freevec(v);
	 continue;
      case hunk_bss:
	 if (!getlong(&size)) goto errorlab;

	 writes ( "\012hunk.bss [" );
	 writelong( size );
	 writes( "\002]\n" );

	 continue;

      case hunk_reloc32:
      case hunk_reloc16:
      case hunk_reloc8:
	 while (true)
	 {
	    long hnum;

	    if (!getlong(&size)) goto errorlab;
	    if (size == 0L) break;

	    writef ( "\015HUNK.RELOC%N ",
		     htype == hunk_reloc32 ? (long)32 :
		     htype == hunk_reloc16 ? (long)16 : (long)8 );

	    if (!getlong(&hnum)) goto errorlab;

	    writef ( "\011Hunk = %N", hnum );
	    printw ( size );
	 }
	 continue;

      case hunk_symbol:
	 if (!getlong(&size)) goto errorlab;

	 writef ( "\014HUNK.SYMBOL\n" );

	 while (true)
	 {
	    long val;
	    if (size == 0L) break;
	    printc(size);
	    if (!getlong(&val)) goto errorlab;
	    writef ( "\013 VAL = %X8\n", (long)val );
	    if (!getlong(&size)) goto errorlab;
	 }
	 continue;

      case hunk_ext:
	 while (true)
	 {
	   long code;
	   unsigned t;

	   if (!getlong(&code)) goto errorlab;
	   if (code == 0L) break;

	   size = code & 0xFFFFFF;
	   t = (code >> 24) & 0xFF;

	   writef ( "\015HUNK.EXT [%N]", (long)t );
	   printc ( size );

	   switch ( t )
	   {
	     case ext_ref32:
	     case ext_ref16:
	     case ext_ref8:
		if (!getlong(&size)) goto errorlab;
		printw(size);
		break;
	     case ext_defr:
		if (!getlong(&v)) goto errorlab;
		writef ( "\016Rel Val = %X8\n", v );
		break;
	     case ext_defa:
		if (!getlong(&v)) goto errorlab;
		writef( "\016Abs Val = %X8\n", v );
		break;
	     default:
		writef( "\024Unknown EXT byte %N\n", (long)t );
		return;
	   }
	 }

	 continue;

      case hunk_header:
	 {
	    long f;
	    long l;

	    writef ( "\014HUNK.HEADER\n" );

	    while (true)
	    {
	       if (!getlong(&size)) goto errorlab;
	       if (size == 0L) break;
	       printc(size);
	    }

	    if (!getlong(&size)) goto errorlab;
	    if (!getlong(&f))	 goto errorlab;
	    if (!getlong(&l))	 goto errorlab;

	    writef ( "\005[%N] ", size );
	    writef ( "\005[%N] ", f );
	    writef ( "\005[%N] ", l );

	    printw ( ( l - f ) + 1 );
	 }
	 continue;

      case hunk_cont:
	 {
	    long n;
	    long o;

	    if (!getlong(&n))	 goto errorlab;
	    if (!getlong(&o))	 goto errorlab;
	    if (!getlong(&size)) goto errorlab;

	    writef ( "\023HUNK.CONT [%N %X8]\n", n, o );

	    printw ( size );
	 }
	 continue;

      case hunk_overlay:
	 {
	    long t;
	    long m;

	    if (!getlong(&t))	 goto errorlab;
	    if (!getlong(&size)) goto errorlab;

	    writef ( "\025HUNK.OVERLAY [%N %N]\n", t, size );

	    m = size - 2;
	    printw ( m + 1 );
	    printw ( t - (m + 1) );
	 }
	 continue;

      default:
	 writef ( "\021Unknown code %X8\n", htype );
	 return;
      }

errorlab:
      writes ( "\017Unexpected eof\n" );
      return;

   }
}

int getwords( v, s )
char *v;
long s;
{
    int nbytes = s*bytesper68000word;
    int ch;
    int i;

    for (i=0; i<nbytes; i++)
      v[i] = (ch = rdch());

    return (ch != endstreamch);
}

#ifdef SUN
int getlong( v )
{
  return getwords( v, (long)1 );
}

writelong( val )
long val;
{
  writef ( "\004%X8 ", val );
}
#endif

#ifdef IBM
int getlong( v )
long *v;
{
  int  i;
  int ch;

  for (i=0; i<4; i++)
    ((char*)(v))[3-i] = (ch = rdch());

  return (ch != endstreamch);
}

writelong( val )
long val;
{
  int i;
  char *buff = (char*)(&val);

  for (i=0; i<=3; i++)
    writehex ( (long)(buff[i]), 2 );

  wrch(' ');
}
#endif

printw ( size )
long size;
{
   long i;
   long code;

   for ( i = 1 ; i <= size ; i++ )
   {
      getlong(&code);
      writef ( "\006 [%X8]", code );
   }

   newline();
}


printc ( size )
long size;
{
   long i;
   long t;

   wrch ( '[' );

   for ( i = 1 ; i <= size ; i++ )
   {
      int j;
      getlong(&t);

      for ( j = 0 ; j <= 3 ; j++ )
      {
	 int ch = (t & 0xFF000000) >> 24;
	 t <<= 8;
	 wrch ( ch == 0 ? '.' : ch );
      }
   }

   wrch ( ']' );
}


