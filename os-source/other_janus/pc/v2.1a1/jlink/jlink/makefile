AS	=	masm
ASF	=	/V /ZI
LN	=	link
LF	=	/CO


OBJ\jlink.obj : jlink.asm
	$(AS) $(ASF) jlink.asm,OBJ\jlink.obj ;

jlink.com : OBJ\jlink.obj
	$(LN) $(LF) OBJ\jlink,,NUL.MAP,c:\src\oldjanus\lib\jlib.lib;
	exe2bin jlink jlink.com
	del jlink.exe

