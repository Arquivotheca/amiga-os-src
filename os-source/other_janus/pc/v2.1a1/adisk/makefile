CC	= cl
CF	= /AL /c /Otil /Za /Zi
AS	= masm
ASF     =  /ZI
LN  	= link
LF	= /CO

adisk.obj : adisk.asm
	$(AS) $(ASF) adisk.asm,Adisk.obj ;

adisk.exe : adisk.obj 
	$(LN) $(LF) OBJ\adisk , ,NUL.MAP, ;

