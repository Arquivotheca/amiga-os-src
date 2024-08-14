#****************************************************************************
# MakeFile for Janus.lib, PC C function bindings
#****************************************************************************
CC  	= cl
CCF	= /AL /Foobj\ /c /Otil /Za
LN  	= link
LF  = /CO

obj\jlink.obj: jlink.c
	$(CC) $(CCF) jlink.c

jlink.exe: obj\jlink.obj ..\..\dslib\dslib.obj
	$(LN) $(LF) /NOE obj\jlink ..\..\dslib\dslib, ,NUL.MAP , ljanus ;
