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
#include <stdio.h>
#include <string.h>

#define geta4 __builtin_geta4
void geta4(void);

UBYTE cname[]="TEST Client";
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
    ULONG *args[4];
    register struct Library *DOSBase;
    register struct Library *NIPCBase;
    register struct Library *SysBase;
    struct RDArgs *rdargs;
    APTR MyEntity;
    APTR RemoteEntity;
    struct Transaction *trans;
    BPTR SourceFile;
    ULONG error, actual, buffsize;
    UBYTE *filebuffer;

    geta4();

	SysBase = (*(struct Library **)4L);
    buffsize = 8192;

    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        args[3] = 0L;

        if(rdargs=ReadArgs("SourceFile/A,DestHost/A,DestFile/A,BUF=BUFFER/K/N",(LONG *)args,NULL))
        {
            if(args[3])
                buffsize=*args[3];

            if(NIPCBase = OpenLibrary("nipc.library",0L))
            {
                if(MyEntity = CreateEntityA((struct TagItem *)&cetags))
                {
                    if(RemoteEntity = FindEntity((UBYTE *)args[1],"nipc.filed",MyEntity,&error))
                    {
                        if(SourceFile = Open((UBYTE *)args[0],MODE_OLDFILE))
                        {
                            if(filebuffer = AllocMem(buffsize,MEMF_PUBLIC|MEMF_CLEAR))
                            {
                                PutStr("Got file buffer.\n");
                                if(trans = AllocTransactionA((struct TagItem *)&ttags))
                                {
                                    trans->trans_Flags=0;
                                    trans->trans_RequestData=(UBYTE *)args[2];
                                    trans->trans_ReqDataActual=strlen((UBYTE *)args[2]) + 1;
                                    trans->trans_Command=1;
                                    trans->trans_Error=0;
                                    trans->trans_RespDataActual=0L;
                                    DoTransaction(RemoteEntity,MyEntity,trans);
                                    if(!trans->trans_Error)
                                    {
                                        PutStr("Sending file...\n");
                                        while(actual = Read(SourceFile, filebuffer, buffsize))
                                        {
                                            trans->trans_Command=2;
                                            trans->trans_Error=0;
                                            trans->trans_RequestData=filebuffer;
                                            trans->trans_ReqDataActual=actual;
                                            trans->trans_RespDataLength=0L;
                                            DoTransaction(RemoteEntity,MyEntity,trans);
                                        }
                                        trans->trans_Command=3;
                                        trans->trans_Error=0;
                                        trans->trans_RequestData="oog";
                                        trans->trans_ReqDataActual=3;
                                        trans->trans_RespDataLength=0L;
                                        DoTransaction(RemoteEntity,MyEntity,trans);
                                        PutStr("File sent.\n");
                                    }
                                    FreeTransaction(trans);
                                }
                                FreeMem(filebuffer,buffsize);
                            }
                            Close(SourceFile);
                        }
                        LoseEntity(RemoteEntity);
                    }
                    DeleteEntity(MyEntity);
                }
                CloseLibrary(NIPCBase);
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
}
