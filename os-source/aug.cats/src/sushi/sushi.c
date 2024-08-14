
/* sushi.c
 * Raw serial debugging output catcher
 * C. Scheppner 07/92
 *
 * 37.10 - fix EMPTY option to send CTRL-E, not CTRL-F
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <workbench/startup.h>
#include <devices/timer.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/alib_protos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern struct Library *SysBase;
extern struct Library *DOSBase;

#ifdef LATTICE
int  CXBRK(void) { return(0); }  /* Disable Lattice CTRL/C handling */
void chkabort(void) { return; }  /* really */

#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
#endif


#define MYDEBUG 0
#define KDEBUG 0

char *vers = "\0$VER: sushi 37.10";
char *Copyright = 
  "sushi v37.10\n(c) Copyright 1992-93 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = 
 "Sushi - intercepts raw serial debugging output on your own machine. Opts:\n" 
 "startup: [ON] [BUFK=n (pow 2 only)] [NOPROMPT] [QUIET] [ASKEXIT] [ASKSAVE]\n"
 "runtime: [OFF] [SAVE] [SAVEAS filename] [EMPTY]\n";
 
char *wtpname = "sushi_CAS_port";

BOOL UseMyWbCon = FALSE;
UBYTE  *MyWbCon = "";

void kprintf (UBYTE *fmt,...);

/* for testing only */
UBYTE enfname[] = "_The Enforcer_";
UBYTE defname[] = "t:sushi.out";
UBYTE wrapstr[] = "BUFFER WRAPPED - This is the most recent captured data\n\n";
 
#if KDEBUG
#define bug kprintf
#elif DDEBUG
#define bug dprintf
#else	/* else changes all bug's to printf's */
#define bug printf
#endif

#if MYDEBUG
#define DEBTIME 0
#define D(x) (x); if(DEBTIME>0) Delay(DEBTIME)
#else
#define D(x) ;
#endif


typedef LONG far (*PFL)();


struct AppStruct {
   UBYTE	*subuf;
   ULONG	subufsize;
   ULONG	subufi;
   ULONG	subufli;
   ULONG	wrapcnt;
   ULONG	wrapmask;	

   struct Task	*extsigtask;
   LONG		extsignum;
   ULONG	extsignal;

   ULONG	sumicros;

   struct Task	*sutask;
   LONG		susignum;
   ULONG	susignal;

   ULONG	temp1;
   ULONG	temp2;
   ULONG	temp3;
   };



struct WTP {
   struct MsgPort wtPort;
   struct AppStruct *appstruct;
   ULONG	*Wedge;
   ULONG	WedgeSize;
   PFL		Grab_Em;
   PFL		Free_Em;
   ULONG	*ptrToUseCount;
   char		Name[48];
   };

struct WTP MyWTP = { 0 };

/* don't copy wedge to allocated memory */
#define COPYIT 0

struct WTP *wtp;

/* in the assembler part */
extern ULONG far startpatch, endpatch, usecount;
extern ULONG far grab_em_offs, free_em_offs, usecount_offs, appstruct_offs;


LONG  (*GrabFunc)(void);
LONG  (*FreeFunc)(void);

PFL	GrabFunc;
PFL	FreeFunc;


BOOL  FromWb, NoPrompt, Quiet, AskSave, TimerOn, EmptyBuf, Save, AskExit;
ULONG wSize;
UBYTE *toggle;


#define MICROS 100000
struct timerequest *tiReq;
struct MsgPort  *tiPort;
char   *tiPortName = "sushi_CAS_TimerPort";
BOOL   TiOpen = FALSE;

/* our protos */
BOOL takeout(struct WTP *wtp, UBYTE *name);
VOID sendtimereq(struct timerequest *tio, ULONG micros);
void closetimer(void);
void bye(struct WTP *wtp,UBYTE *s, int err);
void cleanup(struct WTP *wtp);
BOOL savesushi(struct AppStruct *aps, UBYTE *filename);

/* must be 32K plus at least one extra byte to work with current asm part
 */

#define DEFBUFSIZE	(1024*32)
ULONG   bufsize = DEFBUFSIZE;
ULONG	bufextra = 8;


void main(int argc, char **argv)
    {
#if MYDEBUG
    struct MsgPort *test;
#endif
#if COPYIT
    ULONG  patchsize;
#endif
    struct AppStruct *aps;
    ULONG  *reloc, asig, tsig, waitmask, signals, result, utemp, umask;
    LONG asignal;
    UBYTE *buf, *name;
    UWORD bufi, bufli;
    BOOL TakeOut = FALSE, Done = FALSE, Saved;
    int k,j;
    UBYTE ans[32], filename[80];

    FromWb = (argc==0) ? TRUE : FALSE;

    name = "sushi";
    strcpy(filename,defname);

    if((argc == 2)&&(*argv[1]=='?'))
	{
	if(!FromWb)  printf("%s\n%s\n",Copyright,usage);
      	bye(NULL,"",RETURN_OK);
      	}

     toggle = "ON";


     /* All BOOLS here are init'd to 0 (FALSE) above main */
     for(k=1; k < argc; k++)
     	{
	D(bug("argv[%ld] is %s\n",k,argv[k]));

        if     (!(stricmp(argv[k],"OFF")))       toggle     = "OFF";
        else if(!(stricmp(argv[k],"ON")))  	 toggle     = "ON";
        else if(!(stricmp(argv[k],"NOPROMPT")))  NoPrompt   = TRUE;
        else if(!(stricmp(argv[k],"QUIET")))     Quiet      = TRUE;
        else if(!(stricmp(argv[k],"ASKSAVE")))   AskSave    = TRUE;
        else if(!(stricmp(argv[k],"ASKEXIT")))   AskExit    = TRUE;
        else if(!(stricmp(argv[k],"TIMERON")))   TimerOn    = TRUE;
        else if(!(stricmp(argv[k],"EMPTY")))	 EmptyBuf   = TRUE;
        else if(!(stricmp(argv[k],"SAVE")))	 Save 	    = TRUE;
        else if(!(stricmp(argv[k],"SAVEAS")))
	    {
   	    Save    = TRUE;
	    k++;
	    strcpy(filename,argv[k]);
	    }
        else if(!(strnicmp(argv[k],"BUFK=",5)))
	    {
	    utemp = atoi(&argv[k][5]);
	    if(utemp < 4)
		{
		printf("BUFK %ldK too small - setting to 4K\n",utemp);
		utemp = 4;
		}
	    if(utemp > 2048)
		{
		printf("BUFK %ldK too big (2048K is max) - setting to 512K\n",utemp);
		utemp = 512;
		}
	    utemp = utemp * 1024;
	    for(j = 31; j> 0; j--)
		{
		umask = 1L << j;
		if(utemp & umask)
		    {
		    utemp &= umask;
		    break;
		    }
		}
	    bufsize = utemp;
	    D(bug("would use buffer size $%lx (%ld), mask $%08lx\n",
				bufsize, bufsize, bufsize-1));
	    }
	else printf("%s: unknown option \"%s\"\n",name,argv[k]);
        }

    D(bug("NoPrompt=%ld  Quiet=%ld\n",NoPrompt, Quiet));

    Forbid();
    if(wtp = (struct WTP *)FindPort(wtpname))
      	{
	if(FromWb)	TakeOut = TRUE;
	else if(!(stricmp(toggle,"OFF")))   TakeOut = TRUE;
	}

    if((!wtp) && (TakeOut || Save || EmptyBuf))
	{
	Permit();
	if(!FromWb)	printf("%s not currently installed\n",argv[0]);
 	exit(RETURN_WARN);
	}


    if((wtp) && (Save || EmptyBuf || TakeOut))
	{
	if(Save)	savesushi(wtp->appstruct, filename);
	if(EmptyBuf)	Signal(wtp->appstruct->sutask,SIGBREAKF_CTRL_E);
	if(TakeOut)
	    {
	    Signal(wtp->appstruct->sutask,SIGBREAKF_CTRL_C);
	    if(!FromWb)	printf("Signalling %s to exit\n",argv[0]);
	    }
	Permit();
	exit(0L);
	}

    if((wtp) && (!TakeOut))
	{
	Permit();
	if(!FromWb)	printf("%s already installed\n",argv[0]);
 	exit(RETURN_WARN);
	}

    Permit();

    /* TakeOut is FALSE and we are not currently installed
     * So we'll install ourself
     */
#if MYDEBUG
    if(test = FindPort(enfname))
	{
	D(bug("Enforcer port at $%lx, name at $%lx, code at $%lx ?\n",
		test, test->mp_Node.ln_Name, test->mp_Node.ln_Name + 0x147E));
	}
    else D(bug("Enforcer port not found\n"));
#endif

    /* Open timer device */   
    if(!(tiPort = (struct MsgPort *)CreatePort(tiPortName,0)))
        bye(NULL,"Can't open ti port\n",10);
    if(!(tiReq = (struct timerequest *)CreateStdIO(tiPort)))
        bye(NULL,"Can't alloc ti request\n",10);
    if(OpenDevice("timer.device",UNIT_VBLANK,tiReq,0))
        bye(NULL,"Can't open timer device\n",10);

    sendtimereq(tiReq, MICROS);
    tsig = 1L << tiPort->mp_SigBit;
    TiOpen = TRUE;

    /* loaded location of assembler code */
    reloc = &startpatch;
    wSize = 0;

#if COPYIT

    if(!(reloc = (ULONG *)AllocMem(wSize,MEMF_PUBLIC|MEMF_CLEAR)))
    	{
    	bye(NULL,"Not enough memory",RETURN_FAIL);
        }
#endif

    if(!(buf = (UBYTE *)AllocMem(bufsize + bufextra,MEMF_PUBLIC|MEMF_CLEAR)))
        {
#if COPYIT
	FreeMem(reloc,wSize);
#endif
        bye(NULL,"Not enough memory",RETURN_FAIL);
        }
    
    D(bug("Loaded code at $%lx, Reloc at $%lx\n",&startpatch,reloc));

#if COPYIT
    patchsize = (ULONG)&endpatch - (ULONG)&startpatch;
    wSize = patchsize+sizeof(struct WTP)+16;

    /* Copy relocatable assembler code to alloc'd memory */
    CopyMem(&startpatch,reloc,patchsize);
    if(SysBase->lib_Version >= 37)	CacheClearU();
    D(bug("After CopyMem\n"));

    /* Then wtPort... Set up port */
    wtp = (struct WTP *)((ULONG)reloc + (ULONG)patchsize);
#else
    wtp = &MyWTP;
#endif

    wtp->ptrToUseCount = (ULONG *)((ULONG)reloc + usecount_offs);
    wtp->Wedge = reloc;
    wtp->WedgeSize = wSize;
    strcpy(wtp->Name,wtpname);
    wtp->wtPort.mp_Node.ln_Name = wtp->Name;
    wtp->wtPort.mp_Node.ln_Type = NT_MSGPORT;

    aps  		= (struct AppStruct *)((ULONG)reloc + appstruct_offs);
    wtp->appstruct  	= aps;

    aps->subuf    = buf;
    aps->subufsize = bufsize;
    aps->subufi   = 0;
    aps->subufli  = 0;
    aps->susignum = -1;
    aps->wrapcnt  = 0;
    aps->wrapmask = bufsize - 1;
    aps->sumicros = MICROS;

    D(bug("Ready to FindPort\n"));

    Forbid();

    /* If we find another wtPort at this point, Free ours and abort */
    if(FindPort(wtpname))
        {
        Permit();
        bye(wtp,"Aborted... Another wt installation occurred",
                         RETURN_WARN);
        }

    /* else set up our port and SetFunction to our code */
    wtp->Grab_Em=(PFL)((ULONG)reloc + grab_em_offs);
    wtp->Free_Em=(PFL)((ULONG)reloc + free_em_offs);


    aps->sutask = FindTask(NULL);

    D(bug("Allocating signal\n"));

    if((asignal = AllocSignal(-1)) > 0)
	{
	aps->susignum = asignal;
	aps->susignal = 1L << asignal;
	}
    else
	{
	aps->susignum = SIGBREAKB_CTRL_D;
	aps->susignal = SIGBREAKF_CTRL_D;
	}

    asig = aps->susignal;

    GrabFunc = wtp->Grab_Em;
    FreeFunc = wtp->Free_Em;


    D(bug("reloc=$%lx  grab_em_offs=$%lx  &startpatch=$%lx  GrabFunc=$%lx\n",
			reloc, grab_em_offs, &startpatch, GrabFunc));

    D(bug("reloc=$%lx  free_em_offs=$%lx  &startpatch=$%lx  FreeFunc=$%lx\n",
			reloc, free_em_offs, &startpatch, FreeFunc));

    D(bug("Ready to call GrabFunc at $%lx\n",GrabFunc));
    if(result = (*GrabFunc)())
	{
        AddPort(wtp);		
	if((!FromWb)&&(!(NoPrompt || Quiet)))
	    {
	    printf("%s installed (CTRL-C or BREAK to remove)\n",name);
	    if((!(result & 0x10))&&(!(FindTask("« Enforcer »"))))
		{
		printf("Ignore this if you are running Enforcer RAWIO 37.x or higher\n");
		printf("Enforcer 2.8b (or 2.6f megastack) could not be found or patched.\n");
		printf("If you start 2.8b or 2.6f now, stop and restart %s to patch Enforcer.\n",name);
		}
	    }

	if((!(result & 0x10))&&(!TimerOn)) /* No need for timer if no Enforcer */
	    {
	    closetimer();
	    tsig = 0L;
	    }

	D(bug("temp1=$%lx  temp2=$%lx  temp3=$%lx aps->sutask=$%lx\n",
		aps->temp1, aps->temp2, aps->temp3, aps->sutask));
	}
    else
	{
	Permit();
	bye(wtp,"Installation failed\n",RETURN_WARN);		    
	}

    Permit();


    waitmask =
	(asig | tsig | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F );

    while(!Done)
	{
	signals = Wait(asig | tsig | 
		SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_E | SIGBREAKF_CTRL_F );

	if( signals & SIGBREAKF_CTRL_F)	/* file save */
	    {
	    if(savesushi(aps,filename))
		printf("%s buffer saved as %s\n",name,filename);
	    }

	if( signals & SIGBREAKF_CTRL_E)	/* empty the buffer */
	    {
	    Forbid();
	    aps->subufi  = 0;
	    aps->subufli = 0;
	    aps->wrapcnt = 0;
	    Permit();
	    printf("%s buffer cleared\n",name);
	    }

	if(signals & SIGBREAKF_CTRL_C)
	    {
	    if(aps->extsigtask)
		{
		printf("%s can not exit - external task $%lx still using %s.\n",
			name, aps->extsigtask, name);
		}
	    else
		{
		strcpy(ans,"y");
		if(AskExit)
		    {
		    printf("\nsushi: CTRL-C received - really exit (y or n) ? ");
		    gets(ans);
		    }
		if(!(stricmp(ans,"y")))
		    {
	    	    Forbid();
	    	    if(takeout(wtp,name))
		    	{
		    	Done = TRUE;
		    	}
	    	    Permit();
		    }
		}
   	    }

	if(signals & tsig)
	    {
	    GetMsg(tiPort);
	    sendtimereq(tiReq, aps->sumicros);
	    }

	if((!Done)&&(signals & ( asig | tsig )))
	    {
#if MYDEBUG
	    D(bug("Received signal from asm, i=%ld li=%ld\n",
				aps->subufi, aps->subufli));
#endif
	    bufi  = aps->subufi;
	    bufli = aps->subufli;

	    if((bufi > bufli)&&(!Quiet))		/* not wrapped */
		{
		D(bug("i=%ld li=%ld",bufi,bufli));
		Write(Output(),&buf[bufli],bufi - bufli);
		}
	    else if((bufli > bufi)&&(!Quiet))	/* wrapped */		
		{
		aps->wrapcnt++;
		D(bug("WRAP: wrapcount=%ld i=%ld li=%ld",aps->wrapcnt,bufi,bufli));
		Write(Output(),&buf[bufli],aps->subufsize - bufli); /* end of buffer */
		Write(Output(),buf,bufi);			    /* start of buffer */
		}

	    aps->subufli = bufi;

	    /* Signal external task if there is one */
	    if((bufi != bufli)&&(aps->extsigtask))
		{
		Signal(aps->extsigtask, aps->extsignal);
		}
	    }
	}

    /* saving buffer to file */
    Saved = FALSE;
    filename[0] = 0xFF;
    bufi  = aps->subufi;
    bufli = aps->subufli;
    if(bufi < bufli)	aps->wrapcnt++;
    if(AskSave && (bufi || aps->wrapcnt))
	{
	while(!Saved)
	    {
	    fprintf(stderr, "Filename for saving buffer, or <RET>: ");
	    gets(filename);
	    if(*filename)
	    	{
		if(filename[0] == 0xFF)
		    {
		    printf("\nNo stdin? Saving buffer as %s\n",defname);
		    strcpy(filename,defname);
		    }
		Saved = savesushi(aps,filename);
		}
	    else Saved = TRUE;
	    }
	printf("Done\n");
	}
    bye(wtp,"",RETURN_OK);
    }


BOOL savesushi(struct AppStruct *aps, UBYTE *filename)
    {
    LONG file;
    UBYTE *buf = aps->subuf;
    ULONG bufi = aps->subufi;
    ULONG bufsize = aps->subufsize;
    BOOL  result;

    if(file = Open(filename,MODE_NEWFILE))
        {
	if(aps->wrapcnt)
	    {
	    Write(file,wrapstr,strlen(wrapstr));
	    Write(file,&buf[bufi],bufsize-bufi);
	    }
    	Write(file,buf,bufi);
	Close(file);
	result=TRUE;
	}
    else
 	{
	fprintf(stderr, "Error opening %s\n",filename);
	result=FALSE;
	}
    return(result);
    }

/* Must be called in a Forbid() */
BOOL takeout(struct WTP *wtp, UBYTE *name)
    {
    BOOL result = FALSE;
    
    if(*(wtp->ptrToUseCount))
        {
        printf("%s may not be removed or changed while in use\n",name);
	return(result);
	}

    /* Try to unwedge */
    FreeFunc = wtp->Free_Em;
    D(bug("Ready to call FreeFunc at $%lx\n",FreeFunc));
    if(!(*FreeFunc)())
	{
        /* Someone else has changed the vectors */
        /* We put theirs back - can't exit yet  */
        printf("Can't remove %s right now - another SetFunction present",name);
        }
    else
        {
	D(bug("FreeFunc succeeded\n"));
        RemPort(wtp);
        while(*(wtp->ptrToUseCount))  Delay(20);
	printf("%s removed\n",name);
	result=TRUE;
      	}
    return(result);
    }


void sendtimereq(struct timerequest *tio, ULONG micros)
    {
    tio->tr_node.io_Command = TR_ADDREQUEST;
    tio->tr_time.tv_secs  = 0;
    tio->tr_time.tv_micro = micros;
    SendIO(tiReq);
    }

void bye(struct WTP *wtp,UBYTE *s, int err)
    {
    D(bug("bye:\n"));
    if((*s)&&(!FromWb))
  	{
      	printf("%s\n",s);
      	}
    cleanup(wtp);
    exit(err);
    }


void closetimer()
    {
    D(bug("closing timer\n"));
    if(TiOpen)
	{
	WaitIO(tiReq);
	CloseDevice(tiReq);
	}
    if(tiReq)   DeleteStdIO((struct IOStdReq *)tiReq);
    if(tiPort)  DeletePort(tiPort);
    TiOpen = FALSE;
    tiReq  = NULL;
    tiPort = NULL;
    }

void cleanup(struct WTP *wtp)
    {
    D(bug("cleanup: wtp = $%lx\n",wtp));

    closetimer();

    if(wtp)
	{
	D(bug("cleanup: subuf at $%lx, size $%lx\n",
		wtp->appstruct->subuf, wtp->appstruct->subufsize));
        if(wtp->appstruct->subuf)
		FreeMem(wtp->appstruct->subuf, wtp->appstruct->subufsize + bufextra);
	if(wtp->appstruct->susignum != -1)
		FreeSignal(wtp->appstruct->susignum);
#if COPYIT
	D(bug("cleanup: wedge at $%lx, size $%lx\n",
		wtp->Wedge, wtp->WedgeSize));
        if(wtp->Wedge)
		FreeMem(wtp->Wedge, wtp->WedgeSize);
#endif
	}
    }




