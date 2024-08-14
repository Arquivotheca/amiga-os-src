CC  = cl
CCF = /AL /Otil /c /Za
LN  = link
LF  = 

dates.obj : dates.c
	$(CC) $(CCF) dates.c ;

awrite.obj : awrite.c
	$(CC) $(CCF) awrite.c ;

awrite.exe : awrite.obj dates.obj ..\dslib\dslib.obj
	$(LN) $(LF) /STACK:8192 awrite dates ..\dslib\dslib, ,awrite.MAP , ljanus ;

aread.obj : aread.c
	$(CC) $(CCF) aread.c ;

aread.exe : aread.obj dates.obj ..\dslib\dslib.obj
	$(LN) $(LF) /STACK:8192 aread dates ..\dslib\dslib, ,aread.MAP , ljanus ;

