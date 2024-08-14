###
###	CDFS Makefile (Manx C 5.0d)
###

VERSION	=	24
REMOTE	=	CDTV:src/cdfs
BACKUP	=	vax:cdfs

OBJ1	=	start.o main.o init.o global.o actions.o quickio.o
OBJ2	=	iso.o path.o dir.o lock.o file.o
OBJ3	=	cdio.o str.o lib.o 
OBJS	=	$(OBJ1) $(OBJ2) $(OBJ3)

CFLAGS	=	-3 +L +C +Iram:main.pre -k
AFLAGS	=	-C -N -iinclude: -icdtv:include/

cdfs.ld:	ram:main.pre $(OBJS) 
		-delete cdfs.ld
		ln -w -o cdfs.ld $(OBJS)

global.o:	global.c global.h
		cc -3 +L +C global.c

start.o:	start.asm cdfs_rev.i
		as $(AFLAGS) start.asm

ram:main.pre:	main.h global.h
		cc -3 +L +C -o NIL: +Hram:main.pre main.h

rev:
		bumprev $(VERSION) cdfs_rev

clean:
		-delete #?.o ram:main.pre cdfs.ld

internal:
		protect $(REMOTE)/(#?.asm|#?.c|#?.h|#?.i|cdfs.ld|expansion.lvo|makefile|cdfs_rev.rev) +w
		copy (#?.asm|#?.c|#?.h|#?.i|cdfs.ld|expansion.lvo|makefile|cdfs_rev.rev) $(REMOTE)

backup:
		protect $(BACKUP)/(#?.asm|#?.c|#?.h|#?.i|cdfs.ld|expansion.lvo|makefile|cdfs_rev.rev) +w
		copy (#?.asm|#?.c|#?.h|#?.i|cdfs.ld|expansion.lvo|makefile|cdfs_rev.rev) $(BACKUP)

