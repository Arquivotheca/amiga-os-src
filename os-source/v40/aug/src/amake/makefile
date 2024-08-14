OBJECTS	=	ident.o main.o doname.o misc.o files.o dosys.o gram.o amain.obj
LIBES	= 
LINT	=	lint -ps
CFLAGS	=	-DASCARCH -DAMIGA -d3
SUBSYS	=	amake
TARGET	=	/usr/commodore/bin

CMDPREFIX=msg = '1B'X || '[7m Starting target $@ ' || '1B'X || '[0m'; say center(msg, 70, '-'); SIGNAL ON ERROR;
CMDSUFFIX=;msg = '1B'X || '[7m Ending target $@ ' || '1B'X || '[0m'; say center(msg, 70, '-'); exit 0; ERROR: exit RC

##############################################################################

all :	$(SUBSYS)

$(SUBSYS) :	$(OBJECTS)
	@"blink with $(SUBSYS).WITH"

$(OBJECTS) :  defs.h

clean :
	@-"delete \#?.o $(SUBSYS) a.out errs"

install :
	@install $(SUBSYS) $(DESTDIR)/$(TARGET)/$(SUBSYS)

lint :  dosys.c doname.c files.c main.c misc.c ident.c gram.c
	@$(LINT) dosys.c doname.c files.c main.c misc.c ident.c gram.c
	@delete -f gram.c
