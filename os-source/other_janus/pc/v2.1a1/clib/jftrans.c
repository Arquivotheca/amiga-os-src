#undef PRINTING
/*
#define PRINTING
*/
/****** ljanus.lib/j_file_transfer ******************************************
*
*   NAME   
*		j_file_transfer -- Transfer files MS-DOS <-> AmigaDOS.
*
*   SYNOPSIS
*		Error = j_file_transfer(Infile,Outfile,Direction,Mode,Transtable,
*										Convert);
*
*		int j_file_transfer(char *,char *,int,int,unsigned char *,int);
*
*   FUNCTION
*		Transfers files from AmigaDOS to MS-DOS and vice versa. This function
*		provides a user interface to the PCDisk program on the Amiga.
*
*   INPUTS
*		Infile -     Pointer to NULL terminated string which is the name of
*                  the input file to open. If Direction==JFT_AMIGA_PC Infile
*                  will point to string containing a valid AmigaDos
*                  filespec as documented in AREAD.DOC. If Direction==
*                  JFT_PC_AMIGA Infile will point to a string containing
*                  a valid PC filespec as documented in AWRITE.DOC.      
*		Outfile -    Pointer to NULL terminated string which is the name of
*                  the output file to open. If Direction==JFT_AMIGA_PC
*                  Infile will point to a string containing a valid MS-DOS
*                  filespec as documented in AREAD.DOC. If Direction==
*                  JFT_PC_AMIGA Infile will point to a valid AmigaDOS
*                  filespec as documented in AWRITE.DOC.
*           
*		Direction -  Direction flag from jftrans.h
*                  JFT_AMIGA_PC - Indicates that the transfer is from the
*                  Amiga to the PC so infile will be
*                  an Amiga filespec and Outfile will be
*                  an MS-DOS filespec.
*                  JFT_PC_AMIGA - Indicates that the transfer is from the
*                  PC to the Amiga. Infile will be a MS-DOS
*                  filespec and Outfile will be an Amiga
*                  filespec.
*		Mode -       Mode flag from jftrans.h
*                  JFT_CRLF   - Causes CRLF conversions to take place.
*                  JFT_BINARY - Suppresses CRLF conversions.
*		Transtable - Pointer to an optional character array to be used for
*                  character translations. The format for the array is
*                  char table[256] = {
*                     0xnn,      Entry 0 
*                     0xnn,      Entry 1 
*                       .        .   
*                       .        .   
*                       .        .   
*                     0xnn,      Entry 254 
*                     0xnn };    Entry 255 
*                  j_file_transfer() uses the following line to perform
*                  its translations:
*                  writechar = transtable[readchar];
*                  so the input character is used as an index into the
*                  table and the and the character contained in that entry
*                  is the character sent as output. In this way all 255
*                  ASCII characters can be converted. If a NULL is provided
*                  for this field the default translation tables are used.
*                  The default translations are documented in AREAD.DOC and
*                  AWRITE.DOC.
*		Convert -    Conversion flag from jftrans.h
*                  JFT_CONVERT   - Convert characters according to
*                  Transtable provided or use defaults if NULL given as
*                  Transtable.
*                  JFT_NO_CONVERT - Perform no conversion of characters.
*                  Character value written will be the
*                  same as that read.
*
*   RESULT
*		Error - Error flag from jftrans.h
*       JFT_NOERROR                     - Indicates a successful transfer.
*       JFT_ERR_INVALID_MODE            - An invalid mode was specified
*                                         Valid modes are JFT_CRLF and
*                                         JFT_BINARY.
*       JFT_ERR_INVALID_DIRECTION       - An invalid direction was
*                                         specified. Valid directions
*                                         are JFT_PC_AMIGA and
*                                         JFT_AMIGA_PC.
*       JFT_ERR_NO_JANUS                - The Janus library has not been loaded
*       JFT_ERR_NO_SERVER               - The Amiga file server, DOSServ,
*                                         could not be loaded.
*       JFT_ERR_PC_OPEN                 - The PC file could not be opened.
*       JFT_ERR_AMIGA_OPEN              - The Amiga file could not be
*                                         opened.
*       JFT_ERR_AMIGA_READ              - There was an error while reading
*                                         from the Amiga.
*       JFT_ERR_AMIGA_WRITE             - There was an error while writing
*                                         to the Amiga.     
*       JFT_ERR_INVALID_CONVERSION_MODE - An invalid conversion mode was
*                                         specified, Valid modes are
*                                         JFT_CONVERT and JFT_NO_CONVERT.
*       JFT_ERR_PC_READ                 - There was an error while reading
*                                         from the PC.
*       JFT_ERR_PC_WRITE                - There was an error while writing
*                                         to the PC.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
*****************************************************************************
*
*/
#define LINT_ARGS                         /* Enable type checking       */
#include <stdio.h>
#include <dos.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <malloc.h>
#undef NULL
#include <janus/janus.h>
#include <janus/dosserv.h>
#include <janus/jftrans.h> 
#include <janus/jfuncs.h>
                                               
#define   FALSE      0

/* amiga<->PC ascii translation table */
#include "xlattbl.c"

/* things set up by open_dosserv() */
static struct ServiceData *sd_ds;       /* pointer to DOSServ ServiceData struct */
static struct DOSServReq *ds_rq;        /* pointer to DOSServ DOSServReq struct */
static unsigned char *ds_buf;           /* pointer to DOSServ shmem data buffer */
static int ds_buf_len;                  /* size of ds_buf */

static unsigned char *tbuf;      /* local buffer copy of shmem data */

/* local functions */
static void afclose(ULONG afile);
static int afwrite(void *buf, int item_size, int n_items, ULONG afile, int *error);
static int afread(void *buf, int item_size, int n_items, ULONG afile, int *error);
static ULONG afopen(char *fname, char *modestr);
static int jft_translate(unsigned char *buf, int count, unsigned char *table);
static UBYTE close_dosserv();
static UBYTE open_dosserv();
static UBYTE myCallService(struct ServiceData *sd);

/************************************************************************
 *
 * j_file_transfer(infile, outfile, direction, mode, transtable)
 *
 * Transfer a file from PC->Amiga or Amiga->PC
 *
 *   Infile    -  File to read
 *   Outfile   -  File to write
 *   Direction -  JFT_PC_AMIGA or JFT_AMIGA_PC
 * Mode        -  JFT_BINARY or JFT_CR_LF
 * Transtable  -  One dimensional array of unsigned chars where input
 *                char is used as index and char from that entry is sent
 *                as output char. Allows user defined translation tables.
 *                If Transtable == NULL default table is used.
 *
 ************************************************************************/
int j_file_transfer(infile,outfile,direction,mode,transtable,convert)
char    *infile, *outfile;
int     direction,mode;
unsigned char    *transtable;
int convert;
{
   FILE           *localfile;       /* ms-dos file handle */
   ULONG          remotefile;       /* DOSServ amiga file handle */
   char           modestr[3];       /* rb/rt/wb/wt etc */
   int            status;           /* return value (error code) */
	int				amiga_error;		/* flag for amiga read/write errors */
	int				msdos_error;		/* flag for msdos read/write errors */
	int            n_read;           /* bytes read from pc/amiga */
	char           *pcfname;         /* ptr to filename for pc file */
	char           *amfname;         /* ptr to filename for amiga file */

   /* validate parameters */
   if(mode!=JFT_BINARY && mode!=JFT_CR_LF)
      return(JFT_ERR_INVALID_MODE);

   if(direction!=JFT_PC_AMIGA && direction!=JFT_AMIGA_PC)
      return(JFT_ERR_INVALID_DIRECTION);

   if(convert!=JFT_CONVERT && convert!=JFT_NO_CONVERT)
      return(JFT_ERR_INVALID_CONVERSION_MODE);

   /* build mode string for pc file */
   if(mode==JFT_BINARY)
      modestr[1]='b';
   else
      modestr[1]='t';
   
   if(direction==JFT_AMIGA_PC)
      modestr[0]='w';
   else
      modestr[0]='r';

   modestr[2] = 0;

   if (direction == JFT_AMIGA_PC) {
      pcfname = outfile;
      amfname = infile;
   } else {
      pcfname = infile;
      amfname = outfile;
   }

   /* open the pc file */
   if(direction==JFT_PC_AMIGA)
	{	  
		int result;
		struct stat buf;

		result=stat(infile,&buf);
		if(result!=0) 
			return(JFT_ERR_PC_OPEN);

		if(buf.st_mode&S_IFDIR) 
			return(JFT_ERR_PC_DIRECTORY);
	}
   if (!(localfile = fopen(pcfname, modestr)))
      return(JFT_ERR_PC_OPEN);            /* failed */
    
   /* get janus DOSServ information.  sets up sd_ds (pointer to ServiceData
    * struct for DOSServ) and ds_rq (pointer to DOSServReq struct)
    */
   if (status = open_dosserv()) {
      /* oops.  couldn't get DOSServ for some reason. */
      fclose(localfile);
      return(status);   
   }

   /* extract buffer information from the DOSServReq structure */

	/* try to allocate buffer */
   if (!(tbuf = (unsigned char *) malloc(ds_buf_len))) {
      /* malloc failed */
      fclose(localfile);
      close_dosserv();
      return(JFT_ERR_AMIGA_OPEN);
   }
      
   /* build mode string for amiga file */
   if(direction==JFT_AMIGA_PC)         /* open appropriate PC file      */
      modestr[0] = 'r';
   else
      modestr[0] = 'w';

   modestr[1] = 0;

   /* open the amiga file */
   if (!(remotefile = afopen(amfname, modestr))) {
      /* hmm.  can't get the amiga file. */
      fclose(localfile);
      close_dosserv();
      return(JFT_ERR_AMIGA_OPEN);
   }

   /* assume no errors */
   amiga_error = FALSE;
   msdos_error = FALSE;
   status = JFT_NOERROR;

   /* copy the file */
   if (direction == JFT_AMIGA_PC) {

      /* read amiga, write pc */
      while (n_read = afread(tbuf, 1, ds_buf_len, remotefile, &amiga_error)) {

         /* translate block if necessary */
	      if (convert == JFT_CONVERT) {
	         if (transtable)
	           jft_translate(tbuf, n_read ,transtable);
	         else
	           jft_translate(tbuf, n_read, direction ? amiga_pc_table : pc_amiga_table);
	      }

         /* write the block */
         if (fwrite(tbuf, 1, n_read, localfile) != n_read) {
            msdos_error = TRUE;
            break;      /* error! */
         }

         if (feof(localfile)) {
            msdos_error = TRUE;
            break;      /* error! */
         }
      }

   } else {

      /* read pc, write amiga */
      while (n_read = fread(tbuf, 1, ds_buf_len, localfile)) {

         /* translate block if necessary */
	      if (convert == JFT_CONVERT) {
	         if (transtable)
	           jft_translate(tbuf, n_read ,transtable);
	         else
	           jft_translate(tbuf, n_read, direction ? pc_amiga_table : amiga_pc_table);
	      }

         /* write the block */
         if (afwrite(tbuf, 1, n_read, remotefile, &amiga_error) != n_read) {
            amiga_error = TRUE;
            break;      /* error! */
         }

         if (amiga_error) {
            break;      /* error! */
         }
      }

      if (ferror(localfile))
         msdos_error = TRUE;
   }

   /* if we stopped due to an ms-dos file read/write error, say so */
   if (msdos_error) {
      if (direction == JFT_AMIGA_PC)
         status = JFT_ERR_PC_WRITE;
      else
         status = JFT_ERR_PC_READ;
   }

   /* if we stopped due to an amiga file read/write error, fail */
   if (amiga_error) {
      if (direction == JFT_AMIGA_PC)
         status = JFT_ERR_AMIGA_READ;
      else
         status = JFT_ERR_AMIGA_WRITE;
   }

   /* put the candle back */
   fclose(localfile);
   afclose(remotefile);
   close_dosserv();

   /* tell him how things went */
   return(status);
}

/***************************************************************************
 * jft_translate(unsigned char *buf, int count, unsigned char *table)
 *
 * look up each byte in buf and convert it by indexing into table.
 **************************************************************************/
static int jft_translate(unsigned char *buf, int count, unsigned char *table)
{
int x;

   for(x = 0; x < count; x++)
      buf[x] = table[buf[x]];
} 

static UBYTE myCallService(struct ServiceData *sd)
{
char t[100];
UBYTE rval;

#ifdef PRINTING
	printf("myCallService (pre): dsr_Function = %d\n", ds_rq->dsr_Function);
	printf("myCallService (pre): dsr_Buffer_Seg:Off = %p\n",
	 (((ULONG)ds_rq->dsr_Buffer_Seg) << 16) | ds_rq->dsr_Buffer_Off);
	printf("myCallService (pre): dsr_Buffer_Size = %d\n", ds_rq->dsr_Buffer_Size);
	printf("myCallService (pre): dsr_Arg1_h = %d\n", ds_rq->dsr_Arg1_h);
	printf("myCallService (pre): dsr_Arg1_l = %d\n", ds_rq->dsr_Arg1_l);
	printf("myCallService (pre): dsr_Arg2_h = %d\n", ds_rq->dsr_Arg2_h);
	printf("myCallService (pre): dsr_Arg2_l = %d\n", ds_rq->dsr_Arg2_l);
	printf("myCallService (pre): dsr_Arg3_h = %d\n", ds_rq->dsr_Arg3_h);
	printf("myCallService (pre): dsr_Arg3_l = %d\n", ds_rq->dsr_Arg3_l);
#endif

	rval = CallService(sd);
	WaitService(sd);

#ifdef PRINTING
	printf("myCallService (post): retval = %d\n", rval);
	printf("myCallService (post): dsr_Err = %d\n", ds_rq->dsr_Err);
	printf("myCallService (post): dsr_Arg1_h = %d\n", ds_rq->dsr_Arg1_h);
	printf("myCallService (post): dsr_Arg1_l = %d\n", ds_rq->dsr_Arg1_l);
	printf("myCallService (post): dsr_Arg2_h = %d\n", ds_rq->dsr_Arg2_h);
	printf("myCallService (post): dsr_Arg2_l = %d\n", ds_rq->dsr_Arg2_l);
	printf("myCallService (post): dsr_Arg3_h = %d\n", ds_rq->dsr_Arg3_h);
	printf("myCallService (post): dsr_Arg3_l = %d\n", ds_rq->dsr_Arg3_l);

	puts("hit return: ");
	gets(t);
#endif

	return rval;
}

static UBYTE close_dosserv()
{
   return ReleaseService(sd_ds);
}

static UBYTE open_dosserv()
{
UBYTE error;
int i;

USHORT j_param_seg, j_param_offset, j_buffer_seg;

   /* first wake up janus */
   for (i = 0; i < 2; i++) {
      error = GetBase(JSERV_AMIGASERVICE,&j_param_seg,&j_param_offset,&j_buffer_seg);
   }
 
   /* is janus alive? */
   if (error != JSERV_OK)
      return(JFT_ERR_NO_JANUS); 					/* Error no Amiga service */
 
   /* now try to get DOSServ */
   error = GetService(&sd_ds, (ULONG) DOSSERV_APPLICATION_ID,
                      (UWORD) DOSSERV_LOCAL_ID, (ULONG) 0,
                      (UWORD) GETS_WAIT | GETS_ALOAD_A);

/* kludge until GetService() finishes all init before returning */
JanusLock(&sd_ds->sd_ServiceDataLock);
JanusUnlock(&sd_ds->sd_ServiceDataLock);

   /* did it work? */
   if (error == JSERV_OK) {
      ds_rq = (struct DOSServReq *) sd_ds->sd_PCMemPtr;         /* yes! */
      ds_buf = (unsigned char *)
         ((((ULONG) ds_rq->dsr_Buffer_Seg) << 16) + ds_rq->dsr_Buffer_Off);
      ds_buf_len = ds_rq->dsr_Buffer_Size;
      error = 0;
   } else {
      sd_ds = 0;                          /* no.  oh well. */
      ds_rq = 0;
      ds_buf = 0;
      ds_buf_len = 0;
      error = JFT_ERR_NO_SERVER;
   }
#ifdef PRINTING
printf("open_dosserv: sd_ds = %p, ds_rq = %p, ds_buf = %p, ds_buf_len = %d, retval = %d\n",
	sd_ds, ds_rq, ds_buf, ds_buf_len, error);
#endif
	
   /* return error code from GetService() */
   return error;
}

static ULONG afopen(char *fname, char *modestr)
{
ULONG retval;
int err;
#ifdef PRINTING
printf("afopen: checking filename\n");
#endif

   /* go poof if filename is huge */
   if (strlen(fname) + 1 >= ds_buf_len)
      return 0;

   /* lock the buffer */
   JanusLock(&ds_rq->dsr_Lock);

   /* install filename in buffer */
   strcpy(ds_buf, fname);

#ifdef PRINTING
printf("afopen: checking mode string\n");
#endif

   /* figure out flavor of open */
   if (strcmp(modestr, "r") == 0) {
      /* read from start of file (file must exist) */
      ds_rq->dsr_Function = DSR_FUNC_OPEN_OLD;
   } else if (strcmp(modestr, "w") == 0) {
      /* write to start of file (file will be created if doesn't exist,
       * or truncated to 0 length if it does exist)
       */
      ds_rq->dsr_Function = DSR_FUNC_OPEN_NEW;
   } else {
      /* unsupported mode */
      JanusUnlock(&ds_rq->dsr_Lock);
      return 0;
   }

#ifdef PRINTING
printf("afopen: calling service\n");
#endif

   /* try to call the service */
   if ((err = myCallService(sd_ds)) == JSERV_OK) {
      /* service call worked.  did the open work? */
      if (ds_rq->dsr_Err == DSR_ERR_OK) {
         /* yes - return the file handle + 1 */
         retval = (((ULONG) ds_rq->dsr_Arg1_h) << 16) + ds_rq->dsr_Arg1_l;
         retval++;
      } else {
         /* open failed */
         retval = 0;
      }
   } else {
      /* service failed */
      retval = 0;
   }

#ifdef PRINTING
printf("afopen: callservice = %d, dsr_Err = %d, retval = %d\n",
 err, ds_rq->dsr_Err, retval);
#endif

   /* unlock the buffer */
   JanusUnlock(&ds_rq->dsr_Lock);

   return retval;
}

static int afread(void *buf, int item_size, int n_items, ULONG afile, int *error)
{
ULONG retval;
int length;

   /* normalize */
   afile--;
   length = n_items * item_size;

   /* ensure sane length */
   if (length > ds_buf_len)
      return 0;

   /* grab DOSServReq */
   JanusLock(&ds_rq->dsr_Lock);

   /* arg1 gets filehandle */
   ds_rq->dsr_Arg1_l = afile & 0xffff;
   ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

   /* arg2 gets num of bytes to read */
   ds_rq->dsr_Arg2_l = length & 0xffff;
   ds_rq->dsr_Arg2_h = (length >> 16) & 0xffff;

#ifdef PRINTING
printf("afread: afile = %ld, length = %d\n", afile, length);
#endif

   /* set function */
   ds_rq->dsr_Function = DSR_FUNC_READ;

   /* try to call the service */
   if (myCallService(sd_ds) == JSERV_OK) {
      /* service worked.  did the read work? */
      if (ds_rq->dsr_Err == DSR_ERR_OK) {
         /* read worked */
#ifdef PRINTING
printf("afread: read worked\n");
#endif
         retval = (((ULONG) ds_rq->dsr_Arg3_h) << 16) + ds_rq->dsr_Arg3_l;
         memcpy(buf, ds_buf, retval);
#ifdef PRINTING
printf("afread: before div, retval = %ld\n", retval);
#endif
         /* set retval to # of whole items read */
         retval = retval / item_size;
         *error = FALSE;
      } else {
         /* read failed */
         retval = 0;
         *error = TRUE;
      }
   } else {
      /* service failed */
      retval = 0;
      *error = TRUE;
   }

   /* someone else can have it now */
   JanusUnlock(&ds_rq->dsr_Lock);

#ifdef PRINTING
printf("afread: retval = %ld, error = %d\n", retval, *error);
#endif

   return (int) retval;
}

static int afwrite(void *buf, int item_size, int n_items, ULONG afile, int *error)
{
int retval;
int length;

   /* normalize */
   afile--;
   length = n_items * item_size;

   /* ensure sane length */
   if (length > ds_buf_len)
      return 0;

   /* grab DOSServReq */
   JanusLock(&ds_rq->dsr_Lock);

   /* arg1 gets filehandle */
   ds_rq->dsr_Arg1_l = afile & 0xffff;
   ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

   /* arg2 gets num of bytes to write */
   ds_rq->dsr_Arg2_l = length & 0xffff;
   ds_rq->dsr_Arg2_h = (length >> 16) & 0xffff;

   /* set function */
   ds_rq->dsr_Function = DSR_FUNC_WRITE;

   /* stick data in buffer */
   memcpy(ds_buf, buf, length);   

   /* try to call the service */
   if (myCallService(sd_ds) == JSERV_OK) {
      /* service worked.  did the write work? */
      if (ds_rq->dsr_Err == DSR_ERR_OK) {
         /* write worked */
         /* set retval to # of whole items written */
         retval = (((ULONG) ds_rq->dsr_Arg3_h) << 16) + ds_rq->dsr_Arg3_l;
         retval = retval / item_size;
         *error = FALSE;
      } else {
         /* write failed */
         retval = 0;
         *error = TRUE;
      }
   } else {
      /* service failed */
      retval = 0;
      *error = TRUE;
   }

   /* someone else can have it now */
   JanusUnlock(&ds_rq->dsr_Lock);

   return retval;
}

static void afclose(ULONG afile)
{
   /* normalize */
   afile--;

  /* grab DOSServReq */
   JanusLock(&ds_rq->dsr_Lock);

   /* do a close */
   ds_rq->dsr_Function = DSR_FUNC_CLOSE;

   /* arg1 gets filehandle */
   ds_rq->dsr_Arg1_l = afile & 0xffff;
   ds_rq->dsr_Arg1_h = (afile >> 16) & 0xffff;

   /* who *CARES* if a close fails? */
   myCallService(sd_ds);

   /* someone else can have it now */
   JanusUnlock(&ds_rq->dsr_Lock);
}
