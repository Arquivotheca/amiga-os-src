CC	= cl
CF	= /AL /c /FoOBJ\ /Otil /Za /Zi
AS	= masm
ASF     =  /ZI
LN  	= link
LF	= /CO

OBJ\lockc.obj : lockc.c
	$(CC) $(CF) lockc.c

OBJ\lock.obj : lock.asm
	$(AS) $(ASF) lock.asm,OBJ\lock.obj ;

lock.exe : OBJ\lock.obj 
	$(LN) $(LF) OBJ\lock , ,NUL.MAP, ;

lockc.exe : OBJ\lockc.obj
	$(LN) $(LF) OBJ\lockc, ,NUL.MAP , ljanus ;

