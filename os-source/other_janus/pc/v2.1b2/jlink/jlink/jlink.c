#include <stdio.h>
#include <ctype.h>
#include <bios.h>

#include <exec/types.h>
#include "..\..\dslib\dslib.h"

#include "..\include\jdisk.h"

#define D(x)

char filename[256] = "";	/* amiga filename */
int driveid = -1;		/* ms-dos drive letter (a: = 0, b: = 1, ...) */
int switch_u = 0;		/* 'delink' switch */
int switch_r = 0;		/* 'read only' switch */
int switch_n = 0;		/* 'no messages' switch */
int switch_c = 0;		/* 'create' switch */
int switch_d = 0;		/* 'debug' switch */
long disksize = 0;		/* size of disk in kbytes for create */

struct Ident id;
struct ILink il;

struct ILink *(ftable[4]);
struct ILink **ftbl;

/* dosserv/janus variables */
struct dslib_struct ds;
ULONG afile = 0;

/* first sector of a jlinked volume */
UBYTE BPB[512];
/*	0..2	UBYTE	BootJmp[3];	Near Jump To Boot Code (not needed) */
#define BPB_BootJmp	(BPB[0])
/*	3..10	UBYTE	OEMName[8];	ASCII Ident. */
#define BPB_OEMName	(BPB[3])
/*	11..12	UWORD	SectorLength;	Bytes Per Sector */
#define BPB_SectorLength	_WORD((BPB[11]))
/*	13.	UBYTE	ClusterSecs;	Sectors Per Cluster */
#define BPB_ClusterSecs	(BPB[13])
/*	14..15	UWORD	ResrvdSecs;	Reserved Sectors */
#define BPB_ResrvdSecs	_WORD((BPB[14]))
/*	16.	UBYTE	NumFATs;	Number of FAT's */
#define BPB_NumFATs	(BPB[16])
/*	17..18	UWORD	RootEntries;	Number of Root Dir Entries */
#define BPB_RootEntries	_WORD((BPB[17]))
/*	19..20	UWORD	NumSecs;	Number of Sectors in Log. Image */
#define BPB_NumSecs	_WORD((BPB[19]))
/*	21.	UBYTE	MediaDescr;	Media Descriptor */
#define BPB_MediaDescr	(BPB[21])
/*	22..23	UWORD	FATSecs;	Number of FAT Sectors */
#define BPB_FATSecs	_WORD((BPB[22]))
/*	24..25	UWORD	TrackSecs;	Sectors Per Track */
#define BPB_TrackSecs	_WORD((BPB[24]))
/*	26..27	UWORD	NumHeads;	Number of Heads */
#define BPB_NumHeads	_WORD((BPB[26]))
/*	28..20	UWORD	HiddenSecs;	Number of hidden Sectors */
#define BPB_HiddenSecs	_WORD((BPB[28]))
/*	30..511	UBYTE	pad[512-30];	rest of block */
#define BPB_pad		(BPB[30])

/* dos root dir block */
UBYTE ROOT[512];

/*	0..10	UBYTE	volname[11]; */
#define ROOT_volname	(ROOT[0])
/*	11.	UBYTE	attribute; */
#define ROOT_attribute	(ROOT[11])
/*	12..21	UBYTE	dosrsvd[10]; */
#define ROOT_dosrsvd	(ROOT[12])
/*	22..23	UWORD	time; */
#define ROOT_time	_WORD((ROOT[22]))
/*	24..25	UWORD	date; */
#define ROOT_date	_WORD((ROOT[24]))
/*	26..511	UBYTE	pad[512-26]; */
#define ROOT_pad	(ROOT[26])


/* first FAT sector */
UBYTE FAT[512];

/* a blank sector */
UBYTE emptysec[512];

#define _WORD(x) (*((UWORD *)(&(x))))

#define segment_of(x) ((((unsigned long)((x)))>>16) & 0xffff)
#define offset_of(x) (((unsigned long)((x))) & 0xffff)


void dumpbytes(void *ptr, int len)
{
unsigned char *p;

	p = ptr;
	while (len--) {
		D(printf(" %02x", *p++));
	}
	D(printf("\n"));
}

void quit(int retval)
{
	/* if the afile is open, close it */
	if (afile) {
		j_Close(&ds, afile);
	}

	/* if we've got dosserv open, close it */
	if (ds.sd_ds) {
		j_close_dosserv(&ds);

	}

	exit(retval);
}

int msdos_version()
{
union REGS r;

	r.h.ah = 0x30;
	int86(0x21, &r, &r);
	return r.h.al*100 + r.h.ah;
}

void msdos_reset_disks()
{
union REGS r;

	r.h.ah = 0x0d;
	int86(0x21, &r, &r);
}

USHORT get_date()
{
union REGS r;

	r.h.ah = 0x2a;
	int86(0x21, &r, &r);

	return ((((USHORT)r.x.cx) - 1980) << 9) + (((USHORT)r.h.dh) << 5) + (r.h.dl);
}

USHORT get_time()
{
union REGS r;

	r.h.ah = 0x2c;
	int86(0x21, &r, &r);

	return (((USHORT)r.h.ch) << 11) + (((USHORT)r.h.cl) << 5) + (r.h.dh >> 1);
}

int get_driver_state(struct Ident *id)
{
int i;
union REGS r;
struct SREGS sr;
UWORD *ft;

	ftbl = 0;

	/* scan drives c..z (a=1, b=2, c=3, ...) */
	for (i = 'c'-'a'+1; i < 'z'-'a'+1; i++) {
		segread(&sr);
		r.h.ah = 0x44;			/* DOS IOCTL */
		r.h.al = 4;			/* get status */
		r.h.bl = i;			/* from this unit */
		r.x.cx = sizeof(struct Ident);	/* into... */
		r.x.dx = offset_of(id);		/* this... */
		sr.ds = segment_of(id);		/* structure */
		int86x(0x21, &r, &r, &sr);
		/* if carry set (error) */
		if (r.x.cflag & 0x1) {
			/* and error == 0fh (illegal drive) */
			if (r.x.ax == 0x0f) {
				/* stop scanning */
				return 1;
			}
		} else {
			/* no error - is this a JD device? */
			if (r.h.al == sizeof(struct Ident) && id->I_Marker == JD) {
				/* yes - set up ftbl and return ok status */
				ft = (UWORD *)((((ULONG)id->I_CS) << 16) | id->I_FTbl);
				ftbl = &ftable[0];
				for (i = 0; i < id->I_Units; i++) {
					if (ft[i])
						ftbl[i] = (struct ILink *)((((ULONG)id->I_CS) << 16) | ft[i]);
					else
						ftbl[i] = 0;
				}
				return 0;
			}
		}
	}

	/* never found a JD device */
	return 1;
}

int send_packet(int driveid)
{
union REGS r;
struct SREGS sr;
D(printf("handle = %d\n", il.IL_Handle));
	segread(&sr);
	r.h.ah = 0x44;				/* DOS IOCTL */
	r.h.al = 5;				/* send command */
	r.h.bl = driveid + id.I_BaseDrv + 1;	/* to this unit */
	r.x.cx = sizeof(struct ILink);		/* from... */
	r.x.dx = offset_of(&il);		/* this... */
	sr.ds = segment_of(&il);		/* structure */
	int86x(0x21, &r, &r, &sr);
	if (r.x.cflag & 0x1) {
		return 1;
	} else {
		return 0;
	}
}

void delink(int driveid)
{
union REGS r;
struct SREGS sr;

	if (ftbl[driveid] == 0) {
		fprintf(stderr, "Drive %c: is not linked\n", driveid + id.I_BaseDrv + 'A');
		quit(1);
	}

	msdos_reset_disks();

	il.IL_Command = IL_UnLink;
	il.IL_Handle = ftbl[driveid]->IL_Handle;

	if (send_packet(driveid)) {
		fprintf(stderr, "Error unlinking\n");
		quit(1);
	} 
}

void link(int driveid, char *filename, int switch_r)
{
	if (ftbl[driveid]) {
		fprintf(stderr, "Drive %c: is already linked\n",
			    driveid + id.I_BaseDrv + 'A');
		quit(1);
	}

	il.IL_Command = IL_Link;
	strcpy(il.IL_Name, filename);
	if (switch_r) {
		il.IL_Flag = ILF_RO;
	} else {
		il.IL_Flag = 0;
	}
	il.IL_Handle = j_Open(&ds, filename, MODE_OLDFILE);

	if (!il.IL_Handle) {
		fprintf(stderr, "couldn't open file %s\n", filename);
		quit(1);
	}

	afile = il.IL_Handle;
	il.IL_Handle--;

	if (send_packet(driveid)) {
		fprintf(stderr, "Error linking\n");
		quit(1);
	} 

	/* we don't own it anymore - the driver does */
	afile = 0;
}

int writezeros(ULONG afile, int nsects)
{
int i;

	for (i = 0; i < nsects; i++) {
		if (j_Write(&ds, afile, &emptysec[0], 512) != 512)
			return 1;
	}
	
	return 0;
}

int writefat(ULONG afile)
{
	if (j_Write(&ds, afile, &FAT[0], 512) != 512)
		return 1;

	return writezeros(afile, BPB_FATSecs - 1);
}

void create(int driveid, char *filename, int switch_r, long drivesize)
{
int i;
UWORD fatsize, clusters;

D(printf("driveid = %d, filename = %s, switch_r = %d, drivesize = %ld\n",
	driveid, filename, switch_r, drivesize));

	if (ftbl[driveid]) {
		fprintf(stderr, "Drive %c: is already linked\n", driveid + id.I_BaseDrv + 'A');
		quit(1);
	}

	if (il.IL_Handle = j_Open(&ds, filename, MODE_OLDFILE)) {
		j_Close(&ds, il.IL_Handle);
		fprintf(stderr, "File %s already exists\n", filename);
		quit(1);
	}

	if (!(il.IL_Handle = j_Open(&ds, filename, MODE_NEWFILE))) {
		fprintf(stderr, "Couldn't create file %s\n", filename);
		quit(1);
	}

	/* initialize BPBsect */
	memset(&BPB_BootJmp, 0, 3);
	memcpy(&BPB_OEMName, "Jdisk.00", 8);
	BPB_SectorLength = 512;
	BPB_ClusterSecs = 16;
	BPB_ResrvdSecs = 1;
	BPB_NumFATs = 2;
	BPB_RootEntries = 256;
	BPB_NumSecs = 0x0ffe0;
	BPB_MediaDescr = 0x0fb;
	BPB_FATSecs = 16;
	BPB_TrackSecs = 17;
	BPB_NumHeads = 4;
	BPB_HiddenSecs = 1;
	memset(&BPB_pad, 0, 512-30);

	/* initialize root block */
	memcpy(&ROOT_volname, "Jdisk V1.1 ", 11);
	ROOT_attribute = 8;
	memset(&ROOT_dosrsvd, 0, 10);
	ROOT_time = get_time();
	ROOT_date = get_date();
	memset(&ROOT_pad, 0, 512-26);

	/* initialize 1st FAT sector */
	FAT[0] = 0xfb;
	FAT[1] = 0xff;
	FAT[2] = 0xff;
	FAT[3] = 0xff;
	memset(&FAT[4], 0, 512-4);

	/* initialize empty sector */
	memset(&emptysec[0], 0, sizeof(emptysec));

	/* if we screw up, close the file */
	afile = il.IL_Handle;

	/* number of sectors (512 bytes each) = Kbytes * 2 */
	BPB_NumSecs = ((UWORD)drivesize) << 1;
D(printf("BPB_NumSecs = %u\n", BPB_NumSecs));

	/* sectors per cluster = sectors / 4096, truncated up */
	BPB_ClusterSecs = (((ULONG)BPB_NumSecs) + 4095) / 4096;
D(printf("BPB_ClusterSecs = %u\n", BPB_ClusterSecs));

	/* numsecs will be 320..65534 (160K..32767K disk).  this gives a sects
	 * per cluster of 0..16.  but sects per clust must be a power of 2 (1,
	 * 2, 4, 8, 16).  the loop below coerces to a power of 2.
	 */
	for (i = 1; i < 0x100; i = i << 1)
		if (i >= BPB_ClusterSecs)
			break;
	BPB_ClusterSecs = i;
D(printf("BPB_ClusterSecs = %u\n", BPB_ClusterSecs));

	/* # of FAT entries (clusters) = sectors / cluster size, truncated up */
	fatsize = (((ULONG)BPB_NumSecs) + BPB_ClusterSecs-1) / BPB_ClusterSecs;
D(printf("fatsize = %u\n", fatsize));

	/* convert # of fat entries to # of bytes for all fat entries */
	if (fatsize > 4085) {
		/* 16 bit fat (2 bytes each) */
		fatsize *= 2;
	} else {
		/* 12 bit fat (1.5 bytes each) */
		fatsize = fatsize + (fatsize >> 1) + (fatsize & 1);
		FAT[3] = 0;
	}
D(printf("fatsize = %u, FAT[3] = %u\n", fatsize, FAT[3]));

	/* sectors of FAT = bytes of FAT / 512, truncated up */
	BPB_FATSecs = (fatsize + 511) / 512;
D(printf("BPB_FATSecs = %u\n", BPB_FATSecs));

	clusters = BPB_FATSecs * 512;
D(printf("clusters = %u\n", clusters));

	if (FAT[3]) {
		/* 16 bit, so clusters = bytes of fat / 2 */
		clusters /= 2;
	} else {
		/* 12 bit, so clusters = bytes of fat / 1.5 */
		clusters = (clusters * 2) / 3;
	}
D(printf("clusters = %u\n", clusters));

	clusters -= 2;	/* 2 clusters lost */
D(printf("clusters = %u\n", clusters));

	/* sectors = clusters * sectors per cluster */
	BPB_NumSecs = clusters * BPB_ClusterSecs;
D(printf("BPB_NumSecs = %u\n", BPB_NumSecs));

	/* start at the beginning */
	if (j_Seek(&ds, afile, 0, OFFSET_BEGINNING) == -1) {
		fprintf(stderr, "Couldn't seek within file\n");
		quit(1);
	}

	/* write BPB sector */
	if (j_Write(&ds, afile, &BPB_BootJmp, 512) != 512) {
		fprintf(stderr, "Write error while creating file\n");
		quit(1);
	}

	/* write FAT 1 */
	if (writefat(afile)) {
		fprintf(stderr, "Write error while creating file\n");
		quit(1);
	}

	/* write FAT 2 */
	if (writefat(afile)) {
		fprintf(stderr, "Write error while creating file\n");
		quit(1);
	}

	/* write root dir block */
	if (j_Write(&ds, afile, ROOT, 512) != 512) {
		fprintf(stderr, "Write error while creating file\n");
		quit(1);
	}

	/* and an empty directory */
	if (writezeros(afile, 15)) {
		fprintf(stderr, "Write error while creating file\n");
		quit(1);
	}

	/* *now* link it. */
	il.IL_Command = IL_Link;
	strcpy(il.IL_Name, filename);
	if (switch_r) {
		il.IL_Flag = ILF_RO;
	} else {
		il.IL_Flag = 0;
	}

	il.IL_Handle--;
	
	/* tell driver */
	if (send_packet(driveid)) {
		fprintf(stderr, "Error linking\n");
		quit(1);
	} 

	/* we don't own it anymore - the driver does */
	afile = 0;
}

void report()
{
struct ILink *il;
int drive;

	get_driver_state(&id);

	printf("\nVDrive  Status  Linked to\n");
	printf("--------------------------------------------------\n");

	for (drive = 0; drive < id.I_Units; drive++) {
		printf("%c:", drive + id.I_BaseDrv + 'A');

		if (il = ftbl[drive]) {
			if (il->IL_Flag & ILF_RO)
				printf("\tR/O\t");
			else
				printf("\tR/W\t");

			printf("%s", il->IL_Name);
		}

		printf("%\n");
	}
}

void usage()
{
	fprintf(stderr, "JLINK Version 2.0 Usage:\n");
	fprintf(stderr, "\nJLINK\n");
	fprintf(stderr, "    Display list of currently JLINKed drives.\n");
	fprintf(stderr, "\nJLINK drive: amigafile [/R] [/N]\n");
	fprintf(stderr, "    Link MS-DOS drive 'drive:' to existing Amiga file 'amigafile'.\n");
	fprintf(stderr, "    If /R option is used, the MS-DOS drive will be read only.\n");
	fprintf(stderr, "    If /N option is used, the list of linked drives will not be displayed.\n");
	fprintf(stderr, "\nJLINK drive: amigafile /C[:kbytes] [/R]\n");
	fprintf(stderr, "    Create and initialize the Amiga file 'amigafile', then link MS-DOS drive\n");
	fprintf(stderr, "    'drive:' to it.  If ':kbytes' is given, the drive will be that number\n");
	fprintf(stderr, "    of kilobytes in size.  'kbytes' may range from 160 to 32767 (160K to\n");
	fprintf(stderr, "    32 megabytes).\n");
	fprintf(stderr, "    If /R option is used, the MS-DOS drive will be read only.\n");
	fprintf(stderr, "    If /N option is used, the list of linked drives will not be displayed.\n");
	fprintf(stderr, "\nJLINK drive: /U [/N]\n");
	fprintf(stderr, "	Unlink MS-DOS drive 'drive:' from the Amiga file it is linked to.\n");
	fprintf(stderr, "    If /N option is used, the list of linked drives will not be displayed.\n");
}

void main(int argc, char *argv[])
{
int i, j, c;
char *p;

	if (msdos_version() < 300) {
		fprintf(stderr, "Need DOS 3.0 or higher\n");
		quit(1);
	}

	if (j_tickle_janus()) {
		fprintf(stderr, "Janus handler not loaded\n");
		quit(1);
	}

	if (j_open_dosserv(&ds)) {
		fprintf(stderr, "DOSServ service not available\n");
		quit(1);
	}

	if (get_driver_state(&id)) {
		fprintf(stderr, "JDISK.SYS not installed\n");
		quit(1);
	}

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '/') {
			c = argv[i][1];
			if (isupper(c))
				c = tolower(c);
			switch (c) {
			case 'u':
				if (argv[i][2] != 0) {
					fprintf(stderr, "Illegal switch\n");
					quit(1);
				}
				switch_u = 1;
				break;
			case 'r':
				if (argv[i][2] != 0) {
					fprintf(stderr, "Illegal switch\n");
					quit(1);
				}
				switch_r = 1;
				break;
			case 'n':
				if (argv[i][2] != 0) {
					fprintf(stderr, "Illegal switch\n");
					quit(1);
				}
				switch_n = 1;
				break;
			case 'c':
				if (argv[i][2] == 0) {
					disksize = 32767;	/* kbytes */
				} else if (argv[i][2] == ':') {
					disksize = atol(&argv[i][3]);
					if ((disksize < 160) || (disksize > 32767)) {
						fprintf(stderr, "Illegal disk size\n");
						quit(1);
					}
				}
				switch_c = 1;
				break;
			case 'd':
				if (argv[i][2] != 0) {
					fprintf(stderr, "Illegal switch\n");
					quit(1);
				}
				switch_d = 1;
				break;
			default:
				fprintf(stderr, "Illegal switch\n");
				quit(1);
				break;
			}
		} else {

			/* either a drive letter (x:) or a filename (["]name["]) */
			if (driveid == -1) {
				if (strlen(argv[i]) != 2) {
					fprintf(stderr, "Illegal MS-DOS drive ID\n");
					quit(1);
				}
			
				if (argv[i][1] != ':') {
					fprintf(stderr, "Illegal MS-DOS drive ID\n");
					quit(1);
				}
			
				c = argv[i][0];
				if (isupper(c))
					c = tolower(c);

				if (c < 'a' || c > 'z') {
					fprintf(stderr, "Illegal MS-DOS drive ID\n");
					quit(1);
				}

				driveid = c - 'a';
				if ((driveid < id.I_BaseDrv) || (driveid >= id.I_BaseDrv + id.I_Units)) {
					fprintf(stderr, "Illegal JLINK drive ID\n");
					quit(1);
				}

				driveid -= id.I_BaseDrv;
			} else if (filename[0] == 0) {
				p = argv[i];
				if (*p == '"') {
					p++;
					if (*p == 0) {
						fprintf(stderr, "Illegal Amiga filename\n");
						quit(1);
					}
					j = strlen(p) - 1;
					if (p[j] != '"') {
						fprintf(stderr, "Illegal Amiga filename\n");
						quit(1);
					}
					p[j] = 0;
				}
				if (strchr(p, '"')) {
					fprintf(stderr, "Illegal Amiga filename\n");
				}
				if ((strlen(p) > 255) || (strlen(p) == 0)) {
					fprintf(stderr, "Illegal Amiga filename\n");
				}
				strcpy(filename, p);
			} else {
				fprintf(stderr, "Syntax error\n");
				usage();
				quit(1);
			}
		}
	}

#if 0
D(printf("id.I_Marker = %02x\n", id.I_Marker));
D(printf("id.I_FTbl = %02x\n", id.I_FTbl));
D(printf("id.I_CS = %02x\n", id.I_CS));
D(printf("id.I_Length = %d\n", id.I_Length));
D(printf("id.I_Version = %d\n", id.I_Version));
D(printf("id.I_BaseDrv = %d\n", id.I_BaseDrv));
D(printf("id.I_Units = %d\n", id.I_Units));
D(printf("id.I_Init = %d\n", id.I_Init));
D(printf("id.I_pad = %d\n", id.I_pad));

D(printf("driveid = %d\n", driveid));
D(printf("filename = %s\n", filename));
#endif

	if (switch_u && !switch_r && !switch_c && driveid != -1 && filename[0] == 0) {
		delink(driveid);
		if (!switch_n)
			report();
	} else if (switch_c && !switch_u && driveid != -1 && filename[0] != 0) {
		create(driveid, filename, switch_r, disksize);
		if (!switch_n)
			report();
	} else if (!switch_c && !switch_u && driveid != -1 && filename[0] != 0) {
		link(driveid, filename, switch_r);
		if (!switch_n)
			report();
	} else if (argc == 1) {
		report();
	} else {
		usage();
	}

	quit(0);
}
