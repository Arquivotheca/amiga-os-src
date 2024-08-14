#****************************************************************************
# MakeFile for Janus.lib, PC C function bindings
#****************************************************************************
CC  	= cl
CCF	= /AL /Foobj\ /c /Otil /Za

OBJS1	= OBJ\GetBase.obj 	OBJ\AlocJMem.obj 	OBJ\FreeJMem.obj
LOBJS1	= -+OBJ\GetBase       	-+OBJ\AlocJMem		-+OBJ\FreeJMem

OBJS2	= OBJ\SetParam.obj	OBJ\CallAmi.obj		OBJ\WaitAmi.obj
LOBJS2	= -+OBJ\SetParam.obj	-+OBJ\CallAmi		-+OBJ\WaitAmi

OBJS3	= OBJ\CheckAmi.obj	OBJ\PrintSD.obj		OBJ\perror.obj
LOBJS3	= -+OBJ\CheckAmi	-+OBJ\PrintSD		-+OBJ\perror

OBJS4	= OBJ\AddS.obj		OBJ\GetS.obj		OBJ\CallS.obj
LOBJS4	= -+OBJ\AddS		-+OBJ\GetS		-+OBJ\CallS

OBJS5	= OBJ\ReleaseS.obj	OBJ\DeleteS.obj		OBJ\LockSD.obj
LOBJS5	= -+OBJ\ReleaseS	-+OBJ\DeleteS		-+OBJ\LockSD

OBJS6	= OBJ\UnLockSD.obj	OBJ\jftrans.obj		OBJ\waits.obj
LOBJS6	= -+OBJ\UnLockSD	-+OBJ\jftrans		-+OBJ\waits

OBJS7	= OBJ\initlock.obj	OBJ\unlock.obj		OBJ\lock.obj
LOBJS7	= -+OBJ\initlock	-+OBJ\unlock		-+OBJ\lock

OBJS8	= OBJ\alock.obj		OBJ\allocjr.obj		OBJ\attachjr.obj
LOBJS8	= -+OBJ\alock		-+OBJ\allocjr		-+OBJ\attachjr

OBJS9	= OBJ\freejr.obj	OBJ\allocsm.obj		OBJ\freesm.obj
LOBJS9	= -+OBJ\freejr		-+OBJ\allocsm		-+OBJ\freesm

OBJS10  = obj\ints.obj
LOBJS10 = -+obj\ints

OBJS	= $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(OBJS6) $(OBJS7) $(OBJS8) $(OBJS9) $(OBJS10)




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

obj\waits.obj : waits.c
	$(CC) $(CCF) waits.c ;

obj\initlock.obj : initlock.c
	$(CC) $(CCF) initlock.c ;

obj\unlock.obj : unlock.c
	$(CC) $(CCF) unlock.c ;

obj\lock.obj : lock.c
	$(CC) $(CCF) lock.c ;

obj\alock.obj : alock.c
	$(CC) $(CCF) alock.c ;

obj\allocjr.obj : allocjr.c
	$(CC) $(CCF) allocjr.c ;

obj\attachjr.obj : attachjr.c
	$(CC) $(CCF) attachjr.c ;

obj\freejr.obj : freejr.c
	$(CC) $(CCF) freejr.c ;

obj\allocsm.obj : allocsm.c
	$(CC) $(CCF) allocsm.c ;

obj\freesm.obj : freesm.c
	$(CC) $(CCF) freesm.c ;

obj\ints.obj : ints.asm
	masm ints.asm, obj\ints.obj, nul, nul;

#****************************************************************************
# Bindings for C functions that exist only in the link lib
#****************************************************************************

obj\jftrans.obj : jftrans.c
	$(CC) $(CCF) jftrans.c ;

obj\PrintSD.obj : PrintSD.c
	$(CC) $(CCF) PrintSD.c ;

obj\Perror.obj : Perror.c
	$(CC) $(CCF) Perror.c ;


ljanus.lib : $(OBJS)
	lib ljanus.lib $(LOBJS1) $(LOBJS2) ;
	lib ljanus.lib $(LOBJS3) ;
	lib ljanus.lib $(LOBJS4) $(LOBJS5) ;
	lib ljanus.lib $(LOBJS6) ;
	lib ljanus.lib $(LOBJS7) ;
	lib ljanus.lib $(LOBJS8) ;
	lib ljanus.lib $(LOBJS9) ;
	lib ljanus.lib $(LOBJS10) ;
	copy ljanus.lib e:\lib
	del ljanus.bak
	type adds.c >adoc.tmp
	type alocjmem.c >>adoc.tmp
	type callami.c	>>adoc.tmp
	type calls.c	>>adoc.tmp
	type checkami.c	>>adoc.tmp
	type deletes.c	>>adoc.tmp
	type freejmem.c	>>adoc.tmp
	type getbase.c	>>adoc.tmp
	type gets.c	>>adoc.tmp
	type jftrans.c	>>adoc.tmp
	type locksd.c	>>adoc.tmp
	type perror.c	>>adoc.tmp
	type printsd.c	>>adoc.tmp
	type releases.c	>>adoc.tmp
	type setparam.c	>>adoc.tmp
	type unlocksd.c	>>adoc.tmp
	type waitami.c	>>adoc.tmp
	type waits.c	>>adoc.tmp
	type initlock.c >>adoc.tmp
	type unlock.c	>>adoc.tmp
	type lock.c	>>adoc.tmp
	type alock.c	>>adoc.tmp
	type allocjr.c  >>adoc.tmp
	type attachjr.c >>adoc.tmp
	type freejr.c	>>adoc.tmp
	type allocsm.c	>>adoc.tmp
	type freesm.c	>>adoc.tmp
	\src\adoc\autodoc -C -I -t3 adoc.tmp >adoc
	del adoc.tmp
