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
*	will not handle being aborted gracefully.  if user does ctrlc/ctrlbreak
*	or there is an MS-DOS error and the user replies ABORT, then dosserv
*	will not be closed and the amiga file will not be closed (leaving a
*	lock on the file).
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
#include <signal.h>
#undef NULL
#include <janus/janus.h>
#include <janus/dosserv.h>
#include <janus/jftrans.h> 
#include "..\dslib\dslib.h"
                                               
#define   FALSE      0

/* amiga<->PC ascii translation table */
#include "xlattbl.c"

/* things set up by open_dosserv() */
static struct dslib_struct ds;

static unsigned char *tbuf;      /* local buffer copy of shmem data */

/* local functions */
static int jft_translate(unsigned char *buf, int count, unsigned char *table);


/************************************************************************
 *
 * j_file_xfer(infile, outfile, direction, mode, transtable)
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
int j_file_xfer(struct dslib_struct *ds, FILE *pcfile, ULONG amfile,
				int direction, int mode, int convert,
				unsigned char *transtable)
{
   int            status;           /* return value (error code) */
	int				amiga_error;		/* flag for amiga read/write errors */
	int				msdos_error;		/* flag for msdos read/write errors */
	int            n_read;           /* bytes read from pc/amiga */

   /* validate parameters */
   if(mode!=JFT_BINARY && mode!=JFT_CR_LF)
      return(JFT_ERR_INVALID_MODE);

   if(direction!=JFT_PC_AMIGA && direction!=JFT_AMIGA_PC)
      return(JFT_ERR_INVALID_DIRECTION);

   if(convert!=JFT_CONVERT && convert!=JFT_NO_CONVERT)
      return(JFT_ERR_INVALID_CONVERSION_MODE);

   /* extract buffer information from the DOSServReq structure */

	/* try to allocate buffer */
   if (!(tbuf = (unsigned char *) malloc(ds->ds_buf_len))) {
      /* malloc failed */
      return(JFT_ERR_AMIGA_OPEN);
   }
      
   /* assume no errors */
   amiga_error = FALSE;
   msdos_error = FALSE;
   status = JFT_NOERROR;

   /* copy the file */
   if (direction == JFT_AMIGA_PC) {

      /* read amiga, write pc */
      while (n_read = j_Read(ds, amfile, tbuf, ds->ds_buf_len)) {
		 /* if got an error, stop */
		 if (n_read == -1) {
		 	amiga_error = TRUE;
		 	break;
		 }

         /* translate block if necessary */
	      if (convert == JFT_CONVERT) {
	         if (transtable)
	           jft_translate(tbuf, n_read ,transtable);
	         else
	           jft_translate(tbuf, n_read, amiga_pc_table);
	      }

         /* write the block */
         if (fwrite(tbuf, 1, n_read, pcfile) != n_read) {
            msdos_error = TRUE;
            break;      /* error! */
         }

         if (feof(pcfile)) {
            msdos_error = TRUE;
            break;      /* error! */
         }
      }
   } else {

      /* read pc, write amiga */
      while (n_read = fread(tbuf, 1, ds->ds_buf_len, pcfile)) {
         /* translate block if necessary */
	      if (convert == JFT_CONVERT) {
	         if (transtable)
	           jft_translate(tbuf, n_read ,transtable);
	         else
	           jft_translate(tbuf, n_read, pc_amiga_table);
	      }

         /* write the block */
         if (j_Write(ds, amfile, tbuf, n_read) != n_read) {
            amiga_error = TRUE;
            break;      /* error! */
         }

         if (amiga_error) {
            break;      /* error! */
         }
      }

      if (ferror(pcfile))
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

	free(tbuf);

   /* tell him how things went */
   return(status);
}

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
   char           modestr[3];       /* rb/rt/wb/wt etc */
   int            status;           /* return value (error code) */
	int				amiga_error;		/* flag for amiga read/write errors */
	int				msdos_error;		/* flag for msdos read/write errors */
	int            n_read;           /* bytes read from pc/amiga */
	char           *pcfname;         /* ptr to filename for pc file */
	char           *amfname;         /* ptr to filename for amiga file */
	FILE           *localfile;       /* ms-dos file handle */
	ULONG          remotefile;       /* DOSServ amiga file handle */
	LONG			amode;			/* amiga file mode */

	localfile = 0;
	remotefile = 0;

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

   /* get janus DOSServ information.  sets up sd_ds (pointer to ServiceData
    * struct for DOSServ) and ds_rq (pointer to DOSServReq struct)
    */
	if (j_tickle_janus()) {
		return(JFT_ERR_NO_JANUS); 					/* Error no Amiga service */
	}

	/* make sure dosserv present */
	if (j_open_dosserv(&ds)) {
		return(JFT_ERR_NO_JANUS); 					/* Error no Amiga service */
	}

   /* open the pc file */
   if (!(localfile = fopen(pcfname, modestr))) {
   	j_close_dosserv(&ds);
      return(JFT_ERR_PC_OPEN);            /* failed */
    }
 
   /* extract buffer information from the DOSServReq structure */

   /* build mode string for amiga file */
   if(direction==JFT_AMIGA_PC)         /* open appropriate PC file      */
      amode = MODE_OLDFILE;
   else
      amode = MODE_NEWFILE;

   /* open the amiga file */
   if (!(remotefile = j_Open(&ds, amfname, amode))) {
      /* hmm.  can't get the amiga file. */
      fclose(localfile);
      j_close_dosserv(&ds);
      return(JFT_ERR_AMIGA_OPEN);
   }

	status = j_file_xfer(&ds, localfile, remotefile, direction, mode,
			     convert, transtable);

   /* put the candle back */
   fclose(localfile);
   j_Close(&ds, remotefile);
   j_close_dosserv(&ds);

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

