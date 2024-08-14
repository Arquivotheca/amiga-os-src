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
 * 37.x - combined config and showconfig for 68040 awareness, hopefully
 *   clean non-identication of unknown chip revs
 * 38.1 - changed version, added checks for Zorro III extended size boards
 * 40.2 - add A4091, SAS6-ify
 */

#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <libraries/configvars.h>
#include <dos/rdargs.h>
#include <hardware/custom.h>
#include <dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/expansion_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#include <pragmas/expansion_pragmas.h>

#include <string.h>

#include "showconfig_rev.h"


struct ExecBase *SysBase;
struct Library  *DOSBase;
struct Library  *ExpansionBase;

extern struct Custom far custom;

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
    { 514,  84, "CBM", "A4091 SCSI HD controller"},

    { 513,  103,"CBM", "A2386-SX Bridgeboard"},
    { 513,  1,  "CBM", "Bridgeboard"},
    { 1030, 0,  "CBM", "A2410 Hires Graphics card"},
    { 1053, 1,  "Ameristar", "Ethernet card"},

    { NULL }
    };

UBYTE *memsize_s[8]
  ={"8meg","64K","128K","256K","512K","1meg","2meg","4meg"};

/* For Zorro III */
UBYTE *xmemsize_s[8]
  ={"16meg","32meg","64meg","128meg","256meg","512meg","1gig","RESERVED"};
UBYTE *submemsize_s[16]
  ={"same","autosized","64K", "128K", "256K", "512K", "1meg",    "2meg",
    "4meg","6meg",     "8meg","10meg","12meg","14meg","RESERVED","RESERVED"};

#define denise_id ((custom.deniseid)&0x00FF)
#define agnus_id  (((custom.vposr)&0x7F00)>>8)

#define MAXAGNUSIDS 10
UWORD agnusids[] =
{
    0x00, 0x10,
    0x20, 0x30,
    0x21, 0x31,		/* AA Rev 1 */
    0x22, 0x32,		/* AA Rev 2 */
    0x23, 0x33		/* AA Rev 3 */
};

UBYTE *agnus_s[] =
{
    "Normal PAL Agnus","Normal NTSC Agnus",
    "ECS PAL Agnus","ECS NTSC Agnus",
    "ECS PAL Agnus","ECS NTSC Agnus",
    "AA PAL Alice", "AA NTSC Alice",
    "AA PAL Alice", "AA NTSC Alice",
    "AA PAL Alice", "AA NTSC Alice",
    "UNKNOWN Alice", "UNKNOWN Alice"
};

/* F8 is actually AA Lisa */
#define MAXDENISEIDS 4
UWORD deniseids[] =
{
    0x00, 0xFC,
    0xEC, 0xF8
};
UBYTE *denise_s[] =
{
    "Normal Denise", "ECS Denise",
    "ECS Denise", "AA Lisa",
     "UNKNOWN"
};

UBYTE *line =
 "=======================================================================\n";

struct Library   *ExpansionBase = NULL;

static void showboards(void);
static void showchips(void);
static void showmem(void);
static void showos(void);
ULONG GetCPUType(void);
ULONG GetFPUType(void);
ULONG GetMMUType(void);

BOOL deb=FALSE, kickit=FALSE;
ULONG romstart;


/*****************************************************************************/


#define TEMPLATE  "DEBUG/S" VERSTAG
#define OPT_DEBUG 0
#define OPT_COUNT 1


/*****************************************************************************/


long main(void)
{
BPTR              output = NULL;
BPTR              old;
struct WBStartup *WBenchMsg = NULL;
struct Process   *process;
LONG              failureLevel = RETURN_FAIL;
LONG              opts[OPT_COUNT];
struct RDArgs    *rdargs;

    geta4();

    SysBase = (*((struct ExecBase **) 4));

    process = (struct Process *)FindTask(NULL);
    if (!process->pr_CLI)
    {
        WaitPort(&process->pr_MsgPort);
        WBenchMsg = (struct WBStartup *)GetMsg(&process->pr_MsgPort);
    }

    /* Cheap test for where rom is, kickit'd machine */
    romstart = ((ULONG)(SysBase->LibNode.lib_IdString)) & 0xFFFF0000;
    kickit = (romstart == 0x00200000) ? TRUE : FALSE;

    if (DOSBase = OpenLibrary("dos.library",37))
    {
        if (ExpansionBase = OpenLibrary("expansion.library",37))
        {
            if (WBenchMsg)
            {
                if (output = Open("CON:0/0//180/ShowConfig/WAIT/CLOSE",MODE_NEWFILE))
                {
                    old = SelectOutput(output);
                    failureLevel = RETURN_OK;
                }
            }
            else
            {
                memset(opts,0,sizeof(opts));
                if (rdargs = ReadArgs(TEMPLATE,opts,NULL))
                {
                    deb = opts[OPT_DEBUG] != 0;
                    FreeArgs(rdargs);
                    failureLevel = RETURN_OK;
                }
                else
                {
                    PrintFault(IoErr(),NULL);
                }
            }

            if (failureLevel == RETURN_OK)
            {
                showchips();
                showos();
                showmem();
                showboards();
            }

            CloseLibrary(ExpansionBase);
        }
        CloseLibrary(DOSBase);
    }

    if (WBenchMsg)
    {
        if (output)
        {
            SelectOutput(old);
            Close(output);
        }

        Forbid();
        ReplyMsg(WBenchMsg);
    }

    return(failureLevel);
}


/*****************************************************************************/


static void showmem(void)
    {
    struct MemHeader *mem, *firstmem;
    ULONG  nods[32], mems[32], ends[32], atts[32];
    ULONG  membytes, rmembytes, memhmeg, memmeg, memk;
    ULONG  memCnt = 0;
    UBYTE  *s, *about;
    int    k;

    Forbid();
    firstmem = (struct MemHeader *)SysBase->MemList.lh_Head;

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

    Printf("RAM:");
    for(k=0; k<memCnt; k++)
	{
        if(atts[k] & MEMF_CHIP) s = "CHIP";
	else if(atts[k] & MEMF_FAST) s = "FAST";
	else s = "????";
	Printf("\tNode type $%lx, Attributes $%lx (%s), at $%lx-$%lx ",
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
		Printf("%s%ld.%ld meg)\n",about, memmeg, memhmeg * 5);
		}
	else if(memk = (membytes / 0x400))	Printf("(%ld K)\n",memk);
	else Printf("(%ld bytes)\n",membytes);
	}
    }


/*****************************************************************************/


static void
showchips(void )
{
    ULONG           cpu, fpu = 0L, mmu = 0L;
    int             i, di, ai;
    UWORD           a, d;
    struct GfxBase *gfx;

    /* motherboard chip id's */
    Printf("PROCESSOR:\t");

    cpu = GetCPUType();
    if (cpu > 68010) {
	mmu = GetMMUType();
	fpu = GetFPUType();
    }
    Printf("CPU %ld", cpu);

    if (fpu)
	Printf("/%ldfpu", fpu);
    if (mmu)
	Printf("/%ldmmu", mmu);

    if (gfx = (struct GfxBase *) OpenLibrary("graphics.library", 0L))
    {
	/*********/
	/* AGNUS */
	/*********/
	a = agnus_id;
	for (ai = 0; ai < (MAXAGNUSIDS); ai++) {
	    if (agnusids[ai] == a)
		break;
	}
	if (a == 0) {		/* May be old dip-style agnus with no ID */
	    if (gfx->DisplayFlags & NTSC)
		ai = 1;
	}

	/**********/
	/* DENISE */
	/**********/
	d = denise_id;
	for (i = 0; i < 16; i++) {
	    if (d != denise_id) {
		d = 0;
		break;
	    }
	}
	if (d == 0xFF)
	    d = 0;

	for (di = 0; di < (MAXDENISEIDS); di++) {
	    if (deniseids[di] == d)
		break;
	}

	/***************/
	/* Show it all */
	/***************/
	Printf("\nCUSTOM CHIPS:\t");
	Printf("%s (id=$%04lx), %s (id=$%04lx)\n",
	       agnus_s[ai], agnus_id, denise_s[di], denise_id);

	CloseLibrary((struct Library *) gfx);
    }
}


/*****************************************************************************/


static void showos(void)
    {
    struct Library *lib;

    Printf("VERS:\tKickstart version %ld.%ld",SysBase->LibNode.lib_Version,SysBase->SoftVer);

	Printf(", Exec version %ld.%ld, ",
		SysBase->LibNode.lib_Version,SysBase->LibNode.lib_Revision);

    if(lib = OpenLibrary("version.library",0L))
	{
	Printf("Disk version %ld.%ld",
		lib->lib_Version,lib->lib_Revision);
	CloseLibrary(lib);
	}
    Printf("\n");
    }


/*****************************************************************************/


static void showboards(void)
    {
    struct ConfigDev *myCD;
    ULONG addr,diagcopy,bdcnt=0L;
    UWORD m,t,f,k,isub;
    UBYTE p,msbits,btype;
    BOOL ismem,hasrom,pref8,noshut,chained,xsized;

    /*--------------------------------------------------*/
    /* FindConfigDev(oldConfigDev,manufacturer,product) */
    /* oldConfigDev = NULL for the top of the list      */
    /* manufacturer = -1 for any manufacturer           */
    /* product      = -1 for any product                */
    /*--------------------------------------------------*/

    Printf("BOARDS:\n");


    if(kickit)
	{
	if(deb) Printf(line);
        Printf(" Initial RAM-containing board configured by KickIt (?)\n");
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
	xsized = (f & ERFF_EXTENDED);	/* big Zorro III board */

	isub   = (f &  ERT_Z3_SSMASK);

	addr=(ULONG)myCD->cd_BoardAddr;
	diagcopy = *(ULONG *)&myCD->cd_Rom.er_Reserved0c;


	if(deb) Printf(line);

	for(k=0; boards[k].manu; k++)
            {
            if((boards[k].manu == m)&&(boards[k].prod == p))
                {
                Printf(" %s %s",boards[k].manu_s,boards[k].prod_s);
                break;
                }
            }
	if(boards[k].manu == 0)
	    {
 	    if(ismem) Printf(" RAM (unidentified)");
            else if(hasrom) Printf(" Board + ROM (HD?) (unidentified)");
	    else Printf(" Board (unidentified)");
	    }


	if(btype == 2)	/* Zorro III */
	    {

	    Printf(":   Prod=%ld/%ld($%lx/$%lx)\n     (@$%lx, size %s, subsize %s%s\n",m,p,m,p,addr,
                      	xsized ? xmemsize_s[msbits] : memsize_s[msbits],
		      	submemsize_s[isub],
			ismem ? " Mem)" : ")");
	    }
	else
	    {
	    Printf(":   Prod=%ld/%ld($%lx/$%lx) (@$%lx %s%s\n",m,p,m,p,addr,
	    			memsize_s[msbits], ismem ? " Mem)" : ")");
	    }


	if(deb)
	    {
            Printf(" ConfigDev structure found at location $%lx\n",myCD);
            Printf("==== Board ID (ExpansionRom) information:\n");
            Printf("er_Manufacturer         =%ld=$%lx=(~$%4lx)\n",m,m,(UWORD)~m);
            Printf("er_Product              =%ld=$%lx=(~$%2lx)\n",p,p,(UBYTE)~p);

            Printf("er_Type                 =$%lx\n",t);
	    if(btype == 2)	/* Zorro III */
		{
            	Printf("  (type %ld (Zorro III), size %s, subsize %s)\n",
                      	btype,
                      	xsized ? xmemsize_s[msbits] : memsize_s[msbits],
		      	submemsize_s[isub]);
		Printf("  (%s, %s, %s)\n",
                      	ismem  ? "add to free list" : "not for free list",
                      	hasrom ? "ROM diag vec valid" : "no ROM diag vec",
                      	chained ? "chained to next" : "not chained");
		}
	    else
		{
            	Printf("  (type %ld, size %s, %s, %s, %s)\n",
                      btype,
                      memsize_s[msbits],
                      ismem  ? "add to free list" : "not for free list",
                      hasrom ? "ROM diag vec valid" : "no ROM diag vec",
                      chained ? "chained to next" : "not chained");
		}

            Printf("er_Flags                =$%lx\n",f);


            Printf("  (%s, %s)\n",
                      pref8  ? "prefers 8-meg space" : "no space preference",
                      noshut ? "can not be shut up" : "can be shut up");


            Printf("er_InitDiagVec          =$%lx\n",myCD->cd_Rom.er_InitDiagVec);
            if(diagcopy) Printf("DiagCopy at             =$%lx\n",diagcopy);
            /* These values are generated when the AUTOCONFIG(tm) software
             * relocates the board
             */
            Printf("==== Configuration (ConfigDev) information:\n");
            Printf("cd_BoardAddr            =$%lx\n",addr);
            Printf("cd_BoardSize            =$%lx (%ldK)\n",
               myCD->cd_BoardSize,((ULONG)myCD->cd_BoardSize)/1024);

            Printf("cd_Flags                =$%lx",myCD->cd_Flags);
            Printf("  (CONFIGME bit %s)\n",
                     myCD->cd_Flags & CDF_CONFIGME ? "still set" : "cleared");
            }

	}
    if(bdcnt==0) Printf(" None\n");
    else if(deb) Printf(line);
    }

