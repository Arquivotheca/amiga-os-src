#****************************************************************************
# MakeFile for Janus.lib, PC C function bindings
#****************************************************************************
CC  	= cl
CCF	= /AL /c /Otil /Za
LN  	= link
LF  = /CO

dslib.obj: dslib.c
	$(CC) $(CCF) dslib.c

adoc: dslib.c
	\src\adoc\autodoc -C -I -t3 dslib.c >adoc
