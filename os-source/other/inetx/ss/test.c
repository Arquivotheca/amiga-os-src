/* program to test development of the shared socket.lib */

/* MANX 50E1 !! */

/* multiple function call args */

#include <functions.h>
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <stdio.h>
#include "sslib.h"

#define OUTPUT(x) Write(Output(),(char *)(x),(long)strlen((char *)(x)))

/* my defined library base */

struct SSLib *SSBase ;

void output( char *str ) ;

extern int Func0( void ) ;
extern int Func1( void ) ;
extern int Func2( long arg ) ;
extern int Func3( void ) ;
extern int Func4( void ) ;
extern int Func5( void ) ;
extern int Func6( void ) ;
extern int Func7( void ) ;

struct Library *InetBase ;
#define INETLIBNAME "inet:libs/inet.library"

#pragma amicall(SSBase, 0x1e, Func0())
#pragma amicall(SSBase, 0x24, Func1())
#pragma amicall(SSBase, 0x2a, Func2(a0))
#pragma amicall(SSBase, 0x30, Func3())
#pragma amicall(SSBase, 0x36, Func4())
#pragma amicall(SSBase, 0x3c, Func5())
#pragma amicall(SSBase, 0x42, Func6())
#pragma amicall(SSBase, 0x48, Func7())

#pragma regcall(output(A0))

VOID _cli_parse( struct Process *p, char *a) {}   /* make stubs of these */
VOID _wb_parse( struct Process *p, struct WBStartup *wbm ) {}

main()
{
	UBYTE buffer[256] ;
			
	if(InetBase = (struct Library *)OpenLibrary(INETLIBNAME,0L ))
		{
		mysprintf(buffer,"InetBase   = %08lx\n", InetBase) ;
		output((char *)buffer) ;
		CloseLibrary( InetBase ) ;
		}
	else
		{ 
		mysprintf(buffer,"CAN'T OPEN INET LIBRARY IN TEST.C\n") ;
		output((char *)buffer) ;
		}

	if(SSBase = (struct SSLib *)OpenLibrary( "ss.library", 1L ))
		{
		mysprintf(buffer, "ThisProc   = %08lx\n", (ULONG)FindTask(0L)) ;
		output((char *)buffer) ;
		mysprintf(buffer, "SSBase     = %08lx\t(library base)\n", SSBase ) ;
		output((char *)buffer) ;
		
		mysprintf( buffer, "function 0 = %08lx\n", Func0()) ;
		output((char *)buffer) ; 

		mysprintf( buffer, "function 1 = %08lx\t(inetbase)\n", Func1()) ;
		output((char *)buffer) ;
		mysprintf( buffer, "function 3 = %08lx\n", Func3()) ;
		output((char *)buffer) ;
		mysprintf( buffer, "function 4 = %08lx\n", Func4()) ;
		output((char *)buffer) ;
		mysprintf( buffer, "function 5 = %08lx\n", Func5()) ;
		output((char *)buffer) ;
		mysprintf( buffer, "function 6 = %08lx\n", Func6()) ;
		output((char *)buffer) ;   
		mysprintf( buffer, "function 7 = %08lx\n", Func7()) ;
		output((char *)buffer) ;

		mysprintf( buffer, "func2 (C)  = %08lx\t(%ld)\n",Func2(256L),Func2(256L)) ;
		output((char *)buffer) ;


		CloseLibrary( SSBase ) ;
		
		}
	else 
		output("no lib open\n") ;
	
	return(0) ;
		
}



VOID
output( register char *string )
{
	OUTPUT(string) ;
}


/*
* mysprintf - uses RawDoFmt.  Can't be in C  !
*/

;
#asm

	XREF _LVORawDoFmt

SysBase	EQU	4

_mysprintf:
		movem.l	a2/a3/a6,-(a7)
		move.l	20(a7),a0
		lea	24(a7),a1
		lea	myputstr(pc),a2
		move.l	16(a7),a3
		move.l	SysBase,a6
		jsr	_LVORawDoFmt(a6)
		movem.l	(a7)+,a2/a3/a6
		rts

myputstr:
		move.b	d0,(a3)+
		rts

#endasm
;
	


