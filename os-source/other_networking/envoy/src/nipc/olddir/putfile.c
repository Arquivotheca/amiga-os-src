#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
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
BPTR out;
struct Library *DOSBase;
struct Library *NIPCBase;


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

void DoTabs(UBYTE tabs);
ULONG GetRemoteDirEntryType(UBYTE *host, UBYTE *filename);

void _main(void)
{
    ULONG *args[5];
    struct RDArgs *rdargs;
    APTR MyEntity;
    APTR RemoteEntity;
    struct Transaction *trans;
    BPTR SourceFile,oldlock;
    ULONG error, actual, buffsize;
    struct FileInfoBlock *rdest;
    UBYTE *filebuffer,*pathbuffer,i,tabs,*kill;
    struct AnchorPath *ap;
    BOOL done,destdir;
    UBYTE destname[256];

    geta4();


    buffsize = 8192;
    done = FALSE;
    tabs=3;

    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        for(i=0;i<5;i++)
            args[i]=0;

        if(rdargs=ReadArgs("FROM/A,Host/A,TO/A,BUF=BUFFER/K/N,ALL/S",(LONG *)args,NULL))
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
                                    trans->trans_ReqDataLength=strlen((UBYTE *)args[2]) + 1;
                                    trans->trans_Command=1;
                                    trans->trans_Error=0;
                                    trans->trans_RespDataLength=0L;
                                    DoTransaction(RemoteEntity,MyEntity,trans);
                                    if(!trans->trans_Error)
                                    {
                                        PutStr("Sending file...\n");
                                        while(actual = Read(SourceFile, filebuffer, buffsize))
                                        {
                                            trans->trans_Command=2;
                                            trans->trans_Error=0;
                                            trans->trans_RequestData=filebuffer;
                                            trans->trans_ReqDataLength=actual;
                                            trans->trans_RespDataLength=0L;
                                            DoTransaction(RemoteEntity,MyEntity,trans);
                                        }
                                        trans->trans_Command=3;
                                        trans->trans_Error=0;
                                        trans->trans_RequestData="oog";
                                        trans->trans_ReqDataLength=3;
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
