CC	= cl
CF	= /AL /c /FoOBJ\ /Otil /Za /Zi
AS	= masm
ASF     =  /ZI
LN  	= link
LF	= /CO

OBJ\lock.obj : lock.asm
	$(AS) $(ASF) lock.asm,OBJ\lock.obj ;

lock.exe : OBJ\lock.obj 
	$(LN) $(LF) OBJ\lock , ,NUL.MAP, ;

