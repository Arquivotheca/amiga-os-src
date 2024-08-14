/* ram_includes.h */

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include "dos/dos.h"
#include "dos/dosextens.h"

#include <internal/librarytags.h>

/* Private ROM routine... */
struct Library *TaggedOpenLibrary(LONG);
#pragma syscall TaggedOpenLibrary 32a 001

#include "clib/exec_protos.h"
#include "clib/dos_protos.h"
#include "pragmas/exec_pragmas.h"
#include "pragmas/dos_pragmas.h"
#include <string.h>

#include "protos.h"
