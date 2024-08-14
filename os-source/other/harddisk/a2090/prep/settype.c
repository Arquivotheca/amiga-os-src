#include "exec/types.h"
#include "exec/memory.h"
#include "exec/exec.h"
#include "libraries/dos.h"
#include "libraries/dosextens.h"
#include "libraries/filehandler.h"
#include "prep.h"

#define OUTOFMEMORY 52

extern LONG getlong();
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
	307,				/* Park cylinder		*/
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
    register LONG *longp;
    register DISK_INFO *dp;	/* Pointers to disk type information */
    register DISK_INFO *btp;
    register LONG *env;		/* Points to environment in boot*/
    int j, err, typeno;		/* Index in to drive type table	*/
    char c_buffer[80];		/* Input buffer			*/
    static LONG defs[3]={-1, -1, -1};
    static LONG lows[3]={0,0,0};
    static LONG highs[3];
    static LONG results[3];

    puts("Drive types:\n");
    for (i = 0; i < (sizeof(driveTable) / sizeof(driveTable[0])); ++i)
	printf("%3ld) %s\n", i, driveTable[i].d_name);
    typeno = getlong("Select drive type",
	6,			/* 6 is default	*/
	0,			/* 0 is minumum	*/
				/* Max drive type is max */
	(LONG)((sizeof(driveTable) / sizeof(driveTable[0])) - 1));

    dp = &driveTable[typeno].d_info;
    btp = &(format_data->bootinfo.b_disk_info);

    if(!typeno) { /* user wants to specify parameters */
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
	    (LONG)btp->ds_cylinders,/* max cyl. is default */
	    0L,			/* 0 is minumum	*/
	    (LONG)btp->ds_cylinders);/* max cyl. is limit */
	btp->ds_reduce = btp->ds_precomp;
    }
    else format_data->bootinfo.b_disk_info = driveTable[typeno].d_info;

    if (yesreply("\nDo you want the heads parked automatically after\n 3 seconds of inactivity ?", "N"))
	btp->ds_park = getlong("Cylinder to park heads at",
	    (LONG)btp->ds_cylinders,	/* max cyl. is default */
	    1L,				/* 1 is minumum	*/
	    (LONG)btp->ds_cylinders);	/* max cyl. is limit */
    else format_data->bootinfo.b_disk_info.ds_park = 0;

		/* copy information to boot block */
    format_data->bootinfo.b_magic = 0xBABE0000;	/* MAGIC */

		/* Set AmigaDOS environment for 1st partition "constants" */
    env = (LONG *)format_data->bootinfo.b_environment;
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
	"\nLast cylinder used by first partition\n (range 2 to [default])",
	((LONG)btp->ds_cylinders - 1),/* max cyl. is default */
	2L,			/* 2 is minumum	*/
	((LONG)btp->ds_cylinders - 1));/* max cyl. is limit */
    env[DE_NUMBUFFERS] = getlong("\nNumber of AmigaDOS sector buffers",
	30,			/* A moderate number	*/
	1,			/* 1 is minumum		*/
	10000);			/* Pretty wastful	*/

    i = 0;
    err=TRUE;
    if(yesreply("\nWould you like to mark any blocks on the disk as bad ?", "N")){
	printf("Press the RETURN key when done.\n");
	do {
	    /* Get bad areas cylinder number	*/

	    highs[0]= (LONG)btp->ds_cylinders - 1;
	    highs[1]= (LONG)btp->ds_heads - 1;
 	    highs[2]= btp->ds_sectors * 1000;

	    if(getlongs("\nCylinder; Head; Offset",defs,lows,highs,results,3)) {

	        format_data->baodata.bad[i].bao_cyl = results[0];
	        format_data->baodata.bad[i].bao_head = results[1];
	        format_data->baodata.bad[i].bao_offset = results[2];

	    	printf("Cylinder [%ld] Head [%ld] Offset [%ld] ... ",
		
		format_data->baodata.bad[i].bao_cyl,
		format_data->baodata.bad[i].bao_head,
		format_data->baodata.bad[i].bao_offset);

	        if (yesreply("Is this correct ?", "Y"))i++;
	    }
	    else err=yesreply("Any more bad blocks to add ?", "N");
	} while ((i < NBAD) && (err));
    }
		/* Mark end of table */
    format_data->baodata.bad[i].bao_head = 0xFF;
}

LONG getlongs(prompt, defs, lows, highs,results,n)
char *prompt;			/* User prompt */
LONG defs[];
LONG lows[], highs[];			/* Valid range */
LONG results[];
int n;	/* number of values expected */
{
    char *t_ptr;		/* Temp pointer to test "gets" status */
    int index;
    LONG inp=0;
    char num_input[80];		/* work area for inputting numbers */
    char *pointer;
    char c;		/* Holds input character */
    int success=1;		/* = n when finished */
    int i,val;


    do {
    	pointer = num_input;
	printf("%s ", prompt);	/* Display prompt */
	for(i=0; i<n; i++) {
		if(defs[i] != -1)printf(" %ld;",defs[i]);
		results[i]=0;
	}
	printf(": ");

	t_ptr = gets(pointer);

	if (t_ptr == NULL) {
	    *pointer = '\0'; /* If just got a return */
	    for (i=0; i<n; i++) {
		if(defs[i] != -1)results[i]=defs[i]; /* use defaults */
	    }
	    return(0);
	}
	else  {
	    for (i=0; i<n; i++) {	
	        val=sscanf(&pointer, &inp);

		if(!val) {
		    if(defs[i] != -1) results[i]=defs[i]; /* use defaults */
		    success++;
		}
	        else {
		    results[i]=inp; 
		    success++;
		}
	    }
	}
        for (i=0; i<n; i++)
	    if( (results[i]<lows[i]) || (results[i]>highs[i]) ) {
	    	if(results[i] != -1)printf("Value %ld (=%ld) is out of range (min=%ld, max=%ld).  Please try again.\n",
			i+1, results[i],lows[i],highs[i]);
		success=0;
	    }
    } while (success < n); /* Loop until done */
    
    return(n);
}

LONG getlong(prompt, def, low, high )	/* input integer from console */
char *prompt;			/* User prompt */
LONG def; /* value returned if empty str; def = -1 means don't display def */
LONG low, high;			/* Valid range */
{
    char *t_ptr=NULL;		/* Temp pointer to test "gets" status */
    int index;
    LONG inp=0;
    char num_input[80];		/* work area for inputting numbers */
    char *pointer;
    char c;		/* Holds input character */
    int success=0,val=0;	/* = 1 when successful */

    do {
        pointer = num_input;
	printf("%s ", prompt);	/* Display prompt */
	if (def != -1)printf("[%ld]",def); /* If default is to be displayed */
	printf(": ");
	t_ptr = gets(pointer);
	if (t_ptr == NULL)*pointer = '\0'; /* If get failed, indicate it */
	success = (t_ptr != NULL) && (val=sscanf(&pointer, &inp));
	if (!val)	 {
	    success = TRUE; /* Use default	*/
	    inp = def;
	}
        if((inp < low) || (inp > high)) {
	    if(inp != -1)printf("Value (=%ld) is out of range (min=%ld, max=%ld).  Please try again.\n",inp,low,high);
	    success=0;
	}
    } while (!success);
    return(inp);	/* Return valid input */
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
