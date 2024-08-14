# lmkfile - SAS C 5.10b -- Requires V37 or better compiling environment
SOURCE      = Test_Suite:OS/DOS/FileSystem/Suite/MemServer/src/
DEST        = Test_Suite:OS/DOS/FileSystem/Suite/MemServer/bin/
CC          = LC:lc
INCLUDE     = include:
CFLAGS      = -cs -t -Hcommon_h.pre -i$(INCLUDE)
CFLAGS2     = -cs -t -i$(INCLUDE)
LINKER      = LC:BLink
LNKOPTS     = sc sd nd define __main=__tinymain
LIBS        = lib:lc.lib lib:amiga.lib lib:debug.lib
RESLIBS     = lib:lc.lib
CLIENTSTART = lib:cres.o
SERVERSTART = lib:c.o
REVTOOL     = BuildTools:RevIt
ASM         = asm:casm -a
ASMOPTS     = -o
INCLUDE     = include:
SOURCES     = $(SOURCE)MemServer.c $(SOURCE)MemCpy.c $(SOURCE)ReadFM.c $(SOURCE)WriteMF.c $(SOURCE)bsprintf.asm $(SOURCE)bsprintf2.asm
OBJ         = $(SOURCE)MemServer.o $(SOURCE)MemCpy.o $(SOURCE)ReadFM.o $(SOURCE)WriteMF.o $(SOURCE)bsprintf.o $(SOURCE)bsprintf2.o
CLIENTS     = $(DEST)MemCpy $(DEST)ReadFM $(DEST)WriteMF
SERVER      = $(DEST)MemServer
BINARIES    = $(SERVER) $(CLIENTS) $(DEST)KillServer $(DEST)PortsAway


all:  $(BINARIES)

# Note:  V37 Compiling environment.
# CAPE assembler used.
# RevIt used.
# All clients are meant to be residentable, and DO take advantage of pragmas.

$(SOURCE)common_h: $(SOURCE)common.h
    copy $(SOURCE)common.h $(SOURCE)common_tmp.c clone
    $(CC) -ph -o$(SOURCE)common_h.pre $(SOURCE)common_tmp.c
    delete $(SOURCE)common_tmp.c force quiet




# Generic rules
#.pre.o:
#	echo "lmk sucks"

.c.o:
    @echo $<
    $(CC) $(CFLAGS) -o$*.o $*.c

.asm.o:
    $(ASM) $*.asm $(ASMOPTS) $*.o

.h.o:
    @echo "lmk sucks!!"

# Specific C-compiler rules

.pre.o: $(SOURCE)common.h
    copy $(SOURCE)common.h $(SOURCE)common_tmp.c clone
    $(CC) -ph -o$(SOURCE)common_h.pre $(SOURCE)common_tmp.c
    delete $(SOURCE)common_tmp.c force quiet

$(SOURCE)common_h.c: $(SOURCE)common.h
    copy $(SOURCE)common.h $(SOURCE)common_tmp.c clone
    $(CC) -ph -o$(SOURCE)common_h.pre $(SOURCE)common_tmp.c
    delete $(SOURCE)common_tmp.c force quiet

# Various dependencies
$(SOURCE)bsprintf.o:    $(SOURCE)bsprintf.asm
$(SOURCE)bsprintf2.o:   $(SOURCE)bsprintf2.asm
$(SOURCE)MemServer.o:   $(SOURCE)common_h.pre $(SOURCE)MemServer.c server.h
$(SOURCE)ReadFM.o:      $(SOURCE)common_h.pre $(SOURCE)client2server.h $(SOURCE)client2server.c $(SOURCE)ReadFM.c
$(SOURCE)WriteMF.o:     $(SOURCE)common_h.pre $(SOURCE)client2server.h $(SOURCE)client2server.c $(SOURCE)WriteMF.c
$(SOURCE)MemCpy.o:      $(SOURCE)common_h.pre $(SOURCE)MemCpy.c
$(SOURCE)KillServer.o:  $(SOURCE)common_h.pre $(SOURCE)KillServer.c
$(SOURCE)PortsAway.o:   $(SOURCE)PortsAway.c

# More specific rules (link, etc) ... cd bit is dicey.

$(DEST)MemServer: server.h $(SOURCE)common.h MemServer.o $(SOURCE)bsprintf2.o
    cd $(SOURCE)
    $(REVTOOL) MemServer time com "by J.W. Lockhart, © 1992 Commodore"
    $(ASM) MemServer_rev.asm $(ASMOPTS) $(SOURCE)MemServer_rev.o
    $(LINKER) <WITH <
FROM $(SERVERSTART)+$(SOURCE)MemServer_rev.o+$(SOURCE)MemServer.o+$(SOURCE)bsprintf2.o
TO $(DEST)MemServer
LIB $(LIBS)
$(LNKOPTS)
<

$(DEST)ReadFM: $(SOURCE)common_h.pre ReadFM.o client2server.h client2server.c
    cd $(SOURCE)
    $(REVTOOL) ReadFM time com "by J.W. Lockhart, © 1992 Commodore"
    $(ASM) ReadFM_rev.asm $(ASMOPTS) $(SOURCE)ReadFM_rev.o
    $(LINKER) <WITH <
FROM $(CLIENTSTART)+$(SOURCE)ReadFM_rev.o+$(SOURCE)ReadFM.o+$(SOURCE)bsprintf.o
FROM $(SOURCE)bsprintf2.o
TO $(DEST)ReadFM
LIB $(RESLIBS)
$(LNKOPTS)
<

$(DEST)PortsAway:   $(SOURCE)common_h.pre PortsAway.o
    cd $(SOURCE)
    $(REVTOOL) PortsAway com "by J.W. Lockhart, © 1992 Commodore"
    $(ASM) PortsAway_rev.asm $(ASMOPTS) $(SOURCE)PortsAway_rev.o
    $(LINKER) <WITH <
FROM $(CLIENTSTART)+$(SOURCE)PortsAway_rev.o+$(SOURCE)PortsAway.o
TO $(DEST)PortsAway
LIB $(RESLIBS)
$(LNKOPTS)
<

$(DEST)WriteMF: $(SOURCE)common_h.pre WriteMF.o $(SOURCE)client2server.h $(SOURCE)client2server.c
    cd $(SOURCE)
    $(REVTOOL) WriteMF time com "by J.W. Lockhart, © 1992 Commodore"
    $(ASM) WriteMF_rev.asm $(ASMOPTS) $(SOURCE)WriteMF_rev.o
    $(LINKER) <WITH <
FROM $(CLIENTSTART) $(SOURCE)WriteMF_rev.o $(SOURCE)WriteMF.o $(SOURCE)bsprintf.o
FROM $(SOURCE)bsprintf2.o
TO $(DEST)WriteMF
LIB $(RESLIBS)
$(LNKOPTS)
<

$(DEST)MemCpy: $(SOURCE)common_h.pre $(SOURCE)bsprintf2.o $(SOURCE)bsprintf.o MemCpy.o
    cd $(SOURCE)
    $(REVTOOL) MemCpy time com "by J.W. Lockhart, © 1992 Commodore"
    $(ASM) MemCpy_rev.asm $(ASMOPTS) $(SOURCE)MemCpy_rev.o
    $(LINKER) <WITH <
FROM $(CLIENTSTART) $(SOURCE)MemCpy_rev.o $(SOURCE)MemCpy.o
FROM $(SOURCE)bsprintf.o $(SOURCE)bsprintf2.o
TO $(DEST)MemCpy
LIB $(RESLIBS)
$(LNKOPTS)
<

$(DEST)killserver: $(SOURCE)common_h.pre killserver.o
    cd $(SOURCE)
    $(REVTOOL) KillServer
    $(ASM) KillServer_rev.asm $(ASMOPTS) $(SOURCE)KillServer_rev.o
    $(LINKER) <WITH <
FROM $(SERVERSTART) $(SOURCE)KillServer_rev.o $(SOURCE)KillServer.o
TO $(DEST)KillServer
LIB $(LIBS)
$(LNKOPTS)
<

