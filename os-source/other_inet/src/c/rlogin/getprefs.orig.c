/* -----------------------------------------------------------------------
 * getprefs.c     rlogin 38.0
 *
 *                This file contains the routines needed to absorb and
 *                understand the needs and wants of the user. 
 *
 * $Locker:  $
 *
 * $Id: getprefs.c,v 38.0 93/04/08 15:26:16 bj Exp $
 *
 * $Revision: 38.0 $
 *
 * $Log:	getprefs.c,v $
 * Revision 38.0  93/04/08  15:26:16  bj
 * This is the complete rewrite of rlogin (v 38.xx.)
 * These are the functions that handle what the user wants
 * 
 *
 * $Header: AS225:src/c/rlogin/RCS/getprefs.c,v 38.0 93/04/08 15:26:16 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/* ==========================================================================
 * GetUserPrefs()  Boolean.  Returns FALSE if anything went wrong, else TRUE. 
 *
 * this file contains the functions related to the process of
 * determining what the user wants. First we have to parse 
 * the original command line to see if the user specified 
 * a specific config file with the '-f=file' option. The function
 * "FindFileName()" accomplishes this. If the user did specify 
 * a config file we have to make TWO pases through ReadArgs with
 * two different templates.  
 *
 * The problem stems from the fact that the 'host' argument to
 * rlogin is mandatory.  This makes it a /A readargs type.
 * the config file format is simply that of an ascii list of
 * arguments exactly the same as those you would enter on the
 * command line. The program simply reads the entire file into
 * memory, changes all but the last newline to a space and feeds
 * the string to ReadArgs().  But, since the 'host' argument is
 * a "must have" this means that the user would be _required_
 * to specify the host machine both on the command line and in
 * the config file - unless you use two templates.  In this case
 * I copy the template into allocated memory and then 'fix' the
 * string by copying the word 'HOST' onto itself but offset
 * by two characters. This turns 'HOST/A,' to 'HOST,'. The result
 * is that you _can_ specify the host name in the configfile but it
 * will be ignored. You _must_ specify the host on the command
 * line - even if you already have it in the file.
 * =========================================================================
 */


#include "rl.h"

#define ARRAYSIZE 512L
#define MAXREAD  ARRAYSIZE - 2L

BOOL 
GetUserPrefs( struct glob *g )
{    
	REGISTER SYSBASE ;
	REGISTER DOSBASE(g) ;
	char array[ARRAYSIZE] ;
	char *template ;
	char *template_plus2 ;
	 
	g->g_error = ERR_GETPREFS ;

//	CopyMem("this is fkey #1\n", g->g_fkeys[0], MAX_FKEYLEN) ;
//	CopyMem("this is fkey #2\n", g->g_fkeys[1], MAX_FKEYLEN) ;

	CopyMem(GetArgStr(), g->g_user_argstring, MAXREAD ) ;
	 
	if( template = AllocVec((LONG)sizeof(TEMPLATE)+4L, MEMF_CLEAR))
	{
		template_plus2 = template + 2 ;
		CopyMem(TEMPLATE,template,(LONG)sizeof(TEMPLATE)) ;
		CopyMem("HOST", template_plus2 , 4L) ;
		if(	FindFileName(g) ) // see if user specified a config file
		{
			if(ReadConfigFile( array, g)) // read it & parse it
			{
				if(RAParse( array, template_plus2, g ) == FALSE)
				{
					return(FALSE) ;
				}
			}
		}
		FreeVec(template) ;
		     // now parse the original command line 
		return(RAParse( g->g_user_argstring, TEMPLATE, g)) ; 
	}
	return(FALSE) ;
}

/* ========================================================================
 *  RAParse()  Boolean.  Returns FALSE if anything went wrong, else TRUE.
 *
 * This function is passed a pointer to a 'command line' (which must have
 * a terminating newline character) and a proper ReadArgs template string.
 * The strings are passed to ReadArgs() for a result. This function can get
 * called twice - once to parse a user specified config file (if one was 
 * actually specified) and once to parse the actual command line.
 * ========================================================================
 */
     
BOOL
RAParse( UBYTE *args, char *template, struct glob *g)
{
	REGISTER SYSBASE ;
	REGISTER DOSBASE(g) ;
	REGISTER UTILITYBASE(g) ;
	REGISTER struct glob *gl = g ;
	struct RDArgs *ra_result ;
	BOOL parse_success ;
	LONG opts[OPT_COUNT] ;
	LONG unit ;
	LONG len ;
	int x, y ;
	ULONG delay ;
	UBYTE *p ;

	struct RDArgs ra ;

	ra.RDA_Source.CS_Buffer = args ;
	ra.RDA_Source.CS_Length = (LONG)strlen(args) ;
	ra.RDA_Source.CS_CurChr = 0L ;
	ra.RDA_Buffer = NULL ;
	ra.RDA_BufSiz = 0L ;
	ra.RDA_Flags  = 0L ;

	memset( opts, 0, sizeof(opts)) ;

	if(ra_result = ReadArgs(template, opts, &ra))
	{
		parse_success = TRUE ;

		if( opts[OPT_HOST] ) // the machine we wish to talk to
		{
			CopyMem((char *)opts[OPT_HOST], gl->g_host, MAX_HOST) ;
		}

		if( opts[OPT_USER] ) // the user name if specified
		{
			CopyMem((char *)opts[OPT_USER], gl->g_user, MAX_USER) ;
		}

		if( opts[OPT_FILE] )
		{
			CopyMem((UBYTE *)opts[OPT_FILE],gl->g_user_configfile, 250L) ;
		}

		if( opts[OPT_LEFT] )  // this affects the window ppositio on both WB
		{                     // and custom screens                     
			gl->g_user_left = *((LONG *)(opts[OPT_LEFT])) ;
		}


		if( opts[OPT_TOP] )   // this affects the window position on both WB
		{                     // and custom screens                     
			gl->g_user_top = (ULONG)(*((LONG *)(opts[OPT_TOP]))) ;
		}

		if( opts[OPT_WIDE] )  // this affects the window size on both WB
		{                     // and custom screens                     
			gl->g_user_width = *((LONG *)(opts[OPT_WIDE])) ;
		}

		if( opts[OPT_HIGH] )  // this affects the window size on both WB
		{                     // and custom screens
			gl->g_user_height = *((LONG *)(opts[OPT_HIGH])) ;
		}

			/* check for screen option before checking for pen options */
			/* since pen opts are pointless with use_screen = FALSE    */
			
		if(opts[OPT_SCREEN]) // boolean TRUE is user wants custom screen
		{
			gl->g_use_screen = TRUE ;
		}

		if(opts[OPT_TERMTYPE]) // termtype and baudrate
		{
			CopyMem((UBYTE *)opts[OPT_TERMTYPE],gl->g_termtype, MAX_TERMTYPE) ;
		}

		if(opts[OPT_MODE]) // user can specify specific display modes
		{
			CopyMem((UBYTE *)opts[OPT_MODE], gl->g_user_modeid, MAX_USERMODEID) ;
		}

		if( opts[OPT_DEPTH] ) // this works with some obvious constraints
		{
			gl->g_user_depth = *((LONG *)(opts[OPT_DEPTH])) ;
		}

		if( opts[OPT_CONUNIT] ) // default is 3L
		{
			if(((unit = *((LONG *)(opts[OPT_CONUNIT])))==0L) || (unit == 1L))
			{
				gl->g_user_conunit = unit ;
			}
		}

		if( opts[OPT_PASTE] ) // user wants a different default paste delay
		{
			delay = *((LONG *)(opts[OPT_PASTE])) ;
			if( delay >= 10L && delay <= 50L )
			{
				gl->g_paste_delay = UMult32( UDivMod32(delay,10L), 10L) ;
			}
		}

		if( gl->g_use_screen )  // if the user specified PEN color values ...
		{
			for(x = OPT_PEN0 ; x < OPT_PENMAX ; x++ )
			{
				if( opts[x] )
				{
					if(SetPen( x - OPT_PEN0 , (UBYTE *)opts[x], g) == FALSE)
					{
						parse_success = FALSE ;
					}
				}
			}
		}

		for( x = OPT_FKEY1 ; x < OPT_FKEYMAX ; x++ )
		{
			if( opts[x] )
			{
				p = (UBYTE *)opts[x] ;
				len = strlen(p) ;
				p += (len-2) ;
//				mysprintf(g->g_buffer, "fkey %ld defined.\n", x - OPT_FKEY1) ;
//				PutStr(g->g_buffer) ;
//				mysprintf(g->g_buffer, "*p = %ld\n", (long)*p) ;
//				PutStr(g->g_buffer) ;
				if( (*p == 92) )
				{
					p++ ;
					if( *p == 'n' || *p == 'N')
					{
//						PutStr("LF terminator!\n") ;
						g->g_fklf[x-OPT_FKEY1] = 1 ;
						len -= 2 ;
//						mysprintf(g->g_buffer, "GP: len = %ld\n", (long)len) ;
//						PutStr(g->g_buffer) ;
					}
				}
					
				CopyMem((UBYTE *)opts[x], g->g_fkeys[x - OPT_FKEY1], len) ;
			}
		}

		if(opts[OPT_MINSIZE]) // user wants a 24 row by 60 column window
		{
			gl->g_minsize = TRUE ;
		}

		if(opts[OPT_NORESIZE]) // user wants no resize gadget
		{
			gl->g_user_resize = 0 ;
		}
		if(opts[OPT_NOJUMP]) // user wants jump scroll turned off
		{
			gl->g_user_jump = 0 ;
		}
		if(opts[OPT_NOWRAP]) // user wants line wrap turned off
		{
			gl->g_user_linewrap = 0 ;
		}
		if(opts[OPT_LOCALFKEY])
		{	
			gl->g_localfkeys = TRUE ;
		}

		FreeArgs(ra_result) ;
	}
	else
	{
		PrintFault(IoErr(), "rl") ;
		parse_success = FALSE ;
	}
	return(parse_success) ;
}

/* ============================================================================
 * ReadConfigFile()  Boolean.  Returns FALSE if anything went wrong, else TRUE. 
 *
 * Reads the configfile that the user specified, places the file (the first 254
 * bytes, anyway) into the passed array, changes all newlines to spaces (except
 * for the last one) and returns.
 *
 * Again, this reads only the first 254 bytes of the file. 
 *
 * The passed array MUST be large enough to handle this (minimum size = 256)
 * ============================================================================
 */

BOOL
ReadConfigFile( STRPTR array, struct glob *g)
{
	DOSBASE(g) ;
	BPTR fh ;
	UBYTE *bp = array ;
	int x ;
	LONG len ;
	BOOL success = FALSE ;

	if( fh = Open( g->g_user_configfile, MODE_OLDFILE))
	{
//		PutStr("cf opened\n") ;
		if((len = Read(fh, bp, (ARRAYSIZE-2))) < (ARRAYSIZE-2))
		{
//			PutStr("cf read\n") ;
			success = TRUE ;
			for( bp = array, x = 0 ; x < len ; x++, bp++ )
			{
				if(*bp == '\n')
				{
					*bp = ' ' ;
				}
			}
			*bp++ = '\n' ;
			*bp = '\0' ;
//			PutStr("RCF: array = '") ;
//			PutStr(array) ;
//			PutStr("'\n") ;
		}
		else
		{
			PutStr("Error Reading config file\n") ;
		}
		Close(fh) ;
	}
	return(success) ;
}
                          
/* ==========================================================================
 * FindFileName()  Boolean.  Returns FALSE if anything went wrong, else TRUE. 
 *
 * This function searches through the original command line entered by 
 * the user for the '-f' or 'file' tokens. If it finds them it copies the
 * next token (allgedly the path&name of the config file) into the global
 * array for further processing.
 * ==========================================================================
 */

BOOL 
FindFileName( struct glob *g )
{
	SYSBASE ;
	DOSBASE(g) ;
	UTILITYBASE(g) ;
	struct CSource cs ;
	LONG error ;
	char *buf = g->g_buffer ; // use the global 'scratch' buffer

	 

	cs.CS_Buffer = GetArgStr() ;
	cs.CS_Length = (long)strlen(cs.CS_Buffer) ;
	cs.CS_CurChr = 0L ;

	while((error = ReadItem(buf, 254L, &cs)) != ITEM_NOTHING)
	{
//		PutStr("FFN 1\n") ;
		if( (Stricmp(buf, "-F") == 0L) || (Stricmp(buf,"FILE") == 0L))
		{
//			PutStr("FFN 2\n") ;
			if((error = ReadItem(buf,254L, &cs)) > 0L)
			{
//				PutStr("FFN 3\n") ;
				CopyMem(buf, g->g_user_configfile, 255L) ;
//				PutStr(g->g_user_configfile) ;
//				PutStr("\n") ;
				return(TRUE) ;
			}
		}
	}
	return(FALSE) ;
}
