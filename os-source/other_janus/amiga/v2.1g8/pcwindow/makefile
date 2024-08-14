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
         reqsupp.o      revamp.o            \
         sext.o         shared.o       strings.o      \
         stuff.o        super.o        taskinit.o     \
         wait.o         xread.o

CC    =  lc
CCF  =  -b1 -cfrt -d4 -O -q500w1e -v -dDEBUG1=0 -dDEBUG2=0 -dDEBUG3=0
xxCCF  =  -v -cafrst -q500w1e -rr -b1 -d2 -d -dDEBUG1=0 -dDEBUG2=0 -dDEBUG3=0
xCCF  =  -v -cafrst -q500w1e -rr -b1
LN    =  blink
xLNF   =  SMALLCODE SMALLDATA ADDSYM
LNF   = ADDSYM SC SD WIDTH 75 PLAIN XREF zaphod.xref define __main=__tinymain
xLIB   =  LIB:debug.lib+LIB:lcr.lib+LIB:amiga.lib
LIB   =  LIB:lc.lib+LIB:debug.lib+LIB:amiga.lib+v:lib/amiga.lib

zaphod.ld.nostrip:  $(AOBJS) $(COBJS)
   bumprev 2 pcwindow_rev
   delete pcwindow_rev.i
   $(CC) $(CCF) -oalert.o alert.c
   $(LN) with zaphod.wth TO $@ LIBRARY $(LIB) $(LNF)
   $(LN) $@ to zaphod.ld NODEBUG

auxtask.o : auxtask.c
   $(CC) $(CCF) -y -o$*.o $*

cstask.o : cstask.c
   $(CC) $(CCF) -y -o$*.o $*

imtask.o : imtask.c
   $(CC) $(CCF) -y -o$*.o $*

.c.o:
   $(CC) $(CCF) -o$*.o $*

.asm.obj:
   casm -a $*.asm -y -o $*.obj -cs -i work:lc/include

adoc:
   echo  >ram:adoc " "
   spat "type >>ram:adoc" \#?.c
   spat "type >>ram:adoc" \#?.asm
   autodoc >PCWindow.adoc -i ram:adoc 

release:
   copy \#?         Other:janus/Amiga/cur/pcwindow
   spat "chmod 755" Other:janus/Amiga/cur/pcwindow/\#?
   delete           Other:janus/Amiga/cur/pcwindow/\#?.(o|obj|bak)

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy zaphod.ld       sys:pc/PCWindow
   copy "pc color.info" "sys:pc/PC Color.info"
   copy "pc mono.info"  "sys:pc/PC Mono.info"
   lock sys: on

disk:
   copy zaphod.ld PCInstall:pc/PCWindow
   copy "pc color.info" "PCInstall:pc/PC Color.info"
   copy "pc mono.info"  "PCInstall:pc/PC Mono.info"
