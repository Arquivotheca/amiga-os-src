ASRC = epar.asm loadconfig.asm
AOBJS = epar.o loadconfig.o

all : envoyprint.device

$(AOBJS) :
           casm -a $*.asm -o $*.o -i include: -cry

epar.o : epar.asm epar.i
loadconfig.o : loadconfig.asm epar.i

envoyprint.device : $(AOBJS)
                  blink $(AOBJS) to envoyprint.device lib lib:amiga.lib lib:debug.lib lib:nipc.lib lib:services.lib NODEBUG\
                  MAP envoypar.map S X F H
                  copy envoyprint.device devs:

bump:
         co -l envoyprint.rev
         bumprev 37 envoyprint
         ci -u -m"" envoyprint.rev

release:
         copy envoyprint.i envoyprint.rev epar.asm epar.i loadconfig.asm makefile envoy:src/printer/client NOPRO
         copy envoyprint.device envoy:src/printer/client NOPRO
         protect envoy:src/printer/client/#? +rewd QUIET


