/*
 * Diagnostic for the Commodore-Amiga Hard Disk Controller
 * 
 * Uses Commodore's SASI-like command block structure.
 * October 29, 1985 (lce)
 *
 */
#include <intuition/intuition.h>
#include "ddefs.h"
#include <libraries/filehandler.h>

extern	void	check_disk_type();
	char	xx;
static	char	erropt = 0;
static	unsigned long boot_block = 0L; /* # of 1st good block on disk */
extern	char	ram_prompt;	/* Not 0 means need to prompt for ram address */
extern	long	next_good();	/* Get next good block			      */
extern	int	soft_error();	/* Returns TRUE if not an error		      */
extern	void	write_block();
extern	void	wdwait();
extern	char	getch();
extern	struct	Window *Window;
extern	struct	MsgPort intPort;
extern	char	server_active;
extern	ULONG   WaitMask;
extern	int	delay_val;		/* Time to wait for next command */
extern	int	inKey, kcnt, scnt;	/* Defined & used by iortns.c */
extern	struct	MsgPort consoleMsgPort;
extern	struct	MsgPort timerMsgPort;
extern	struct	MsgPort HdMsgPort;
extern	int	findlong();
extern	int	HdError;
WDCMD	*CMDBLKPADDR;			/* Point to Command block */
char	*LONGBUFPADDR;			/* Big buffer pointer */
char	*BUFPADDR;			/* Single block buffer pointer*/
WDCMD	xCMDBLKPADDR;		/* Allocate space for copy of Command block */
char	xBUFPADDR[512];			/* Single block buffer */
static	char	xLONGBUFPADDR[69632];	/* Big buffer - 8 heads supported*/
static	int interleave = INTERLEAVE;
char	record_bad_blocks = 1;
int	wdread();
int	wdwrite();
int	wdblock();
unsigned pass = 0;
long bad_blocks[300];			/* bad block list */
int start_cylinder;			/* First cylinder to be tested */
long start_block;			/* First block to be tested */
int idx_bad_blocks = -1;
ERR_LIST err_list[MAX_HISTORY];		/* Error history */
int idx_err_list = -1;
int stop_test = 0;			/* Not 0 means <ctrl-c> typed */
char	stop_on_comp_error = 0;		/* Stop when compare error if true */
int abort=0;				/* Not 0 means exit program */
	BOOTINFO bootinfo;		/* Boot sector for selected disk */

WDINFO	wdinfo[] = {			/* add entries as needed */
	{ SIXTEENuS, (2<<4)|(2047/256), 2047%256, 1023/16, 1023/16, NSEC,
	  1, "User defined" },
	{ SIXTEENuS, (2<<4)|(612/256), 612%256, 128/16, 128/16, NSEC,
	  613, "2 head 10MB" },
	{ SIXTEENuS, (4<<4)|(306/256), 306%256, 128/16, 128/16, NSEC,
	  307, "4 head 10MB" },
	{ SIXTEENuS, (4<<4)|(612/256), 612%256, 128/16, 128/16, NSEC,
	  652, "4 head 20MB Miniscribe" },
	{ SIXTEENuS, (4<<4)|(615/256), 615%256, 300/16, 300/16, NSEC,
	  0, "4 head 20MB Seagate" },
	{ SIXTEENuS, (4<<4)|(20/256), 20%256, 20/16, 20/16, NSEC,
	  20, "4 head 20 cylinder test disk" },
	{ SIXTEENuS, (7<<4)|(704/256), 704%256, 704/16, 704/16, NSEC,
	  704, "7 head 42MB ATASI" },
	{ SIXTEENuS, (5<<4)|(977/256), 977%256, 977/16, 977/16, NSEC,
	  977, "5 head 40MB Seagate" }
};

char	*wdmsgs[] = {
	"\nOption Function\n",
	"-------------------------------\n",
#ifdef	DO_RAMTEST
	"0	RAM TEST\n", 
#endif
	"1,2	track format,format verification\n",
	"3,4,5	block write, block read, random read\n",
	"6,7	cylinder read, cyl write/read/compare\n",
	"8,9	pseudo-random data write, read\n",
	"b	add block to bad-block list\n",
	"d,c,l	display, clear bad-block list, err history\n",
#ifdef	ALLOW_INIT
	"i,t	initialize boot block, write bad block table\n",
#endif
	"T	Test for bad blocks\n",
	"o	add cyl-head-offset to bad-block list\n",
	"r,R	Disable, enable bad block recording\n",
	"L,S,s	list error history, Stop/don't stop if comp. err\n",
	"    Pattern Selection:\n",
	"  A = [00-FF], B = [FF], C = [FFxx], D = [xxFF], G = [00]\n",
	"  E = shifting [6DB6] F = block number (F: 3,4,5 only)\n",
	"  H = Different pattern per sector for '7'\n",
	"Type a <ctrl-c> to stop a test\n",
	"Enter desired options: "
};

int	passlimit;			/* 0 means no limit */
int	timeouts;			/* number of timeouts */
int	cmd_index;
char	commands[100];			/* requested commands */
int	total_errors;			/* total errors all passes */
int	errors;				/* total # of errors curent pass */
int	s_errors;			/* total # of soft errors curent pass */
int	typeno;				/* drive type selected from table */
int	unit=1;				/* which physical drive ? */
int	nsec;				/* # of sectors per track */
int	ntrk;				/* # of tracks per cylinder */
int	ncyl;				/* # of cylinders */
long	nblk;				/* number of blocks on device */
long	nblkcyl;			/* number of blocks on a cylinder */
char	databuf[512];
char	patno = 4;			/* selected pattern number */
char	shift_count;			/* patt_fill uses for pattern shift */
char	rd_error;			/* Set to 1 by read error */
char	geterrbits();

/*
 * simple test routine to try and format the hard disk using the new
 * Western Digital controller for the Commodore Z8000HR...
 */
void test()
{
	register long l;
	register DISK_TYPE *wdp;
	register char	**strp;
#ifdef	ALLOW_INIT
	register int	i;
#endif

	CMDBLKPADDR = &xCMDBLKPADDR;	/* Initialize storage pointers */
	LONGBUFPADDR = &xLONGBUFPADDR;
	BUFPADDR = &xBUFPADDR;
	WaitMask =(1 << consoleMsgPort.mp_SigBit) |
	(1 << Window->UserPort->mp_SigBit) |
	(1 << timerMsgPort.mp_SigBit);

	unit=1;
	if (sizeof(BOOTINFO) != 512)	/* check for valid sector definition */
	{
		puts("\n\nINVALID BOOT BLOCK DEFINITION!!!\nSize is ");
		putint(sizeof(BOOTINFO));
		puts("\n\n");
	}

/*	do { */
		puts("Select drive unit number [");
		putint(unit);
		puts("]: ");
		unit = getint(unit);
/*		if (unit < 0 || unit > 9)
			puts("Illegal unit number!\n");
	} while (unit < 0 || unit > 9); */

	if ((HdError = HdOpen(unit+1)) != NULL)
	{
		puts("\nHddisk open failed\n");
		cleanup();
	}
	else
	{

	unit = unit % 10;
	if (unit > 1) unit -= 2;	/* Adjust for SCSI devices */
	WaitMask = WaitMask | (1 << HdMsgPort.mp_SigBit);

	check_disk_type();	/* determine what type of disk is present */
	wdp = &wdinfo[typeno].p_type;
	nsec = wdp->d_sectors;
	ntrk = wdp->d_headhicyl >> 4;		/* get number of heads */
	ncyl = (((int)wdp->d_headhicyl & 0x0F) << 8) + wdp->d_cyl; /* cylinders */
	nblkcyl = nsec * ntrk;			/* fits in an int */
	nblk = nblkcyl * (long)ncyl;		/* fits in a long */
	puts("\nWhat cylinder should these tests start at: [0]: ");
	start_cylinder = getint(0);
	start_block = start_cylinder * nblkcyl;
	puts("\nInterleave factor 1:? [");
	putint(interleave);
	puts("]:");
	interleave = getint(interleave);
	delay_val=30;		/* Medium delay for initialize command */
	wdsetparam();

  do {	/* loop 1 */

	long block_number;
	int cyl;	/* cylinder number */
	int trk;	/* track number */
	int offset;	/* offset from index */

	total_errors = pass = timeouts = 0;
	ram_prompt = 1;
	stop_test = 0;
	passlimit = 1;	/* default is 1 */
	puts("Number of passes [");
	if (passlimit)
		putint(passlimit);
	else
		puts("unlimited");
	puts("]: ");
	passlimit = getint(passlimit);
	puts("Disk parameters:\n  unit=");
	putint(unit);
	puts(" sectors=");
	putint(nsec);
	puts(" heads=");
	putint(ntrk);
	puts(" cylinders=");
	putint(ncyl);
	puts(" interleave=");
	putint(interleave);
	puts(" passes=");
	if (passlimit)
		putint(passlimit);
	else
		puts("unlimited");
	for (strp = wdmsgs; strp <
		&wdmsgs[(sizeof(wdmsgs)/sizeof(wdmsgs[0]))]; strp++)
		puts(*strp);
	gets(commands);
	puts("\n");

    if (!abort) do { /* loop 2 */
	errors = s_errors = 0;	/* reset error count for this pass */
	shift_count = 0; /* reset pattern shift count */
	for (cmd_index = 0; (commands[cmd_index] && !stop_test && !abort);
			cmd_index++)
	{
	switch (commands[cmd_index]) {

#ifdef DO_RAMTEST
	case '0':
		puts("Starting RAM test\n");
		RAMTEST();
		break;
#endif
	case '1':
		puts("Starting disk format\n");
		for (cyl = start_cylinder; cyl < ncyl; ++cyl)
			for (trk = 0; (trk < ntrk) && (!check_stop_test()); ++trk)
				wdfmttrk(trk, cyl);
		break;

	case '2':
		puts("Starting verify phase:\n");
		for (cyl = start_cylinder; cyl < ncyl; ++cyl)
			for (trk = 0; (trk < ntrk) && (!check_stop_test()); ++trk)
				wdvfytrk(trk, cyl);
		break;
	case '3':
		puts("Starting block write phase\n");
		shift_count++;
		if (patno != 5)  /* gen pattern to write */
			patt_fill((char *)BUFPADDR,512L,0x0L);
		/* do sector interleaving */
		for (cyl = start_cylinder; (cyl < ncyl) && (!check_stop_test()); ++cyl)
		    for (trk = 0; (trk < ntrk) && !check_stop_test(); ++trk)
		    {
			long blk_offs;	/* block # of 1st block on track */
			int int_offs;	/* block # offset this interleave */

			blk_offs = (nsec * (long)trk) + (nblkcyl * (long)cyl);
			for (int_offs = 0;
				(int_offs < interleave) && !check_stop_test();
				 ++int_offs)
			    for (l = blk_offs + int_offs;
			    	 (l < blk_offs + nsec) && !check_stop_test();
				  l += interleave)
			    {
				if (patno == 5) /* write block # if patno 5*/
					patt_fill((char *)BUFPADDR,512L,l);
				puts(" ");
				putlong(l);
				puts("  \r");
				wdwrite(l);
			    }
		    }
		break;

	case '4':
		puts("Starting block read phase\n");
		if (patno != 5)		/* gen pattern to be read */
			patt_fill(databuf,512L,0x6DB6DB6DL);
		/* do sector interleaving */
		for (cyl = start_cylinder; (cyl < ncyl) && (!check_stop_test()); ++cyl)
		    for (trk = 0; (trk < ntrk) && !check_stop_test(); ++trk)
		    {
			long blk_offs;	/* block # of 1st block on track */
			int  int_offs;	/* block # offset this interleave */

			blk_offs = (nsec * (long)trk) + (nblkcyl * (long)cyl);
			for (int_offs = 0;
			    (int_offs < interleave) && !check_stop_test();
			     ++int_offs)
			    for (l = blk_offs + int_offs;
			    	 (l < blk_offs + nsec) && !check_stop_test();
				  l += interleave)
			    {
				if (patno == 5) /* write block # if patno 5*/
					patt_fill(databuf,512L,l);
				puts(" ");
				putlong(l);
				puts("  \r");
				wdread(l);
			    }
		    }
		break;

	case '5':
		puts("Starting random read phase\n");
		if (patno != 5) /* gen pattern to be read */
			patt_fill(databuf,512L,0x6DB6DB6DL);
		for (l = 0; (l < 1000L) && (!check_stop_test()); ++l)
		{
			long blockno;

			do {
				blockno = ((long)rand()) % nblk;
			} while (blockno < start_block);
			if (patno == 5)
				patt_fill(databuf,512L,blockno);
			puts(" ");
			putlong(blockno);
			puts("      \r");
			wdread(blockno);
		}
		break;
	case '6':
		puts("Starting long read phase - ");
		putint(nblkcyl);
		puts(" sectors/read\n");
		patt_fill(databuf,512L,0x6DB6DB6DL);/* gen pattern to be read */
		for (l = start_cylinder; (l < (long)ncyl) &&
			(!check_stop_test()); ++l)
			wdlread(l*nblkcyl);
		break;

	case 'T':
		puts("Starting long scan phase - ");
		putint(nblkcyl);
		puts(" sectors/read\n");
		patt_fill(databuf,512L,0x6DB6DB6DL);/* gen pattern to be read */
		for (l = start_cylinder; (l < (long)ncyl) &&
			(!check_stop_test()); ++l)
			wdlscan(l*nblkcyl);
		break;

	case '7':
		puts("Starting cylinder write/read/compare phase\n");
		shift_count++;
		rd_error = 1; /* rd_error causes write pattern to be renewed */
		for (l = start_cylinder;
			(l < (long)ncyl) && (!check_stop_test()); ++l)
			wdcrdwr(l*nblkcyl,(long)ncyl);
		break;

	case '8':
		puts("Starting pseudo-random write phase\n");
		srand(0);		/* seed for random # generator */
		for (l = start_block; (l < nblk) && (!check_stop_test()); ++l)
			wdprwrite(l);
		break;

	case '9':
		puts("Starting pseudo-random read phase\n");
		srand(0);		/* seed for random # generator */
		for (l = start_block; (l < nblk) && (!check_stop_test()); ++l)
			wdprread(l);
		break;
	case 'b':	/* add block to bad_blocks */
		do
		{
		    puts("\nEnter block number : ");
		    block_number = getlong(-1L);
		    if (block_number != -1L)
		    {
			puts("Mark block ");
			putlong(block_number);
			puts(" bad? (y/n) : ");
			xx = getch();
			xx = toupper(xx);
			if (xx == 'Y')
			    addlong(block_number,bad_blocks,&idx_bad_blocks,
				((sizeof(bad_blocks)/sizeof(bad_blocks[0]))-1));
		    }
		} while (block_number != -1L);
		break;

	case 'c':	/* clear bad_blocks list */
		idx_bad_blocks = -1;
		puts("Bad-block list cleared\n");
		break;

	case 'r':	/* Don't add bad blocks to list */
		record_bad_blocks = 0;
		break;

	case 'R':	/* Add bad blocks to list */
		record_bad_blocks = 1;
		break;

	case 'd':	/* display bad_blocks list */
		display_bad(bad_blocks,idx_bad_blocks);
		break;

	case 'S':	/* Stop if compare error detected */
		stop_on_comp_error++;
		break;

	case 's':	/* Don't stop if compare error detected */
		stop_on_comp_error = 0;
		break;

	case 'L':	/* display error history */
		display_history();
		break;

	case 'l':	/* clear error history */
		clear_hist();
		break;

#ifdef	ALLOW_INIT
	case 't':	/* Write bad block table to disk */
		write_bad_block_table();
		break;

	case 'i':	/* Write disk info to 1st good block */
		puts("\n");

		i = 0;
		while (findlong(i,bad_blocks,idx_bad_blocks) != -1)
			++i;	/* Skip any bad blocks */
		write_block(i,&bootinfo);
		i = geterrbits();
		if (i & 0x7f)
			derror("info block write",i);
		break;
#endif

	case 'o':	/* add block to bad_blocks */
		block_number = cyl = trk = offset = 0;
		do {
		    puts("\nEnter cylinder : ");
		    cyl = getint(-1);
		    if (cyl != -1)
		    {
			puts("Enter head : ");
			trk = getint(0);
			puts("Enter offset : ");
			offset = getint(0);
			block_number = block_from_offset(cyl,trk,offset);
			puts("Mark block number ");
			putlong(block_number);
			puts("\n  at cylinder ");
			putint(cyl);
			puts(" track ");
			putint(trk);
			puts(" offset ");
			putint(offset);
			puts(" bad? (y/n) : ");
			xx = getch();
			xx = toupper(xx);
			if (xx == 'Y')
			    addlong(block_number,bad_blocks,&idx_bad_blocks,
				((sizeof(bad_blocks)/sizeof(bad_blocks[0]))-1));
		    }
		} while (cyl != -1);
		break;

	case 'x':	/* exit program */
	case 'q':	/* exit program */
		abort = 1;
		break;
	case 'A':	/* select pattern 00-FF */
		patno = 0;
		break;
	case 'B':	/* select pattern FF */
		patno = 6;
		break;
	case 'C':	/* select pattern FFxx */
		patno = 2;
		break;
	case 'D':	/* select pattern xxFF */
		patno = 3;
		break;
	case 'E':	/* select pattern shifting 6DB6 */
		patno = 4;
		break;
	case 'F':	/* select pattern shifting 6DB6 */
		patno = 5;
		break;
	case 'G':	/* select pattern 00 */
		patno = 1;
		break;
	case 'H':	/* select pattern 00 */
		patno = 7;
		break;
	default:	/* illegal command */
		puts("\nIllegal Command!\n");
		break;

	}	/* end switch */
	}
	total_errors += errors + s_errors;
	puts("\nUnit ");
	putint(unit);
	puts(": pass count = ");
	putint(++pass);
	puts("\n   timeouts = ");
	putint(timeouts);
	puts(", hard errors this pass =");
	putint(errors);
	puts(", soft errors this pass =");
	putint(s_errors);
	puts(",\n   total errors =");
	putint(total_errors);
	puts("\n");
    } while ((!abort) && (!check_stop_test()) &&
		(!passlimit || (pass < passlimit)));
  } while (!abort); /* loop 1 */
  }
}

char geterrbits()
{
	register WDCMD *cbp;

	cbp = CMDBLKPADDR;
	return(cbp->c_errorbits);
}

void select_d_type(init_boot)	/* ask user what type drive he has */
char init_boot;			/* if non-zero, reset all fields of boot */
{
	register int i;
	register long	*wp;
	register DISK_TYPE *wdp;
	register DISK_INFO *btp;
	register long	*env;

	typeno = 3;
	puts("Drive types:\n");
	for (i = 0; i < (sizeof(wdinfo)/sizeof(wdinfo[0])); ++i)
	{
		putint(i);
		puts(") ");
		puts(wdinfo[i].p_devname);
		puts("\n");
	}  /* end for */
	do {
		puts("Select drive type [");	/* show default */
		putint(typeno);
		puts("]: ");
		typeno = getint(typeno);	/* get type from console */
		if (typeno > (sizeof(wdinfo)/sizeof(wdinfo[0])))
			puts("Illegal type number!\n");
	} while (typeno > (sizeof(wdinfo)/sizeof(wdinfo[0])));

		/* initialize boot block */

	for (wp=(long *)&bootinfo,i=0;i < 128;i++)
		*wp++ = 0;	/* Zero out block */

	wdp = &wdinfo[typeno].p_type;
	btp = &bootinfo.b_disk_info;

	if (!typeno)	/* user wants to specify parameters */
	{
		puts("\nNumber of heads [4]: ");
		btp->ds_heads = getint(4);
		wdp->d_headhicyl = btp->ds_heads << 4;
		puts("Number of cylinders [1] : ");
		btp->ds_cylinders = ncyl = getint(1);
		wdp->d_headhicyl = wdp->d_headhicyl + (ncyl >> 8);
		wdp->d_cyl = ncyl;
		puts("Number of sectors per track [");
		putint(NSEC);
		puts("]:");
		btp->ds_sectors = wdp->d_sectors = getint(NSEC);
		puts("Write pre-comp cylinder [");
		putint(ncyl);
		puts("]:");
		btp->ds_reduce = btp->ds_precomp = getint(ncyl);

		wdp->d_precomp = btp->ds_precomp/16;
		wdp->d_optstep = 0x4F;
		puts("\nCylinder to park heads at for shipment [");
		putint(ncyl);
		puts("]:");
		btp->ds_park = wdp->d_park_cyl = getint(ncyl);
	}
	else {
		btp->ds_cylinders =
			((wdp->d_headhicyl & 0x0F) << 8) | wdp->d_cyl;
		btp->ds_precomp = wdp->d_precomp * 16;
		btp->ds_park = wdp->d_park_cyl;
		btp->ds_sectors = wdp->d_sectors;
		btp->ds_heads = wdp->d_headhicyl >> 4;
	}

		/* copy information to boot block */

	bootinfo.b_magic = 0xBABE;	/* MAGIC */
		/* Set AmigaDOS environment for 1st partition "constants" */
	
	env = &bootinfo.b_environment;
	env[DE_TABLESIZE] 	= 11;
	env[DE_SIZEBLOCK] 	= 128;
	env[DE_SECORG] 		= 0;
	env[DE_SECSPERBLK]	= 1;
	env[DE_RESERVEDBLKS]	= 2;
	env[DE_LOWCYL]		= 2;	/* 1st two cylinders used for bad blks*/
	env[DE_NUMBUFFERS]	= 30;	/* Can always ADDBUFERS */
	env[DE_MEMBUFTYPE]	= 0;	/* FAST memory is fine	*/
	env[DE_NUMHEADS]	= btp->ds_heads;
	env[DE_BLKSPERTRACK]	= env[DE_PREFAC] = btp->ds_sectors;
	ncyl = (((int)wdp->d_headhicyl & 0x0F) << 8) + wdp->d_cyl; /* cylinders */
	puts("\nLast cylinder used by first partition [");
	putint(ncyl-1);
	puts("]:");
	env[DE_UPPERCYL]	= getint(ncyl-1);

}

void check_disk_type()	/* determine what type of disk is present */
{
	register char *cp;
	register char *bp;
	register DISK_TYPE *wdp;
	register DISK_INFO *btp;
	register int i;
	char d_type_known;		/* Non-zero if disk type known */
	char init_boot;			/* flag indicating no valid */
					/* boot sector existed */

	d_type_known = 0;	/* don't know the drive type yet */	
	wdp = &wdinfo[0].p_type; /* point to correct drive information */
	errors = 0;
	delay_val=30;		/* Medium delay for initialize command */

		/* Read in 1st good block - try 30 times */

	boot_block = 0L;
	do {
		initcmdblk(WDREAD,1,BUFPADDR,boot_block);
		wdwait();			/* wait for i/o complete */
		i = geterrbits();
		boot_block++;
	} while ((boot_block < 31L) &&	/* Repeat until good read or > 30 */
		 (!soft_error(i)));
	if (init_boot = soft_error(i))
	{	/* read successful, check id */
		for (cp = (char *)BUFPADDR, bp = &bootinfo, i = 0;
		     i < sizeof(bootinfo); i++)
			*bp++ = *cp++;	/* copy buffer to bootinfo */
		if (d_type_known = (bootinfo.b_magic == (short)0xBABE))
		{	/* Boot sector valid, extract needed informatiomn */
			/* step rate & options, show no retry on errors */
		    init_boot = 0;	/* preserve boot block data */
		    wdp = &wdinfo[0].p_type;
		    btp = &bootinfo.b_disk_info;
		    wdp->d_optstep =  0x4F;
			/* heads <6:4>, hi cyl count <3:0> */
		    wdp->d_headhicyl =	(btp->ds_heads << 4) |
					(btp->ds_cylinders >> 8);
			/* low byte of cyl count */
		    wdp->d_cyl = btp->ds_cylinders;
			/* precomp cyl / 16 */
		    wdp->d_precomp = btp->ds_precomp / 16;
			/* reduced write current / 16 */
		    wdp->d_reduce = btp->ds_precomp / 16;
			/* sectors per track */
		    wdp->d_sectors = btp->ds_sectors;
			/* cylinder to park heads at */
		    wdp->d_park_cyl = btp->ds_park;
		}
	}
	if (d_type_known)
	{
		puts("Drive type parameters set from block 0.\n");
		puts("   Do you want to change them? (y/n): [n] ");
		xx = getch();
		xx = toupper(xx);
		d_type_known = (xx != 'Y');
		puts("\n");
	}
	if (!d_type_known)		/* need to select drive type */
		select_d_type(init_boot); /* ask user what type drive he has */
		
} /* end check_disk_type */
