AS	= masm
ASS     =  /V /ZI
ASF	=
LN  	= link


amouse.obj : amouse.asm egalib.asm graphc.asm init.asm
	$(AS) $(ASF) amouse.asm,, ;


amouse.com : amouse.obj 
	$(LN) amouse ;
	exe2bin amouse.exe amouse.com
