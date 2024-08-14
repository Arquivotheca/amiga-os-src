CC	= cl
CF	= /AL /c /FoOBJ\ /Otil /Za /Zi
AS	= masm
ASF     =  /ZI
LN  	= link
LF	= /CO

OBJ\atime.obj : atime.c
	$(CC) $(CF) atime.c

OBJ\asettime.obj : asettime.asm
	$(AS) $(ASF) asettime.asm,OBJ\ASetTime.obj ;

asettime.exe : OBJ\asettime.obj 
	$(LN) $(LF) OBJ\asettime , ,NUL.MAP, ;
	zwrite asettime.asm pcinstall:docs/developer/examples/asettime/ASetTime.asm
	zwrite asettime.doc pcinstall:docs/user/ASetTime.DOC
	zwrite asettime.txt pcinstall:docs/developer/examples/asettime/ASetTime.ReadMe
	zwrite makefile     pcinstall:docs/developer/examples/asettime/MakeFile

atime.exe : OBJ\atime.obj
	$(LN) $(LF) OBJ\atime, ,NUL.MAP , ljanus ;
	zwrite atime.c   pcinstall:docs/developer/examples/atime/ATime.c
	zwrite atime.doc pcinstall:docs/user/ATime.DOC
	zwrite atime.txt pcinstall:docs/developer/examples/atime/ATime.ReadMe
	zwrite makefile  pcinstall:docs/developer/examples/atime/MakeFile

