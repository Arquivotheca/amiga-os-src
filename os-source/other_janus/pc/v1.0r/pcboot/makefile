AS   = c:\microa\masm
ASF  =
LN   = link
OBJ1 = load_irq.obj irq3ser.obj ji_han.obj ji_en.obj chgint.obj
OBJ2 = hdpart.obj   hdint.obj   int86.obj  memrw.obj outhxw.obj
OBJ3 = outdec.obj   var.obj

l.bin : $(OBJ1) $(OBJ2) $(OBJ3)
	link @load_irq.lnk
	exe2bin load_irq l.bin
	dir *.bin
	calc4 l
	awrite l.bin pcboot:pc.boot /b /nc

LOAD_IRQ.obj : LOAD_IRQ.asm dskstruc.inc janusvar.inc macros.inc equ.inc pc_at.inc mes.inc data.inc
	$(AS) $(ASF) LOAD_IRQ.asm,,load_irq.lst ;

IRQ3SER.obj :  IRQ3SER.asm janusvar.inc macros.inc vars_ext.inc mes.inc equ.inc
	$(AS) $(ASF) IRQ3SER.asm,,irq3ser.lst ;

JI_HAN.obj :   JI_HAN.asm macros.inc data.inc equ.inc handler.inc janusvar.inc mes.inc vars_ext.inc  
	$(AS) $(ASF) JI_HAN.asm,,ji_han.lst    ;

JI_EN.obj :    JI_EN.asm equ.inc
	$(AS) $(ASF) JI_EN.asm,,ji_en.lst ;

CHGINT.obj :   CHGINT.asm 
	$(AS) $(ASF) CHGINT.asm,,chgint.lst ;

HDPART.obj :   HDPART.asm dskstruc.inc macros.inc vars_ext.inc mes.inc equ.inc
	$(AS) $(ASF) HDPART.asm,,hdpart.lst ;

HDINT.obj :    HDINT.asm macros.inc dskstruc.inc words.inc mes.inc equ.inc data.inc
	$(AS) $(ASF) HDINT.asm,,hdint.lst     ;

INT86.obj :    INT86.asm syscall.inc macros.inc equ.inc
	$(AS) $(ASF) INT86.asm,,int86.lst ;

MEMRW.obj :    MEMRW.asm memrw.inc words.inc macros.inc data.inc
	$(AS) $(ASF) MEMRW.asm,,memrw.lst ;

OUTHXW.obj :   OUTHXW.asm equ.inc
	$(AS) $(ASF) OUTHXW.asm,,outhxw.lst ;

OUTDEC.obj :   OUTDEC.asm macros.inc
	$(AS) $(ASF) OUTDEC.asm,,outdec.lst ;

VAR.obj :      VAR.asm equ.inc dskstruc.inc vars.inc
	$(AS) $(ASF) VAR.asm,,var.lst ;


