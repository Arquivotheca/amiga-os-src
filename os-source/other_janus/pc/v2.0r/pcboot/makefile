AS   = masm5
ASF  = /V /ZI
LN   = link5 /CO
OBJ1 = 	OBJ\irq3ser.obj OBJ\ji_han.obj 	OBJ\ji_en.obj	OBJ\load_irq.obj	
OBJ2 =	OBJ\hdpart.obj  OBJ\hdint.obj 	OBJ\int86.obj	OBJ\memrw.obj
OBJ3 = 	OBJ\outdec.obj  OBJ\outhxw.obj	OBJ\chgint.obj	OBJ\var.obj
OBJ4 =	OBJ\service1.obj OBJ\service2.obj OBJ\abhandle.obj OBJ\scroll.obj

janus.bin : $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)
	$(LN) @janus.lnk
	exe2bin janus janus.bin
	dir janus.bin
	calc12 janus
	zwrite janus.bin pcinstall:pc/system/pc.boot /b

		
OBJ\LOAD_IRQ.obj :	LOAD_IRQ.asm 
	$(AS) $(ASF) LOAD_IRQ.asm,obj\load_irq.obj,lst\load_irq.lst; 

OBJ\IRQ3SER.obj :  	IRQ3SER.asm 
	$(AS) $(ASF) IRQ3SER.asm,obj\irq3ser.obj,lst\irq3ser.lst; 

OBJ\SERVICE1.obj :  	SERVICE1.asm 
	$(AS) $(ASF) SERVICE1.asm,obj\service1.obj,lst\service1.lst;

OBJ\SERVICE2.obj :  	SERVICE2.asm 
	$(AS) $(ASF) SERVICE2.asm,obj\service2.obj,lst\service2.lst;

OBJ\SCROLL.obj :  	SCROLL.asm
	$(AS) $(ASF) scroll.asm,obj\scroll.obj,lst\scroll.lst; 

OBJ\JI_HAN.obj :   	JI_HAN.asm 
	$(AS) $(ASF) JI_HAN.asm,obj\ji_han.obj,lst\ji_han.lst; 

OBJ\JI_EN.obj :    	JI_EN.asm 
	$(AS) $(ASF) JI_EN.asm,obj\ji_en.obj,lst\ji_en.lst;

OBJ\CHGINT.obj :   	CHGINT.asm 
	$(AS) $(ASF) CHGINT.asm,obj\chgint.obj,lst\chgint.lst;

OBJ\HDPART.obj :   	HDPART.asm 
	$(AS) $(ASF) HDPART.asm,obj\hdpart.obj,lst\hdpart.lst;

OBJ\HDINT.obj :    	HDINT.asm 
	$(AS) $(ASF) HDINT.asm,obj\hdint.obj,lst\hdint.lst; 

OBJ\INT86.obj :    	INT86.asm 
	$(AS) $(ASF) INT86.asm,obj\int86.obj,lst\int86.lst; 

OBJ\MEMRW.obj :    	MEMRW.asm 
	$(AS) $(ASF) MEMRW.asm,obj\memrw.obj,lst\memrw.lst; 

OBJ\OUTHXW.obj :   	OUTHXW.asm 
	$(AS) $(ASF) OUTHXW.asm,obj\outhxw.obj,lst\outhxw.lst;

OBJ\OUTDEC.obj :   	OUTDEC.asm 
	$(AS) $(ASF) OUTDEC.asm,obj\outdec.obj,lst\outdec.lst;

OBJ\ABHANDLE.obj :  	ABHANDLE.asm abequ.inc abdata.inc abvars.inc
	$(AS) $(ASF) abhandle.asm,obj\abhandle.obj,lst\abhandle.lst;

OBJ\VAR.obj :      	VAR.asm 
	$(AS) $(ASF) VAR.asm,obj\var.obj,lst\var.lst;

