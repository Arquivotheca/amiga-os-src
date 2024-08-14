# :ts=8 bk=0
#
#   makefile
#*
#*	William A. Ware						D314
#*
#*****************************************************************************
#*   This information is CONFIDENTIAL and PROPRIETARY						*
#*   Copyright 1993, Silent Software, Incorporated.							*
#*   All Rights Reserved.													*
#****************************************************************************/
##
# makefile
#

TESTOBJ=tester_t.o title_t.o
TOBJ=title.o

OBJ=color.o task.o sound.o decompdltax.o star.o vblank.o \
	sprite.o dltax.o pal1.o back.o back3plane.o number.o \
	sound1.o sound2.o viols.o

DEBUG = DEBUG=SF
NDEBUG = NODEBUG

EXE = title

#CFLAGS = CPU=68020 STREQ NOSTKCHK CPU=ANY OPTSIZE STREQU IDIR=INCLUDE OPTIMIZE
#CFLAGS = CPU=68020 STREQ NOSTKCHK CPU=ANY OPTSIZE STREQU IDIR=INCLUDE
CFLAGS = IDIR=INCLUDE3: STREQ NOSTKCHK CPU=ANY OPTSIZE STREQU IDIR=INCLUDE

AFLAGS = -d

LIBS = LIB:sc.lib LIB:amiga3.0.lib


.c.o:
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c

.a.o:
	asm $(AFLAGS) -iinclude: $>.a
	
.pic.o:
	tools/sbox -R $>.pic ram:$>.x
	tools/d2o ram:$>.x _$> $>.o -h "_NOMERGE"
	delete ram:$>.x


$(EXE): ram:gst.gst $(OBJ) $(TESTOBJ) version.o
	slink LIB:c.o $(TESTOBJ) $(OBJ) version.o to $(EXE) BATCH NOICONS LIB $(LIBS) 
	@bumpit -h -i -!
	@sc >nil: $(CFLAGS) version.c
	@touch >nil: $(EXE)
	net
	slink title to net:work/title NODEBUG

$(EXE).library: ram:gst.gst romtag.o libinit.o $(OBJ) $(TOBJ) last.o tester
	slink TO $(EXE).library BATCH NOICONS LIBFD title_lib.fd LIBPREFIX __LIB\
		FROM romtag.o libinit.o $(TOBJ) $(OBJ) last.o \
		NODEBUG LIB lib:sc.lib lib:Amiga3.0.lib MAP ram:$(EXE).map H		
	@bumpit -c -h -i -!
	@sc >nil: $(CFLAGS) version.c
	@touch >nil: $(EXE)
	net
	slink title.library to net:work/title.library NODEBUG

tester:	tester.o 
	slink LIB:c.o tester.o to tester BATCH NOICONS LIB $(LIBS)

version.o:

ram:gst.gst: gst.gst
	copy gst.gst ram:gst.gst

gst.gst: gst.c
	sc mgst=gst.gst $(CFLAGS) $(DEBUG) gst.c

net: title
	slink title to will:bbs/title nodebug

tester_t.o: tester.c
	sc GST=RAM:gst.gst DEFINE=NONTASK $(CFLAGS) $(DEBUG) OBJNAME=tester_t.o $*.c

title_t.o: title.c title.h
	sc GST=RAM:gst.gst DEFINE=NONTASK $(CFLAGS) $(DEBUG) OBJNAME=title_t.o $*.c


color.o: color.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c

task.o: task.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c

sound.o: sound.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c

decompdltax.o: decompdltax.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c

star.o: star.c title.h
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c


romtag.o: romtag.a version.i 
	asm -u $(AFLAGS) -dROM -iinclude: $>.a

libinit.o: 



last.o:
	tools/d2o last.txt __EndData last.o -h "_NOMERGE"

####

tester.o: title_pragmas.h tester.c 
	sc GST=RAM:gst.gst $(CFLAGS) $(DEBUG) $>.c

title_pragmas.h: title_lib.fd
	fd2pragma title_lib.fd title_pragmas.h


#######################
# Special compression.#
#######################
sprite.o: sprite.data
	tools/d2o sprite.data _SAnim sprite.o -h "_NOMERGE"

sample.o: sample
	tools/sumsound sample t:sample.ss
	tools/sbox -S t:sample.ss to t:sample.x
	tools/d2o t:sample.x _sample sample.o -h "_NOMERGE"
	delete t:sample.x
	delete t:sample.ss

back3plane.o: back3plane.pic
	tools/sbox -C -R back3plane.pic t:back3plane.x
	tools/d2o t:back3plane.x _back3plane back3plane.o -h "_NOMERGE"
	delete t:back3plane.x

back3plane.o: back3plane.pic
	tools/sbox -1 back3plane.pic t:back3plane.x
	tools/d2o t:back3plane.x _back3plane back3plane.o -h "_NOMERGE"
	delete t:back3plane.x

sound1.o: sound1.snd
	tools/sumsound sound1.snd t:sound.ss
	sbox -x0 t:sound.ss t:sound.x 
	tools/d2o t:sound.x _sound1 sound1.o -h "_NOMERGE"
	delete t:sound.x

sound2.o: sound2.snd
	tools/sumsound sound2.snd t:sound.ss
	sbox -x0 t:sound.ss t:sound.x 
	tools/d2o t:sound.x _sound2 sound2.o -h "_NOMERGE"
	delete t:sound.ss t:sound.x

viols.o: viols.snd
	tools/sumsound $>.snd t:$>.ss
	sbox -x0 t:$>.ss t:$>.x 
	tools/d2o t:$>.x _$> $>.o -h "_NOMERGE"
	delete t:$>.ss t:$>.x
