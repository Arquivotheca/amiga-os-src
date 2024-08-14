AS	=	masm
ASF	=	/V /ZI
LN	=	link
LF	=	/CO

OBJ\jdisk.obj : jdisk.asm
	$(AS) $(ASF) jdisk.asm,OBJ\jdisk.obj ;

jdisk.sys : OBJ\jdisk.obj
	$(LN) $(LF) OBJ\jdisk , ,NUL.MAP,;
	exe2bin jdisk jdisk.sys
	del jdisk.exe
