AOBJS =  blast.obj      curse.obj      drawtext.obj   \
         newlines.obj   uline.obj      ztext.obj

COBJS =  adjust.o       alert.o        ascii2pc.o     \
         asciitrans.o   autosize.o     auxtask.o      \
         border.o       cbio.o         clip.o         \
         clip2.o        color.o        colordat.o     \
         crt.o          cstask.o       disptask.o     \
         fileio.o       global.o       help.o         \
         imtask.o       init.o         lock.o         \
         main.o         menuhand.o     menus.o        \
         misc.o         open.o         privacy.o      \
         recalc.o       region.o       render.o       \
         reqsupp.o      revamp.o       selmatic.o     \
         sext.o         shared.o       strings.o      \
         stuff.o        super.o        taskinit.o     \
         tinymain.o     wait.o         xread.o

CC    =  lc
CCF  =  -v -cafrstu -q500w1e -rr -b1 -O -d -dDEBUG1=0 -dDEBUG2=0 -dDEBUG3=0
xCCF  =  -v -cafrstu -q500w1e -rr -b1
LN    =  blink
LNF   =  SMALLCODE SMALLDATA NODEBUG
LIB   =  LIB:debug.lib+LIB:lcr.lib+LIB:lc.lib+LIB:amiga.lib

zaphod:  $(AOBJS) $(COBJS)
   $(LN) with zaphod.wth TO ZAPHOD LIBRARY $(LIB) $(LNF)

auxtask.o : auxtask.c
   $(CC) $(CCF) -y -o$*.o $*

cstask.o : cstask.c
   $(CC) $(CCF) -y -o$*.o $*

imtask.o : imtask.c
   $(CC) $(CCF) -y -o$*.o $*

.c.o:
   $(CC) $(CCF) -o$*.o $*

.asm.obj:
   assem $*.asm -c w200000 -i fh1:languages/latticec5.02/lc/include -o $*.obj
