/*
**  find entity test..
**
**  (C) Copyright 1992 Commodore-Amiga, Inc.
**      All Rights Reserved.
*/
#include <string.h>
#include <exec/exec.h>
#include <appn/nipc.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>
#include <clib/nipc_protos.h>
#include <pragmas/nipc_pragmas.h>

struct Library *NIPCBase;
extern struct Library *SysBase;

main()
{
APTR entity, srcentity;
struct Transaction *trans;
LONG scratch;
char *data = "Can you hear me?";

        NIPCBase = OpenLibrary("nipc.library", 0L);
        if(!NIPCBase)
        {
                printf("Can't open nipc.library!!!\n");
                exit();
        }
        srcentity = CreateEntity(ENT_AllocSignal, &scratch,
                                 TAG_DONE);
        if(!srcentity)
        {
                printf("Failed to CreateEntity().\n");
                goto ERROR1;
        }
        entity = FindEntity("scratchy", "DemoServerOne", srcentity, &scratch);
        if(!entity)
        {
                printf("Failed to FindEntity().\n");
        }else
        {
                LoseEntity(entity);
        }
        DeleteEntity(srcentity);
ERROR1:
        CloseLibrary(NIPCBase);
}








