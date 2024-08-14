#	Makefile for Make
#	(c) Copyright 1991 by Ben Eng, All Rights Reserved
#
#	LIST of OPTIONS
#	-DMAXLINE=1024 sets the maximum line length to 1024
#	-DMAXSUFFIX=16 sets the maximum suffix length to 16
#	-DMAXPATHNAME=108 sets the maximum length of a path name to 108
#	-DMAX_MACRONAME=512	to override internal default of 256
#	-DFNCALLS=0 to disable function calls
#	-DDEBUG=0 to disable the debugging features
#	-gs enables dynamic stack growth
#	-s enables symbolic information
#	MiniMake OPTIONS := -DDEBUG=0 -DFNCALLS=0
#	BigMake  OPTIONS :=
#	CC := dcc
OPTIONS :=
OBJDIR := bmk:
CFLAGS := -r -ms -proto $(OPTIONS)
LIBS := -lben

CNOBJS := $(addprefix $(OBJDIR),cond.o input.o read.o recipe.o)
DHOBJS := $(addprefix $(OBJDIR),depend.o  dumprules.o make.o \
	builtin.o macro.o expand.o fncall.o pattern.o) $(CNOBJS)
OBJS := $(addprefix $(OBJDIR),lists.o snode.o param.o touch.o log.o \
	parsing.o) $(DHOBJS)
BMAIN := $(OBJDIR)main.o
GMAIN := $(OBJDIR)guimain.o
GUIOBJS := $(OBJDIR)guicode.o
BVERS := $(OBJDIR)bversion.o
GVERS := $(OBJDIR)gversion.o

all: bmake
	if exists(make.log)
		-delete make.log
	endif

.PHONY: all

bmake: $(BVERS)
	$(CC) $(CFLAGS) -o $@ $(BVERS) $(BMAIN) $(OBJS) $(LIBS)

guimake: $(GVERS)
	$(CC) $(CFLAGS) -o $@ $(GVERS) $(GMAIN) $(GUIOBJS) $(OBJS) $(LIBS)

$(BVERS): $(BMAIN) $(OBJS)
$(GVERS): $(GMAIN) $(GUIOBJS) $(OBJS)
$(BMAIN) $(GMAIN) $(OBJS): make.h
$(BMAIN) $(GMAIN) $(DHOBJS): depend.h
$(CNOBJS): cond.h

clean:
	-delete $(OBJS) $(VERS)

install:
	copy bmake sys:bin/make

#	pattern rule to handle object files in a different directory
#	than the source files
$(OBJDIR)%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
