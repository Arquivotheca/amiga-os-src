
#include <stdio.h>
#include <exec/types.h>
#include <exec/ports.h>
#include "fs:/fs.h"


main()
{

    struct MsgPort *x;

    x = (struct MsgPort *) DeviceProc("Scratchy:");
    if (x)
    {
        ULONG res1, res2;
        struct UserInfo anid;
        printf("Port is %lx\n",x);
        res1 = DoPkt(x,ACTION_UID_TO_USERINFO,2,&anid);
        res2 = IoErr();
        printf("res1 %lx res2 %lx\n",res1,res2);
        printf("name is %s\n",anid.ui_UserName);
    }
}

