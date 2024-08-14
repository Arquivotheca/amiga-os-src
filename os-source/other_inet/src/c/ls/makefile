# -------------------------------------------------
# LS 2.3 makefile by Justin V. McCormick 89-07-05
# -------------------------------------------------
CFLAGS = -cfmtu -d1 -m0 -rr -v -iinc: -inetinc: -O
AFLAGS = -iainc: -cvy
CC = lc

OBJS = ls.o lssup.o 
DEST = ls
 
.c.o:  $(OBJS)
	lc $(CFLAGS) -o$*.o $*.c

lssup.o: lssup.a
	casm -a lssup.a -olssup.o -cvq

ls:	$(OBJS) ls.c
	BLINK WITH linkfile
	@beep 300 
