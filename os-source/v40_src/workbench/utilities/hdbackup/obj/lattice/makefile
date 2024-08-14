# BruShell Application makefile

SRC =		/src
CC =		lc
DEFS =		-damiga=1 -dLATTICE=1 $(DBUG)
REXXGLUE =	root:arexx/tools/rexxglue.o

# Uncomment one of the following lines, as desired.  If DBUG is defined,
# then the macros are expanded to include debugging code, however this
# code does nothing unless enabled with the appropriate call to DBUG_PUSH.

DBUG =
#DBUG =		-dDBUG

INCLUDE =	-i$(SRC)/
CFLAGS =	-cf -O $(DEFS) $(INCLUDE)

OBJ =		backup.o bailout.o brushell.o dbug.o dirtree.o eventloop.o \
		filereq.o getdisks.o images.o libfuncs.o logfile.o \
		mainwin.o menus.o gadgets.o reqs.o rexxcom.o rexxglue.o \
		slist.o scansort.o wild.o

brushell:	$(OBJ)
		blink with brushell.lnk

backup.o :	$(SRC)/backup.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/brushell.h $(SRC)/dirtree.h $(SRC)/eventloop.h \
		$(SRC)/mainwin.h $(SRC)/menus.h $(SRC)/reqs.h \
		$(SRC)/rexxcom.h $(SRC)/slist.h $(SRC)/backup.h \
		$(SRC)/filereq.h $(SRC)/dbug.h $(SRC)/scansort.h \
		$(SRC)/logfile.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/backup.c

bailout.o :	$(SRC)/bailout.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/brushell.h $(SRC)/mainwin.h $(SRC)/dbug.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/bailout.c
	
brushell.o :	$(SRC)/brushell.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/brushell.h $(SRC)/dirtree.h $(SRC)/mainwin.h \
		$(SRC)/slist.h $(SRC)/menus.h $(SRC)/dbug.h \
		$(SRC)/rexxcom.h $(SRC)/eventloop.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/brushell.c

dbug.o :	$(SRC)/dbug.c $(SRC)/dbug.h $(SRC)/vargs.h
		$(CC) $(CFLAGS) -o$@ -DNO_VARARGS=1 $(SRC)/dbug.c

dirtree.o :	$(SRC)/dirtree.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/dirtree.h $(SRC)/eventloop.h $(SRC)/mainwin.h \
		$(SRC)/scansort.h $(SRC)/dbug.h $(SRC)/reqs.h \
		$(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/dirtree.c

eventloop.o :	$(SRC)/eventloop.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/brushell.h $(SRC)/dirtree.h $(SRC)/backup.h \
		$(SRC)/eventloop.h $(SRC)/mainwin.h $(SRC)/menus.h \
		$(SRC)/reqs.h $(SRC)/rexxcom.h $(SRC)/scansort.h \
		$(SRC)/slist.h $(SRC)/dbug.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/eventloop.c

filereq.o :	$(SRC)/filereq.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/images.h $(SRC)/mainwin.h $(SRC)/filereq.h \
		$(SRC)/dbug.h $(SRC)/getdisks.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/filereq.c

gadgets.o :	$(SRC)/gadgets.c $(SRC)/standard.h $(SRC)/dbug.h \
		$(SRC)/images.h $(SRC)/mainwin.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/gadgets.c

getdisks.o :	$(SRC)/getdisks.c $(SRC)/standard.h $(SRC)/getdisks.h \
		$(SRC)/dbug.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/getdisks.c

images.o :	$(SRC)/images.c $(SRC)/standard.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/images.c

libfuncs.o :	$(SRC)/libfuncs.c $(SRC)/standard.h $(SRC)/dbug.h \
		$(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/libfuncs.c

logfile.o :	$(SRC)/logfile.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/dirtree.h $(SRC)/logfile.h $(SRC)/mainwin.h \
		$(SRC)/scansort.h $(SRC)/dbug.h $(SRC)/reqs.h \
		$(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/logfile.c

mainwin.o :	$(SRC)/mainwin.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/brushell.h $(SRC)/dirtree.h $(SRC)/eventloop.h \
		$(SRC)/mainwin.h $(SRC)/menus.h $(SRC)/scansort.h \
		$(SRC)/dbug.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/mainwin.c

menus.o :	$(SRC)/menus.c $(SRC)/standard.h $(SRC)/dbug.h $(SRC)/menus.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/menus.c

reqs.o :	$(SRC)/reqs.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/images.h $(SRC)/mainwin.h $(SRC)/reqs.h \
		$(SRC)/dbug.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/reqs.c

rexxcom.o :	$(SRC)/rexxcom.c $(SRC)/standard.h $(SRC)/backup.h \
		$(SRC)/bailout.h $(SRC)/brushell.h $(SRC)/dirtree.h \
		$(SRC)/eventloop.h $(SRC)/mainwin.h $(SRC)/rexxcom.h \
		$(SRC)/scansort.h $(SRC)/dbug.h $(SRC)/usermsg.h \
		$(SRC)/reqs.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/rexxcom.c

rexxglue.o :	$(REXXGLUE)
		copy $(REXXGLUE) rexxglue.o

scansort.o :	$(SRC)/scansort.c $(SRC)/standard.h $(SRC)/scansort.h \
		$(SRC)/bailout.h $(SRC)/brushell.h $(SRC)/dirtree.h \
		$(SRC)/eventloop.h $(SRC)/mainwin.h $(SRC)/dbug.h \
		$(SRC)/libfuncs.h $(SRC)/reqs.h $(SRC)/wild.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/scansort.c

slist.o :	$(SRC)/slist.c $(SRC)/standard.h $(SRC)/bailout.h \
		$(SRC)/brushell.h $(SRC)/dirtree.h $(SRC)/mainwin.h \
		$(SRC)/slist.h $(SRC)/dbug.h $(SRC)/libfuncs.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/slist.c

wild.o:		$(SRC)/wild.c $(SRC)/standard.h $(SRC)/wild.h $(SRC)/dbug.h
		$(CC) $(CFLAGS) -o$@ $(SRC)/wild.c

#BRUrexx:	BRUrexx.o
#		ln $(LFLAGS) BRUrexx.o rexxglue.o -ldbug32 -lc32
