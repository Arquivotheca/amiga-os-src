# production prep makefile for lmk

HDRS =  prep_strux.h hardblocks.h global.h
OBJ = obj/
LCFLAGS = -cfmisq -. -O -v -o$(OBJ) -d -iv:inc.lattice:
LC = lc
LIB = LIB:

prod_prep.ld: $(OBJ)main.o \
	$(OBJ)write.o \
	$(OBJ)global.o \
	$(OBJ)verify.o \
	$(OBJ)format.o \
	$(OBJ)define.o \
	$(OBJ)rdb.o \
	$(OBJ)part.o
	blink with prod.lnk
        blink from prod_prep.ld to prod_prep.ld.strip ND

.c.o:
	$(LC) $(LCFLAGS) $*

# the user must do a generate source command

$(OBJ)main.o: main.c dummy.h scsidisk.h scsi.h $(HDRS)

$(OBJ)write.o: write.c scsidisk.h scsi.h  $(HDRS)

$(OBJ)format.o: format.c scsidisk.h scsi.h  $(HDRS)

$(OBJ)global.o: global.c $(HDRS)

$(OBJ)verify.o: verify.c scsidisk.h scsi.h $(HDRS)

$(OBJ)define.o: define.c $(HDRS)

$(OBJ)rdb.o: rdb.c $(HDRS)

$(OBJ)part.o: part.c $(HDRS)

readrdsk: $(OBJ)readrdsk.o
	blink with readrdsk.lnk

$(OBJ)readrdsk.o: readrdsk.c $(HDRS)

clean:
	delete $(OBJ) all
	makedir obj

#backup:
#	rsh rebma -l jesup chmod u+w prep/src/prod/*
#	copy g:prepsrc/prod sun:prep/src/prod


version:
	@BumpRev 39 prod_prep_rev
