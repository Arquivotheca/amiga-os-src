INCLUDE = INCLUDE:
COMPILE_FLAGS = -r0 -v -ccisw
QUAD = QUAD:
DEBUG_FLAGS = -d3
LIBS = LIB:LCM.lib+LIB:LC.lib+LIB:Amiga.lib
STARTUP = LIB:C.o
LINK_FLAGS = VERBOSE BATCH

*: Receive \
   Transmit

Receive: Receive.o
    BLink FROM $(STARTUP)+Receive.o LIB $(LIBS) TO Receive $(LINK_FLAGS)

Receive.o: Receive.c
    LC -i$(INCLUDE) $(COMPILE_FLAGS) $(DEBUG_FLAGS) -q$(QUAD) Receive

Transmit: Transmit.o
    BLink FROM $(STARTUP)+Transmit.o LIB $(LIBS) TO Transmit $(LINK_FLAGS)

Transmit.o: Transmit.c
    LC -i$(INCLUDE) $(COMPILE_FLAGS) $(DEBUG_FLAGS) -q$(QUAD) Transmit
