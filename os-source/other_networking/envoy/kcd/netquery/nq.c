#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <envoy/nipc.h>
#include <graphics/gfxbase.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/nipc_protos.h>
#include <clib/utility_protos.h>
#include <clib/alib_protos.h>
#include <clib/alib_stdio_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/nipc_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <dos.h>
#include <string.h>

struct NQGlobals
{
	struct Library	*nq_SysBase;
	struct Library	*nq_DOSBase;
	struct Library	*nq_NIPCBase;
	struct Library	*nq_UtilityBase;
	struct MinList	 nq_QueryList;
	struct Hook	 nq_Hook;
	struct RDArgs	*nq_RDArgs;
	LONG		 nq_MaxTime;
	LONG		 nq_Args[18];
	LONG		 nq_Tags[64];
	UBYTE		 nq_StrBuff[256];
	UBYTE		 nq_LibName[256];
};

struct HostInfo
{
	struct Node	 hi_Link;
	UWORD		 hi_Pad0;
	ULONG		 hi_IPAddr;
	ULONG		 hi_AttnFlags;
	ULONG		 hi_ChipRevBits;
	ULONG		 hi_KickVersion;
	ULONG		 hi_NIPCVersion;
	ULONG		 hi_LibVersion;
	ULONG		 hi_WBVersion;
	ULONG		 hi_MaxFastMem;
	ULONG		 hi_MaxChipMem;
	ULONG		 hi_AvailFastMem;
	ULONG		 hi_AvailChipMem;
	UBYTE		 hi_HostName[128];
};

#define SysBase	(*(struct Library **)4L)

#define DOSBase	nq->nq_DOSBase
#define	NIPCBase nq->nq_NIPCBase
#define UtilityBase nq->nq_UtilityBase

#define TEMPLATE	"Realm/K,Host/K,CPU/S,GFX/S,IP/S,FAST/S,CHIP/S,WBVERS/S,KICKVERS/S,MEM/S,CHIPS/S,NET/S,VERS/S,ALL/S,MAX/K/N,NIPCVERS/S,LIBVERS/K"
#define	ARG_REALM	0
#define	ARG_HOSTNAME	1
#define	ARG_CPU		2
#define ARG_GFX		3
#define ARG_IPADDR 	4
#define ARG_FASTMEM	5
#define ARG_CHIPMEM	6
#define ARG_WBVERSION	7
#define	ARG_KICKVERSION	8
#define	ARG_MEM		9
#define	ARG_CHIPS	10
#define	ARG_NET		11
#define	ARG_VERS	12
#define	ARG_ALL		13
#define ARG_MAX		14
#define ARG_NIPCVERSION 15
#define ARG_LIBVERSION	16

typedef	struct NQGlobals *NQGPtr;
typedef ULONG (*HOOK_FUNC)();

ULONG __asm __saveds HookServer(register __a0 struct Hook *hook,
			  register __a2 struct Task *sigTask,
			  register __a1 struct TagItem *tagList);

ULONG __saveds NQuery(void)
{
	NQGPtr nq;
	struct HostInfo *hi;
	LONG tagNum=0;

	if(nq = AllocMem(sizeof(struct NQGlobals),MEMF_CLEAR))
	{
	    NewList((struct List *)&nq->nq_QueryList);
	    nq->nq_Hook.h_Entry = (HOOK_FUNC)&HookServer;
	    nq->nq_Hook.h_Data	= nq;

	    if(DOSBase = OpenLibrary("dos.library",37L))
	    {
	    	if(UtilityBase = OpenLibrary("utility.library",37L))
	    	{
                    if(NIPCBase = OpenLibrary("nipc.library",37L))
                    {
                        if(nq->nq_RDArgs = ReadArgs(TEMPLATE,nq->nq_Args,NULL))
                        {
                            nq->nq_Tags[tagNum++]=QUERY_HOSTNAME;
                            nq->nq_Tags[tagNum++]=0;

                            if(nq->nq_Args[ARG_REALM])
                            {
                                nq->nq_Tags[tagNum++]=MATCH_REALM;
                                nq->nq_Tags[tagNum++]=nq->nq_Args[0];
                            }
                            if(nq->nq_Args[ARG_HOSTNAME])
                            {
                                nq->nq_Tags[tagNum++]=MATCH_HOSTNAME;
                                nq->nq_Tags[tagNum++]=nq->nq_Args[1];
                            }
                            if(nq->nq_Args[ARG_CPU] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_CHIPS])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_ATTNFLAGS;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_GFX] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_CHIPS])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_CHIPREVBITS;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_IPADDR] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_NET])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_IPADDR;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_FASTMEM] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_MEM])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_MAXFASTMEM;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_CHIPMEM] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_MEM])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_MAXCHIPMEM;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_KICKVERSION] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_VERS])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_KICKVERSION;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_WBVERSION] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_VERS])
                            {
                                nq->nq_Tags[tagNum++]=QUERY_WBVERSION;
                                nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_NIPCVERSION] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_VERS])
                            {
                            	nq->nq_Tags[tagNum++]=QUERY_NIPCVERSION;
                            	nq->nq_Tags[tagNum++]=0;
                            }
                            if(nq->nq_Args[ARG_LIBVERSION])
                            {
                            	nq->nq_Tags[tagNum++]=MATCH_LIBVERSION;
                            	nq->nq_Tags[tagNum++]=(LONG)nq->nq_LibName;
                            	nq->nq_LibName[1]=37; /* Hack */
                            	strcpy(&nq->nq_LibName[4],(STRPTR)nq->nq_Args[ARG_LIBVERSION]);
                            }
                            nq->nq_Tags[tagNum]=TAG_DONE;

			    if(nq->nq_Args[ARG_MAX])
			    	nq->nq_MaxTime = *((LONG *)nq->nq_Args[ARG_MAX]);
			    else
			    	nq->nq_MaxTime = 2;

                            NIPCInquiryA(&nq->nq_Hook,nq->nq_MaxTime,0xffffffff,(struct TagItem *)nq->nq_Tags);
                            Wait(SIGF_NET);

                            while(hi = (struct HostInfo *)RemHead((struct List *)&nq->nq_QueryList))
                            {
                                Printf("%-20s  ",hi->hi_HostName);

                                if(nq->nq_Args[ARG_IPADDR] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_NET])
                                {
                                    sprintf(nq->nq_StrBuff,"%ld.%ld.%ld.%ld",hi->hi_IPAddr >> 24,
                                                                         (hi->hi_IPAddr >>16) & 0xff,
                                                                         (hi->hi_IPAddr >>8)  & 0xff,
                                                                         (hi->hi_IPAddr) & 0xff);
                                    Printf("%16s ",nq->nq_StrBuff);
                                }
                                if(nq->nq_Args[ARG_CPU] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_CHIPS])
                                {
                                    if(hi->hi_AttnFlags & AFF_68040)
                                        PutStr("040");
                                    else if(hi->hi_AttnFlags & AFF_68030)
                                        PutStr("030");
                                    else if(hi->hi_AttnFlags & AFF_68020)
                                        PutStr("020");
                                    else if(hi->hi_AttnFlags & AFF_68010)
                                        PutStr("010");
                                    else
                                        PutStr("68k");

                                    if(hi->hi_AttnFlags & AFF_FPU40)
                                        PutStr("     ");
                                    else if(hi->hi_AttnFlags & AFF_68882)
                                        PutStr("/882 ");
                                    else if(hi->hi_AttnFlags & AFF_68881)
                                        PutStr("/881 ");
                                    else
                                        PutStr("     ");
                                }
                                if(nq->nq_Args[ARG_GFX] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_CHIPS])
                                {
                                    if((hi->hi_ChipRevBits & SETCHIPREV_AA) == SETCHIPREV_AA)
                                        PutStr("AA  ");
                                    else if((hi->hi_ChipRevBits & SETCHIPREV_ECS) == SETCHIPREV_ECS)
                                        PutStr("ECS ");
                                    else if((hi->hi_ChipRevBits & SETCHIPREV_A) == SETCHIPREV_A)
                                        PutStr("A   ");
                                    else
                                        PutStr("OLD ");
                                }
                                if(nq->nq_Args[ARG_FASTMEM] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_MEM])
                                {
                                    Printf("F: %5ldKb ",hi->hi_MaxFastMem/1024);
                                }
                                if(nq->nq_Args[ARG_CHIPMEM] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_MEM])
                                {
                                    Printf("C: %ldMb ",(hi->hi_MaxChipMem+5248)/1048576);
                                }
                                if(nq->nq_Args[ARG_KICKVERSION] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_VERS])
                                {
                                    Printf("Sys: %2ld.%-4ld ",hi->hi_KickVersion >> 16, (hi->hi_KickVersion) & 0xffff);
                                }
                                if(nq->nq_Args[ARG_WBVERSION] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_VERS])
                                {
                                    Printf("WB: %2ld.%-3ld ",hi->hi_WBVersion >> 16, (hi->hi_WBVersion) & 0xffff);
                                }
                                if(nq->nq_Args[ARG_NIPCVERSION] || nq->nq_Args[ARG_ALL] || nq->nq_Args[ARG_VERS])
                                {
                                    Printf("NIPC: %2ld.%-3ld ",hi->hi_NIPCVersion >> 16, (hi->hi_NIPCVersion) & 0xffff);
                                }
                                if(nq->nq_Args[ARG_LIBVERSION])
                                {
                                    Printf("%s %2ld.%-3ld ",&nq->nq_LibName[4],hi->hi_LibVersion >> 16, (hi->hi_LibVersion) & 0xffff);
                                }

                                PutStr("\n");

                                FreeMem(hi,sizeof(struct HostInfo));
                            }
                            FreeArgs(nq->nq_RDArgs);
                        }
                        CloseLibrary(NIPCBase);
                    }
                    CloseLibrary(UtilityBase);
		}
		CloseLibrary(DOSBase);
	    }
	    FreeMem(nq,sizeof(struct NQGlobals));
	}
	return 0;
}

ULONG __asm __saveds HookServer(register __a0 struct Hook *hook,
			  register __a2 struct Task *sigTask,
			  register __a1 struct TagItem *tagList)
{
    NQGPtr nq;
    struct TagItem *tag,*scan;
    struct HostInfo *hi;

    nq = (NQGPtr) hook->h_Data;

    scan = tagList;

    if(tagList)
    {
        if(tag = FindTagItem(QUERY_HOSTNAME, tagList))
        {
            if(!FindName((struct List *)&nq->nq_QueryList,(STRPTR)tag->ti_Data))
            {
                if(hi = (struct HostInfo *)AllocMem(sizeof(struct HostInfo),MEMF_CLEAR))
                {
                    hi->hi_Link.ln_Name = hi->hi_HostName;

                    while(tag = NextTagItem(&scan))
                    {
                        switch(tag->ti_Tag)
                        {
                            case QUERY_HOSTNAME:    stccpy(hi->hi_HostName,(STRPTR)tag->ti_Data,128);
                                                    break;

                            case QUERY_IPADDR:      hi->hi_IPAddr = (ULONG)tag->ti_Data;
                                                    break;

                            case QUERY_ATTNFLAGS:   hi->hi_AttnFlags = (ULONG)tag->ti_Data;
                                                    break;

                            case QUERY_CHIPREVBITS: hi->hi_ChipRevBits = (ULONG)tag->ti_Data;
                                                    break;

                            case QUERY_MAXFASTMEM:  hi->hi_MaxFastMem = (ULONG)tag->ti_Data;
                                                    break;

                            case QUERY_MAXCHIPMEM:  hi->hi_MaxChipMem = (ULONG)tag->ti_Data;
                                                    break;

                            case QUERY_KICKVERSION: hi->hi_KickVersion = (ULONG)tag->ti_Data;
                                                    break;

                            case QUERY_WBVERSION:   hi->hi_WBVersion = (ULONG)tag->ti_Data;
                                                    break;

			    case QUERY_NIPCVERSION: hi->hi_NIPCVersion = (ULONG)tag->ti_Data;
			    			    break;

			    case MATCH_LIBVERSION:  hi->hi_LibVersion = *((ULONG *)tag->ti_Data);
			    			    break;
                        }
                    }
                    AddTail((struct List *)&nq->nq_QueryList,(struct Node *)hi);
                }
            }
        }
    }
    else
    	Signal(sigTask,SIGF_NET);

    return 1;
}
