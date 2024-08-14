CC	= cl
CF	= /AL /c /FoOBJ\ /Otil /Za /Zi
AS	= masm
ASF     =  /ZI
LN  	= link
LF	= /CO

OBJ\atime.obj : atime.c
	$(CC) $(CF) atime.c

OBJ\asettime.obj : asettime.asm
	$(AS) $(ASF) asettime.asm,OBJ\ASetTime.obj ;

asettime.exe : OBJ\asettime.obj 
	$(LN) $(LF) OBJ\asettime , ,NUL.MAP, ;
	zwrite asettime.asm janus_dev_disk:developer_docs/examples/asettime/ASetTime.asm
	zwrite asettime.doc pcinstall:userdocs/ASetTime.DOC
	zwrite asettime.txt janus_dev_disk:developer_docs/examples/asettime/ASetTime.ReadMe
	zwrite makefile     janus_dev_disk:developer_docs/examples/asettime/MakeFile

atime.exe : OBJ\atime.obj
	$(LN) $(LF) OBJ\atime, ,NUL.MAP , ljanus ;
	zwrite atime.c   janus_dev_disk:developer_docs/examples/atime/ATime.c
	zwrite atime.doc pcinstall:userdocs/ATime.DOC
	zwrite atime.txt janus_dev_disk:developer_docs/examples/atime/ATime.ReadMe
	zwrite makefile  janus_dev_disk:developer_docs/examples/atime/MakeFile

