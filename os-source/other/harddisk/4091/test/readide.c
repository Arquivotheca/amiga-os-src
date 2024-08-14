#include <stdio.h>

#include <exec/interrupts.h>
#include <hardware/intbits.h>
#include <dos/dosextens.h>
#include <dos/rdargs.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>



#define TEMPLATE \
	"file/A,cyl/N,head/N,sector/N,number/N,Slave/S,Fast/S,Word/S,Swap/S" 

#define OPT_FILE	0
#define OPT_CYL		1
#define OPT_HEAD	2
#define OPT_SECTOR	3
#define OPT_NUMBER	4
#define OPT_SLAVE	5
#define OPT_FAST	6
#define OPT_WORD	7
#define OPT_SWAP	8
#define OPT_COUNT	9

// offsets from IDE base address (0xda0000 or 0xda2000)

#ifndef OFF_BY_1
#define Data		0
#define Error		4
#define SectorCnt	8
#define SectorNum	0x0c
#define CylLow		0x10
#define CylHigh		0x14
#define DriveHead	0x18
#define Status		0x1c
#define Command		0x1c
#define AltStatus	0x1018
#define DigitalOut	0x1018
#define DriveAddress	0x101c

#else

#define Data		0
#define Error		5
#define SectorCnt	9
#define SectorNum	0x0d
#define CylLow		0x11
#define CylHigh		0x15
#define DriveHead	0x19
#define Status		0x1d
#define Command		0x1d
#define AltStatus	0x1019
#define DigitalOut	0x1019
#define DriveAddress	0x101d
#endif OFF_BY_1

// ide commands
#define READ	0x20
#define WRITE	0x30
#define SEEK	0x70
#define IDENTIFY 0xEC
#define DIAG	0x90

// Status bits
#define IDEF_ERR	(1)
#define IDEF_DRQ	(1<<3)
#define IDEF_RDY	(1<<6)
#define IDEF_BSY	(1<<7)

/*
#define SET(offset,value)	(word ? \
				 (*((UWORD *) &(ide[(offset)])) = (swap ? \
					(value) : (value) << 8)) : \
				 (ide[(offset)] = (value)))

#define GET(offset)		(word ? \
				 (swap ? *((UWORD *) &(ide[(offset)])) : \
				 	 *((UWORD *) &(ide[(offset)])) >> 8) :\
				 ide[(offset)])
*/
#define SET(offset,value)	(ide[offset] = (value))
#define GET(offset)		ide[offset]

LONG __saveds __asm __interrupt ide_int_server (register __a1 UBYTE *ide);
int WaitNotBusy (UBYTE *ide, ULONG max);

struct Interrupt MyInt = {
	NULL, NULL,
	-120,
	NT_INTERRUPT,
	"ide.device",
	NULL,		/* fill in with value of ide */
	(VOID (*)()) ide_int_server,
};

UWORD buffer[256];
struct Process *myproc;

extern struct DosLibrary *DOSBase;
extern struct Library *SysBase;

int swap = FALSE;
int word = FALSE;
UBYTE * volatile ide = (UBYTE *) 0xda0000;

int
main (argc,argv)
	int argc;
	char **argv;
{
	LONG opts[OPT_COUNT];
	struct RDargs *rdargs;
	char *file;
	LONG rc = 20;
	LONG cyl = 0, head = 0, sector = 0, number = 1;
	LONG slave = FALSE;
	LONG fast = FALSE;
	int i;
	BPTR fh;
	UBYTE status,error;

      myproc = (void *) FindTask(0);

      memset((char *)opts, 0, sizeof(opts));
      if (DOSBase->dl_lib.lib_Version < 36)
      {
		rdargs = (void *) 1;
		if (argc > 1)
			file = argv[1];

		if (argc > 2)
			cyl = atoi(argv[2]);
		if (argc > 3)
			head = atoi(argv[3]);
		if (argc > 4)
			sector = atoi(argv[4]);
		if (argc > 5)
			number = atoi(argv[5]);
      } else {
		rdargs = ReadArgs(TEMPLATE, opts, NULL);
		if (!rdargs)
			PrintFault(IoErr(), NULL);
		else {
			file = (char *) opts[OPT_FILE];
			if (opts[OPT_CYL])
				cyl = *((LONG *)opts[OPT_CYL]);
			if (opts[OPT_HEAD])
				head = *((LONG *)opts[OPT_HEAD]);
			if (opts[OPT_SECTOR])
				sector = *((LONG *)opts[OPT_SECTOR]);
			if (opts[OPT_NUMBER])
				number = *((LONG *)opts[OPT_NUMBER]);
			slave = opts[OPT_SLAVE];
			fast  = opts[OPT_FAST];
			word  = opts[OPT_WORD];
			swap  = opts[OPT_SWAP];

			if (fast)
				ide = (UBYTE *) 0xda2000;
		}
      }

      if (rdargs)
      {
	if (fh = Open(file,MODE_NEWFILE))
	{
		MyInt.is_Data = ide;
		AddIntServer(INTB_EXTER,&MyInt);

		// stupid ide interface generates INTB_VERT!!
 printf("Alt Status = 0x%lx\n",GET(AltStatus));

#ifdef NEVER
		// reset the IDE drive
		SET(DigitalOut,0x04);		// reset
		Delay(1);			// wait a while (too long)
		SET(DigitalOut,0x00);		// un-reset
		if (WaitNotBusy(ide,1000))
		{
			printf("Reset timeout!\n");
		}
		if ((error = GET(Error)) != 0x01)
		{
			printf("Drive failed to reset properly! (0x%lx)\n",
				error);
		}

		// self-diagnostics
		SET(Command,DIAG);
		if (WaitNotBusy(ide,30000))
			printf("Diag timeout\n");
		error = GET(error);
		if (error & 0x80)
		{
			printf("Slave failed\n");
		}
		if (error & 0x7f)
		{
			printf("Diag error: 0x%lx\n",error);
		}
#endif

		(void) GET(SectorCnt);
		(void) GET(SectorNum);
		(void) GET(CylLow);
		(void) GET(CylHigh);
		(void) GET(DriveHead);

		SET(DigitalOut,0x02);		// turn off interrupts

		// set up all the other regs
		SET(SectorCnt,number);
		SET(SectorNum,sector);
		SET(CylLow,cyl & 0xff);
		SET(CylHigh,cyl >> 8);
		SET(DriveHead,head | 0xa0 | (slave ? 0x10 : 0x00));
					  // a0 needed for write??

 printf("SectorCnt = %ld\n",GET(SectorCnt));
 printf("SectorNum = %ld\n",GET(SectorNum));
 printf("Cyl = %ld\n",(GET(CylHigh) << 8) | GET(CylLow));
 printf("drive/head = 0x%lx\n",GET(DriveHead));

 printf("Reading Cyl %ld, Head %ld, Sector %ld...\n",cyl,head,sector);
 Delay(250);
		// ok, here goes
		SET(Command,READ);

		// suck in 256*16 bits * number of sectors, write to file
		while (number--)
		{
			// wait for interrupt (drq = 1 when we're ready)
			while (!((status = GET(Status)) & IDEF_DRQ))
{ printf("waiting...(0x%lx)\n",status);
  Delay(25);
				;
}
 printf("Got DRQ!\n");
			if (status & IDEF_ERR)
			{
				printf("Error: status 0x%lx, error 0x%lx\n",
					status,GET(Error));
				break;
			}
			for (i = 0; i < 256; i++)
			{
				// Read as UWORD...
				buffer[i] = *((UWORD *) &(ide[Data]));
			}
			Write(fh,(void *) buffer, 512);
 printf("read sector!\n");
		}

		RemIntServer(INTB_EXTER,&MyInt);

		Close(fh);
		rc = 0;
	} else {
		printf("can't open %s\n",file);
		rc = 10;
	}

	if (rdargs != (void *) 1)
		FreeArgs(rdargs);

      }

      return rc;
}


LONG __saveds __asm __interrupt
ide_int_server (register __a1 UBYTE *ide)
{
	UBYTE status;

	// stupid hardware doesn't tell me if an int occurred!
	status = ide[Status];

	// if (int occurred) read status and Signal myproc;
	return 0;
}

// Wait for busy bit to be clear

int
WaitNotBusy (UBYTE *ide, ULONG max)
{
	UBYTE status;

	while ((status = GET(Status)) & IDEF_BSY)
{ if (max-- == 0)
  {
    printf("busy timed out! (0x%lx)\n",status);
    return 1;
  }
				;
}
 printf("Not Busy any more\n");
 return 0;
}
