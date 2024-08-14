

/**********************************************************************
*
* test.c -- main routine for testing janus.library
*
* Copyright (c) 1986, Commodore Amiga Inc., All rights reserved
*
**********************************************************************/



#include "exec/types.h"
#include "exec/nodes.h"
#include "exec/lists.h"
#include "exec/ports.h"
#include "exec/memory.h"
#include "exec/interrupts.h"

#include "hardware/custom.h"
#include "hardware/intbits.h"

#include "janus.h"
#include "janusvar.h"
#include "janusreg.h"
#include "services.h"
#include "memrw.h"

extern struct Custom custom;

struct JanusAmiga *JanusBase;

struct Interrupt keyInt;

void
main()
{

    JanusBase = (struct JanusAmiga *) OpenLibrary( "janus.library", 0 );
    kprintf("janus.library = 0x%lx\n", JanusBase);

    if( ! JanusBase ) return;

    /* OK.  library is open, test a few functions. */
    testmem();
    testints();
    testoffset();
    testmemtype();
    testsendint();
    testreadwrite();

    /* now try the real stuff */
    /* ---- test the keyboard interrupt */
    testkeyboard();



   
    CloseLibrary( JanusBase );
}

struct memarray {
    UWORD size;
    UWORD type;
    ULONG address;
};



struct memarray testarray[] = {
    { 10, MEMF_PARAMETER, 0 },
    { 100, MEMF_PARAMETER, 0  },
    { 10, MEMF_PARAMETER | MEM_BYTEACCESS, 0 },
    { 10, MEMF_PARAMETER | MEM_WORDACCESS, 0 },
    { 10, MEMF_PARAMETER | MEM_GRAPHICACCESS, 0 },
    { 10, MEMF_PARAMETER | MEM_IOACCESS, 0 },
    { 10, MEMF_BUFFER	 | MEM_BYTEACCESS, 0 },
    { 10, MEMF_BUFFER	 | MEM_WORDACCESS, 0 },
    { 10, MEMF_BUFFER	 | MEM_GRAPHICACCESS, 0 },
    { 0, 0, 0 }
};

testmem()
{
    struct memarray *ma;

    for( ma = testarray; ma->size; ma++ ) {
	kprintf( "AllocJanusMem( %ld, %ld )", ma->size, ma->type );
	ma->address = AllocJanusMem( ma->size, ma->type );
	kprintf( " = 0x%lx\n", ma->address );
    }

    for( ma = testarray; ma->size; ma++ ) {
	kprintf( "FreeJanusMem( 0x%lx, %ld )\n", ma->address, ma->size );
	FreeJanusMem( ma->address, ma->size );
    }
}

VOID
intserver()
{
    kprintf( "intserver: called\n" );
}

testints()
{

    int     jintnum = 8;
    struct Interrupt intserv;

    intserv.is_Code = (VOID (*)()) intserver;


    SetJanusHandler( jintnum, &intserv );

    kprintf( "before set enable: req 0x%lx, ena 0x%lx\n",
	    JanusBase->ja_IntReq, JanusBase->ja_IntEna );
    SetJanusEnable( jintnum, 1 );

    kprintf( "before forcing int: req 0x%lx, ena 0x%lx\n",
	    JanusBase->ja_IntReq, JanusBase->ja_IntEna );
    PostFakeInt( jintnum );

    kprintf( "after forcing int: req 0x%lx, ena 0x%lx\n",
	    JanusBase->ja_IntReq, JanusBase->ja_IntEna );
    SetJanusHandler( jintnum, 0 );
    SetJanusEnable( jintnum, 0 );

}



testoffset()
{
    UWORD   oldoffset[MAXHANDLER];
    int     i;

    for( i = 0; i < MAXHANDLER; i++ ) {
	oldoffset[i] = GetParamOffset( i );
	SetParamOffset( i, i );
	kprintf( "offset %ld: was %ld, now is %ld\n",
	i, oldoffset[i], GetParamOffset( i ) );
    }

    for( i = 0; i < MAXHANDLER; i++ ) {
	SetParamOffset( i, oldoffset[i] );
    }

    kprintf( "GetJanusStart() = 0x%lx\n", GetJanusStart() );
}

testmemtype()
{

    struct memarray *me;

    /* use the values in test array to check these calls */
    for( me = testarray; me->size; me++ ) {
	kprintf( "JanusMemType( 0x%lx ) = 0x%lx, want 0x%lx\n",
		me->address, JanusMemType( me->address ), me->type );
	if( JanusMemType( me->address ) != me->type ) {
	    kprintf( "JanusMemType( 0x%lx ) = 0x%lx, not 0x%lx\n",
		    me->address, JanusMemType( me->address ), me->type );
	    Debug();
	}
    }

    for( me = testarray; me->size; me++ ) {
	kprintf( "addr 0x%lx, type 0x%lx: ", me->address, me->type );
	kprintf( "JanusMemBase 0x%lx, ", JanusMemBase( me->type ) );
	kprintf( "Offset 0x%lx\n", JanusMemToOffset( me->address ) );
    }
}


testsendint()
{
    int i;
    int oldval;

    for( i = 0; i < MAXHANDLER; i++ ) {
	oldval = CheckJanusInt( i );
	SendJanusInt( i );
	kprintf( "jint %ld: before %ld after %ld\n",
	    i, oldval, CheckJanusInt( i ) );
    }
}

#define BUFMEMSIZE  1024


testreadwrite()
{
    struct MemReadWrite *mrw;
    UBYTE *bufmem;
    int oldoffset;


    /* ok.  do what we can to fake reading and writing */

    /* alloc parameter area */
    if( ! (mrw = (struct MemReadWrite *)
	AllocJanusMem( sizeof( struct MemReadWrite ),
	MEMF_PARAMETER | MEM_WORDACCESS )) )
    {
	kprintf( "testreadwrite: could not allocate param mem\n" );
	return 0; 
    }

    if( ! (bufmem = (UBYTE *)
	AllocJanusMem( BUFMEMSIZE, MEMF_BUFFER | MEM_WORDACCESS )) )
    {
	kprintf( "testreadwrite: could not allocate buffer mem\n" );
	return 0;
    }

    kprintf( "testreadwrite: param 0x%lx, buffer 0x%lx\n", mrw, bufmem );
    oldoffset = GetParamOffset( JSERV_READAMIGA );
    SetParamOffset( JSERV_READAMIGA, JanusMemToOffset( mrw ) );

    mrw->mrw_Command = MRWC_NOP;
    mrw->mrw_Count = 30;
    mrw->mrw_Address = 4;
    mrw->mrw_Buffer = JanusMemToOffset( bufmem );
    mrw->mrw_Status = 0xff00;

    PostFakeInt( JSERV_READAMIGA );

    kprintf( "testreadwrite: after int: status is 0x%lx\n", mrw->mrw_Status );

    mrw->mrw_Status = 0xff00;
    mrw->mrw_Command = MRWC_READ;

    PostFakeInt( JSERV_READAMIGA );

    kprintf( "testreadwrite: after 2 int: stat 0x%lx, data 0x%lx\n",
	    mrw->mrw_Status, * (ULONG *) bufmem );

    SetParamOffset( JSERV_READAMIGA, oldoffset );
    FreeJanusMem( bufmem, BUFMEMSIZE );
    FreeJanusMem( mrw, sizeof( struct MemReadWrite ) );
}


PostFakeInt( jintnum )
int jintnum;
{
    struct JanusAmiga *ja = JanusBase;
    struct JanusBase *jb = (struct JanusBase *) ja->ja_ParamMem;
    UBYTE (*intvec)[2];

    intvec = (UBYTE (*)[2] ) ((ULONG)jb + jb->jb_Interrupts);

    intvec[jintnum][1] = JPCSENDINT;

    ja->ja_LibNode.ja_Flags |= JAF_FAKEINT;
    custom.intreq = INTF_PORTS | INTF_SETCLR;
}


keyintcode()
{
    kprintf("keyboard interrupt received.\n");
}

testkeyboard()
{
    UBYTE *ioBase;

    keyInt.is_Code = (VOID (*)()) keyintcode;
    SetJanusHandler( JSERV_ENBKB, &keyInt );
    SetJanusEnable( JSERV_ENBKB, 1 );
    ioBase = GetJanusStart()+IoAccessOffset+IoRegOffset;	    
    for (;;) {
	kprintf("Enter keycode (hex):");
	ioBase[jio_KeyboardData] = GetHex();		  
	ioBase[jio_PcIntGen] = JPCKEYINT;    
    }
}

GetHex()
{
    int i, c;

    i = 0;
    do {
	c = KGetCh();
	if ((c >= '0') && (c <= '9')) {
	    i *= 16;
	    i += c - '0';
	}
	else if ((c >= 'a') && (c <= 'f')) {
	    i *= 16;
	    i += c - 'a' + 10;
	} 
	else if ((c >= 'A') && (c <= 'F')) {
	    i *= 16;
	    i += c - 'F' + 10;
	} 
	else c = -1;
    }
	while (c != -1);
    return( i );
}

