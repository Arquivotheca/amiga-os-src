CC  = cl
CCF = /AL /Otil /Foobj\ /c /Za /Zi
LN  = link
LF  = /CO

OBJ\aread.obj : aread.c
	$(CC) $(CCF) aread.c ;

OBJ\awrite.obj : awrite.c
	$(CC) $(CCF) awrite.c ;

aread.exe : OBJ\aread.obj
	$(LN) $(LF) OBJ\aread , ,NUL.MAP , ljanus ;

awrite.exe : OBJ\awrite.obj
	$(LN) $(LF) /NOE OBJ\awrite \lib\setargv.obj, ,NUL.MAP , ljanus ;

