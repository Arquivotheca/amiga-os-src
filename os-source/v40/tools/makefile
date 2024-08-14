
all: makemeta makemeta.sub

makemeta: makemeta.nostrip Makefile
	sed -e '/^#/d' -e '/^$$/d' $@.nostrip > $@.new
	rm -f $@
	mv $@.new $@
	chmod -w $@

makemeta.sub: makemeta Makefile
	sed -e '/^rom:/s/ld/lib/' makemeta > makemeta.sub.new
	rm -f makemeta.sub
	mv makemeta.sub.new makemeta.sub
	chmod -w makemeta.sub

makeamiga: makeamiga.nostrip Makefile
	sed -e '/^#/d' -e '/^$$/d' $@.nostrip > $@.new
	rm -f $@
	mv $@.new $@
	chmod -w $@

tags:
	maketags
