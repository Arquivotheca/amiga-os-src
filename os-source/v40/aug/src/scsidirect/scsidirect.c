#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <ctype.h>

#define SAME 0

#include "scsi.h"
#include "scsidirect.h"

/* case-ignoring string compare */
#ifndef LATTICE
static int stricmp(const char *ss1, const char *ss2)
{
       unsigned char *s1=(unsigned char *)ss1, *s2=(unsigned char *)ss2;
       while (tolower(*s1) == tolower(*s2))
               if (!*s1++)
                       return 0;
               else
                       ++s2;
       return tolower(*s2) - tolower(*s1);
}
#endif

/* number parsing routines; return nonzero for invalid number */
static int decnum(const char *s, long *ip)
{
       char *p;
       *ip = strtol(s, &p, 0);
       return (int) *p;
}
static int hexnum(const char *s, long *ip)
{
       char *p;
       *ip = strtol(s, &p, 16);
       return (int) *p;
}

#ifndef AMIGA
typedef long LONG;
typedef unsigned short UWORD;
#define FALSE 0
#define TRUE 1
#else
#include <exec/types.h>
#endif

void dumpsense (int code);
void doreadcapacity(void);

/* so the compiler doesn't get upset */
void help(int *done);
void quit(int *done);
void source(int *done);
void showbuffer(int *done);
void writebuffer(int *done);
void setbuffer(int *done);
void testunitready(int *done);
void modeselect(int *done);
void modesense(int *done);
void modifypage(int *done);
void showpage(int *done);
void inquiry(int *done);
void readcapacity(int *done);
void rezerounit(int *done);
void startstopunit(int *done);
void verify(int *done);
void synchronizecache(int *done);
void prefetch(int *done);
void lockunlockcache(int *done);
void mediumremoval(int *done);
void read10(int *done);
void write10(int *done);
void read6(int *done);
void write6(int *done);
void writeverify(int *done);
void docommand(int *done);
void readdefectdata(int *done);
void reassignblocks(int *done);
void formatunit(int *done);
/* more commands can be _easily_ added */

struct commands {
	char *name;
	char *helpstr;
	void (*rtn)(int *done);
} comm[] = {
	"?"," - list all commands",help,
	"quit"," - exit this program",quit,
	"Source","<file> [echo] - read commands from file",source,
	"SetBuffer","[offset] [hex <hex digits>]|<string>",setbuffer,
	"ShowBuffer","[hex] [offset [length]] - show part of buffer",
		showbuffer,
	"WriteBuffer","<file> [hex] [offset [length]] - write buffer to file",
		writebuffer,
	"Command","<cmd len> <cmd bytes> <data offset> <data len> Read|Write",
		docommand,
	"FormatUnit","<interleave>\n"
"\t[pattern <modifier> <type> [<pattern>...]]\n"
"\t[defects <format> [<defects>...] [disable_primary] [disable_certify]\n"
"\t [stop_format] [disable_page_save] [immediate]]\n"
"\t[grown]",
		formatunit,
	"Inquiry","<page> <offset> <length> - get basic information",
		inquiry,
	"LockUnlockCache","<start block> <blocks> lock|unlock [relative]",
		lockunlockcache,
	"MediumRemoval","prevent|allow - prevent or allow medium removal",
		mediumremoval,
	"ModeSelect","<offset> <length> - select parameters",modeselect,
	"ModeSense",
		"<page> <type> <offset> <length> [noblock] - get parameters",
		modesense,
	"ModifyPage",
		"<page> <byte1> <value1> ... <byteN> <valueN> [save] [scsi2]",
		modifypage,
	"Prefetch","<start block> <blocks> [relative] [immed]",
		prefetch,
	"Read","<start block> <blocks> <offset>", read6,
	"Read10","<start block> <blocks> <offset> [relative] [DPO] [force]",
		read10,
	"ReadCapacity","<starting block> [relative] [next_delay]",
		readcapacity,
	"ReadDefectData","<offset> <len> <format> [primary] [grown]",
		readdefectdata,
	"ReassignBlocks","[block] ... - reassign a list of bad blocks",
		reassignblocks,
	"RezeroUnit"," - rezeros a unit (vender-specific results)",
		rezerounit,
	"ShowPage","<page> [<type:0-4>] - show a page of mode data",showpage,
	"StartStopUnit","start|stop [immed] [load_eject]n",startstopunit,
	"SynchronizeCache","<start block> <blocks> [relative] [immed]",
		synchronizecache,
	"TestUnitReady","- determine if a unit is ready",testunitready,
	"Verify","<start block> <blocks> [relative] [bytecheck <offset>] [DPO]",
		verify,
	"Write","<start block> <blocks> <offset>", write6,
	"Write10","<start block> <blocks> <offset> [relative] [DPO] [force]",
		write10,
	"WriteVerify","<start block> <blocks> <offset> [relative] [DPO] [check]",
		writeverify,
};

/* filehandle for input */
FILE *fp = stdin;

SCSIDEVICE *ior = NULL;

extern void clearcommand(void);

unsigned char command[20];
int addr,lun;
char commandbuff[1024];
int echo = FALSE;		/* echo commands/prompts for Source */

int
main (argc,argv)
	int argc;
	char **argv;
{
	int done = FALSE,len,i;
	char *command;

	if (argc != 4)
	{
		printf("usage: %s device tid lun\n",argv[0]);
		exit(10);
	}

	printf("%s 1.3 by Randell Jesup - use ? to get a list of commands\n\n",
	       argv[0]);
	printf("\tWarning: there is little error checking in this program.\n");
	printf("You could destroy all the data on your drive!\n");
	printf("You HAVE been warned!\n");

	addr = atol(argv[2]);
	lun  = atol(argv[3]);
	printf("Opening device %s, TID %d, LUN %d\n", argv[1], addr, lun);

	ior = InitSCSI(argv[1], addr, lun);
	if (!ior)
	{
		printf("error on open\n");
		return 10;
	}

	/* Find the block size for use later... */
	doreadcapacity();

	while (!done)
	{
		if (fp == stdin || echo)
		{
			printf("%s> ",argv[0]);
			fflush(stdout);
		}
		if (fgets(commandbuff,sizeof(commandbuff),fp) == NULL)
		{
			if (fp == stdin)
				break;			/* eof */
			else {
				fp = stdin;		/* end of 'source' */
				echo = FALSE;
				continue;
			}
		}

		len = strlen(commandbuff);		/* kill \n */
		if (len && commandbuff[len-1] == '\n')
			commandbuff[len-1] = '\0';

		if (echo)
			puts(commandbuff);

		command = strtok(commandbuff," ");
		if (!command || !*command)		/* no command */
			continue;

		for (i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
		{
			if (stricmp(command,comm[i].name) == SAME)
			{
				/* found it - execute */

				clearcommand();
				(*(comm[i].rtn))(&done);
				break;
			}
		}
		if (i >= sizeof(comm)/sizeof(comm[0]))	/* failure */
			printf("Error: command %s not known!\n",command);
	}

	QuitSCSI(ior);
	
	return 0;
}

void
help (int *done)
{
	int i;

	for (i = 0; i < sizeof(comm)/sizeof(comm[0]); i++)
	{
		printf(" %s %s\n",comm[i].name,comm[i].helpstr);
	}
	printf(" <offset> refers to an offset into the buffer\n");
}

void
quit (int *done)
{
	*done = TRUE;
}

void
source (int *done)
{
	char *arg,*opt;
	FILE *myfp;

	arg = strtok(NULL," ");
	if (!arg)
	{
err:		printf("error: use source <filename> [echo]\n");
		return;
	}
	opt = strtok(NULL," ");
	if (opt)
	{
		if (stricmp(opt,"echo") == SAME)
		{
			echo = TRUE;
		} else
			goto err;
	}

	myfp = fopen(arg,"r");
	if (!myfp)
	{
		printf("error: Can't open %s!\n",arg);
		echo = FALSE;
		return;
	}

	fp = myfp;

}

/* make sure it's _big_ - 1024 512-byte sectors max (or 256 2K sectors) */

unsigned char __far buffer[512*1024];

void
showbuffer (int *done)
{
	char *arg;
	int i;
	int hex = FALSE;
	long offset = 0;
	long len    = 80;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"hex") == SAME)
		{
			hex = TRUE;
			arg = strtok(NULL," ");
		}
	}

	/* optional offset */
	if (arg)
	{
		if (decnum(arg,&offset))
		{
err:		    printf("error: bad number (%s)\n",arg);
		    printf("error: use showbuffer [hex] [offset [length]]\n");
		    return;
		}

		arg = strtok(NULL," ");
		if (arg)
		{
			if (decnum(arg,&len))
			{
				goto err;
			}
		}
	}

	for (i = 0; i < len; i++)
	{
		if (!hex)
		{
			if (!buffer[offset+i])
				break;
			putchar(buffer[offset+i]);
		} else {
			if (i % 20 == 0)
			{
				printf("\n%04ld: ",i+offset);
			}
			printf("%02x ",buffer[offset+i]);
		}
	}
	putchar('\n');
}

void
writebuffer (int *done)
{
	char *arg,*name;
	int i;
	int hex = FALSE;
	long offset = 0;
	long len    = 80;
	FILE *myfp;

	name = strtok(NULL," ");
	if (!name)
	{
err:	    printf("error: use writebuffer <file> [hex] [offset [length]]\n");
	    return;
	}

	/* optional "hex" */
	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"hex") == SAME)
		{
			hex = TRUE;
			arg = strtok(NULL," ");
		}
	}

	/* optional offset */
	if (arg)
	{
		if (decnum(arg,&offset))
		{
		    printf("error: bad number (%s)\n",arg);
		    goto err;
		}

		arg = strtok(NULL," ");
		if (arg)
		{
			if (decnum(arg,&len))
			{
				goto err;
			}
		}
	}

	myfp = fopen(name,"w+");
	if (!myfp)
	{
		printf("error: can't open %s!\n",name);
		return;
	}

	for (i = 0; i < len; i++)
	{
		if (!hex)
		{
			putc(buffer[offset+i],myfp);
		} else {
			if (i % 20 == 0)
			{
				fprintf(myfp,"\n%04ld: ",i+offset);
			}
			fprintf(myfp,"%02x ",buffer[offset+i]);
		}
	}
	if (hex)
		putc('\n',myfp);

	fclose(myfp);
	printf("file %s written\n",name);
}

void
setbuffer (int *done)
{
	char *str;
	long offset;
	char hexdigit[4];
	long digit;

	str = strtok(NULL," ");
	if (!str)
	{
err:	    printf("Usage: SetBuffer [offset] [hex <hex digits>]|<string>\n");
	    return;
	}
	if (decnum(str,&offset))
	{
		offset = 0;
	} else {
		/* got offset, get next string */
		str = strtok(NULL," ");
		if (!str)
			goto err;
	}

	if (stricmp(str,"hex") == SAME)
	{
		str = strtok(NULL," ");
		if (!str)
			goto err;

		while (*str)
		{
			hexdigit[0] = *str++;
			if (!*str)
				goto err;

			hexdigit[1] = *str++;
			hexdigit[2] = 0;

			if (hexnum(hexdigit,&digit) || digit > 0xff)
			{
				printf("Bad hex value '%s'!\n",hexdigit);
				return;
			}

			buffer[offset++] = digit;
		}
	} else {
		/* string */
		strcpy(&(buffer[offset]),str);
	}
}

int
argsdone()
{
	char *args;

	args = strtok(NULL," ");
	if (args && *args)
	{
		printf("error: too many arguments (%s)\n",args);
		return FALSE;
	}
	return TRUE;
}

/* data */

char *lastarg;		/* for getnum failures */

int
gethexnum (long *num)
{
	lastarg = strtok(NULL," ");
	if (!lastarg)
	{
		printf("error: missing number\n");
		return FALSE;
	}

	if (hexnum(lastarg,num))
	{
		printf("error: bad number (%s)\n",lastarg);
		return FALSE;
	}

	return TRUE;
}

int
getnum (long *num)
{
	lastarg = strtok(NULL," ");
	if (!lastarg)
	{
		printf("error: missing number\n");
		return FALSE;
	}

	if (decnum(lastarg,num))
	{
		printf("error: bad number (%s)\n",lastarg);
		return FALSE;
	}

	return TRUE;
}

int
get2nums (long *num1, long *num2)
{
	if (!getnum(num1))
		return FALSE;

	return getnum(num2);
}

int
get3nums (long *num1, long *num2, long *num3)
{
	if (!getnum(num1) || !getnum(num2))
		return FALSE;

	return getnum(num3);
}

/* end standard code */

/************************************************************************/

/* store a longword at an arbitrary address, must work on 68000 */
#define PUTLONG(buf,value)	((char *) (buf))[0] = (value >> 24); \
				((char *) (buf))[1] = (value >> 16) & 0xff; \
				((char *) (buf))[2] = (value >> 8) & 0xff; \
				((char *) (buf))[3] = value & 0xff
#define PUT3BYTES(buf,value)	((char *) (buf))[0] = (value >> 16) & 0xff; \
				((char *) (buf))[1] = (value >> 8) & 0xff; \
				((char *) (buf))[2] = value & 0xff
#define PUTWORD(buf,value)	((char *) (buf))[0] = (value >> 8) & 0xff; \
				((char *) (buf))[1] = value & 0xff

/* these are mostly for showpage... */
#define LEN(x)			(pagelen >= (x))
#define P(offset,statement)	{ if (pagelen >= (offset)) \
					statement; }
#define NORMAL_LEN(x)	if (pagelen-1 != (x)) { \
    printf("\tNormal SCSI-2 Page Len is %ld, page len returned is %ld\n", \
		(x),pagelen-1); }
#define GETBYTE(x)	(pagestart[(x)])
#define GETWORD(x)	((pagestart[(x)] << 8) | pagestart[1+(x)])
#define GET3BYTES(x)	((pagestart[(x)] << 16) | \
			 (pagestart[1+(x)] << 8) | pagestart[2+(x)])
#define GETLONG(x)	((pagestart[(x)] << 24)  | \
			 (pagestart[1+(x)] << 16) | \
			 (pagestart[2+(x)] << 8) | pagestart[3+(x)])


unsigned char mybuf[256];
unsigned long blocksize = 512;

int internal_modesense (void *buf, LONG len, LONG type,LONG page, int dbd);
int internal_modeselect (void *buf, LONG len, int save, int scsi2);

void
clearcommand ()
{
	int i;

	for (i = 0; i < sizeof(command); i++)
		command[i] = 0;
}

void
docommand (int *done)
{
	LONG clen,byte,offset,dlen,rc;
	int rw = 0,i;
	char *buf,*arg;

	if (!getnum(&clen))
	{
err:		printf(
   "use Command <cmd len> <cmd bytes> <data offset> <data len> Read|Write\n"
   "\t(Note: you must specify a data area and direction even if the\n"
   "\t command transfers no data.  Sense will be done automaticly.)\n");
		return;
	}

	if (clen != 6 && clen != 10)
		printf("Non-standard command len (%ld)!\n",clen);
	if (clen > sizeof(command))
		goto err;

	for (i = 0; i < clen; i++)
	{
		if (!gethexnum(&byte) || byte > 255 || byte < 0)
			goto err;

		command[i] = byte;
	}

	if (!get2nums(&offset,&dlen))
		goto err;

	buf = &(buffer[offset]);

	if (offset+dlen > sizeof(buffer))
	{
		printf("offset and/or data length out of range! (max %ld)\n",
			sizeof(buffer));
		return;
	}

	arg = strtok(NULL," ");
	if (!arg)
		goto err;
	if (stricmp(arg,"read") == SAME)
		rw = 0;
	else if (stricmp(arg,"write") == SAME)
		rw = 1;
	else
		goto err;

	if (!argsdone())
		goto err;

	rc = DoSCSI(ior,command,clen,(UWORD *) buf,dlen,
		    (rw ? SCSI_READ : SCSI_WRITE)|SCSI_AUTOSENSE);

	printf("command $%lx returned %ld\n",command[0],rc);
	if (rc)
		dumpsense(rc);
}

char *sensecodes[] = {
	"Good",
	"",
	"Check Condition",
	"",
	"Condition Met/Good",
	"","","",
	"Busy",
	"","","","","","","",
	"Intermediate/Good",
	"","","",
	"Intermediate/Condition Met/Good",
	"","","",
	"Reservation Conflict",
	"","","","","","","","","",
	"Command Terminated",
	"","","","","",
	"Queue Full",
};

char *sensekeys[] = {
	"No Sense",
	"Recovered Error",
	"Not Ready",
	"Medium Error",
	"Hardware Error",
	"Illegal Request",
	"Unit Attention",
	"Data Protect",
	"Blank Check",
	"Vendor Specific",
	"Copy Aborted",
	"Aborted Command",
	"Equal",
	"Volume Overflow",
	"Miscompare",
	"reserved",
};

struct fullcodes {
	unsigned char code;
	unsigned char qual;
	char *str;
} codenames[] = {
	{ 0,0,"no additional information" },
	{ 1,0,"no index/sector signal" },
	{ 2,0,"no seek complete" },
	{ 3,0,"peripheral device write fault" },
	{ 4,0,"Logical Unit not ready, cause not reportable" },
	{ 4,1,"Logical Unit is in process of becoming ready" },
	{ 4,2,"Logical Unit not ready, initializing command required" },
	{ 4,3,"Logical Unit not ready, manual intervention required" },
	{ 4,4,"Logical Unit not ready, FORMAT in progress" },
	{ 5,0,"Logical Unit does not respond to selection" },
	{ 6,0,"no reference position found -> (track 0 or equivalent)" },
	{ 7,0,"multiple peripheral devices selected" },
	{ 8,0,"Logical Unit communication failure" },
	{ 8,1,"Logical Unit communication timeout" },
	{ 8,2,"Logical Unit communication parity error" },
	{ 9,0,"track following error" },
	{ 0x0a,0,"error log overflow" },
	{ 0x0c,1,"write error recovered with auto reallocation" },
	{ 0x0c,2,"write error - auto reallocation failed" },
	{ 0x10,0,"ID crc or ECC error" },
	{ 0x11,0,"unrecovered read error" },
	{ 0x11,1,"read retries exhausted" },
	{ 0x11,2,"error too long to correct" },
	{ 0x11,3,"multiple read errors" },
	{ 0x11,4,"unrecovered read error - auto reallocate failed" },
	{ 0x11,0x0a,"miscorrected error" },
	/* need to type in more */

	{ 0x1a,0,"parameter list length error" },

	{ 0x1c,0,"defect list not found" },

	{ 0x20,0,"invalid command operation code (command not supported)" },
	{ 0x21,0,"Logical Block Address out of range" },
	{ 0x24,0,"invalid field in CDB -> check Field Pointer in SENSE data" },
	{ 0x25,0,"Logical Unit not supported" },
	{ 0x26,0,"invalid field in parameter list -> check field pointer" },
	{ 0x26,1,"parameter not supported -> check field pointer" },
	{ 0x26,2,"parameter value invalid -> check field pointer" },

	{ 0x27,0,"write protected" },

};

void
dumpsense (int code)
{
	int i,key,subcode,qual,len;

	if (code > sizeof(sensecodes)/sizeof(sensecodes[0]))
	{
		printf("IO Error %ld\n",code);
		return;
	}
	printf("Sense code: %ld (%s)\n",code,sensecodes[code]);
	if ((sensedata[0] & 0x7e) == 0x70)
	{
		if (sensedata[2] & 0x80)
			printf("positioned at filemark\n");
		if (sensedata[2] & 0x40)
			printf("positioned at end of medium\n");
		if (sensedata[2] & 0x20)
			printf("bad length (block size or Read/Write Long)\n");

		key = sensedata[2] & 0x0f;
		printf("Sense key is %ld (%s)\n",key,sensekeys[key]);

		if (sensedata[0] & 0x80)
		{
			/* data is valid */
			printf("information block is valid: ");
			for (i = 3; i <= 6; i++)
				printf("%02lx ",sensedata[i]);
			printf("\n");
		}

		len = sensedata[7];	/* number of bytes after # 7 */

		if (len >= 6)
		{
			subcode = sensedata[12];
			qual = sensedata[13];

			printf("Addition sense: 0x%lx, qualifier 0x%lx\n",
				subcode,qual);

			for (i = 0; i < sizeof(codenames)/sizeof(codenames[0]);
			     i++)
			{
				if (subcode == codenames[i].code &&
				    qual    == codenames[i].qual)
				{
					puts(codenames[i].str);
					break;
				}
			}
		}

		if (len >= 7)
			printf("Field Replacable Unit = %ld\n",sensedata[14]);

		if (len >= 10)
		{
			if (sensedata[15] & 0x80)
			{
				if (key == 5)	/* illegal request */
				{
				  if (sensedata[15] & 0x40)
				    printf("Error is in command bytes\n");
				  else
				    printf("Error is in Data bytes\n");

				  if (sensedata[15] & 0x08)
				    printf("Bit number: %ld\n",
					   sensedata[15] & 0x07);

				  printf("Byte in error is %ld\n",
					 (sensedata[16] << 8) | sensedata[17]);

				} else if (key == 1 || key == 3 || key == 4) {

				  printf("%ld retries were made\n",
					 (sensedata[16] << 8) | sensedata[17]);

				} else if (key == 2) {
				  printf("format is %ld%% complete\n",
				    (((sensedata[16]<<8)|sensedata[17])*100)/
				    65536);
				}
			}
		}
	}

	printf("\nCommand was: ");
	for (i = 0; i < SCSI_CmdLength; i++)
	{
		printf("%02lx ",command[i]);
	}
	printf("\n");

	printf("\n%ld data bytes transferred out of %ld\n\n",
		SCSI_Actual,SCSI_Length);

	printf("\nRaw Sense data: (%ld bytes)\n",SCSI_SenseActual);

	for (i = 0; i < SCSI_SenseActual; i++)
	{
		if (i % 16 == 0)
			printf("%04lx: ",i);
		printf("%02lx ",sensedata[i]);
		if ((i+1) % 16 == 0)
			printf("\n");
	}
	if (i % 16 != 0)
		printf("\n");
}

void
testunitready (int *done)
{
	int rc;

	/* no args */
	if (!argsdone())
	{
		printf("usage: TestUnitReady\n");
		return;
	}

	command[0] = S_TEST_UNIT_READY;
	/* lun is set by scsi device */

	rc = DoSCSI(ior,command,6,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	printf("Test Unit Ready returned %ld\n",rc);
	if (rc)
		dumpsense(rc);
}


void
rezerounit (int *done)
{
	int rc;

	/* no args */
	if (!argsdone())
	{
		printf("usage: RezeroUnit\n");
		return;
	}

	command[0] = S_REZERO_UNIT;
	/* lun is set by scsi device */

	rc = DoSCSI(ior,command,6,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	printf("Rezero Unit returned %ld\n",rc);
	if (rc)
		dumpsense(rc);
}

void
startstopunit (int *done)
{
	int rc;
	char *arg;
	int immed = FALSE;
	int start = FALSE;
	int eject = FALSE;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"start") == SAME)
		{
			start = TRUE;
		} else if (stricmp(arg,"stop") != SAME) {
err:			printf(
		"error: use StartStopUnit start|stop [immed] [load_eject]\n");
			return;
		}
	} else
		goto err;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"immed") == SAME)
		{
			immed = TRUE;
			arg = strtok(NULL," ");
		} 
	}

	if (arg)
	{
		if (stricmp(arg,"load_eject") == SAME)
		{
			eject = TRUE;
			arg = strtok(NULL," ");
		} else
			goto err;
	}
	if (arg)
		goto err;

	if (!argsdone())
		goto err;

	command[0] = S_START_STOP_UNIT;
	/* lun is set by scsi device */
	if (immed)
		command[1] = 1;
	if (start)
		command[4] = 1;
	if (eject)
		command[4] |= 2;

	rc = DoSCSI(ior,command,6,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	printf("Start/Stop Unit returned %ld\n",rc);
	if (rc)
		dumpsense(rc);
}


void
mediumremoval (int *done)
{
	int rc;
	char *arg;
	int prevent = FALSE;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"prevent") == SAME)
		{
			prevent = TRUE;
		} else if (stricmp(arg,"allow") != SAME) {
err:			printf(
		"error: use MediumRemoval prevent|allow\n");
			return;
		} else
			goto err;
	} else
		goto err;

	if (!argsdone())
		goto err;

	command[0] = S_PREVENT_ALLOW_REMOVAL;
	/* lun is set by scsi device */
	if (prevent)
		command[4] = 1;

	rc = DoSCSI(ior,command,6,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	printf("Prevent/Allow Medium Removal returned %ld\n",rc);
	if (rc)
		dumpsense(rc);
}


char *device_qualifier[] = {
"Peripheral connected and is specified by Device Type",
"Peripheral no connected, Device type indicates type could be connected",
"qualifier Reserved!",
"",
"Vendor specific qualifier",
"Vendor specific qualifier",
"Vendor specific qualifier",
"Vendor specific qualifier",
};

char *device_type[] = {
	"Direct-access device (disk)",
	"Sequential-access device (tape)",
	"Printer device",
	"Processor device",
	"WORM device (disk)",
	"CD-ROM device",
	"Scanner device",
	"Optical memory device (disk)",
	"Medium changer device (jukebox)",
	"Communications device (LAN, serial, etc)",
};

char *ansi_name[] = {
	"Not an ansi-standard device",
	"SCSI-1 device (X3.131-1986)",
	"SCSI-2 device (X3.131-1990)",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

char *inquiry_format[] = {
	"SCSI-1 standard Inquiry format",
	"CCS standard Inquiry format",
	"SCSI-2 standard Inquiry format",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
};

#define L(x,y)	if (len+4 >= (x)) y

char strbuf[256];

char *
getstring (char *buf, int x, int y)
{
	memcpy(strbuf,&(buf[x]),(y-x)+1);
	strbuf[(y-x)+1] = 0;
	return strbuf;
}

void
inquiry (int *done)
{
	LONG offset,len,rc,page,i;
	unsigned char *buf;

	if (!get3nums(&page,&offset,&len))
	{
err:	printf("error: use Inquiry <page> <offset> <length>\n");
	printf("\tpage 0 gives default information\n");
	printf("\t(currently, no other pages are supported.)\n");
		return;
	}

	if (!argsdone())
		goto err;

	if (len > 255 || len < 5)
		goto err;

	buf = &(buffer[offset]);

	command[0] = S_INQUIRY;
	if (page)
		command[1] = 1;		/* EVPD */
	command[2] = page;
	command[4] = len;

	/* set up header */
	rc = DoSCSI(ior,command,6,buf,len,
		    SCSI_READ|SCSI_AUTOSENSE);
	if (rc)
		dumpsense(rc);
	else {
		int format;

		switch (page) {
		case 0:
			if (buf[0] == 0x7f)
				printf("This LUN not supported\n");
			else {
				printf("%s\n",device_qualifier[buf[0] >> 5]);
				if ((buf[0] & 0x1f) <= 9)
				     printf("%s\n",device_type[buf[0] & 0x1f]);
				else
				     printf("Reserved device type %ld\n",
					    buf[0] & 0x1f);
			}
			if (buf[1] & 0x80)
				printf("Media is removable\n");
			else
				printf("Media is not removable\n");

			printf("Device type modifier: %ld\n",buf[1] & 0x7f);

			printf("ISO version:  %ld\n",buf[2] >> 6);
			printf("ECMA version: %ld\n",(buf[2] & 0x38) >> 3);
			printf("ANSI version: %ld (%s)\n",buf[2] & 0x07,
				ansi_name[buf[2] & 0x07]);

			if (buf[3] & 0x80)
				printf("Processor device can accept AENs\n");
			else
				printf("Device can't accept AENs\n");

			if (buf[3] & 0x40)
			  printf("Supports TERMINATE I/O PROCESS message\n");
			else
			  printf("Doesn't support TERMINATE I/O message\n");

			format = buf[3] & 0x0f;
			printf("Inquiry response format: %ld (%s)\n",
				format,inquiry_format[format]);
			len = buf[4];

			if (len+4 < 35)
			{
			 printf("Returned smaller than normal length (%ld)!\n",
				len);
			}

			if (len+4 >= 7)
			{
			    if (buf[7] & 0x80)
				printf("Relative addressing supported\n");
			    else
				printf("Relative addressing not supported\n");

			    if (buf[7] & 0x40)
				printf("32-bit transfers supported\n");
			    else
				printf("32-bit transfers not supported\n");

			    if (buf[7] & 0x20)
				printf("16-bit transfers supported\n");
			    else
				printf("16-bit transfers not supported\n");

			    if (buf[7] & 0x10)
				printf("Synchronous transfers supported\n");
			    else
				printf("Synchronous transfers not supported\n");

			    if (buf[7] & 0x08)
				printf("Linked commands supported\n");
			    else
				printf("Linked commands not supported\n");

			    if (buf[7] & 0x02)
				printf("Tagged command queuing supported\n");
			    else
				printf("Tagged commands not supported\n");

			    if (buf[7] & 0x01)
				printf("RST causes 'soft' reset\n");
			    else
				printf("RST causes 'hard' reset\n");
			}

			L(15,printf("Vendor: %s\n",getstring(buf,8,15)));
			L(31,printf("Product: %s\n",getstring(buf,16,31)));
			L(35,printf("Revision: %s\n",getstring(buf,32,35)));

			printf("Vendor-specific bytes:\n");
			for (i = 0; i <= 55-36 && i+36 <= len+4; i++)
			{
				if (i % 10 == 0)
					printf("%04ld: ",i+36);
				printf("%02lx ",buf[i+36]);
				if ((i+1) % 10 == 0)
					printf("\n");
			}
			if (i % 10 != 0)
				printf("\n");

			for (i = 0; i+96 <= len+4; i++)
			{
				if (i % 10 == 0)
					printf("%04ld: ",i+96);
				printf("%02lx ",buf[i+96]);
				if ((i+1) % 10 == 0)
					printf("\n");
			}
			if (i % 10 != 0)
				printf("\n");

		}
	}
}	

void
modeselect (int *done)
{
	LONG offset,len;
	char *buf,*arg;
	int save = FALSE;
	int scsi2 = FALSE;

	if (!get2nums(&offset,&len))
	{
err:	printf("error: use ModeSelect <pageoffset> <length> [save] [scsi2]\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"save") == SAME)
		{
			save = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"scsi2") == SAME)
		{
			scsi2 = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (len > 255 || len < 0)
		goto err;

	buf = &(buffer[offset]);

	printf("Mode Select returned %ld\n",
		internal_modeselect(buf,len,save,scsi2));
}

int
internal_modeselect (void *buf, LONG len, int save, int scsi2)
{
	int rc;

	command[0] = S_MODE_SELECT;
	if (save)
		command[1] = 1;
	if (scsi2)
		command[1] |= 0x10;		/* scsi-2 parameters */
	command[4] = len;

	/* set up header */
	rc = DoSCSI(ior,command,6,buf,len,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);

	return rc;
}	

void
doreadcapacity ()
{
	LONG rc;

	command[0] = S_READ_CAPACITY;

	/* set up header */
	rc = DoSCSI(ior,command,10,mybuf,8,
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else {
		if (SCSI_Actual >= 8)
		{
			blocksize = (mybuf[4] << 24) | (mybuf[5] << 16) |
				    (mybuf[6] << 8)  | mybuf[7];
			printf("Block length = %ld bytes\n",blocksize);
		}
	}
}	

void
readcapacity (int *done)
{
	LONG block,rc;
	char *arg;
	int relative = FALSE;
	int pmi = FALSE;

	if (!getnum(&block))
	{
err:	printf(
	"error: use ReadCapacity <starting block> [relative] [next_delay]\n"
	"\t('relative' only applies to linked commands)\n"
	"\t('next_delay' means return the position of the next delay that\n"
	"\t will be hit in reading after <starting block>.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"next_delay") == SAME)
		{
			pmi = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	command[0] = S_READ_CAPACITY;
	if (relative)
		command[1] = 1;
	PUTLONG(&(command[2]),block);
	if (pmi)
		command[8] = 1;

	/* set up header */
	rc = DoSCSI(ior,command,10,mybuf,8,
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else {
		if (SCSI_Actual >= 4)
		{
			printf("Returned Logical Block Address %ld\n",
				(mybuf[0] << 24) | (mybuf[1] << 16) |
				(mybuf[2] << 8)  | mybuf[3]);
		}
		if (SCSI_Actual >= 8)
		{
			printf("Returned Block length = %ld bytes\n",
				(mybuf[4] << 24) | (mybuf[5] << 16) |
				(mybuf[6] << 8)  | mybuf[7]);
		}
	}
}	

void
verify (int *done)
{
	LONG block,length,rc;
	char *arg,*buf;
	int relative = FALSE;
	int bytecheck = FALSE;
	LONG offset = 0;
	int dpo = FALSE;

	if (!get2nums(&block,&length))
	{
err:	printf(
"error: Verify <start block> <blocks> [relative] [bytecheck <offset>] [DPO]\n"
"\t('relative' only applies to linked commands.)\n"
"\t('bytecheck' means that verify will compare to data from <offset>.)\n"
"\t('DPO' means data should be considered low priority for caching.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"bytecheck") == SAME)
		{
			bytecheck = TRUE;
			if (!getnum(&offset))
				goto err;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"dpo") == SAME)
		{
			dpo = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_VERIFY;
	if (relative)
		command[1] = 1;
	if (bytecheck)
		command[1] |= 0x02;
	if (dpo)
		command[1] |= 0x10;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	buf = &(buffer[offset]);

	/* set up header */
	rc = DoSCSI(ior,command,10,buf,length*blocksize,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
}	

void
read6 (int *done)
{
	LONG block,length,offset,rc;
	char *buf;

	if (!get3nums(&block,&length,&offset))
	{
err:	printf("error: Read <start block> <blocks> <offset>\n");
		return;
	}

	if (!argsdone())
		goto err;

	if (block > 0x1fffff)
	{
		printf("Block out of range of Read(6) command!\n");
		goto err;
	}
	if (length <= 0 || length > 256)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_READ;
	PUT3BYTES(&(command[1]),block);
	command[4] = (length == 256 ? 0 : length);

	buf = &(buffer[offset]);
	if (length*blocksize + offset > sizeof(buffer))
	{
		length = (sizeof(buffer) - offset)/blocksize;
		printf(
		"Forced length to %ld blocks to avoid overrunning buffer\n");
	}

	/* set up header */
	rc = DoSCSI(ior,command,6,buf,length*blocksize,
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else
		printf("Transferred %ld bytes.\n",SCSI_Actual);
}	

void
write6 (int *done)
{
	LONG block,length,offset,rc;
	char *buf;

	if (!get3nums(&block,&length,&offset))
	{
err:	printf("error: Write <start block> <blocks> <offset>\n");
		return;
	}

	if (!argsdone())
		goto err;

	if (block > 0x1fffff)
	{
		printf("Block out of range of Write(6) command!\n");
		goto err;
	}
	if (length <= 0 || length > 256)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_WRITE;
	PUT3BYTES(&(command[1]),block);
	command[4] = (length == 256 ? 0 : length);

	buf = &(buffer[offset]);

	/* set up header */
	rc = DoSCSI(ior,command,6,buf,length*blocksize,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else
		printf("Transferred %ld bytes.\n",SCSI_Actual);
}	

void
read10 (int *done)
{
	LONG block,length,offset,rc;
	char *arg,*buf;
	int relative = FALSE;
	int force = FALSE;
	int dpo = FALSE;

	if (!get3nums(&block,&length,&offset))
	{
err:	printf(
"error: Read10 <start block> <blocks> <offset> [relative] [DPO] [force]\n"
"\t('relative' only applies to linked commands.)\n"
"\t('force' means data must be read from the disk, not from cache.)\n"
"\t('DPO' means data should be considered low priority for caching.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"dpo") == SAME)
		{
			dpo = TRUE;
		    	arg = strtok(NULL," ");
		} else
			goto err;
	}

	if (arg)
	{
		if (stricmp(arg,"force") == SAME)
		{
			force = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_READ10;
	if (relative)
		command[1] = 1;
	if (force)
		command[1] |= 0x08;
	if (dpo)
		command[1] |= 0x10;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	buf = &(buffer[offset]);
	if (length*blocksize + offset > sizeof(buffer))
	{
		length = (sizeof(buffer) - offset)/blocksize;
		printf(
		"Forced length to %ld blocks to avoid overrunning buffer\n");
	}

	/* set up header */
	rc = DoSCSI(ior,command,10,buf,length*blocksize,	/* FIX! */
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else
		printf("Transferred %ld bytes.\n",SCSI_Actual);
}	

void
write10 (int *done)
{
	LONG block,length,offset,rc;
	char *arg,*buf;
	int relative = FALSE;
	int force = FALSE;
	int dpo = FALSE;

	if (!get3nums(&block,&length,&offset))
	{
err:	printf(
"error: Write10 <start block> <blocks> <offset> [relative] [DPO] [force]\n"
"\t('relative' only applies to linked commands.)\n"
"\t('force' means data must be read from the disk, not from cache.)\n"
"\t('DPO' means data should be considered low priority for caching.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"dpo") == SAME)
		{
			dpo = TRUE;
		    	arg = strtok(NULL," ");
		} else
			goto err;
	}

	if (arg)
	{
		if (stricmp(arg,"force") == SAME)
		{
			force = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_WRITE10;
	if (relative)
		command[1] = 1;
	if (force)
		command[1] |= 0x08;
	if (dpo)
		command[1] |= 0x10;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	buf = &(buffer[offset]);

	/* set up header */
	rc = DoSCSI(ior,command,10,buf,length*blocksize,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else
		printf("Transferred %ld bytes.\n",SCSI_Actual);
}	

void
writeverify (int *done)
{
	LONG block,length,offset,rc;
	char *arg,*buf;
	int relative = FALSE;
	int bytecheck = FALSE;
	int dpo = FALSE;

	if (!get3nums(&block,&length,&offset))
	{
err:	printf(
"error: WriteVerify <start block> <blocks> <offset> [relative] [DPO] [check]\n"
"\t('relative' only applies to linked commands.)\n"
"\t('check' means data must be compared byte by byte for the verify.)\n"
"\t('DPO' means data should be considered low priority for caching.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"dpo") == SAME)
		{
			dpo = TRUE;
		    	arg = strtok(NULL," ");
		} else
			goto err;
	}

	if (arg)
	{
		if (stricmp(arg,"check") == SAME)
		{
			bytecheck = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_WRITE_VERIFY;
	if (relative)
		command[1] = 1;
	if (bytecheck)
		command[1] |= 0x02;
	if (dpo)
		command[1] |= 0x10;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	buf = &(buffer[offset]);

	/* set up header */
	rc = DoSCSI(ior,command,10,buf,length*blocksize,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
	else
		printf("Transferred %ld bytes.\n",SCSI_Actual);
}	

void
synchronizecache (int *done)
{
	LONG block,length,rc;
	char *arg;
	int relative = FALSE;
	int immed = FALSE;

	if (!get2nums(&block,&length))
	{
err:	printf(
"error: use SynchronizeCache <start block> <blocks> [relative] [immed]\n"
"\t('relative' only applies to linked commands.)\n"
"\t('immed' means it returns immediately, before the cache is flushed.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"immed") == SAME)
		{
			immed = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_SYNCHRONIZE_CACHE;
	if (relative)
		command[1] = 1;
	if (immed)
		command[1] |= 0x02;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	/* set up header */
	rc = DoSCSI(ior,command,10,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
}	

void
prefetch (int *done)
{
	LONG block,length,rc;
	char *arg;
	int relative = FALSE;
	int immed = FALSE;

	if (!get2nums(&block,&length))
	{
err:	printf(
"error: use Prefetch <start block> <blocks> [relative] [immed]\n"
"\t('relative' only applies to linked commands.)\n"
"\t('immed' means it returns immediately, before the data is read.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		    	arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"immed") == SAME)
		{
			immed = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_PREFETCH;
	if (relative)
		command[1] = 1;
	if (immed)
		command[1] |= 0x02;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	/* set up header */
	rc = DoSCSI(ior,command,10,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
}	

char *defect_formats[] = {
	"Block format",
	"Vendor-specific",
	"Vendor-specific",
	"Vendor-specific",
	"Bytes From Index format",
	"Physical Sector format",
	"Vendor-specific",
	"Vendor-specific",
};

void
readdefectdata (int *done)
{
	LONG offset,len,format,rc;
	int primary = FALSE, grown = FALSE;
	char *arg,*buf;
	unsigned char *pagestart;
	LONG pagelen;

	if (!get3nums(&offset,&len,&format))
	{
err:		printf(
"error: use ReadDefectData <offset> <len> <format> [primary] [grown]\n"
"\t(<format> is one of:\n"
"\t\t0 - block format\n"
"\t\t4 - bytes from index format\n"
"\t\t5 - physical sector format\n"
"\t\t1,2,3,6,7 - vendor-specific format.\n"
"\t You should normally specify one or both of primary and grown.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"primary") == SAME)
		{
			primary = TRUE;
			arg = strtok(NULL," ");
		}
	}

	if (arg)
	{
		if (stricmp(arg,"grown") == SAME)
			grown = TRUE;
		else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (format > 7 || format < 0)
		goto err;

	buf = &(buffer[offset]);

	command[0] = S_READ_DEFECT_DATA;
	command[2] = format;
	if (primary)
		command[2] |= 0x10;
	if (grown)
		command[2] |= 0x08;
	PUTWORD(&(command[7]),len);

	rc = DoSCSI(ior,command,10,buf,len,
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);

	/* handle data coming back in format other than asked for */
	if (!rc || (rc == CHECK_CONDITION && SCSI_SenseActual >= 13 &&
		    (sensedata[2] & 0x0f) == RECOVERED_ERROR &&
		    (sensedata[12] == 0x1c /* defect list not found */)))
	{
		int size;

		pagestart = (unsigned char *) buf;

		if (GETBYTE(1) & 0x10)
			printf("Primary defects are returned\n");
		if (GETBYTE(1) & 0x08)
			printf("Grown defects are returned\n");

		format = GETBYTE(1) & 0x07;
		printf("Defect data format: %s\n",defect_formats[format]);
		if (format == 4 || format == 5)
			size = 8;
		else
			size = 4;

		pagelen = min(GETWORD(2),SCSI_Actual - 4);
		if (GETWORD(2) > SCSI_Actual - 4)
		{
			printf("%ld defects not shown\n",
			       (GETWORD(2) - (SCSI_Actual-4))/size);
		}

		pagestart = (unsigned char *) &(buf[4]); 
					/* ptr to first defect descriptor */

		printf("Defects:\n");
		while (pagelen > 0)
		{
			switch(format) {
			case 0:		/* block format */
				printf("\tblock = %ld\n",GETLONG(0));
				break;
			case 4:		/* bytes from index format */
				printf(
				   "\tCylinder = %ld, head = %ld, BFI = %ld\n",
					GET3BYTES(0),GETBYTE(3),GETLONG(4));
				break;
			case 5:		/* physical sector format */
				printf(
				"\tCylinder = %ld, head = %ld, sector = %ld\n",
					GET3BYTES(0),GETBYTE(3),GETLONG(4));
				break;
			default:
				printf("data = 0x%lx\n",GETLONG(4));
			}

			pagelen -= size;
			pagestart += size;
		}
	}
}

void
reassignblocks (int *done)
{
	LONG rc,len;
	char *arg;
	long *ptr = (long *) &(mybuf[4]); /* better have right alignment */

	while ((arg = strtok(NULL," ")) != NULL)
	{
		if (decnum(arg,ptr))
		{
			printf("error: bad number (%s)\n",arg);
			return;
		}
printf("block %ld\n",*ptr);
		ptr++;
	}
	mybuf[0] = 0;
	mybuf[1] = 0;
	len = ptr - ((long *) &(mybuf[4]));
printf("len = %ld\n",len);
	PUTWORD(&(mybuf[2]),len * 4);
	if (len == 0)
	{
		printf("error: use ReassignBlocks [block] ...\n");
		return;
	}

	command[0] = S_REASSIGN_BLOCKS;

	rc = DoSCSI(ior,command,6,mybuf,4+len*4,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
}

void
formatunit (int *done)
{
	LONG rc,byte,modifier,type,format = 0,interleave;
	int fov = FALSE, dpry = FALSE, dcrt = FALSE, stpf = FALSE,
	    dsp = FALSE, immed = FALSE, pattern = FALSE, defects = FALSE,
	    grown = TRUE;
	char *arg,*ptr;
	int dlen = 0,plen = 0,offset;

	if (!getnum(&interleave))
	{
err:		printf(
"use FormatUnit <interleave>\n"
"\t[pattern <modifier> <type> [<pattern>...]]\n"
"\t[defects <format> [<defects>...] [disable_primary] [disable_certify]\n"
"\t [stop_format] [disable_page_save] [immediate]]\n"
"\t[grown]\n\n"
"  (<defects> are <block> for format 0 (block format),\n"
"                 <cyl> <head> <BFI> for format 4 (bytes from index format),\n"
"                 <cyl> <head> <sec> for format 5 (physical sector format).\n"
"   <modifier> is 0 = use pattern as given,\n"
"                 1 = put logical block # in 1st 4 bytes (msb first),\n"
"                 2 = put logical block # in 1st 4 bytes of each physical\n"
"                     block in the logical block (msb first),\n"
"                 3 = reserved.\n"
"   <type> is 0 = use default pattern (error if pattern given),\n"
"             1 = repeat pattern as required to fill logical block.\n"
"   grown means that existing grown defects should be retained.)\n");
		return;
	}

	offset = 4;
	mybuf[0] = 0;
	mybuf[1] = 0;
	mybuf[2] = 0;
	mybuf[3] = 0;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"pattern") == SAME)
		{
			if (!get2nums(&modifier,&type))
				goto err;

			mybuf[4] = modifier << 6;
			mybuf[5] = type;

			plen = 0;
			if (type == 1)
			{
				ptr = &(mybuf[8]);
				while ((arg = strtok(NULL," ")) != NULL)
				{
					if (decnum(arg,&byte))
						break;	/* may be defects */

					*ptr++ = byte;
				}
				plen = ptr - &(mybuf[8]);
			} else
				arg = strtok(NULL," ");

			PUTWORD(&(mybuf[6]),plen);

			offset = 8 + plen;
			pattern = TRUE;
		}
	}

	if (arg)
	{
		if (stricmp(arg,"defects") == SAME)
		{
			if (!getnum(&format))
				goto err;

			switch (format)
			{
			case 0:			/* logical block format */
				ptr = &(mybuf[offset]);
				while ((arg = strtok(NULL," ")) != NULL)
				{
					if (decnum(arg,&byte))
						break;

					PUTLONG(ptr,byte);
					ptr += 4;
				}
				dlen = ptr - &(mybuf[offset]);
				break;
			case 4:			/* BFI format */
			case 5:			/* physical sector format */
				ptr = &(mybuf[offset]);
				while ((arg = strtok(NULL," ")) != NULL)
				{
					if (decnum(arg,&byte))
						break;

					PUT3BYTES(ptr,byte);
					ptr += 3;

					arg = strtok(NULL," ");
					if (!arg)
						goto err;

					if (decnum(arg,&byte))
					{
					printf("error: bad number (%s)\n",arg);
						return;
					}
					*ptr = byte;
					ptr += 1;

					arg = strtok(NULL," ");
					if (!arg)
						goto err;

					if (decnum(arg,&byte))
					{
					printf("error: bad number (%s)\n",arg);
						return;
					}
					PUTLONG(ptr,byte);
					ptr += 4;
				}
				dlen = ptr - &(mybuf[offset]);
				break;
			default:
				printf("Unknown defect format!\n");
				goto err;
			}
			PUTWORD(&(mybuf[2]),dlen);

			if (arg)
			{
				if (stricmp(arg,"disable_primary") == SAME)
				{
					fov = TRUE;
					dpry = TRUE;
					arg = strtok(NULL," ");
				}
			}

			if (arg)
			{
				if (stricmp(arg,"disable_certify") == SAME)
				{
					fov = TRUE;
					dcrt = TRUE;
					arg = strtok(NULL," ");
				}
			}

			if (arg)
			{
				if (stricmp(arg,"stop_format") == SAME)
				{
					fov = TRUE;
					stpf = TRUE;
					arg = strtok(NULL," ");
				}
			}

			if (arg)
			{
				if (stricmp(arg,"disable_page_save") == SAME)
				{
					fov = TRUE;
					dsp = TRUE;
					arg = strtok(NULL," ");
				}
			}

			if (arg)
			{
				if (stricmp(arg,"immediate") == SAME)
				{
					fov = TRUE;
					immed = TRUE;
					arg = strtok(NULL," ");
				}
			}

			defects = TRUE;
		}
	}

	if (arg)
	{
		if (stricmp(arg,"grown") == SAME)
		{
			grown = FALSE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (fov)
		mybuf[1] |= 0x80;
	if (dpry)
		mybuf[1] |= 0x40;
	if (dcrt)
		mybuf[1] |= 0x20;
	if (stpf)
		mybuf[1] |= 0x10;
	if (pattern)
		mybuf[1] |= 0x08;
	if (dsp)
		mybuf[1] |= 0x04;
	if (immed)
		mybuf[1] |= 0x02;

	command[0] = S_FORMAT_UNIT;
	command[1] = format;
	if (pattern || defects)
		command[1] |= 0x10;
	else
		offset = 0;		/* no data */
	if (grown)			/* inverse */
		command[1] |= 0x08;

	PUTWORD(&(command[3]),interleave);
	
	rc = DoSCSI(ior,command,6,mybuf,offset+dlen,
		    SCSI_WRITE|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
}

void
lockunlockcache (int *done)
{
	LONG block,length,rc;
	char *arg;
	int relative = FALSE;
	int lock = FALSE;

	if (!get2nums(&block,&length))
	{
err:	printf(
"error: use LockUnlockCache <start block> <blocks> lock|unlock [relative]\n"
"\t('relative' only applies to linked commands.)\n");
		return;
	}

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"lock") == SAME)
			lock = TRUE;
		else if (stricmp(arg,"unlock") == SAME)
			lock = FALSE;
		else
			goto err;
	} else
		goto err;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"relative") == SAME)
		{
			relative = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	if (length < 0 || length > 65535)
	{
		printf("Length out of range!\n");
		goto err;
	}

	command[0] = S_LOCK_UNLOCK_CACHE;
	if (relative)
		command[1] = 1;
	if (lock)
		command[1] |= 0x02;
	PUTLONG(&(command[2]),block);
	PUTWORD(&(command[7]),length);

	/* set up header */
	rc = DoSCSI(ior,command,10,buffer,sizeof(buffer),
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);
}	

void
modesense (int *done)
{
	LONG offset,len,type,page;
	char *arg,*buf;
	int dbd = FALSE;

	if (!get3nums(&page,&type,&offset))
	{
err:
printf("error: use ModeSense <page> <type:0-4> <offset> <length> [noblock]\n");
printf("\t(noblock means supress Data Block Definitions)\n");
		return;
	}
	if (!getnum(&len))
		goto err;

	if (len > 255 || len < 0 || type > 4 || type < 0)
		goto err;

	arg = strtok(NULL," ");
	if (arg)
	{
		if (stricmp(arg,"noblock") == SAME)
		{
			dbd = TRUE;
		} else
			goto err;
	}

	if (!argsdone())
		goto err;

	buf = &(buffer[offset]);

	printf("Mode Sense returned %ld\n",
		internal_modesense(buf,len,type,page,dbd));
}

int
internal_modesense (void *buf, LONG len, LONG type,LONG page, int dbd)
{
	int rc;

	command[0] = S_MODE_SENSE;
	command[1] = dbd ? 0x08 : 0;	/* DBD inverted */
	command[2] = (type << 6) | page;
	command[4] = len;

	/* set up header */
	rc = DoSCSI(ior,command,6,buf,len,
		    SCSI_READ|SCSI_AUTOSENSE);

	if (rc)
		dumpsense(rc);

	return rc;	
}

void
modifypage (int *done)
{
	LONG page;
	LONG offset, value;
	int scsi2 = FALSE;
	int save = FALSE;
	int size,start;
	char *arg;

	if (!getnum(&page))
	{
err:
printf("Error: use ModifyPage <page>  [save] [scsi2] [<byte> <value> ...]\n");
printf("\t(byte offsets are in decimal, values in hex.  Any number of pairs\n"
       "\t can be used.)\n");
		return;
	}

	if (internal_modesense(mybuf,sizeof(mybuf)-1,0,page,0))
	{
		printf("Failed\n");
		return;
	}
	/* fix up page we read 4 bytes header, N bytes page */
	size = mybuf[0] + 1;	/* size doesn't include size byte */
	mybuf[0] = 0;		/* reserved on select */
	start = 4 + mybuf[3];	/* where page starts */
	mybuf[start] &= 0x3f;	/* and off top two bits (saveable) */

nextarg:
	arg = strtok(NULL," ");
	if (!arg)
		goto err;

	if (stricmp(arg,"save") == SAME)
	{
		save = TRUE;
		goto nextarg;
	}
	else if (stricmp(arg,"scsi2") == SAME)
	{
		scsi2 = TRUE;
		goto nextarg;
	}
	else if (decnum(arg,&offset))
	{
		printf("error: bad number (%s)\n",arg);
		goto err;
	}
	/* get value */
	if (!gethexnum(&value))
		goto err;

	do {
		printf("changed byte %ld from %02lx to %02lx\n",offset+start,
			mybuf[offset+start],value);
		mybuf[offset + start] = value;

		if (!getnum(&offset))
		{
			if (!lastarg)
				break;
			else
				goto err;
		}
		if (!gethexnum(&value))
			goto err;

	} while (1);

	clearcommand();		/* important! */
	if (internal_modeselect(mybuf,size,save,scsi2))
		printf("Failed\n");
	else
		printf("Success\n");
}

char *pin34[] = {
	"Open",
	"Ready",
	"Disk Changed",
	"Reserved",
	"Reserved-00",
	"Reserved-01",
	"Reserved-10",
	"Reserved-11",
};

char *pin4[] = {
	"Open",
	"In Use",
	"Eject",
	"Head Load",
	"Reserved-00",
	"Reserved-01",
	"Reserved-10",
	"Reserved-11",
};

char *pin1[] = {
	"Open (pin 1 not used)",
	"Disk Change Reset",
	"Reserved-0",
	"Reserved-1",
	"Reserved-00",
	"Reserved-01",
	"Reserved-10",
	"Reserved-11",
};

char *interfaces[] = {
	"Small Computer System Interface - X3.131",
	"Storage Module Interface - X3.91M-1987",
	"Enhanced Small Device Interface - X3.170",
	"IPI-2 - X3.130-1986, X3T9.3/1987"
	"IPI-3 - X3.132-1987, X3.147-1988"
};

void
showpage (int *done)
{
	LONG page;
	LONG type;
	int i,start;
	int dump = FALSE;
	char *arg;
	int pagelen;
	unsigned char *pagestart;

	if (!getnum(&page))
	{
err:		printf("error: use ShowPage <page> [<type: 0-4>] [dump]\n");
		return;
	}

	arg = strtok(NULL," ");
	if (!arg || decnum(arg,&type))
	{
		if (arg)
		{
			if (stricmp(arg,"dump") == SAME)
				dump = TRUE;
			else
				goto err;
		} else
			type = 0;	/* current */
	} else {
		arg = strtok(NULL," ");
		if (arg)
		{
			if (stricmp(arg,"dump") == SAME)
				dump = TRUE;
			else
				goto err;
		}
	}

	if (!argsdone())
		goto err;

	if (internal_modesense(mybuf,sizeof(mybuf)-1,type,page,0))
	{
		printf("Failed\n");
		return;
	}

	printf("Mode Data Length = %ld\n",mybuf[0]);
	printf("Medium Type = %ld\n",mybuf[1]);
	printf("Device Specific Info = 0x%lx\n",mybuf[2]);
	printf("Block Descriptor Length = %ld\n",mybuf[3]);

	start = 4 + mybuf[3];
	if (start+2 > SCSI_Actual)
	{
	 printf("didn't return enough data to interpret (returned %ld)!\n",
		SCSI_Actual);
	 printf("Failed\n");
	 return;
	}

	pagelen   = mybuf[start+1]+1;
	pagestart = &(mybuf[start]);

	if (GETBYTE(0) & 0x80)
		printf("page is saveable\n");

	if (page != (GETBYTE(0) & 0x7f))
	{
		printf("Page returned (%ld) not page asked for (%ld)!\n",
			GETBYTE(0) & 0x7f,page);
		page = 255;
	}

	switch (page) {
	case 0x01:
		printf("Read-Write Error Recovery page:\n");
		NORMAL_LEN(0x0a)

		if (LEN(2))
		{
			if (GETBYTE(2) & 0x80)
				printf("\tAutomatic write reallocation on\n");
			if (GETBYTE(2) & 0x40)
				printf("\tAutomatic read reallocation on\n");
			if (GETBYTE(2) & 0x20)
				printf(
			   "\tTransfer before indicating unrecovered error\n");
			else
				printf(
		     "\tDon't Transfer before indicating unrecovered error\n");

			if (GETBYTE(2) & 0x10)
				printf("\tRead continous, ignore errors\n");

			if (GETBYTE(2) & 0x08)
			 printf("\tUse most expedient error recovery\n");
			else
			 printf("\tMinimize miscorrection probability\n");

			if (GETBYTE(2) & 0x04)
			 printf("\tReport recovered errors\n");
			else
			 printf("\tDon't report recovered errors\n");

			if (GETBYTE(2) & 0x02)
			 printf("\tStop data phase after recovered errors\n");
			else
			 printf("\tDon't stop after recovered errors\n");

			if (GETBYTE(2) & 0x01)
			 printf("\tUse ECC for error detection only\n");
			else
			 printf("\tUse ECC for error correction\n");
		}
		P(3,printf("\tRead Retry Count:\t\t%ld\n",GETBYTE(3)))
		P(4,printf("\tCorrection Span:\t\t%ld\n",GETBYTE(4)))
		P(5,printf("\tHead Offset Count:\t\t%ld\n",GETBYTE(5)))
		P(6,printf("\tData Strobe Offset Count:\t%ld\n",GETBYTE(6)))
		P(8,printf("\tWrite Retry Count:\t\t%ld\n",GETBYTE(8)))
		P(11,printf("\tRecovery Time Limit:\t%ldms\n",
			GETWORD(10)))
		break;
	
	case 0x02:
		printf("Disconnect/Reconnect page:\n");
		NORMAL_LEN(0x0e)

		P(2,printf("\tBuffer Full Ratio:\t\t%ld%%\n",
			(GETBYTE(2)*100)/256))
		P(3,printf("\tBuffer Empty Ratio:\t\t%ld%%\n",
			(GETBYTE(3)*100)/256))
		P(5,printf("\tBus Inactivity Limit:\t\t%ld.%01ldms\n",
			GETWORD(4)/10,GETWORD(4)%10))
		P(7,printf("\tDisconnect Time Limit:\t\t%ld\n",
			GETWORD(6)/10,GETWORD(6)%10))
		P(9,printf("\tConnect Time Limit:\t\t%ld\n",
			GETWORD(8)/10,GETWORD(8)%10))
		P(11,printf("\tMaximum Burst Size:\t\t%ld bytes\n",
			GETWORD(10)*512))
		if (LEN(2))
		{
			switch (GETBYTE(12) & 0x03) {
			case 0:
			   printf("\tOther parameters control disconnect\n");
				break;
			case 1:
	printf("\tTransfer all data w/o disconnect; otherwise allowed\n");
				break;
			case 2:
				printf("\tReserved!\n");
				break;
			case 3:
	printf("\tTransfer all data and complete command w/o disconnect;\n"
		"\t\tdisconnect before transfer allowed\n");
			}
		}
		break;

	case 3:
		{
		int format;

		printf("Format Device page:\n");
		NORMAL_LEN(0x16)

		P(3,printf("\tTracks Per Zone:\t\t\t%ld\n",
			GETWORD(2)))
		P(5,printf("\tAlternate Sectors Per Zone:\t\t%ld\n",
			GETWORD(4)))
		P(7,printf("\tAlternate Tracks Per Zone:\t\t%ld\n",
			GETWORD(6)))
		P(9,printf("\tAlternate Tracks Per Logical Unit:\t%ld\n",
			GETWORD(8)))
		P(11,printf("\tSectors per track:\t\t\t%ld\n",
			GETWORD(10)))
		P(13,printf("\tData bytes per physical sector:\t\t%ld\n",
			GETWORD(12)))
		P(15,printf("\tInterleave:\t\t\t\t%ld\n",
			GETWORD(14)))
		P(17,printf("\tTrack Skew Factor:\t\t\t%ld\n",
			GETWORD(16)))
		P(19,printf("\tCylinder Skew Factor:\t\t\t%ld\n",
			GETWORD(18)))

		if (LEN(0))
		{
			format = GETBYTE(20) >> 6;
			if (format == 0)
				printf("\tIllegal format type!\n");
			else if (format == 1)
				printf("\tHard sector formatting only\n");
			else if (format == 2)
				printf("\tSoft sector formatting only\n");
			else if (format == 3)
			 printf("\tHard and soft sector formatting allowed\n");

			if (GETBYTE(20) & 0x20)
				printf("\tMedia is removable\n");
			else
				printf("\tMedia is fixed\n");

			if (GETBYTE(20) & 0x10)
				printf("\tFormats by surface\n");
			else
				printf("\tFormats by cylinder\n");
		}
		}
		break;

	case 4:
		printf("Rigid Disk Geometry Page:\n");
		NORMAL_LEN(0x16)

		P(4,printf("\tNumber of Cylinders:\t\t\t%ld\n",
			GET3BYTES(2)))
		P(5,printf("\tHeads:\t\t\t\t\t%ld\n",GETBYTE(5)))
		P(8,printf("\tStarting Cylinder-Write-Precompensation:%ld\n",
			GET3BYTES(6)))
		P(11,printf("\tStarting Cylinder-Reduced Write Current:%ld\n",
			GET3BYTES(9)))
		P(13,printf("\tDrive Step Rate:\t\t\t%ld\n",
			GETWORD(12)))
		P(16,printf("\tLanding Zone Cylinder:\t\t\t%ld\n",
		       GET3BYTES(14)))
		if (LEN(7))
			if (GETBYTE(17) & 0x02)
				printf("Rotation Position Locked\n");
		P(18,printf("\tRotational Offset:\t\t\t%ld\n",GETBYTE(18)))
		P(20,printf("\tRotation Rate:\t\t\t\t%ld\n",
			GETWORD(19)))
		break;

	case 5:
		printf("Flexible disk page:\n");
		NORMAL_LEN(0x1e)

		P(3,printf("\tTransfer rate:\t\t%ldkbit/s\n",
				GETWORD(2)))
		P(4,printf("\tNumber of Heads:\t\t%ld\n",GETBYTE(4)))
		P(5,printf("\tSectors Per Track:\t\t%ld\n",GETBYTE(5)))
		P(7,printf("\tData Bytes per Sector:\t%ld\n",
			GETWORD(6)))
		P(9,printf("\tNumber of Cylinders:\t%ld\n",
			GETWORD(8)))
		P(11,printf("\tStarting Cylinder-Write-Precompensation:%ld\n",
			GETWORD(10)))
		P(13,printf("\tStarting Cylinder-Reduced Write Current:%ld\n",
			GETWORD(12)))
		P(15,printf("\tDrive Step Rate:\t%ldus\n",
			GETWORD(14)*100))
		P(16,printf("\tDrive Step Pulse Width:\t%ldus\n",GETBYTE(16)))
		P(18,printf("\tHead Settle Delay:\t%ldus\n",
			GETWORD(17)*100))
		P(19,printf("\tMotor on Delay:\t%ld.%01ld\n",
			GETBYTE(19)/10,GETBYTE(19)%10))
		P(20,printf("\tMotor off Delay:\t%ld.%01ld\n",
			GETBYTE(20)/10,GETBYTE(20)%10))
		if (LEN(21))
		{
			if (GETBYTE(21) & 0x80)
				printf("\tTrue ready signal available\n");
			if (GETBYTE(21) & 0x40)
				printf("\tSector numbers start at 1\n");
			else
				printf("\tSector numbers start at 0\n");
			if (GETBYTE(21) & 0x20)
			  printf("\tMotor on pin (16) will not be asserted\n");
		}
		P(22,printf("Number of step pulses per cylinder:\t%ld\n",
			GETBYTE(22) & 0x0f))
		P(23,printf("Write Compensation value:\t%ld\n",GETBYTE(23)))
		P(24,printf("Head Load delay:\t\t%ldms\n",GETBYTE(24)))
		P(25,printf("Head Unload delay:\t\t%ldms\n",GETBYTE(25)))
		P(26,printf("Pin 34 field:\t\t\t%s (Active %s)\n",
			pin34[(GETBYTE(26) >> 4) & 0x07],
			GETBYTE(26) & 0x80 ? "High" : "Low"))
		P(26,printf("Pin 2 field:\t\t\t%ld\n",GETBYTE(26) & 0x0f))
		P(27,printf("Pin 4 field:\t\t\t%s (Active %s)\n",
			pin4[(GETBYTE(27) >> 4) & 0x07],
			GETBYTE(27) & 0x80 ? "High" : "Low"))
		P(27,printf("Pin 1 field:\t\t\t%s (Active %s)\n",
			pin1[GETBYTE(27) & 0x07],
			GETBYTE(27) & 0x08 ? "High" : "Low"))
		P(29,printf("\tMedium Rotation Rate:\t%ldrpm\n",
			GETWORD(28)))
		break;

	case 0x07:
		printf("Verify Error Recovery page:\n");
		NORMAL_LEN(0x0a)

		if (LEN(2))
		{
			if (GETBYTE(2) & 0x08)
			 printf("\tUse most expedient error recovery\n");
			else
			 printf("\tMinimize miscorrection probability\n");

			if (GETBYTE(2) & 0x04)
			 printf("\tReport recovered errors\n");
			else
			 printf("\tDon't report recovered errors\n");

			if (GETBYTE(2) & 0x02)
			 printf("\tStop data phase after recovered errors\n");
			else
			 printf("\tDon't stop after recovered errors\n");

			if (GETBYTE(2) & 0x01)
			 printf("\tUse ECC for error detection only\n");
			else
			 printf("\tUse ECC for error correction\n");
		}
		P(3,printf("\tVerify Retry Count:\t\t%ld\n",GETBYTE(3)))
		P(4,printf("\tVerify Correction Span:\t\t%ld\n",GETBYTE(4)))
		P(11,printf("\tVerify Recovery Time Limit:\t%ldms\n",
			GETWORD(10)))
		break;

	case 8:
		printf("Caching page:\n");
		NORMAL_LEN(0x0a)

		if (LEN(2))
		{
			if (GETBYTE(2) & 0x04)
				printf("\tWrite caching enabled\n");
			if (GETBYTE(2) & 0x02)
				printf("\tPrefetch sizes are multiples\n");
			if (GETBYTE(2) & 0x01)
				printf("\tRead cache disabled\n");		
		}
		if (LEN(3))
		{
			int readpri  = GETBYTE(3) >> 4;
			int writepri = GETBYTE(3) & 0x0f;

			printf("\tRead Retention Priority: 0x%02x\n",readpri);
			if (readpri == 0)
			    printf("\t\tNo distinction\n");
			else if (readpri == 1)
			    printf("\t\tAlready read data is low priority\n");
			else if (readpri == 0x0f)
			    printf("\t\tAlready read data has high priority\n");

			printf("\tWrite Retention Priority: 0x%02x\n",writepri);
			if (writepri == 0)
			    printf("\t\tNo distinction\n");
			else if (writepri == 1)
			    printf("\t\tWritten data is low priority\n");
			else if (writepri == 0x0f)
			    printf("\t\tWritten data has high priority\n");
		}

		P(5,printf("\tDisable Prefetch Transfer Length:\t%ld\n",
			GETWORD(4)))
		P(7,printf("\tMinimum Prefetch:\t\t\t%ld\n",
			GETWORD(6)))
		P(9,printf("\tMaximum Prefetch:\t\t\t%ld\n",
			GETWORD(8)))
		P(11,printf("\tMaximum Prefetch Ceiling:\t\t%ld\n",
			GETWORD(10)))
		break;

	case 0x09:
		printf("Peripheral Device page:\n");
		/* no normal size */

		if (LEN(3))
		{
			unsigned long interface = GETWORD(2);

			if (interface <= 4)
				printf("\tInterface: %s\n",
					interfaces[interface]);
			else {
				if (interface < 0x8000)
					printf("\tInterface reserved!\n");
				else
				      printf("\tVendor-specific interface\n");
			}
		}
		break;

	case 0x0a:
		printf("Control Mode page:\n");
		NORMAL_LEN(0x06)

		if (LEN(2))
			if (GETBYTE(2) & 0x01)
				printf("\tReport log exceptions\n");
		if (LEN(3))
		{
			int queue = GETBYTE(3) >> 4;

			switch (queue) {
			case 0:
			 printf("\tNo reordering: guarantee data integrity\n");
				break;
			case 1: printf("\tUnrestricted re-ordering allowed\n");
				break;
			case 2: case 3: case 4: case 5: case 6: case 7:
				printf("\tQueue type reserved!\n");
				break;
			default:
				printf("\tVendor-specific queue type\n");
			}

			if (GETBYTE(3) & 0x02)
	printf("\tAbort remaining commands in queue after command error\n");
			else
	printf("\tContinue remaining commands in queue after command error\n");

			if (GETBYTE(3) & 0x01)
				printf("\tTagged queuing disabled\n");
			else
				printf("\tTagged queuing enabled\n");

		}
		if (LEN(4))
		{
			if (GETBYTE(4) & 0x80)
			  printf("\tExtended Contingent Allegiance enabled\n");
			else
			 printf("\tExtended Contingent Allegiance disabled\n");

			if (GETBYTE(4) & 0x04)
		 printf("\tTarget may do AEN after initialization\n");
			else
		 printf("\tTarget doesn't do AEN after initialization\n");

			if (GETBYTE(4) & 0x02)
		 printf("\tTarget may do AEN to report a Unit Attention\n");
			else
		 printf("\tTarget doesn't do AEN for a Unit Attention\n");

			if (GETBYTE(4) & 0x01)
		 printf("\tTarget may do AEN to report deferred errors\n");
			else
		 printf("\tTarget doesn't do AEN to report deferred errors\n");


		}
		P(7,printf("\tReady AEN Holdoff Period:\t%ldms\n",
			GETWORD(6)))
		break;

	case 0x0b:
		printf("Medium Types supported page:\n");
		NORMAL_LEN(0x0b)

		P(4,printf("Medium type one supported:\t%ld\n",GETBYTE(4)))
		P(5,printf("Medium type two supported:\t%ld\n",GETBYTE(5)))
		P(6,printf("Medium type three supported:\t%ld\n",GETBYTE(6)))
		P(7,printf("Medium type four supported:\t%ld\n",GETBYTE(7)))
		break;

	case 0x0c:
		{
		int notch_type = 0;

		printf("Notch page:\n");
		NORMAL_LEN(0x16)

		if (LEN(2))
		{
			if (GETBYTE(2) & 0x80)
				printf("\tNotched\n");
			else
				printf("\tno Notches\n");
			if (GETBYTE(2) & 0x40)
			 printf("\tNotch boundaries defined by Logical Blocks\n");
			else
			 printf("\tNotch boundaries defined by physical parameters\n");

			notch_type = GETBYTE(2) & 0x40;
		}
		P(5,printf("\tMaximum number of Notches:\t%ld\n",
			GETWORD(4)))
		P(7,printf("\tActive Notch:\t\t%ld\n",
			GETWORD(6)))
		if (notch_type == 0)
		{
		  P(11,printf("\tStarting Boundary:\tcyl %ld, head %ld\n",
			GET3BYTES(8),GETBYTE(11)))
		} else {
		  P(11,printf("\tEnding Boundary:\tblock %ld\n",
			GETLONG(8)))
		}
		if (notch_type == 0)
		{
		  P(15,printf("\tStarting Boundary:\tcyl %ld, head %ld\n",
			GET3BYTES(12),GETBYTE(15)))
		} else {
		  P(15,printf("\tStarting Boundary:\tblock %ld\n",
			GETLONG(12)))
		}

		if (LEN(3))
		{
			int byte,bit,pagenum;

			printf(
		"\tThe following pages have parameters that can be notched:\n");

			pagenum = 0;
			for (byte = 23; byte >= 16; byte--)
			{
				for (bit = 0; bit <= 7; bit++)
				{
					if (GETBYTE(byte) & (1<<bit))
						printf("\t\tPage %ld\n",
							pagenum);
					pagenum++;
				}
			}
		}
		}
		break;

	default:
		printf("Non-standard page\n");
		dump = TRUE;
	}
	printf("\n");

	if (dump)
	{
		for (i = 0; i <= GETBYTE(1)+1; i++)
		{
			printf("Byte %2ld: %02lx\n",i,GETBYTE(i));
		}
	}
}

