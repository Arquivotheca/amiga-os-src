DFILES= test.o checkstop.o wdwait.o compcrc.o blockfrom.o disperr.o \
getblknum.o acdio.o MakeWindow.o timer.o twd.o iortns.o wdpr.o wdread.o \
wdsetparam.o wdvfytrk.o wdwrite.o wdstuff.o blocklist.o initcmdblk.o \
patfill.o putcmdblk.o wdcrdwr.o wdfmttrk.o wdlread.o wdpark.o ftools.o \
setio.o driver.o writeblk.o badblks.o ramtest.o wdlscan.o

.C.o:
	lc -u -v $*

.asm.o:
	echo "Assembling $*.asm"
	Assem $*.asm -o $@ -i :include  -c W150000 -v $*.err

test: $(DFILES)
	echo "Linking $*"
	assign ldir: dh0:lib
	blink with test.lkp ver tlink.err
	echo "DONE!"

$(DFILES): ddefs.h
