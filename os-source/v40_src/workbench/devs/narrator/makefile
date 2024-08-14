CFLAGS=-mc -md -sf -su -wa -hi objs/includes.pre -wo -pc -m0b
AFLAGS = -c -d


IFILES= narrator.i \
	assembly.i \
	private.i \
	gloequs.i \
	pcequs.i \
	featequ.i

PHONIFILES= phonrules.i
F0IFILES= f0equs.i

HFILES= narrator.h



zut:	objs/device.o	objs/task.o 	objs/spname.o 	objs/narrator.o \
	objs/multchan.o objs/parse.o	objs/strmark.o	objs/syllable.o \
	objs/phonol.o 	objs/prosod.o 	objs/f0init.o 	objs/f0hls.o \
	objs/f0lls.o 	objs/f0enthus.o objs/f0interp.o objs/includes.pre \
	objs/phonet.o 	objs/phonetc4.o objs/phonetc5.o	objs/phonetc6.o \
	objs/phonetc7.o objs/phonetc8.o objs/phonetc9.o objs/phonetc10.o \
	objs/phonetf0.o objs/compcoef.o objs/convert.o	objs/synth.o \
	objs/featmat.o 	objs/parms.o	objs/fric.o 	objs/f1.o \
	objs/f2.o 	objs/f3.o 	objs/dbyte.o	objs/phonetfast.o \
	objs/pi.o	objs/debug.o	objs/sentence.o
		@ln -o narrator.device \
	objs/device.o	objs/task.o 	objs/spname.o 	 objs/narrator.o \
	objs/multchan.o objs/parse.o	objs/strmark.o	 objs/syllable.o \
	objs/phonol.o 	objs/prosod.o 	objs/f0init.o 	 objs/f0hls.o \
	objs/f0lls.o 	objs/f0enthus.o objs/f0interp.o  objs/phonet.o \
	objs/phonetc4.o objs/phonetc5.o objs/phonetc6.o  objs/phonetc7.o \
	objs/phonetc8.o objs/phonetc9.o objs/phonetc10.o objs/phonetf0.o \
	objs/compcoef.o objs/convert.o	objs/synth.o 	 objs/featmat.o \
	objs/parms.o	objs/fric.o 	objs/f1.o 	 objs/f2.o \
	objs/f3.o 	objs/dbyte.o	objs/pi.o	 objs/phonetfast.o \
	objs/debug.o	objs/sentence.o	-lc


cbm:	objs/device.o	objs/task.o 	objs/spname.o 	objs/narrator.o \
	objs/multchan.o objs/parse.o	objs/strmark.o	objs/syllable.o \
	objs/phonol.o 	objs/prosod.o 	objs/f0init.o 	objs/f0hls.o \
	objs/f0lls.o 	objs/f0enthus.o objs/f0interp.o objs/includes.pre \
	objs/phonet.o 	objs/phonetc4.o objs/phonetc5.o	objs/phonetc6.o \
	objs/phonetc7.o objs/phonetc8.o objs/phonetc9.o objs/phonetc10.o \
	objs/phonetf0.o objs/compcoef.o objs/convert.o	objs/synth.o \
	objs/featmat.o 	objs/parms.o	objs/fric.o 	objs/f1.o \
	objs/f2.o 	objs/f3.o 	objs/dbyte.o	objs/phonetfast.o \
	objs/pi.o 	objs/cbmdebug.o objs/sentence.o
		@ln -o narrator.device \
	objs/device.o	objs/task.o 	objs/spname.o 	 objs/narrator.o \
	objs/multchan.o objs/parse.o	objs/strmark.o	 objs/syllable.o \
	objs/phonol.o 	objs/prosod.o 	objs/f0init.o 	 objs/f0hls.o \
	objs/f0lls.o 	objs/f0enthus.o objs/f0interp.o  objs/phonet.o \
	objs/phonetc4.o objs/phonetc5.o objs/phonetc6.o  objs/phonetc7.o \
	objs/phonetc8.o objs/phonetc9.o objs/phonetc10.o objs/phonetf0.o \
	objs/compcoef.o objs/convert.o	objs/synth.o 	 objs/featmat.o \
	objs/parms.o	objs/fric.o 	objs/f1.o 	 objs/f2.o \
	objs/f3.o 	objs/dbyte.o	objs/pi.o	 objs/phonetfast.o \
	objs/cbmdebug.o	objs/sentence.o	-lc


objs/device.o:	device.asm $(IFILES) narrator_rev.i
	as $(AFLAGS) -o $@ device

objs/task.o:	task.asm $(IFILES)
	as $(AFLAGS) -o $@ task

objs/spname.o:	spname.asm $(IFILES) narrator_rev.i
	as $(AFLAGS) -o $@ spname

objs/narrator.o:	narrator.asm $(IFILES)
	as $(AFLAGS) -o $@ narrator

objs/multchan.o:	multchan.asm $(IFILES)
	as $(AFLAGS) -o $@ multchan

objs/parse.o:	parse.asm $(IFILES) parse.i
	as $(AFLAGS) -o $@ parse

objs/sentence.o:	sentence.asm $(IFILES)
	as $(AFLAGS) -o $@ sentence

objs/strmark.o:	strmark.asm $(IFILES)
	as $(AFLAGS) -o $@ strmark

objs/syllable.o:	syllable.asm $(IFILES)
	as $(AFLAGS) -o $@ syllable

objs/phonol.o:	phonol.asm $(IFILES) $(PHONIFILES)
	as $(AFLAGS) -o $@ phonol

objs/prosod.o:	prosod.asm $(IFILES)
	as $(AFLAGS) -o $@ prosod

objs/f0init.o:	f0init.asm $(IFILES) $(F0IFILES)
	as $(AFLAGS) -o $@ f0init

objs/f0hls.o:	f0hls.asm $(IFILES) $(F0IFILES)
	as $(AFLAGS) -o $@ f0hls

objs/f0lls.o:	f0lls.asm $(IFILES) $(F0IFILES)
	as $(AFLAGS) -o $@ f0lls

objs/f0enthus.o:	f0enthus.asm $(IFILES) $(F0IFILES)
	as $(AFLAGS) -o $@ f0enthus

objs/f0interp.o:	f0interp.asm $(IFILES)
	as $(AFLAGS) -o $@ f0interp

objs/synth.o:	synth.asm $(IFILES) syn.i
	as $(AFLAGS) -o $@ synth

objs/compcoef.o:	compcoef.asm $(IFILES) 
	as $(AFLAGS) -o $@ compcoef

objs/featmat.o:	featmat.asm $(IFILES) 
	as $(AFLAGS) -o $@ featmat

objs/convert.o:	convert.asm $(IFILES) $(F0IFILES)
	as $(AFLAGS) -o $@ convert

objs/f1.o:	f1.asm $(IFILES) 
	as $(AFLAGS) -o $@ f1

objs/f2.o:	f2.asm $(IFILES) 
	as $(AFLAGS) -o $@ f2

objs/f3.o:	f3.asm $(IFILES) 
	as $(AFLAGS) -o $@ f3

objs/fric.o:	fric.asm $(IFILES) 
	as $(AFLAGS) -o $@ fric

objs/dbyte.o:	dbyte.asm $(IFILES) 
	as $(AFLAGS) -o $@ dbyte

objs/debug.o:	debug.asm $(IFILES) 
	as $(AFLAGS) -o $@ debug

objs/cbmdebug.o:	cbmdebug.asm $(IFILES) 
	as $(AFLAGS) -o $@ cbmdebug

objs/phonetfast.o:	phonetfast.asm $(IFILES) 
	as $(AFLAGS) -o $@ phonetfast

objs/pi.o:	pi.asm $(IFILES) phonet.i
	as $(AFLAGS) -o $@ pi

objs/includes.pre:	preincludes.c phonet.h pc.h phonetsubs.h gloequs.i $(HFILES)
	cc -mc -md -m0b -ho objs/includes.pre -o objs/preincludes.o -pc -wo preincludes

objs/parms.o:   parms.c parms.h pc.h objs/includes.pre
	   cc $(CFLAGS) -o objs/parms.o parms.c

objs/phonet.o:  phonet.c parms.h phonet.h pc.h objs/includes.pre
	   cc $(CFLAGS) -o objs/phonet.o phonet.c

objs/phonetc4.o:  phonetc4.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc4.c -o objs/phonetc4.o

objs/phonetc5.o:  phonetc5.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc5.c -o objs/phonetc5.o

objs/phonetc6.o:  phonetc6.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc6.c -o objs/phonetc6.o

objs/phonetc7.o:  phonetc7.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc7.c -o objs/phonetc7.o

objs/phonetc8.o:  phonetc8.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc8.c -o objs/phonetc8.o

objs/phonetc9.o:  phonetc9.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc9.c -o objs/phonetc9.o

objs/phonetc10.o:  phonetc10.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetc10.c -o objs/phonetc10.o

objs/phonetf0.o:  phonetf0.c parms.h phonet.h pc.h phonetsubs.h objs/includes.pre
	   cc $(CFLAGS) phonetf0.c -o objs/phonetf0.o
