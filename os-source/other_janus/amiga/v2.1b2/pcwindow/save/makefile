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
         wait.o         xread.o

CC    =  lc
CCF   =  -v -b0 -O -d -dDEBUG=1
xCCF  =  -v -b0 -O

.c.o:
   $(CC) $(CCF) -o$*.o $*

.asm.obj:
   assem $*.asm -c w200000 -i fh1:lc/lc/include -o $*.obj

zaphod:  $(AOBJS) $(COBJS)
   blink with zaphod.wth
