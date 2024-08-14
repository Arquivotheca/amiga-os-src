
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/io.h>
#include <exec/memory.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

extern struct SysBase *SysBase;

void main (int argc, char **argv) {

    CloseLibrary(OpenLibrary("freeanim.library", 0));
    }



