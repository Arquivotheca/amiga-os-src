ASM = masm
.asm.obj:
		$(ASM) $*.asm
SUFFIX = .asm:s .inc:s
DEST = .
EXTHDRS =
HDRS =
MAKEFILE = Makefile
OBJS =
PRINT = pr
SHELL = /bin/csh
SRCS =
all:		$(OBJS)
clean:;		@rm -f $(OBJS) core
clobber:;	@rm -f $(OBJS) core tags
depend:;	@makemake
echo:;		@echo $(HDRS) $(SRCS)
index:;		@ctags -wx $(HDRS) $(SRCS)
print:;		@$(PRINT) $(HDRS) $(SRCS)
tags:       $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)
touch:		@touch $(SRCS)
