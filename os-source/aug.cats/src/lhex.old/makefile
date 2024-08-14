CACA_C = caca.c dynlists.c
CACA_O = caca.o dynlists.o
CACA_H = dynlists.pro

HDR =	lharc.pro append.pro crcio.pro dhuf.pro extract.pro patmatch.pro \
	huf.pro larc.pro lhadd.pro lhlist.pro header.pro lhext.pro \
	maketbl.pro maketree.pro shuf.pro slide.pro util.pro $(CACA_H)

SRC =	lharc.c append.c crcio.c dhuf.c extract.c patmatch.c huf.c \
	larc.c lhadd.c lhlist.c header.c lhext.c maketbl.c maketree.c \
	shuf.c slide.c util.c $(CACA_C)

OBJ = 	lharc.o append.o crcio.o dhuf.o extract.o patmatch.o huf.o \
	larc.o lhadd.o lhlist.o header.o lhext.o maketbl.o maketree.o \
	shuf.o slide.o util.o $(CACA_O)

LIB = LIBRARY LIB:sc.lib+LIB:amiga.lib+LIB:debug.lib
LINKOPTS = VERBOSE BATCH
PROTOOPTS = GenPROTOs GenProtoPARMeters NOICONs
PROLOG = LIB:c.o # LIB:covutil.o
PROTOCFLAGS =
CFLAGS = OPTIMIZE
LINK = SC:C/SLINK
TARGET = lhex

REV=1

All: $(TARGET)

protos:	$(HDR)

$(TARGET):      protos $(OBJ)
	$(LINK) $(LINKOPTS) TO $(TARGET) FROM $(PROLOG) $(OBJ) $(LIB)

clean:
	-delete $(OBJ)

clobber:	clean
	-delete $(TARGET)
	
.c.pro:
	$(CC) $(PROTOCFLAGS) $(PROTOOPTS) GPFILE=$*.pro $*.c
#	bumprev $(REV) $*_rev

$(TARGET)_rev.h:	verupd

verupd:
	bumprev $(REV) $(TARGET)_rev
#	splat -o "define[ 	]" "define	SYS_" $(TARGET)_rev.h

lharc.pro: $(TARGET)_rev.h lharc.c
	$(CC) $(PROTOCFLAGS) $(PROTOOPTS) GPFILE=lharc.pro lharc.c
	bumprev $(REV) $(TARGET)_rev

append.o: append.c append.pro lharc.h
crcio.o: crcio.c crcio.pro lharc.h
dhuf.o: dhuf.c dhuf.pro
extract.o: extract.c extract.pro
header.o: header.c header.pro lharc.h
huf.o: huf.c huf.pro
larc.o: larc.c larc.pro lharc.h
lhadd.o: lhadd.c lhadd.pro lharc.h
lharc.o: lharc.c lharc.pro lharc.h lhex_rev.h dynlists.pro args.h
dynlists.o: dynlists.pro
lhext.o: lhext.c lhext.pro lharc.h
lhlist.o: lhlist.c lhlist.pro lharc.h
maketbl.o: maketbl.c maketbl.pro
maketree.o: maketree.c maketree.pro
patmatch.o: patmatch.c patmatch.pro
shuf.o: shuf.c shuf.pro
slide.o: slide.c slide.pro
util.o: util.c util.pro lharc.h
caca.o: args.h
