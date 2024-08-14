#include "exec/types.h"
#include "exec/memory.h"
#include "exec/exec.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "libraries/filehandler.h"
#include "prep.h"

#define OUTOFMEMORY 52

extern long getlong();
extern char *gets();
extern char yesreply();
extern char *devName;		/* Pointer to string containing device name */

typedef struct d_table_ent {	/* entry of table of known drives */
    DISK_INFO d_info;	/* Disk information */
    char *d_name;	/* ASCII drive name */
} D_TABLE;

D_TABLE	driveTable[] = {		/* add entries as needed */
      { 2047,				/* # of cylinders		*/
	1023,				/* Pre-comp cylinder		*/
	1023,				/* Reduced write current cyl.	*/
	0,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"USER DEFINED" },		/* ASCII drive name		*/
      { 306,				/* # of cylinders		*/
	128,				/* Pre-comp cylinder		*/
	128,				/* Reduced write current cyl.	*/
	0,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	2,				/* # of surfaces		*/
	"Generic 2 head 10MB  (ST506)"},/* ASCII drive name		*/
      { 306,				/* # of cylinders		*/
	128,				/* Pre-comp cylinder		*/
	128,				/* Reduced write current cyl.	*/
	0,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Generic 4 head 10MB  (ST506)"},/* ASCII drive name		*/
      { 615,				/* # of cylinders		*/
	615,				/* Pre-comp cylinder		*/
	615,				/* Reduced write current cyl.	*/
	650,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Epson HMD-726A       (SCSI )"},/* ASCII drive name		*/
      { 612,				/* # of cylinders		*/
	128,				/* Pre-comp cylinder		*/
	128,				/* Reduced write current cyl.	*/
	652,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"MiniScribe 3425/8425 (ST506)"},/* ASCII drive name		*/
      { 810,				/* # of cylinders		*/
	810,				/* Pre-comp cylinder		*/
	810,				/* Reduced write current cyl.	*/
	850,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	6,				/* # of surfaces		*/
	"MiniScribe 3650      (ST506)"},/* ASCII drive name		*/
      { 602,				/* # of cylinders		*/
	602,				/* Pre-comp cylinder		*/
	602,				/* Reduced write current cyl.	*/
	612,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"MiniScribe 8425S     (SCSI )"},/* ASCII drive name		*/
      { 823,				/* # of cylinders		*/
	823,				/* Pre-comp cylinder		*/
	823,				/* Reduced write current cyl.	*/
	0,				/* Park cylinder		*/
	19,				/* Sectors per track		*/
	10,				/* # of surfaces		*/
	"Quantum Q280         (SCSI )"},/* ASCII drive name		*/
      { 306,				/* # of cylinders		*/
	306,				/* Pre-comp cylinder		*/
	306,				/* Reduced write current cyl.	*/
	305,				/* Park cylinder		*/
	33,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Rodime RO 652        (SCSI )"},/* ASCII drive name		*/
      { 615,				/* # of cylinders		*/
	300,				/* Pre-comp cylinder		*/
	300,				/* Reduced write current cyl.	*/
	670,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Seagate ST225        (ST506)"},/* ASCII drive name		*/
      { 820,				/* # of cylinders		*/
	820,				/* Pre-comp cylinder		*/
	820,				/* Reduced write current cyl.	*/
	850,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	6,				/* # of surfaces		*/
	"Seagate ST251        (ST506)"},/* ASCII drive name		*/
      { 613,				/* # of cylinders		*/
	613,				/* Pre-comp cylinder		*/
	613,				/* Reduced write current cyl.	*/
	650,				/* Park cylinder		*/
	26,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Seagate ST138N       (SCSI )"},/* ASCII drive name		*/
      { 613,				/* # of cylinders		*/
	613,				/* Pre-comp cylinder		*/
	613,				/* Reduced write current cyl.	*/
	650,				/* Park cylinder		*/
	26,				/* Sectors per track		*/
	6,				/* # of surfaces		*/
	"Seagate ST157N       (SCSI )"},/* ASCII drive name		*/
      { 615,				/* # of cylinders		*/
	300,				/* Pre-comp cylinder		*/
	300,				/* Reduced write current cyl.	*/
	670,				/* Park cylinder		*/
	NSEC,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Seagate ST225N       (SCSI )"},/* ASCII drive name		*/
      { 818,				/* # of cylinders		*/
	818,				/* Pre-comp cylinder		*/
	818,				/* Reduced write current cyl.	*/
	850,				/* Park cylinder		*/
	26,				/* Sectors per track		*/
	4,				/* # of surfaces		*/
	"Seagate ST251N       (SCSI )"}	/* ASCII drive name		*/
};


select_d_type(format_data)	/* ask user what type drive he has */
FMT_DATA *format_data;
{
    register int i;
    register long *longp;
    register DISK_INFO *dp;	/* Pointers to disk type information */
    register DISK_INFO *btp;
    register long *env;		/* Points to environment in boot*/
    int j, typeno;		/* Index in to drive type table	*/
    char c_buffer[80];		/* Input buffer			*/

    puts("Drive types:\n");
    for (i = 0; i < (sizeof(driveTable) / sizeof(driveTable[0])); ++i)
	printf("%3ld) %s\n", i, driveTable[i].d_name);
    typeno = getlong("Select drive type",
	6L,			/* 3 is default	*/
	0L,			/* 0 is minumum	*/
				/* Max drive type is max */
	(long)((sizeof(driveTable) / sizeof(driveTable[0])) - 1));

    dp = &driveTable[typeno].d_info;
    btp = &(format_data->bootinfo.b_disk_info);

    if(!typeno)			/* user wants to specify parameters */
    {
	btp->ds_heads = getlong("Number of heads",
	    4L,			/* 3 is default	*/
	    1L,			/* 1 is minumum	*/
	    32L);		/* Max of 32 heads */
	btp->ds_cylinders = getlong("Number of cylinders",
	    -1L,		/* no default	*/
	    3L,			/* 3 is minumum	*/
	    0x7FFFFFFFL);	/* Max is very big */
	btp->ds_sectors = getlong("Number of sectors per track",
	    17L,		/* 17 default	*/
	    1L,			/* 1 is minumum	*/
	    0x7FFFL);		/* Max is very big */
	btp->ds_precomp = getlong("Write pre-comp cylinder",
	    (long)btp->ds_cylinders,/* max cyl. is default */
	    0L,			/* 0 is minumum	*/
	    (long)btp->ds_cylinders);/* max cyl. is limit */
	btp->ds_reduce = btp->ds_precomp;
    }
    else format_data->bootinfo.b_disk_info = driveTable[typeno].d_info;

    if (yesreply("Do you want the heads parked automatically after\n 3 seconds of inactivity?", "N"))
	btp->ds_park = getlong("Cylinder to park heads at",
	    (long)btp->ds_cylinders,	/* max cyl. is default */
	    1L,				/* 1 is minumum	*/
	    (long)btp->ds_cylinders);	/* max cyl. is limit */
    else format_data->bootinfo.b_disk_info.ds_park = 0L;

		/* copy information to boot block */
    format_data->bootinfo.b_magic = 0xBABE0000;	/* MAGIC */

		/* Set AmigaDOS environment for 1st partition "constants" */
    env = (long *)format_data->bootinfo.b_environment;
    env[DE_TABLESIZE] 	= 11;
    env[DE_SIZEBLOCK] 	= 128;
    env[DE_SECORG] 	= 0;
    env[DE_SECSPERBLK]	= 1;
    env[DE_RESERVEDBLKS] = 2;
    env[DE_LOWCYL]	= 2;	/* 1st two cylinders used for bad blks*/
    env[DE_MEMBUFTYPE]	= 0;	/* FAST memory is fine	*/
    env[DE_NUMHEADS]	= btp->ds_heads;
    env[DE_BLKSPERTRACK] = env[DE_PREFAC] = btp->ds_sectors;
    env[DE_UPPERCYL] = getlong(
	"Last cylinder used by first partition\n (range 2 to [default])",
	((long)btp->ds_cylinders - 1L),/* max cyl. is default */
	2L,			/* 2 is minumum	*/
	((long)btp->ds_cylinders - 1L));/* max cyl. is limit */
    env[DE_NUMBUFFERS] = getlong("Number of AmigaDOS sector buffers",
	30L,			/* A moderate number	*/
	1L,			/* 1 is minumum		*/
	10000L);		/* Pretty wastful	*/
	
    i = 0;
    if (yesreply("Are there any known bad areas on the disk", "N"))
    {
	do {
	    /* Get bad area's cylinder number	*/
	    format_data->baodata.bad[i].bao_cyl = getlong(
		"Cylinder number of bad area",
		-1L,			/* No default		*/
		0L,			/* 0 is minumum		*/
		((long)btp->ds_cylinders - 1L));/* max cyl. is limit */
	    /* Get bad area's cylinder number	*/
	    format_data->baodata.bad[i].bao_head = getlong(
		"Head number of bad area",
		-1L,			/* No default		*/
		0L,			/* 0 is minumum		*/
		((long)btp->ds_heads - 1L));/* has to be on real head */
	    /* Get bad area's cylinder number	*/
	    format_data->baodata.bad[i].bao_offset = getlong(
		"Offset from index of bad area",
		-1L,			/* No default		*/
		0L,			/* 0 is minumum		*/
		btp->ds_sectors * 1000L); /* Must be on track	*/
	    printf("Cylinder %ld Head %ld Offset %ld\n",
		format_data->baodata.bad[i].bao_cyl,
		format_data->baodata.bad[i].bao_head,
		format_data->baodata.bad[i].bao_offset);
	    if (yesreply("Is this correct", "Y"))i++;
	}while ((i < NBAD) && yesreply("More bad areas to enter", "N"));
    }
		/* Mark end of table */
    format_data->baodata.bad[i].bao_head = 0xFF;
}


long getlong(prompt, def, low, high)	/* input integer from console */
char *prompt;			/* User prompt */
long def; /* value returned if empty str; def = -1 means don't display def */
long low, high;			/* Valid range */
{
    char *t_ptr;		/* Temp pointer to test "gets" status */
    int index;
    long inp;
    char num_input[80];		/* work area for inputting numbers */
    register char c;		/* Holds input character */
    int success;		/* = 1 when successful */

    do {
	printf("%s ", prompt);	/* Display prompt */
	if (def != -1)printf("[%ld]",def); /* If default is to be displayed */
	printf(": ");
	success = 0;

	t_ptr = gets(num_input);
	if (t_ptr == NULL)*num_input = '\0'; /* If get failed, indicate it */
	success = (t_ptr != NULL) && sscanf(num_input, &inp);
	if (*num_input == '\0')	/* Use default	*/
	{
	    success = TRUE;
	    inp = def;
	}
    } while ((!success) || (inp < low) || (inp > high)); /* Looptil success */
    return(inp);		/* Return valid input */
}


char yesreply(prompt, def)	/* Get Yes/No reply from console */
char *prompt;			/* User prompt */
char *def;			/* Default answer 'Y' or 'N' */
{
    char num_input[80];		/* work area for inputting numbers */
    register char c;		/* Holds input character */
    char *t_ptr;		/* Temp pointer to test "gets" status */

    do {
	printf("%s [Y/N] : %s\b",prompt, def);	/* Display prompt */
	t_ptr = gets(num_input);
	if (t_ptr == NULL)	/* If get failed, set to default */
	    c = *def;
	else c = toupper(num_input[0]);
    } while ((c != 'Y') && (c != 'N'));
    return(c == 'Y');	
}
