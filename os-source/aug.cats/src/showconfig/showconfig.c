/* showconfig.c */

/*
 *  showconfig.c - List OS revs, chips, memory, and AUTOCONFIG configuration
 *  Carolyn Scheppner 01/24/90
 *
 * Note - assembler module (by Dave Haynie) contains the following 3 functions:
 *
 *	ULONG GetCPUType(void)
 *
 *		Returns a number, representing the type of CPU in the 
 *		system: 68000L, 68010L, 68020L, 68030L, or 68040L.
 *
 *	ULONG GetFPUType(void)
 *
 *		Returns a number, representing the type of FPU in the 
 *		system: 0L (no FPU), 68881L, or 68882L.
 *
 *	ULONG GetMMUType(void)
 *
 *		Returns a number, representing the type of MMU in the 
 *		system: 0L (no MMU), 68851L, 68030L, or 0xFFFFFFFFL
 *		(FPU responding to MMU address!).
 *
 */

#include "exec/types.h"
#include "exec/execbase.h"
#include "exec/memory.h"
#include "graphics/gfxbase.h"
#include "libraries/configvars.h"
#include "hardware/custom.h"
#include "showconfig_rev.h"

#ifndef AFF_68030
#define AFF_68030 (1<<2)
#define AFF_68040 (1<<3)
#define AFF_68882 (1<<5)
#endif

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/expansion_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
int chkabort(void) { return(0); }  /* really */
extern struct Custom far custom;
#endif

char *vers = VERSTAG;

char *Copyright = 
  "Copyright (c) 1991 Commodore-Amiga, Inc.  All Rights Reserved";

struct Board {
    UWORD manu;
    UBYTE prod;
    UBYTE *manu_s;
    UBYTE *prod_s;
    };

struct Board boards[] = {
    { 514,   1, "CBM", "A2090/A2090A HD controller"},
    { 514,   2, "CBM", "A590/A2091 HD controller"},
    { 514,   3, "CBM", "A590/A2091 HD controller"},
    { 514,   4, "CBM", "A2090B 2090 Autoboot card"},
    { 514,   9, "CBM", "Arcnet card"},
    { 514,  10, "CBM", "A2052/58.RAM | 590/2091.RAM"},
    { 514,  32, "CBM", "A560 memory module"},
    { 514,  69, "CBM", "A2232 serial prototype"},
    { 514,  70, "CBM", "A2232 serial production"},
    { 514,  80, "CBM", "A2620 68020/RAM card"},
    { 514,  81, "CBM", "A2630 68030/RAM card"},
    { 514,  96, "CBM", "Romulator card"},
    { 514,  97, "CBM", "A3000 test fixture"},
    { 514, 112, "CBM", "A2065 Ethernet card"},
    
    { 513,  1, "CBM", "Bridgeboard"},
    { 1030, 0, "CBM", "A2410 Hires Graphics card"},
    { 1053, 1, "Ameristar", "Ethernet card"},

    { NULL }
    };

UBYTE *memsize_s[]={"8meg","64K","128K","256K","512K","1 meg","2meg","4meg"};

#define denise_id ((custom.deniseid)&0x00FF)
#define agnus_id  (((custom.vposr)&0x7F00)>>8)

#define MAXAGNUSIDS 6
UWORD agnusids[MAXAGNUSIDS] =
	{ 0x00, 0x10,
	  0x20, 0x30,
	  0x21, 0x31 };
UBYTE *agnus_s[MAXAGNUSIDS+1] =
	{"Normal PAL","Normal NTSC",
	 "ECS PAL","ECS NTSC",
	 "ECS PAL","ECS NTSC",
	 "UNKNOWN" };

/* F8 is actually AA Lisa */
#define MAXDENISEIDS 4
UWORD deniseids[MAXDENISEIDS] =
	{ 0x00, 0xFC,
	  0xEC, 0xF8};
UBYTE *denise_s[MAXDENISEIDS+1] =
	{"Normal", "ECS",
	 "ECS", "UNKNOWN",
	 "UNKNOWN" };

UBYTE *line = 
 "=======================================================================\n";

struct Library   *ExpansionBase = NULL;

void showboards(void);
void showchips(void);
void showmem(void);
void showos(void);
ULONG GetCPUType(void);
ULONG GetFPUType(void);
ULONG GetMMUType(void);

BOOL deb=FALSE, kickit=FALSE;
ULONG romstart;

void main(int argc, char **argv)
    {
    extern struct ExecBase *SysBase;

    if(argc>1)
	{
	if(argv[1][0]=='?')
	    {
	    printf("DEBUG/K\n");
	    exit(RETURN_OK);
	    }
	else if(((argv[1][0]|0x20)=='d')&&((argv[1][1]|0x20)=='e')) deb = TRUE;
	}

    /* Cheap test for where rom is, kickit'd machine */
    romstart = ((ULONG)(SysBase->LibNode.lib_IdString)) & 0xFFFF0000;
    kickit = (romstart == 0x00200000) ? TRUE : FALSE;    

    if((ExpansionBase=OpenLibrary("expansion.library",0L))==NULL)
	exit(RETURN_FAIL);

    showchips();
    showos();
    showmem();
    showboards();

    CloseLibrary(ExpansionBase);
    exit(RETURN_OK);
    }



void showmem()
    {
    extern struct ExecBase *SysBase;
    struct ExecBase *eb = SysBase;
    struct MemHeader *mem, *firstmem;
    ULONG  nods[32], mems[32], ends[32], atts[32];
    ULONG  membytes, rmembytes, memhmeg, memmeg, memk;
    ULONG  memCnt = 0;
    UBYTE  *s, *about;
    int    k;

    Forbid();
    firstmem = (struct MemHeader *)eb->MemList.lh_Head;

    /* Go to end of MemHeader list */
    for (mem = firstmem;
           mem->mh_Node.ln_Succ;
              mem = (struct MemHeader *)mem->mh_Node.ln_Succ)
	{
	nods[memCnt] = (ULONG)mem->mh_Node.ln_Type;
	mems[memCnt] = (ULONG)mem;
	ends[memCnt] = (ULONG)mem->mh_Upper;
	atts[memCnt] = (ULONG)mem->mh_Attributes;
	memCnt++;
	}
    Permit();

    printf("RAM:");
    for(k=0; k<memCnt; k++)
	{
        if(atts[k] & MEMF_CHIP) s = "CHIP";
	else if(atts[k] & MEMF_FAST) s = "FAST";
	else s = "????";
	printf("\tNode type $%lx, Attributes $%lx (%s), at $%lx-$%lx ",
            nods[k], atts[k], s, mems[k], (ends[k]) - 1);

	membytes  = ends[k] - mems[k];
	/* rounded to nearest 64K */
	rmembytes = ((ends[k]+(32 * 1024)) & 0xFFFF0000) - (mems[k] & 0xFFFF0000);
	if(membytes == rmembytes)
		{
		about = "(";
		}
	else
		{
		about = "(~";
		membytes = rmembytes;
		}

	if((memhmeg = (membytes / 0x80000))&&((memhmeg * 0x80000)==membytes))
		{
		memmeg = memhmeg >> 1;
		memhmeg -= (memmeg << 1);
		printf("%s%ld.%ld meg)\n",about, memmeg, memhmeg * 5);
		}
	else if(memk = (membytes / 0x400))	printf("(%ld K)\n",memk);
	else printf("(%ld bytes)\n",membytes);
	}
    }


void showchips()
    {
    struct GfxBase *gfxbase;
    ULONG cpu,fpu=0L,mmu=0L;
    int i, di, ai;
    UWORD a,d;

    /* motherboard chip id's */
    printf("PROCESSOR:\t");

    cpu = GetCPUType();
    if(cpu > 68010)
       {
       mmu = GetMMUType();
       fpu = GetFPUType();
       }

    printf("CPU %ld",cpu);
    if(fpu) printf("/%ldfpu",fpu);
    if(mmu) printf("/%ldmmu",mmu);

    printf("\nCUSTOM CHIPS:\t");

    a = agnus_id;
    for(ai = 0; ai < (MAXAGNUSIDS); ai++)
	{
	if(agnusids[ai] == a) break;
	}

    d = denise_id;
    for (i = 0; i < 16 ; i++)
	{
    	if ( d != denise_id )
    	    {
            d = 0;
            break;
            }
	}
    if(d == 0xFF) d=0;

    for(di = 0; di < (MAXDENISEIDS); di++)
	{
	if(deniseids[di] == d) break;
	}

    if(a==0)   /* May be old dip-style agnus with no ID */
	{
	if(gfxbase=(struct GfxBase *)OpenLibrary("graphics.library",0L))
	    {
	    if(gfxbase->DisplayFlags & NTSC) ai=1;
	    CloseLibrary((struct Library *)gfxbase);
	    }
	}

    printf("%s Agnus (id=$%04lx), %s Denise (id=$%04lx)\n",
         agnus_s[ai], agnus_id, denise_s[di], denise_id); 
    }


void showos()
    {
    struct Library *lib;
    UWORD romv,romr;

    romv = *(UWORD *)(romstart + 0x000c);
    romr = *(UWORD *)(romstart + 0x000e);

    printf("VERS:\tKickstart version %d.%d",romv,romr);
    if(lib = OpenLibrary("exec.library",0L))
	{
	printf(", Exec version %ld.%ld, ",
		lib->lib_Version,lib->lib_Revision);
	CloseLibrary(lib);
	}
    if(lib = OpenLibrary("version.library",0L))
	{
	printf("Disk version %ld.%ld",
		lib->lib_Version,lib->lib_Revision);
	CloseLibrary(lib);
	}
    printf("\n");
    }


void showboards()
    {
    struct ConfigDev *myCD;
    ULONG addr,diagcopy,bdcnt=0L;
    UWORD m,t,f,k;
    UBYTE p,msbits,btype;
    BOOL ismem,hasrom,pref8,noshut,chained;

    /*--------------------------------------------------*/
    /* FindConfigDev(oldConfigDev,manufacturer,product) */
    /* oldConfigDev = NULL for the top of the list      */
    /* manufacturer = -1 for any manufacturer           */
    /* product      = -1 for any product                */
    /*--------------------------------------------------*/

    printf("BOARDS:\n");


    if(kickit) 
	{
	if(deb) printf(line);
        printf(" Initial RAM-containing board configured by KickIt (?)\n");
	}

    myCD = NULL;
    while(myCD=FindConfigDev(myCD,-1L,-1L))
	{
	bdcnt++;
	/* These were read directly from the board at expansion time */
	m = myCD->cd_Rom.er_Manufacturer;
	p = myCD->cd_Rom.er_Product;

	t = myCD->cd_Rom.er_Type;
	btype = (t >> ERT_TYPEBIT);
	ismem = (t & ERTF_MEMLIST) ? TRUE : FALSE;
	hasrom = t & ERTF_DIAGVALID;
	chained = t & ERTF_CHAINEDCONFIG;
	msbits = t & ERT_MEMMASK;
    
	/* Expansion has already uninverted these, so
 	 * 1configxxx means prefers 8 meg space
 	 * x1configxx means can't be shut up
 	 */
	f = myCD->cd_Rom.er_Flags;
	pref8  = (f & ERFF_MEMSPACE);
	noshut = (f & ERFF_NOSHUTUP);

	addr=(ULONG)myCD->cd_BoardAddr;
	diagcopy = *(ULONG *)&myCD->cd_Rom.er_Reserved0c;


	if(deb) printf(line);

	for(k=0; boards[k].manu; k++)
            {
            if((boards[k].manu == m)&&(boards[k].prod == p))
                {
                printf(" %s %s",boards[k].manu_s,boards[k].prod_s);
                break;
                }
            }
	if(boards[k].manu == 0)
	    {
 	    if(ismem) printf(" RAM (unidentified)");
            else if(hasrom) printf(" Board + ROM (HD?) (unidentified)");
	    else printf(" Board (unidentified)");
	    }
	printf(":   Prod=%d/%d($%x/$%x) (@$%lx %s%s\n",m,p,m,p,addr,
                      memsize_s[msbits], ismem ? " Mem)" : ")");

	if(deb)
	    {
            printf(" ConfigDev structure found at location $%lx\n",myCD);
            printf("==== Board ID (ExpansionRom) information:\n");
            printf("er_Manufacturer         =%d=$%x=(~$%4x)\n",m,m,(UWORD)~m);
            printf("er_Product              =%d=$%x=(~$%2x)\n",p,p,(UBYTE)~p);
            printf("er_Type                 =$%x\n",t);

            printf("  (type %d, size %s, %s, %s, %s)\n",
                      btype,
                      memsize_s[msbits],
                      ismem  ? "add to free list" : "not for free list",
                      hasrom ? "ROM diag vec valid" : "no ROM diag vec",
                      chained ? "chained to next" : "not chained");

            printf("er_Flags                =$%x\n",f);
            printf("  (%s, %s)\n",
                      pref8  ? "prefers 8-meg space" : "no space preference",
                      noshut ? "can not be shut up" : "can be shut up");

            printf("er_InitDiagVec          =$%x\n",myCD->cd_Rom.er_InitDiagVec);
            if(diagcopy) printf("DiagCopy at             =$%x\n",diagcopy);
            /* These values are generated when the AUTOCONFIG(tm) software
             * relocates the board
             */
            printf("==== Configuration (ConfigDev) information:\n");
            printf("cd_BoardAddr            =$%lx\n",addr);
            printf("cd_BoardSize            =$%lx (%ldK)\n",
               myCD->cd_BoardSize,((ULONG)myCD->cd_BoardSize)/1024);

            printf("cd_Flags                =$%x",myCD->cd_Flags);
            printf("  (CONFIGME bit %s)\n",
                     myCD->cd_Flags & CDF_CONFIGME ? "still set" : "cleared");
            }

	}
    if(bdcnt==0) printf(" None\n");
    else if(deb) printf(line);
    }

