ASM = masm

.asm.obj:
		$(ASM) $*.asm;

HDRS = title.inc \
        external.inc \
        equates.inc \
        mess.asm
        
OBJS = CSETUP.obj \
        DATA.obj \
        FILL.obj \
        INT10.obj \
        INT13.obj \
        INT14.obj \
        INT15.obj \
        INT16.obj \
        INT17.obj \
        INT40.obj \
        INT5.obj \
        MISCINTR.obj \
        POD.obj \
        RAMSETUP.obj \
        ROUTINES.obj \
        SIGNON.obj \
        TOP8K.obj \

SRCS = CSETUP.ASM \
        DATA.ASM \
        FILL.ASM \
        INT10.ASM \
        INT13.ASM \
        INT14.ASM \
        INT15.ASM \
        INT16.ASM \
        INT17.ASM \
        INT40.ASM \
        INT5.ASM \
        MISCINTR.ASM \
        POD.ASM \
        RAMSETUP.ASM \
        ROUTINES.ASM \
        SIGNON.ASM \
        TOP8K.ASM \

###

CSETUP.obj: CSETUP.ASM \
        title.inc \
        external.inc \
        equates.inc

DATA.obj: DATA.ASM \
        title.inc

FILL.obj: FILL.ASM \
        title.inc \
        equates.inc

INT10.obj: INT10.ASM \
        title.inc \
        external.inc \
        equates.inc

INT13.obj: INT13.ASM \
        title.inc \
        external.inc \
        equates.inc

INT14.obj: INT14.ASM \
        title.inc \
        external.inc \
        equates.inc

INT15.obj: INT15.ASM \
        title.inc \
        equates.inc \
        external.inc

INT16.obj: INT16.ASM \
        title.inc \
        external.inc \
        equates.inc

INT17.obj: INT17.ASM \
        title.inc \
        external.inc \
        equates.inc

INT40.obj: INT40.ASM \
        title.inc \
        external.inc \
        equates.inc

INT5.obj: INT5.ASM \
        title.inc \
        equates.inc \
        external.inc

MISCINTR.obj: MISCINTR.ASM \
        title.inc \
        external.inc \
        equates.inc

POD.obj: POD.ASM \
        title.inc \
        equates.inc \
        external.inc

RAMSETUP.obj: RAMSETUP.ASM

ROUTINES.obj: ROUTINES.ASM \
        title.inc \
        equates.inc

SIGNON.obj: SIGNON.ASM \
        mess.asm \
        equates.inc

TOP8K.obj: TOP8K.ASM \
        title.inc \
        equates.inc \
        external.inc

bios.exe: $(OBJS) linkbios
	link @linkbios

bios.bin: bios.exe
	exe2bin bios.exe bios.bin

bios8.bin: bios.bin
	check8 bios.bin bios8.bin

bios1.bin: bios.bin
	genchk2

