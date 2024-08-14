	IFGE	INFOLEVEL-255
	MOVEM.L	D0-D1/A0,-(SP)
	PUTMSG	255,<'%s/AllocJanusMem( D0=0x%lx, D1=%ld A0=0x%lx )'>
	LEA	3*4(SP),SP
	ENDC
