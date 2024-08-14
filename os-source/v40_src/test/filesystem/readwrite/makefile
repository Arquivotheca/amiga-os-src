SOURCE = Test_Suite:OS/DOS/FileSystem/suite/sysTest/src/
SOURCE1 = Test_Suite:OS/DOS/FileSystem/tools/readwrite/src/
DEST = Test_Suite:OS/DOS/FileSystem/tools/readwrite/bin/


$(DEST)readwrite:  $(SOURCE1)readwrite.o $(SOURCE)createtestfile.o $(SOURCE)verifytestfile.o

	blink <WITH <
FROM lib:c.o+$(SOURCE1)readwrite.o+$(SOURCE)createtestfile.o+$(SOURCE)verifytestfile.o
LIBRARY lib:lc.lib,lib:amiga.lib
TO $(DEST)readwrite
<

$(SOURCE1)readwrite.o: $(SOURCE1)readwrite.c $(SOURCE)create.h

	lc  $(SOURCE1)readwrite.c

$(SOURCE)createtestfile.o: $(SOURCE)createtestfile.c

	lc  $(SOURCE)createtestfile.c

$(SOURCE)verifytestfile.o: $(SOURCE)verifytestfile.c

	lc  $(SOURCE)verifytestfile.c

