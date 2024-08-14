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
	zwrite aread.doc pcinstall:docs/user/ARead.DOC
	zwrite aread.c   pcinstall:docs/developer/examples/aread/ARead.c
	zwrite aread.txt pcinstall:docs/developer/examples/aread/ARead.ReadMe
	zwrite makefile  pcinstall:docs/developer/examples/aread/MakeFile

awrite.exe : OBJ\awrite.obj
	$(LN) $(LF) /NOE OBJ\awrite \lib\setargv.obj, ,NUL.MAP , ljanus ;
	zwrite awrite.doc pcinstall:docs/user/AWrite.DOC
	zwrite awrite.c   pcinstall:docs/developer/examples/awrite/AWrite.c
	zwrite awrite.txt pcinstall:docs/developer/examples/awrite/AWrite.ReadMe
	zwrite makefile  pcinstall:docs/developer/examples/awrite/MakeFile

