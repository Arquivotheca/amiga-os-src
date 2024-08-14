ZAPHOD=	auxtask.o cstask.o \
	a4.o init.o \
	disptask.o imtask.o \
	\
xprintf.o \
adjust.o reqsupp.o \
	asciitrans.o alert.o autosize.o border.o color.o colordat.o crt.o \
	fileio.o global.o help.o \
	lock.o main.o menuhand.o menus.o misc.o open.o privacy.o \
	recalc.o region.o render.o revamp.o \
	selmatic.o sext.o shared.o \
	strings.o stuff.o super.o \
	taskinit.o wait.o xread.o \
	\
	ascii2pc.o cbio.o clip.o clip2.o \
	\
	blast.o curse.o drawtext.o newlines.o uline.o ztext.o \
	\
	linkage.o lvo.o 

EXTDISK=	df2

DEADMOUSE=	ihandler.o mouse.o sethand.o

DEADSCROLL=	scroll.o scrollsup.o 

.xc.o:
	cc $(CFLAGS) +C +D +L $* -o $*.o

.c.o:
	cc $(CFLAGS) +C +D +L -E200 -Z4000 +Iram:zaphod.pre $* -o $*.o

.asm.o:
	as $(AFLAGS) -CD $* -o $*.o

zaphod:	$(ZAPHOD)
	ln -w +q -o zaphod +cdb $(ZAPHOD) -lm32 -lc32

xzaphod:	$(ZAPHOD)
	ln +q -o zaphod +cdb $(ZAPHOD) -lm32 -lc32

debug:
	cc $(CFLAGS) -A -T +C +D +L +Iram:zaphod.pre imtask -o imtask.lst

include:
	cc +Hinclude.pre include.h
	copy include.pre ram:zaphod.pre

xinclude:
	copy include.pre ram:zaphod.pre

all:
	make include
 	delete #?.o
	make

all2:
 	delete #?.o
	make


o:	$(ZAPHOD)


obj:	$(ZAPHOD)


ops:
	delete $(OPS)
	make

backup:
	x backit


xbackup:
	del #?.o
	del #?.pre
	del #?.lst
	copy makefile zaphod
	del zaphod
	x backit

zdebug.o:
	cc $(CFLAGS) -A -T +C +D +L +Iram:zaphod.pre $* -o $*.lst

chipdata.o:
	lc1 -iINCLUDE: $*
	lc2 -v $*
	atom $*.o $*.ozzz -cdb -pc
	delete $*.o
	rename $*.ozzz $*.o

asmclean:
	del blast.o 
	del curse.o 
	del drawtext.o 
	del newlines.o 
	del uline.o 
	del ztext.o 
	del linkage.o 
	del lvo.o 
	del a4.o 

copy:
	copy zaphod $(EXTDISK):pcwindow

copy2:
	copy zaphod schmatz:pc/pcwindow

