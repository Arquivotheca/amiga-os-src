/* -----------------------------------------------------------------------
 * syslog.c
 *
 * $Locker:  $
 *
 * $Id: syslog.c,v 1.2 92/07/21 15:34:14 bj Exp $
 *
 * $Revision: 1.2 $
 *
 * $Log:	syslog.c,v $
 * Revision 1.2  92/07/21  15:34:14  bj
 * Changed syslog() to s_syslog() to be in step with the rest of the
 * non-Unix functions. 
 * 
 * Used in versions 4.0 and greater of socket.library. R2 AS225.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/syslog.c,v 1.2 92/07/21 15:34:14 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/****** socket.library/s_syslog *************************************************
*
*   NAME
*	s_syslog - log system messages
*
*   SYNOPSIS
*	error = s_syslog( priority, message )
*	D0              A0        D0
*
*	int s_syslog( int, char * ) ;
*
*   FUNCTION
*	s_syslog() writes the 'message' argument to a console window and/or
*	a specified file. The priority field is used to determine which,
*	if any, of the above options is used. 
*
*	The file "inet:s/inet.config" contains the optional fields:
*
*	  filepri        - msgs w/pri >= filepri are sent to file.
*	  windowpri      - msgs w/pri >= windowpri are sent to window.
*	  syslogfilename - the path/name of the file for filepri messages.
*
*	Example:  (Note: this is the exact format to use in the 
*	                 'inet:s/inet.config' file.)
*
*	  filepri=5
*	  windowpri=3
*	  syslogfilename="t:foobar"
*
*	  These entries tell s_syslog how to deal with the messages it is 
*	  sent.  Note that the smaller the priority number, the greater 
*	  it's priority. Therefore, a message of priority 3 is of greater 
*	  importance than a message of priority 4. If you do not add these
*	  fields to you 'inet:s/inet.config' file, the default values will
*	  be used. The default values are:
*
*		windowpri = 4
*		filepri   = 3
*		syslogfilename = "ram:syslog.dat"
*
*	  The above values mean:
*
*		1. A message sent with a priority >= 4 (4,3,2,1,0) will
*		   be sent to BOTH the console window and the file
*		   'ram:syslog.dat'.
*
*		2. A message sent with a priority >= 3 (3,2,1,0) will
*		   be sent to ONLY the console window.
*
*		3. A message sent with a priority < 4 (5,6,7,8) will
*		   be ignored.
*
*   INPUTS
*	pri       - an integer value between 0-7
*	message   - a string to be written to the console window and/or
*	            the syslog file.
*
*   RESULT
*	error     - 0 (zero) is returned if everything went well. 
*	            -1 is returned if anything went wrong.
*
*	  If an error has occurred, you need to examine your errno value
*	  to determine just what went wrong. It is possible, for example,
*	  to have a write to the console window succeed while a write of
*	  the same message to the file failed.  The various error values
*	  that occur during the syslog operation are OR'd together and
*	  returned in 'errno'. See the file "sys/syslog.h" for the values.
*
*	  Example:
*
*	  #include<sys/syslog.h>
*	     ...
*	   extern int errno ;
*	   int error ;
*
*	   error = s_syslog( 5, "This is a test\n") ;
*	   if( error == -1 )
*	   {
*	      if( errno & SYSLOGF_WINDOWOPEN )
*	      {
*	         printf("Could not open syslog window\n") ;
*	      }
*	      if( errno & SYSLOGF_FILEWRITE )
*	      {
*	         printf("Could not write to syslog file\n") ;
*	      }
*	   }
*
*   NOTES
*	1. s_syslog() will clear the global errno value with each call.
*
*	2. Unlike the Unix syslog() function, the Amiga version does not
*	   handle printf-style arguments. This will need to be handled in
*	   the application.
*
*	3. The maximum length of a syslogfilename line in inet.config is
*	   127 characters. This includes the 'syslogfilename=' portion
*	   of the line. 
*
*	   maxlen(path/filename) = 127 - len( "syslogfilename=" )
*
*   BUGS
*	none known
*
*   SEE ALSO
*	inet.config (in the AS225 manual)
*
********************************************************************
*
*/

#ifdef DEBUG
	#define DB(x) kprintf(x)
#else
	#define DB(x) ;
#endif

#include "sslib.h"

#include <dos/dosextens.h>
#include <sys/syslog.h>

#define WINDEF "con:0/0/600/70/ SYSLOG /AUTO/CLOSE"

extern struct SocketLibrary *SockBase ;
extern struct DosLibrary *DOSBase ;
extern struct ExecBase   *SysBase ;


int strlen( char *m ) ;

int __saveds __asm s_syslog( register __d0 int pri, register __a0 char *msg )
{
	BPTR SyslogFileFH ;
	BPTR wfh ;
	LONG len = (LONG)strlen(msg) ;
	struct config *cf = SockBase->ml_origbase->ml_config ;
	char *SyslogFileName ;

	*ss_errno = 0 ;
	if( len )
	{
		ObtainSemaphore( SockBase->ml_origbase->ml_SyslogSemaphore ) ;
			
			/* if file handle in SockBase == NULL then get us one */
		if( SockBase->ml_origbase->ml_SyslogWindowFH == NULL )
		{
			if(wfh = Open(WINDEF, MODE_NEWFILE))
			{
				SockBase->ml_origbase->ml_SyslogWindowFH = wfh ;
			}
			else
			{
				*ss_errno |= SYSLOGF_WINDOWOPEN ;
			}
        }

		if( pri <= cf->syslog_window_pri )
		{
			if( SockBase->ml_origbase->ml_SyslogWindowFH )
			{
				if((Write( SockBase->ml_origbase->ml_SyslogWindowFH , msg, len)) == -1)
				{
					*ss_errno |= SYSLOGF_WINDOWWRITE ;
				}
			}
			else
			{
				*ss_errno |= SYSLOGF_WINDOWWRITE ;
			}
		}


		if( pri <=  cf->syslog_file_pri )
		{
			if( cf && (SyslogFileName = (char *)&cf->syslog_file))
			{
				if( SyslogFileFH = Open(SyslogFileName , MODE_READWRITE ))
				{
					Seek( SyslogFileFH, 0L, OFFSET_END ) ;
					if((Write( SyslogFileFH, msg, len  )) == -1)
					{
						*ss_errno |= SYSLOGF_FILEWRITE ;
					}
					Close( SyslogFileFH ) ;
				}
				else
				{
						*ss_errno |= SYSLOGF_FILEOPEN ;
				}
			}
			else
			{
					*ss_errno |= SYSLOGF_FILEOPEN ;
			}
		}
		ReleaseSemaphore( SockBase->ml_origbase->ml_SyslogSemaphore ) ;
    }
	return( *ss_errno ? -1 : 0 ) ;
}
