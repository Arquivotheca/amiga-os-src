#****************************************************************************
# MakeFile for Janus.lib, PC C function bindings
#****************************************************************************
CC  	= cl
CCF	= /AL /Foobj\ /c /Otil /Za

OBJS1	= OBJ\GetBase.obj 	OBJ\AlocJMem.obj 	OBJ\FreeJMem.obj
LOBJS1	= -+OBJ\GetBase       	-+OBJ\AlocJMem		-+OBJ\FreeJMem

OBJS2	= OBJ\SetParam.obj	OBJ\CallAmi.obj		OBJ\WaitAmi.obj
LOBJS2	= -+OBJ\SetParam.obj	-+OBJ\CallAmi		-+OBJ\WaitAmi

OBJS3	= OBJ\CheckAmi.obj
LOBJS3	= -+OBJ\CheckAmi

OBJS4	= OBJ\AddS.obj		OBJ\GetS.obj		OBJ\CallS.obj
LOBJS4	= -+OBJ\AddS		-+OBJ\GetS		-+OBJ\CallS

OBJS5	= OBJ\ReleaseS.obj	OBJ\DeleteS.obj		OBJ\LockSD.obj
LOBJS5	= -+OBJ\ReleaseS	-+OBJ\DeleteS		-+OBJ\LockSD

OBJS6	= OBJ\UnLockSD.obj	OBJ\jftrans.obj
LOBJS6	= -+OBJ\UnLockSD	-+OBJ\jftrans

OBJS	= $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6)




#****************************************************************************
# Bindings for 1st generation services
#****************************************************************************

obj\GetBase.obj : GetBase.c
	$(CC) $(CCF) GetBase.c ;

obj\Alocjmem.obj : Alocjmem.c
	$(CC) $(CCF) Alocjmem.c ;

obj\Freejmem.obj : Freejmem.c
	$(CC) $(CCF) Freejmem.c ;

obj\SetParam.obj : SetParam.c
	$(CC) $(CCF) SetParam.c ;

obj\CallAmi.obj : CallAmi.c
	$(CC) $(CCF) CallAmi.c ;

obj\WaitAmi.obj : WaitAmi.c
	$(CC) $(CCF) WaitAmi.c ;

obj\CheckAmi.obj : CheckAmi.c
	$(CC) $(CCF) CheckAmi.c ;

#****************************************************************************
# Bindings for 2nd generation services
#****************************************************************************

obj\AddS.obj : AddS.c
	$(CC) $(CCF) AddS.c ;

obj\GetS.obj : 	GetS.c
	$(CC) $(CCF) GetS.c ;

obj\CallS.obj : CallS.c
	$(CC) $(CCF) CallS.c ;

obj\ReleaseS.obj : ReleaseS.c
	$(CC) $(CCF) ReleaseS.c ;

obj\DeleteS.obj : DeleteS.c
	$(CC) $(CCF) DeleteS.c ;

obj\LockSD.obj : LockSD.c
	$(CC) $(CCF) LockSD.c ;

obj\UnLockSD.obj : UnLockSD.c
	$(CC) $(CCF) UnLockSD.c ;

#****************************************************************************
# Bindings for C functions that exist only in the link lib
#****************************************************************************

obj\jftrans.obj : jftrans.c
	$(CC) $(CCF) jftrans.c ;



ljanus.lib : $(OBJS)
	lib ljanus.lib $(LOBJS1) $(LOBJS2) $(LOBJS3) ;
	lib ljanus.lib $(LOBJS4) $(LOBJS5) $(LOBJS6) ;
	copy ljanus.lib c:\lib
	zwrite ljanus.doc pcinstall:docs/developer/PC/LJanus.lib.DOC
	del ljanus.bak
