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

#define ID_PREF  MAKE_ID('P','R','E','F')
#define ID_NDEV  MAKE_ID('N','D','E','V')
#define ID_NRRM  MAKE_ID('N','R','R','M')
#define ID_NLRM  MAKE_ID('N','L','R','M')
#define ID_NIRT  MAKE_ID('N','I','R','T')
#define ID_HOST  MAKE_ID('H','O','S','T')

struct NIPCPrefs
{
    ULONG   np_RealmServer;
    ULONG   np_RealmName;
    UBYTE   np_UserName[16];
    UBYTE   np_AmigaName[64];
    ULONG   np_RealmServerName[64];
};

struct NIPCDevicePrefs
{
    ULONG   ndp_IPType;
    ULONG   ndp_ARPType;
    ULONG   ndp_Unit;
    UBYTE   ndp_HardAddress[16];
    UBYTE   ndp_HardString[40];
    ULONG   ndp_IPAddress;
    ULONG   ndp_IPSubnet;
    UBYTE   ndp_Flags;
    UBYTE   ndp_DevPathName[256];
};

struct NIPCIFFDevice
{
    struct Node            nd_Node;
    struct NIPCDevicePrefs nd_Prefs;
};

struct NIPCRoutePrefs
{
    ULONG   nrp_DestNetwork;
    ULONG   nrp_Gateway;
    UWORD   nrp_Hops;
};

struct NIPCIFFRoute
{
    struct Node             nr_Node;
    UBYTE                   nr_String[64];
    struct NIPCRoutePrefs   nr_Prefs;
};

struct NIPCRealmPrefs
{
    UBYTE   nzp_RealmName[64];
    ULONG   nzp_RealmAddr;
};

struct NIPCIFFRealm
{
    struct Node             nz_Node;
    UBYTE                   nz_String[128];
    struct NIPCRealmPrefs   nz_Prefs;
};

struct NIPCIFFHostPrefs
{
    UBYTE   nhp_HostName[64];
    UBYTE   nhp_RealmName[64];
    ULONG   nhp_RealmServAddr;
    UBYTE   nhp_OwnerName[32];
    ULONG   nhp_HostFlags;
};

#define NHPFLAGB_REALMS		0
#define NHPFLAGB_REALMSERVER	1

#define	NHPFLAGF_REALMS		(1L << NHPFLAGB_REALMS)
#define	NHPFLAGF_REALMSERVER	(1L << NHPFLAGB_REALMSERVER)

#define NDPFLAGB_SUBNET   0
#define NDPFLAGB_ONLINE   1
#define NDPFLAGB_USEARP   2
#define NDPFLAGB_IPTYPE   3
#define NDPFLAGB_ARPTYPE  4
#define NDPFLAGB_HARDADDR 5

#define NDPFLAGF_SUBNET   (1L << NDPFLAGB_SUBNET)
#define NDPFLAGF_ONLINE   (1L << NDPFLAGB_ONLINE)
#define NDPFLAGF_USEARP   (1L << NDPFLAGB_USEARP)
#define NDPFLAGF_IPTYPE   (1L << NDPFLAGB_IPTYPE)
#define NDPFLAGF_ARPTYPE  (1L << NDPFLAGB_ARPTYPE)
#define NDPFLAGF_HARDADDR (1L << NDPFLAGB_HARDADDR)

struct NIPCDevicePrefs ndp;
struct NIPCRoutePrefs  nrp;
struct NIPCRealmPrefs  nrlmp;
struct NIPCIFFHostPrefs   nhp;

struct Library *DOSBase;
struct Library *IFFParseBase;
struct Library *UtilityBase;

#define SysBase (*(struct Library **)4L)

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

                                                                    if(!Stricmp(keybuff,"HOSTINFO"))
                                                                    {
                                                                        if(ReadArgs("HostName/A,RealmName/K,RealmIP/K,Owner/K",fileargs,parseargs))
                                                                        {
                                                                            stccpy(nhp.nhp_HostName,(STRPTR)fileargs[0],64);
									    stccpy(nhp.nhp_OwnerName,(STRPTR)fileargs[3],32);

                                                                            if(fileargs[1])
                                                                            {
                                                                                stccpy(nhp.nhp_RealmName,(STRPTR)fileargs[1],64);
                                                                                nhp.nhp_RealmServAddr = StrToIP((STRPTR)fileargs[2]);
                                                                                nhp.nhp_HostFlags |= NHPFLAGF_REALMS;
                                                                            }

                                                                            if(!PushChunk(iff,0,ID_HOST,sizeof(struct NIPCIFFHostPrefs)))
                                                                            {
                                                                                if(WriteChunkBytes(iff,&nhp,sizeof(struct NIPCIFFHostPrefs)) == sizeof(struct NIPCIFFHostPrefs))
                                                                                {
                                                                                    PopChunk(iff);
                                                                                }
                                                                            }
                                                                            FreeArgs(parseargs);
                                                                        }
                                                                    }
                                                                    else if(!Stricmp(keybuff,"DEVINFO"))
                                                                    {
                                                                        if(ReadArgs("DevName/A,IPAddr/A,IPType/A/N,ARPType/A/N,SubMask/K",fileargs,parseargs))
                                                                        {
                                                                            stccpy(ndp.ndp_DevPathName,(STRPTR)fileargs[0],256);

                                                                            ndp.ndp_IPAddress = StrToIP((STRPTR)fileargs[1]);
                                                                            ndp.ndp_IPType = *((LONG *)fileargs[2]);
                                                                            ndp.ndp_ARPType = *((LONG *)fileargs[3]);
                                                                            ndp.ndp_Flags = NDPFLAGF_ONLINE;

                                                                            if(fileargs[4])
                                                                            {
                                                                                ndp.ndp_Flags |= NDPFLAGF_SUBNET;
                                                                                ndp.ndp_IPSubnet = StrToIP((STRPTR)fileargs[4]);
                                                                            }

                                                                            if(!PushChunk(iff,0,ID_NDEV,sizeof(struct NIPCDevicePrefs)))
                                                                            {
                                                                                if(WriteChunkBytes(iff,&ndp,sizeof(struct NIPCDevicePrefs)) == sizeof(struct NIPCDevicePrefs))
                                                                                {
                                                                                    PopChunk(iff);
                                                                                }
                                                                            }
                                                                            FreeArgs(parseargs);
                                                                        }
                                                                    }
                                                                    else if(!Stricmp(keybuff,"ROUTEINFO"))
                                                                    {
                                                                        if(ReadArgs("Destination/A,Gateway/A,Hopcount/N",fileargs,parseargs))
                                                                        {
                                                                            nrp.nrp_DestNetwork = StrToIP((STRPTR)fileargs[0]);
                                                                            nrp.nrp_Gateway = StrToIP((STRPTR)fileargs[1]);
                                                                            nrp.nrp_Hops = (*(LONG *)fileargs[2]);

                                                                            if(!PushChunk(iff,0,ID_NIRT,sizeof(struct NIPCRoutePrefs)))
                                                                            {
                                                                                if(WriteChunkBytes(iff,&nrp,sizeof(struct NIPCRoutePrefs)) == sizeof(struct NIPCRoutePrefs))
                                                                                {
                                                                                    PopChunk(iff);
                                                                                }
                                                                            }
                                                                            FreeArgs(parseargs);
                                                                        }
                                                                    }
                                                                    else if(!Stricmp(keybuff,"LOCREALM"))
                                                                    {
                                                                        if(ReadArgs("RealmName/A,RealmAddr/A",fileargs,parseargs))
                                                                        {
                                                                            stccpy(nrlmp.nzp_RealmName,(STRPTR)fileargs[0],64);
                                                                            nrlmp.nzp_RealmAddr = StrToIP((STRPTR)fileargs[1]);

                                                                            if(!PushChunk(iff,0,ID_NLRM,sizeof(struct NIPCRealmPrefs)))
                                                                            {
                                                                                if(WriteChunkBytes(iff,&nrlmp,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
                                                                                {
                                                                                    PopChunk(iff);
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                    else if(!Stricmp(keybuff,"REMREALM"))
                                                                    {
                                                                        if(ReadArgs("RealmName/A,Server/A",fileargs,parseargs))
                                                                        {
                                                                            stccpy(nrlmp.nzp_RealmName,(STRPTR)fileargs[0],64);
                                                                            nrlmp.nzp_RealmAddr = StrToIP((STRPTR)fileargs[1]);

                                                                            if(!PushChunk(iff,0,ID_NRRM,sizeof(struct NIPCRealmPrefs)))
                                                                            {
                                                                                if(WriteChunkBytes(iff,&nrlmp,sizeof(struct NIPCRealmPrefs)) == sizeof(struct NIPCRealmPrefs))
                                                                                {
                                                                                    PopChunk(iff);
                                                                                }
                                                                            }
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

