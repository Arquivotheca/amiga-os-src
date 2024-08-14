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
    ULONG *args[5];
    register struct Library *DOSBase;
    register struct Library *NIPCBase;
    struct RDArgs *rdargs;
    APTR MyEntity;
    APTR RemoteEntity;
    struct Transaction *trans;
    BPTR SourceFile;
    ULONG error, actual, buffsize;
    UBYTE *filebuffer,*pathbuffer,i,*kill;
    struct AnchorPath *ap;
    BOOL done;
    UBYTE destname[256];

    geta4();

    buffsize = 8192;
    done = FALSE;
    if(DOSBase=OpenLibrary("dos.library",37L))
    {
        for(i=0;i<5;i++)
            args[i] = 0L;

        if(rdargs=ReadArgs("FROM/A,Host/A,TO/A,BUF=BUFFER/K/N,ALL/S",(LONG *)args,NULL))
        {
            if(args[3])
                buffsize=*args[3];
            if(ap = (struct AnchorPath *) AllocMem(sizeof(struct AnchorPath) + 256,MEMF_PUBLIC|MEMF_CLEAR))
            {
                ap->ap_Strlen = 256;
                ap->ap_BreakBits = SIGBREAKF_CTRL_C;
                strcpy(destname,(UBYTE *)args[2]);

                if(!MatchFirst((UBYTE *)args[0],ap))
                {
                    if(args[4])
                        ap->ap_Flags |= APF_DOWILD;

                    while(!done)
                    {

                        if(ap->ap_Info.fib_DirEntryType > 0)
                        {
                            if(args[4])
                            {
                                if(!(ap->ap_Flags & APF_DIDDIR))
                                {
                                    ap->ap_Flags |= APF_DODIR|APF_DIDDIR;
                                    PutStr(ap->ap_Info.fib_FileName);
                                    PutStr(" (Dir)   [created]\n");
                                    PutStr("  --->   ");
                                    AddPart(destname,ap->ap_Info.fib_FileName,256);
                                    PutStr(destname);
                                    PutStr("\n");
                                }
                                else
                                {
                                    kill = PathPart(destname);
                                    *kill=0;
                                    ap->ap_Flags &= ~APF_DODIR;
                                }
                            }
                            else
                            {
                                PutStr(ap->ap_Info.fib_FileName);
                                PutStr(" (Dir)\n");
                            }
                        }
                        else
                        {
                            ap->ap_Flags &= ~APF_DIDDIR;
                            PutStr(ap->ap_Info.fib_FileName);
                            PutStr("  --->   ");
                            AddPart(destname,ap->ap_Info.fib_FileName,256);
                            PutStr(destname);
                            PutStr("\n");
                            kill = PathPart(destname);
                            *kill=0;
                        }

                        if(MatchNext(ap))
                            done = TRUE;
                    }
                    MatchEnd(ap);
                }
                FreeMem(ap,sizeof(struct AnchorPath));
            }
            FreeArgs(rdargs);
        }
        CloseLibrary(DOSBase);
    }
}

/*
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

*/
