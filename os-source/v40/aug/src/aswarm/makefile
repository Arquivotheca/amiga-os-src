
STL = LIB:lc.lib LIB:amiga.lib
BLO = SC SD ND NOICONS
TNM = DEFINE __main=__tinymain

ASwarm: ASwarm.c
	LC:lc -cfsu -v -O ASwarm.c
	LC:blink FROM LIB:c.o ASwarm.o $(TNM) LIB $(STL) $(BLO) TO ASwarm
