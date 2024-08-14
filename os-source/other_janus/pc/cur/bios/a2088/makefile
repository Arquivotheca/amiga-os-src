A	= masm
AOPT	= 
L	= plink86

OBJ1	=  filler.obj 	romdat.obj	initcd.obj	fdboot.obj
OBJ2	=  eiadsr.obj	kybdsr.obj	dskdsr.obj	prtdsr.obj		
OBJ3	=  crtdsr.obj	misdsr.obj	timdsr.obj	romfnt.obj
OBJ4	=  vectbl.obj	prtscr.obj	powrup.obj 
ALLINC	=  romopt.inc 	romequ.inc   	macros.inc 		
TEST1	=  test_01.inc	test_10.inc	test_11.inc	test_20.inc	test_30.inc
TEST2	=  test_40.inc	test_50.inc	test_60.inc	test_61.inc	test_62.inc
TEST3	=  test_80.inc	test_A0.inc	test_C2.inc	test_C3.inc	
TEST4	=  test_E1.inc	test_E2.inc	test_E3.inc	test_E4.inc    
TEST5	=  test_F1.inc	test_F2.inc	test_F3.inc 	testmisc.inc	testram.inc
TEST	=  $(TEST1) $(TEST2) $(TEST3)	$(TEST4)   
	
		
filler.obj:  filler.asm $(ALLINC) kybequ.inc
	$(A) $(AOPT) >report filler;

romdat.obj:  romdat.asm romopt.inc macros.inc 
	$(A) $(AOPT) >>report romdat;
							
initcd.obj:  initcd.asm $(ALLINC) miscl1.inc miscl2.inc kybequ.inc lghtpn.inc diag.inc 
	$(A) $(AOPT) >>report initcd;
		
fdboot.obj:  fdboot.asm $(ALLINC) 
	$(A) $(AOPT) >>report fdboot;

eiadsr.obj:  eiadsr.asm $(ALLINC) 
	$(A) $(AOPT) >>report eiadsr;

kybdsr.obj:  kybdsr.asm $(ALLINC) kybequ.inc
	$(A) $(AOPT) >>report kybdsr;

dskdsr.obj:  dskdsr.asm $(ALLINC) 
	$(A) $(AOPT) >>report dskdsr;

prtdsr.obj:  prtdsr.asm $(ALLINC) 
	$(A) $(AOPT) >>report prtdsr;

crtdsr.obj:  crtdsr.asm $(ALLINC) grfxsb.inc
	$(A) $(AOPT) >>report crtdsr;

misdsr.obj:  misdsr.asm $(ALLINC) miscl1.inc
	$(A) $(AOPT) >>report misdsr;

timdsr.obj:  timdsr.asm $(ALLINC) 
	$(A) $(AOPT) >>report timdsr;

romfnt.obj:  romfnt.asm $(ALLINC) fontgn.inc
	$(A) $(AOPT) >>report romfnt;

vectbl.obj:  vectbl.asm $(ALLINC)  
	$(A) $(AOPT) >>report vectbl;

prtscr.obj:  prtscr.asm $(ALLINC) 
	$(A) $(AOPT) >>report prtscr;

powrup.obj:  powrup.asm $(ALLINC) 
	$(A) $(AOPT) >>report powrup;


	       	
rom.bin: $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)
	del ROM.EXE
	$(L) @ROM symtab
	ROMCNV ROM abs hex com frame 0F000 offset 0C000
	copy rom.com rom.bin
	del rom.com

