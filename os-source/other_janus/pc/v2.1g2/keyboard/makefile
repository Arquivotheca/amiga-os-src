#**	makefile for keyboard.sys

DEST	=keyboard
MSG	=messages

#	Path definitions

BIOS	=..\..\BIOS
DOS	=..\..\DOS

#	Definitions for assembler

ASM	=masm
AFLAGS	=-Mx -t
AINC	=-I..\..\inc -I$(DOS)

#	Definitions for C compiler

CC	=cl
CFLAGS	=-Ox -Zlp
CINC	=-I..\..\h

#	Definitions for linker

LINK	=link
LIBC	=..\..\libc


# Rules and Dependencies follow
KDF.OBJ:    KDF.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDF;

KDFFR.OBJ:  KDFFR.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFFR;

KDFEOF.OBJ: KDFEOF.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFEOF;

KDFUK.OBJ:  KDFUK.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFUK;

KDFIT.OBJ:  KDFIT.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFIT;

KDFGE.OBJ:  KDFGE.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFGE;

KDFSP.OBJ:  KDFSP.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFSP;

KDFNO.OBJ:  KDFNO.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFNO;

KDFDK.OBJ:  KDFDK.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFDK;

KDFSG.OBJ:  KDFSG.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFSG;

KDFSV.OBJ:  KDFSV.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFSV;

KDFUS.OBJ:  KDFUS.ASM
	 $(ASM) $(AFLAGS) $(AINC) KDFUS;
	

KEYBOARD.EXE:   KDFSP.OBJ KDFFR.OBJ KDFDK.OBJ KDFGE.OBJ KDFIT.OBJ \
		KDFUK.OBJ KDFNO.OBJ KDFSV.OBJ KDFUS.OBJ KDFSG.OBJ \
		KDFEOF.OBJ KDF.OBJ
		LINK @KEYBOARD.LNK
		EXE2BIN KEYBOARD.EXE KEYBOARD.SYS
	        DEL KEYBOARD.EXE
