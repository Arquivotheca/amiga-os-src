// FROM  DoCop()


#if 0
	CMOVE( &td->UCop, custom.bpl1mod, 0x0036 );
	CMOVE( &td->UCop, custom.bpl2mod, 0x0036 );

	CWAIT( &td->UCop, CDY_START,0 );
//	CMOVE( &td->UCop, custom.diwstrt, 0x1581 );
//	CMOVE( &td->UCop, custom.diwstop, 0x06e1 );
	CMOVE( &td->UCop, custom.ddfstrt, 0x0038 );
	CMOVE( &td->UCop, custom.bpl1mod, 0x0036 );
	CMOVE( &td->UCop, custom.bpl2mod, 0x0036 );


	CWAIT( &td->UCop, CDY_END-2,0 );
	CMOVE( &td->UCop, custom.bpl1mod, 0x34 );
	CMOVE( &td->UCop, custom.bpl2mod, 0x34 );


	CWAIT( &td->UCop, CDY_END,0 );
//	CMOVE( &td->UCop, custom.diwstrt, 0x1571 );
//	CMOVE( &td->UCop, custom.diwstop, 0x06e1 );
	CMOVE( &td->UCop, custom.ddfstrt, 0x0030 );
	CMOVE( &td->UCop, custom.bpl1mod, 0x0034 );
	CMOVE( &td->UCop, custom.bpl2mod, 0x0034 );
#endif

