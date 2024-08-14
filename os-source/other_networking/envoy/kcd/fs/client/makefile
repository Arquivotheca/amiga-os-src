CSRC = client.c
ASRC = cpy.asm
COBJS = client.o
AOBJS = cpy.o

all : client

$(AOBJS) :
           casm -a $*.asm -o $*.o -i include: -cry

$(COBJS) :
        lc -. -cc -d1 -b0 -v $*.c
        omd $*.o $*.c >$*.d

client.o : client.c /fs.h
cpy.o    : cpy.asm


client    : $(COBJS) $(AOBJS)
            blink $(COBJS) $(AOBJS) to EnvoyFileSystem lib lib:amiga.lib lib:debug.lib lib:lc.lib NODEBUG\
            MAP client.map S X F H
            copy EnvoyFileSystem l:

bump      :
            co -l efs_rev.rev
            bumprev 37 efs_rev
            ci -u -m"" efs_rev.rev
            touch client.c

release   : client
            copy client.c efs_rev.h makefile envoy:src/fs/client/
            copy /fs.h envoy:src/fs/client/fs.h
            copy EnvoyFileSystem envoy:src/fs/client/EnvoyFileSystem

