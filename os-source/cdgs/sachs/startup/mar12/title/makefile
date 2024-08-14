# :ts=8 bk=0
##
# makefile
#

#
# Remove - tester.o for a non running version. 
#
# task.o Contains the Two calls 'StartAnimation()' and 'SendAnimMessage()'
#

TESTOBJ=tester_t.o title_t.o
TOBJ=tester.o title.o

OBJ=vblank.o color.o task.o sound.o decompdltax.o\
	sprite.o dltax.o pal1.o sample.o back.o back3plane.o number.o

DEBUG = DEBUG=SF
NDEBUG = NODEBUG

EXE = title

CFLAGS = CPU=68020 STREQ NOSTKCHK CPU=ANY OPTSIZE STREQU IDIR=INCLUDE OPTIMIZE

AFLAGS = -d

LIBS = LIB:debug.lib LIB:sc.lib LIB:amiga3.0.lib


.c.o:
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $*.c

.a.o:
	asm -iinclude: $(AFLAGS) $*.a
	
.pic.o:
	tools/sbox -R $*.pic ram:$*.x
	tools/d2o ram:$*.x _$* $*.o -h "_NOMERGE"
	delete ram:$*.x


$(EXE): ram:gst.gst $(OBJ) $(TOBJ) version.o
	slink LIB:c.o $(TOBJ) $(OBJ) version.o to $(EXE) BATCH NOICONS\
	LIB $(LIBS) MAP ram:$(EXE).map H


#
# This produces a single task program for easy debuging.
#
tester: ram:gst.gst $(OBJ) $(TESTOBJ) version.o
	slink LIB:c.o $(TESTOBJ) $(OBJ) version.o to tester BATCH NOICONS LIB $(LIBS) 


version.o:

ram:gst.gst: gst.gst
	copy gst.gst ram:gst.gst

gst.gst: gst.c
	sc mgst=gst.gst $(CFLAGS) $(DEBUG) gst.c

net: title
	slink title to will:bbs/title nodebug

tester_t.o: tester.c
	sc GST=RAM:gst.gst DEFINE=NONTASK $(CFLAGS) $(DEBUG) OBJNAME=tester_t.o $*.c

title_t.o: title.h title.c
	sc GST=RAM:gst.gst DEFINE=NONTASK $(CFLAGS) $(DEBUG) OBJNAME=title_t.o $*.c


sprite.o: sprite.data
	tools/d2o sprite.data _SAnim sprite.o -h "_NOMERGE"


title.o: title.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $*.c

sound.o: sound.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $*.c


#######################
# Special compression.#
#######################


sample.o: sample
	tools/sumsound sample t:sample.ss
	tools/sbox -S t:sample.ss to t:sample.x
	tools/d2o t:sample.x _sample sample.o -h "_NOMERGE"
	delete t:sample.x
	delete t:sample.ss

back3plane.o: back3plane.pic
	tools/sbox -xx -C -R back3plane.pic t:back3plane.x
	tools/d2o t:back3plane.x _back3plane back3plane.o -h "_NOMERGE"
	delete t:back3plane.x

back3plane.o: back3plane.pic
	tools/sbox -x -C -R back3plane.pic t:back3plane.x
	tools/d2o t:back3plane.x _back3plane back3plane.o -h "_NOMERGE"
	delete t:back3plane.x
