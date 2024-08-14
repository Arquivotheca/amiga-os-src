#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <appn/nipc.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <string.h>

#define geta4 __builtin_geta4
void geta4(void);

UBYTE cname[]="nipc.filed";
ULONG sigbit;


struct TagItem cetags[] =
{
    ENT_Name, &cname,
    ENT_Public, 0,
    ENT_AllocSignal, &sigbit,
    TAG_END, 0
};


struct TagItem ttags[] =
{
    TAG_END, 0
};


void _main(void)
{
    register struct Library *DOSBase;
    register struct Library *NIPCBase;
    register struct Library *SysBase;

    APTR MyEntity;
    struct Transaction *trans;
    BPTR DestFile;
    ULONG sigs,mask;
    BOOL die=FALSE;

    geta4();
    SysBase = (*(struct Library **)4L);
    DestFile = NULL;

    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        if(NIPCBase = OpenLibrary("nipc.library",0L))
        {
            if(MyEntity = CreateEntityA((struct TagItem *)&cetags))
            {
                mask = 1<<sigbit | SIGBREAKF_CTRL_C;
                while(1)
                {
                    sigs=Wait(mask);
                    if(sigs & (1<<sigbit))
                    {
                        if(trans=GetTransaction(MyEntity))
                        {
                            switch(trans->trans_Command)
                            {
                                case 1: if(!DestFile)
                                        {
                                            if(DestFile = Open(trans->trans_RequestData,MODE_NEWFILE))
                                                trans->trans_Error=0;
                                            else
                                                trans->trans_Error=IoErr();
                                        }
                                        else
                                            trans->trans_Error=-1;
                                        break;

                                case 2: if(DestFile)
                                        {
                                            Write(DestFile,trans->trans_RequestData,trans->trans_ReqDataActual);
                                            trans->trans_Error=0;
                                        }
                                        else
                                            trans->trans_Error=-1;
                                        break;

                                case 3: if(DestFile)
                                        {
                                            Close(DestFile);
                                            DestFile=NULL;
                                            trans->trans_Error=0;
                                        }
                                        else
                                            trans->trans_Error=-1;
                                        break;

                                default: trans->trans_Error=-2;
                                         break;
                            }
                            trans->trans_ResponseData = NULL;
                            trans->trans_RespDataLength = 0;
                            trans->trans_RespDataActual = 0;
                            ReplyTransaction(trans);
                        }
                    }
                    else if(sigs & SIGBREAKF_CTRL_C)
                    {
                        if(!DestFile)
                            break;
                        else
                            die=TRUE;
                    }
                    if(die && !DestFile)
                        break;
                }
                DeleteEntity(MyEntity);
            }
            CloseLibrary(NIPCBase);
        }
        CloseLibrary(DOSBase);
    }
}
