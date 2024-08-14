CC  = msc
CCF = /AL /Ot
LN  = link

aread.exe : aread.obj jftrans.obj jint.obj 
	$(LN) aread jftrans jint ;

awrite.exe : awrite.obj jftrans.obj jint.obj 
	$(LN) awrite jftrans jint c:\microc\other\lsetargv ;

aread.obj : aread.c
	$(CC) $(CCF) aread.c ;

awrite.obj : awrite.c
	$(CC) $(CCF) awrite.c ;

jint.obj : jint.c
	$(CC) $(CCF) jint.c ;

jftrans.obj : jftrans.c
	$(CC) $(CCF) jftrans.c ;
