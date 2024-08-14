CC	= cl
CF	= /AL /c /Otil /Za /Zi
AS	= masm
ASF     =
LN  	= link
LF	=

setup.obj : setup.asm
	$(AS) $(ASF) setup.asm,setup.obj ;

setup.exe: setup.obj 
	$(LN) $(LF) setup.obj ,setup.exe,NUL.MAP, ;

setup.com: setup.exe
	exe2bin setup.exe setup.com
