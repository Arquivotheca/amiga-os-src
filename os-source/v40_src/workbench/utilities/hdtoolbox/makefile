# hdtoolbox makefile for lmk

HDR	=	prep_strux.h hardblocks.h rdb.h global.h gadgets.h gadgetids.h
OBJ	=	obj/
#LCFLAGS =	-cfim -v -rr -b0 -z30000 -iv:inc.lattice/ -o$(OBJ)
LCFLAGS =	-cfim -v -rr -b0 -z30000 -iinclude: -o$(OBJ)
LC	=	lc
LIB	=	LIB:

VER	=	40
REMOTE	=	V$(VER):src/Workbench/utilities/hdtoolbox
MODULE	=	hdtoolbox

CFILES	=	_main.c rdb.c analyze.c handleprep.c \
	        testmain.c refresh.c handlepart.c partglob.c \
		handlefs.c readwrite.c handledefine.c \
		global.c handlefsm.c newprep.c images.c \
		commit.c badblock.c verify.c \
		handletype.c newgads.c gadgetids.h
AFILES	=
HDRS	=	commit.h gadgets.h global.h hardblocks.h \
		hdtoolbox_rev.h partglob.h prep_strux.h protos.h rdb.h refresh.h \
		scsi.h scsidisk.h sure.h windows.h fstype.h

OBJS	=	$(OBJ)_main.o $(OBJ)rdb.o $(OBJ)analyze.o $(OBJ)handleprep.o \
	        $(OBJ)testmain.o $(OBJ)refresh.o $(OBJ)handlepart.o $(OBJ)partglob.o \
		$(OBJ)handlefs.o $(OBJ)readwrite.o $(OBJ)handledefine.o \
		$(OBJ)global.o $(OBJ)handlefsm.o $(OBJ)newprep.o $(OBJ)images.o \
		$(OBJ)commit.o $(OBJ)badblock.o $(OBJ)verify.o \
		$(OBJ)handletype.o $(OBJ)newgads.o gadgetids.h


$(MODULE): $(OBJS)
#	   blink with $(MODULE).lnk NODEBUG
	   blink with $(MODULE).lnk map $(MODULE).map S
           copy $(MODULE).ld $(MODULE)

.c.o:
	$(LC) $(LCFLAGS) $*

# the user must do a generate source command
#
#preptest.c: pwfiles/partwind
#	pw2

$(OBJ)_main.o: _main.c
	$(LC) $(LCFLAGS) -dTINY _main.c

$(OBJ)rdb.o:            rdb.c protos.h                                  $(HDR)

$(OBJ)analyze.o:        analyze.c protos.h                              $(HDR)

$(OBJ)badblock.o:       badblock.c scsidisk.h scsi.h windows.h protos.h $(HDR)

$(OBJ)handleprep.o:     handleprep.c windows.h protos.h                 $(HDR)

$(OBJ)handletype.o:     handletype.c windows.h protos.h                 $(HDR)

$(OBJ)testmain.o:       testmain.c windows.h hdtoolbox_rev.h protos.h   $(HDR)

$(OBJ)refresh.o:        refresh.c refresh.h sure.h		        $(HDR)

$(OBJ)handlepart.o:     handlepart.c windows.h partglob.h protos.h      $(HDR)

$(OBJ)partglob.o:       partglob.c partglob.h                           $(HDR)

$(OBJ)handlefs.o:       handlefs.c fstype.h windows.h protos.h          $(HDR)

$(OBJ)readwrite.o:      readwrite.c scsidisk.h scsi.h protos.h          $(HDR)

$(OBJ)handledefine.o:   handledefine.c protos.h                         $(HDR)

$(OBJ)global.o:         global.c                                        $(HDR)

$(OBJ)handlefsm.o:      handlefsm.c windows.h protos.h                  $(HDR)

$(OBJ)newprep.o:	newprep.c gadgetids.h windows.h refresh.h       $(HDR)
	$(LC) -o$(OBJ) -adb -b0 newprep.c

$(OBJ)images.o:         images.c                                        $(HDR)

$(OBJ)commit.o:         commit.c commit.h                               $(HDR)

$(OBJ)verify.o:         verify.c scsidisk.h scsi.h windows.h protos.h   $(HDR)

$(OBJ)newgads.o:	newgads.c gadgets.h protos.h fstype.h           $(HDR)


#clean:
#	delete $(OBJ)/#?
#
#backup:
#	rsh rebma -l jesup chmod u+w prep/src/*
#	copy g:prepsrc sun:prep/src all
#
#localbackup:
#	copy g:prepsrc tmp:prepsrc all


version:
	@copy $(REMOTE)/$(MODULE)_rev.rev ""
	@bumprev $(VER) $(MODULE)_rev
	@copy $(MODULE)_rev.\#? $(REMOTE)

release:
	@lmk clean
	@lmk version
	@lmk
	@copy $(MODULE).ld\#? $(REMOTE)
	@execute < <
	failat 21 ;
	cd $(REMOTE) ;
	co $(CFILES) ;
	co $(HDRS) makefile $(MODULE).lnk ;
	<

clean:
	-delete (\#?.o|\#?.tmp|\#?.ld\#?)

internal:
	copy (\#?.i|\#?.h|\#?.rev|\#?.c|\#?.doc|\#?.ld\#?) $(REMOTE)

strip:
	blink $(MODULE).ld ND to $(MODULE).ld.strip
