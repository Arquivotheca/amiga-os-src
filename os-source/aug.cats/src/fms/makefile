
TARGET = fmsdisk.device
TARGETO = fms.o tag.o

CFLAGS = data=near nominc multiplecharconstants streq nostkchk ign=73
LFLAGS =  SD SC ND
LIBS = LIB:SC.lib,LIB:Amiga.lib
ASM = asm
AFLAGS = -iINCLUDE:

all: $(TARGET)

$(TARGET): $(TARGETO)
  slink <WITH <
FROM $(TARGETO)
LIBRARY $(LIBS)
TO $(TARGET) $(LFLAGS)
<

.c.o:
        $(CC) $(CFLAGS) $*.c

.asm.o:
        $(ASM) $(AFLAGS) $*.asm
