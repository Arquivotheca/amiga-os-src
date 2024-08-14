CC	= cl
CF	= /AL /c /Otil /Za /Zi
AS	= masm
ASF     =
LN  	= link
LF	=

adisk.obj : adisk.asm
	$(AS) $(ASF) adisk.asm,Adisk.obj ;

adisk.exe: adisk.obj 
	$(LN) $(LF) adisk.obj ,adisk.exe,NUL.MAP, ;

adisk.com: adisk.exe
	exe2bin adisk.exe adisk.com
