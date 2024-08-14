#****************************************************************************
# MakeFile for Tester.c, PC C function binding test program
#****************************************************************************
CC  	= cl
CCF	= /AL  /c /Otil /Za /Zi



tester.obj : tester.c
	$(CC) $(CCF) tester.c ;

Tester.exe : Tester.obj c:\lib\ljanus.lib 
	link /CO Tester , , , ljanus.lib ;

