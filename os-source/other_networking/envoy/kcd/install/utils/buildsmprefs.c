#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/rdargs.h>
#include <libraries/iffparse.h>
#include <prefs/prefhdr.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/iffparse_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/iffparse_pragmas.h>
#include <pragmas/utility_pragmas.h>

#define ID_ISVC  MAKE_ID('I','S','V','C')
#define ID_PREF  MAKE_ID('P','R','E','F')

struct IFFService
{
    ULONG   is_Flags;
    UWORD   is_UID;
    UWORD   is_GID;
    UBYTE   is_PathName[256];
    UBYTE   is_SvcName[64];
};

struct IFFService isvc;

struct Library *DOSBase;
struct Library *IFFParseBase;
struct Library *UtilityBase;

#define SysBase (*(struct Library **)4L)
#define SVCFLAGB_ENABLE     0
#define SVCFLAGF_ENABLE     (1L << SVCFLAGB_ENABLE)

ULONG StrToIP(STRPTR ipstr);

ULONG buildprefs(VOID)
{
    struct RDArgs *rdargs;
    struct RDArgs *parseargs;

    STRPTR linebuff,curchr;
    LONG val;

    struct IFFHandle *iff;

    BPTR SourceFile, DestFile;

    LONG args[2];
    LONG fileargs[10];
    UBYTE keybuff[32];

    struct PrefHeader hdr;

    UWORD i;

    if(DOSBase = OpenLibrary("dos.library",37L))
    {

    	if(IFFParseBase = OpenLibrary("iffparse.library",37L))
    	{

    	    if(UtilityBase = OpenLibrary("utility.library",37L))
    	    {
                args[0]=args[1]=0;

                if(rdargs = ReadArgs("SourceFile/A,DestFile/A",args,NULL))
                {
                    if(SourceFile = Open((STRPTR)args[0],MODE_OLDFILE))
                    {

                        if(iff = AllocIFF())
                        {

                            if(DestFile = Open((STRPTR)args[1],MODE_NEWFILE))
                            {

                                iff->iff_Stream = DestFile;

                                InitIFFasDOS(iff);

                                if(!OpenIFF(iff, IFFF_WRITE))
                                {

                                    if(parseargs = AllocDosObject(DOS_RDARGS, NULL))
                                    {

                                        if(linebuff = AllocMem(256,MEMF_CLEAR|MEMF_PUBLIC))
                                        {

                                            if (!PushChunk(iff,ID_PREF,ID_FORM,IFFSIZE_UNKNOWN))
                                            {

                                                if(!PushChunk(iff,0,ID_PRHD,sizeof(struct PrefHeader)))
                                                {

                                                    hdr.ph_Version = 0;
                                                    hdr.ph_Type = 0;
                                                    hdr.ph_Flags = 0;

                                                    if (WriteChunkBytes(iff,&hdr,sizeof(struct PrefHeader)) == sizeof(struct PrefHeader))
                                                    {

                                                        if (!PopChunk(iff))
                                                        {

                                                            while(FGets(SourceFile,linebuff,255))
                                                            {

                                                                parseargs->RDA_Source.CS_Buffer = linebuff;
                                                                parseargs->RDA_Source.CS_Length = 255;
                                                                parseargs->RDA_Source.CS_CurChr = 0;

                                                                curchr = linebuff;
                                                                while(*curchr)
                                                                    curchr++;
                                                                *curchr='\n';

                                                                val = ReadItem(keybuff, 31, &parseargs->RDA_Source);

                                                                if(val == ITEM_UNQUOTED)
                                                                {

                                                                    for(i=0;i<10;i++)
                                                                        fileargs[i]=0L;

                                                                    if(!Stricmp(keybuff,"SERVICE"))
                                                                    {
                                                                        if(ReadArgs("ServiceName/A,PathName/A,Active/S",fileargs,parseargs))
                                                                        {
                                                                            stccpy(isvc.is_SvcName,(STRPTR)fileargs[0],64);
                                                                            stccpy(isvc.is_PathName,(STRPTR)fileargs[1],256);
                                                                            if(fileargs[2])
                                                                                isvc.is_Flags = SVCFLAGF_ENABLE;
                                                                            isvc.is_UID = isvc.is_GID = 0;

                                                                            if(!PushChunk(iff,0,ID_ISVC,sizeof(struct IFFService)))
                                                                            {
                                                                                if(WriteChunkBytes(iff,&isvc,sizeof(struct IFFService)) == sizeof(struct IFFService))
                                                                                {
                                                                                    PopChunk(iff);
                                                                                }
                                                                            }
                                                                            FreeArgs(parseargs);
                                                                        }
                                                                    }

                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                                PopChunk(iff);
                                            }
                                            FreeMem(linebuff,256);
                                        }
                                        FreeDosObject(DOS_RDARGS,parseargs);
                                    }
                                    CloseIFF(iff);
                                }
                                Close(DestFile);
                            }
                            FreeIFF(iff);
                        }
                        Close(SourceFile);
                    }
                    FreeArgs(rdargs);
                }
                CloseLibrary(UtilityBase);
            }
            CloseLibrary(IFFParseBase);
        }
        CloseLibrary(DOSBase);
    }
    return(0);
}

ULONG StrToIP(STRPTR ipstr)
{
    STRPTR scan;
    ULONG num,val;

    num=val=0;
    scan = ipstr;

    while(*scan)
    {
    	if(*scan == '.')
    	{
    	    num = (num << 8) | val;
    	    val = 0;
    	}
    	else
    	    val = val * 10 + (*scan - '0');

   	scan++;
   }
   num = (num << 8) | val;
   return(num);
}

