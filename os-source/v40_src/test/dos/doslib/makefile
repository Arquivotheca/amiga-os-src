LOCATION =Test_Suite:OS/DOS/DOSLib/

$(LOCATION)bin/doslib: $(LOCATION)src/doslib.o

	blink with $(LOCATION)src/link.with

$(LOCATION)src/doslib.o: $(LOCATION)src/doslib.c

	lc -o$(LOCATION)src/doslib.o  $(LOCATION)src/doslib.c

