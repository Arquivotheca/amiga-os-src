// c code calling other code through vector table

// GRAPHICS library calls
// graphics intermodule calls throught vector table

#ifndef GBASE
#include "/macros.h"
#endif

#pragma libcall GBASE QBLIT 114 901
#pragma libcall GBASE QBSBLIT 126 901
#pragma libcall GBASE RECTFILL 132 3210905
#pragma libcall GBASE SETRAST ea 902
#pragma libcall GBASE INITBITMAP 186 210804
#pragma libcall GBASE WRITEPIXEL 144 10903
#pragma libcall GBASE READPIXEL 13e 10903
#pragma libcall GBASE SETAPEN 156 902
#pragma libcall GBASE BLTCLEAR 12c 10903
#pragma libcall GBASE ALLOCRASTER 1ec 1002
#pragma libcall GBASE FREERASTER 1f2 10803
#pragma libcall GBASE MOVE f0 10903
#pragma libcall GBASE DRAW f6 10903