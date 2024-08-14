all: NetClip

%.o : %.c
	lc -o$@ -O -v $<

NetClip : NetClip.o
	blink from NetClip.o to NetClip lib lib:amiga.lib lib:nipc.lib SC SD NOICONS

%.c:

NetClip.o : NetClip.c