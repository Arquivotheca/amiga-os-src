###
###	CDFS Makefile (Manx)
###

VERSION	=	26

OBJ1	=	start.o main.o init.o global.o actions.o
OBJ2	=	iso.o path.o dir.o lock.o file.o
OBJ3	=	cdio.o str.o lib.o 
OBJ4	=	ldiv.o lmul.o fse.o extra.o
OBJS	=	$(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)

CFLAGS	=	+L +C +Iram:main.pre
AFLAGS	=	-C -N

rom:		main.pre $(OBJS) 
		ln -o cdfs.ld $(OBJS)
		copy cdfs.ld CDFS clone
#		copy cdfs.ld /rom clone
#		copy cdfs.ld df0:rom clone

packet.o:	packet.c packets.h

global.o:	global.c global.h
		cc +L +C global.c

main.pre:	main.h global.h
		cc +L +C -o NIL: +Hmain.pre main.h
		copy main.pre ram:

dfs:		dfs.o
		cc +P dfs.c
		ln dfs -lc32
		copy dfs df0:C

rev:
		bumprev $(VERSION) cdfs
		make clean
		make

clean:
		delete #?.o #?.pre

back:
		bu * cdfs:main
