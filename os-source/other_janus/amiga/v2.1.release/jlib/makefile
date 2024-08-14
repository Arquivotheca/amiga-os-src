STRTUP     = LIB:astartup.obj
OBJS       = romtag.obj alloc.obj alloc2.obj \
             ints.obj init.obj lib.obj defs.obj amigarw.obj \
             runback.obj services.obj servicesup.obj \
             sfuncs1.obj sfuncs1b.obj sfuncs2.obj sfuncs3.obj sfuncs4.obj \
             gets.o endcode.obj
OBJS1      = romtag.obj alloc.obj alloc2.obj \
             ints_sc.obj init_sc.obj lib.obj defs.obj amigarw.obj \
             runback.obj services.obj servicesup.obj \
             sfuncs1.obj sfuncs1b.obj sfuncs2.obj sfuncs3.obj sfuncs4.obj \
             gets.o endcode.obj
SRC        = romtag.asm alloc.asm alloc2.asm \
             ints.asm ints_sc.asm init.asm lib.asm defs.asm amigarw.asm
	     runback.asm services.asm servicesup.asm \
             sfuncs1.asm sfuncs1b.asm sfuncs2.asm sfuncs3.asm sfuncs4.asm \
             gets.c endcode.asm
LIBS       = LIB:janus.lib+LIB:debug.lib+LIB:amiga.lib
xLIBS      = LIB:janus.lib+expansio.lib+LIB:debug.lib+LIB:amiga.lib
EXTDISK    = df2
CC         = lc
CCF        = -v 
xLN         = alink
LN        = blink

.asm.obj:
	casm -a $*.asm -y -o $*.obj -cs -i work:lc/include

.c.o:
   $(CC) $(CCF) $*.c

janus.library.ld.nostrip: $(OBJS) $(OBJS1)
   bumprev 36 janus.library_rev
   delete janus.library_rev.h
   casm -a romtag.asm -y -o romtag.obj -cs -i work:lc/include
   $(LN) TO $@ WITH lib.wth
   $(LN) $@ to janus.library.ld NODEBUG
   delete janus.library.ld.info
   delete janus.library.ld.nostrip.info
   $(LN) TO janus_sc.library.ld.nostrip WITH lib_sc.wth
   $(LN) janus_sc.library.ld.nostrip to janus_sc.library.ld NODEBUG
   delete janus_sc.library.ld.info
   delete janus_sc.library.ld.nostrip.info

janus.lib:   linkage.asm lvos.asm
   casm -a linkage.asm -y -o linkage.obj -cs -i work:lc/include
   casm -a lvos.asm -y -o lvos.obj -cs -i work:lc/include
   join lvos.obj linkage.obj as janus.lib

adoc: $(SRC)
   type  >ram:temp.adoc romtag.asm
   type >>ram:temp.adoc alloc.asm
   type >>ram:temp.adoc alloc2.asm
   type >>ram:temp.adoc amigarw.asm
   type >>ram:temp.adoc defs.asm
   type >>ram:temp.adoc endcode.asm
   type >>ram:temp.adoc init.asm
   type >>ram:temp.adoc ints.asm
   type >>ram:temp.adoc lib.asm
   type >>ram:temp.adoc linkage.asm
   type >>ram:temp.adoc lvos.asm
   type >>ram:temp.adoc process.asm
   type >>ram:temp.adoc putmsg.asm
   type >>ram:temp.adoc runback.asm
   type >>ram:temp.adoc services.asm
   type >>ram:temp.adoc servicesup.asm
   type >>ram:temp.adoc sfuncs1.asm
   type >>ram:temp.adoc sfuncs1b.asm
   type >>ram:temp.adoc sfuncs2.asm
   type >>ram:temp.adoc sfuncs3.asm
   type >>ram:temp.adoc sfuncs4.asm
   :source/autodoc/autodoc >janus.library.adoc -a -I ram:temp.adoc

o:    $(OBJS)

release:
   copy \#?         Other:janus/Amiga/cur/jlib
   spat "chmod 755" Other:janus/Amiga/cur/jlib/\#?
   delete           Other:janus/Amiga/cur/jlib/\#?.(o|obj|bak)

lib:
   copy janus.lib   V:lib
   spat "chmod 755" v:lib/janus.lib

clean:
   delete \#?.(o|obj|bak)

local:
   lock sys: off
   copy janus.library.ld      sys:expansion/Janus.library
   copy janus.library.info sys:expansion/Janus.library.info
   lock sys: on

disk:
   copy janus.library.ld      PCInstall:expansion/Janus.library
   copy janus.library.info PCInstall:expansion/Janus.library.info
