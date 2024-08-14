/* -----------------------------------------------------------------------
 * ruserpass.c  (SAS C)  (as225)  (ftp)
 *
 * $Locker:  $
 *
 * $Id: ruserpass.c,v 2.2 92/10/30 12:46:58 bj Exp $
 *
 * $Revision: 2.2 $
 *
 * $Log:	ruserpass.c,v $
 * Revision 2.2  92/10/30  12:46:58  bj
 * Binary 2.4
 *  
 * Fixed incorrect buffer reference that caused building of path
 * to .netrc file to fail.
 * 
 * Revision 2.1  92/10/30  11:36:43  bj
 * Binary 2.3
 * 
 * Modified the way that the ".netrc" file is handled. Under Unix, the
 * netrc file must have group and other read access turned off. Under
 * Amiga this makes no sense.  Solution was to have FTP automagically
 * turn off _all_ permissions to the file after accessing it.
 * 
 * Fixes the problem where the netrc open would fail to open because it
 * couldn't pass the (Unix) file permissions test.
 * 
 * The code now attempts a lock on the file and, if successful, reads 
 * current file permissions, sets the permissions so that ftp can
 * read the file, does it's thing, closes the file and then turns
 * off the 'rwed' permissions when through. This preserves at least
 * some sense of security (if only in one's dreams :))
 * 
 * Revision 2.0  92/07/20  15:31:08  bj
 * V2.0 source.  MH.
 * 
 *
 * $Header: Hog:Other/inet/src/c/ftp/RCS/ruserpass.c,v 2.2 92/10/30 12:46:58 bj Exp $
 *
 *------------------------------------------------------------------------
 */
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dosextens.h>

#include <sys/types.h>
#include <stdio.h>
#include <utmp.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include "ftp.h"

#define RWFLAGS  (FIBF_READ | FIBF_WRITE | FIBF_EXECUTE | FIBF_DELETE)

int ruserpass(char *, char **, char **, char **);
static int token(void);

char	*renvlook(), *getpass(), *getlogin();
struct	utmp *getutmp();
static	FILE *cfile;

#define	DEFAULT	1
#define	LOGIN	2
#define	PASSWD	3
#define	ACCOUNT 4
#define MACDEF  5
#define	ID	10
#define	MACH	11

static char tokval[100];

static struct toktab {
	char *tokstr;
	int tval;
} toktab[]= {
	"default",	DEFAULT,
	"login",	LOGIN,
	"password",	PASSWD,
	"passwd",	PASSWD,
	"account",	ACCOUNT,
	"machine",	MACH,
	"macdef",	MACDEF,
	0,		0
};

ruserpass(host, aname, apass, aacct)
	char *host, **aname, **apass, **aacct;
{
	char *hdir, buf[BUFSIZ], *tmp, p2;
	char myname[MAXHOSTNAMELEN], *mydomain;
	int t, i, c, usedefault = 0;
	struct stat stb;
	BPTR lock ;
	struct FileInfoBlock *fib ;
	long orig_protections ;
	static int token();

	memset((char *)&stb,0,sizeof(struct stat)) ;
	if(hdir = getenv("HOME"))
	{
	
		p2 = *(hdir + (strlen(hdir)-1) ) ;
		sprintf(buf,"%s%s%s", hdir, ((p2==':' || p2=='/') ? "" : "/"),".netrc") ;
	}
	else /* user has set no HOME env variable */
	{
		sprintf(buf, "inet:s/.netrc") ;
	}
	
	if( lock = Lock(buf, ACCESS_WRITE))
	{
		if(fib = (struct FileInfoBlock *)AllocVec((long)sizeof(struct FileInfoBlock), MEMF_CLEAR))
		{
			if( Examine( lock, fib))
			{
				orig_protections = fib->fib_Protection ;
			}
			FreeVec(fib) ;
		}
		UnLock(lock) ;
		if(SetProtection(buf, orig_protections & ~FIBF_READ))
		{
			cfile = fopen(buf, "r");
			if (cfile == NULL) 
			{
				if (errno != ENOENT)
				{
					perror(buf);
				}
				return(0);
			}
			if (gethostname(myname, sizeof(myname)) < 0)
			{
				myname[0] = '\0';
			}
			if ((mydomain = index(myname, '.')) == NULL)
			{
				mydomain = "";
			}
next:

			while ((t = token())) switch(t) 
			{
		
			case DEFAULT:
				usedefault = 1;
				/* FALL THROUGH */
		
			case MACH:
				if (!usedefault) 
				{
					if (token() != ID)
					{
						continue;
					}
					/*
					 * Allow match either for user's input host name
					 * or official hostname.  Also allow match of 
					 * incompletely-specified host in local domain.
					 */
					if (strcasecmp(host, tokval) == 0)
						goto match;
					if (strcasecmp(hostname, tokval) == 0)
						goto match;
					if ((tmp = index(hostname, '.')) != NULL &&
					    strcasecmp(tmp, mydomain) == 0 &&
					    strncasecmp(hostname, tokval, tmp-hostname) == 0 &&
					    tokval[tmp - hostname] == '\0')
						goto match;
					if ((tmp = index(host, '.')) != NULL &&
					    strcasecmp(tmp, mydomain) == 0 &&
					    strncasecmp(host, tokval, tmp - host) == 0 &&
					    tokval[tmp - host] == '\0')
						goto match;
					continue;
				}
match:
				while ((t = token()) && t != MACH && t != DEFAULT) switch(t) 
				{
		
				case LOGIN:
					if (token())
						if (*aname == 0) 
						{ 
							*aname = malloc((unsigned) strlen(tokval) + 1);
							(void) strcpy(*aname, tokval);
						} 
						else 
						{
							if (strcmp(*aname, tokval))
							{
								goto next;
							}
						}
					break;
					
				case PASSWD:
					if (token() && *apass == 0) 
					{
						*apass = malloc((unsigned) strlen(tokval) + 1);
						(void) strcpy(*apass, tokval);
					}
					break;
					
				case ACCOUNT:
					if (token() && *aacct == 0) 
					{
						*aacct = malloc((unsigned) strlen(tokval) + 1);
						(void) strcpy(*aacct, tokval);
					}
					break;
				case MACDEF:
					if (proxy) 
					{
						(void) fclose(cfile);
						return(0);
					}
					while ((c=getc(cfile)) != EOF && c == ' ' || c == '\t')
						;
					if (c == EOF || c == '\n') 
					{
						printf("Missing macdef name argument.\n");
						goto bad;
					}
					if (macnum == 16) 
					{
						printf("Limit of 16 macros have already been defined\n");
						goto bad;
					}
					tmp = macros[macnum].mac_name;
					*tmp++ = c;
					for (i=0; i < 8 && (c=getc(cfile)) != EOF && !isspace(c); ++i) 
					{
						*tmp++ = c;
					}
					if (c == EOF) 
					{
						printf("Macro definition missing null line terminator.\n");
						goto bad;
					}
					*tmp = '\0';
					if (c != '\n') 
					{
						while ((c=getc(cfile)) != EOF && c != '\n');
					}
					if (c == EOF) 
					{
						printf("Macro definition missing null line terminator.\n");
						goto bad;
					}
					if (macnum == 0) 
					{
						macros[macnum].mac_start = macbuf;
					}
					else 
					{
						macros[macnum].mac_start = macros[macnum-1].mac_end + 1;
					}
					tmp = macros[macnum].mac_start;
					while (tmp != macbuf + 4096) 
					{
						if ((c=getc(cfile)) == EOF) 
						{
							printf("Macro definition missing null line terminator.\n");
							goto bad;
						}
						*tmp = c;
						if (*tmp == '\n') 
						{
							if (*(tmp-1) == '\0') 
							{
							   macros[macnum++].mac_end = tmp - 1;
							   break;
							}
							*tmp = '\0';
						}
						tmp++;
					}
					if (tmp == macbuf + 4096) 
					{
						printf("4K macro buffer exceeded\n");
						goto bad;
					}
					break;
				default:
					fprintf(stderr, "Unknown .netrc keyword %s\n", tokval);
					break;
				}
				goto done;
			}
		done:
			(void) fclose(cfile);
			SetProtection(buf, orig_protections | RWFLAGS) ;
			return(0);
		bad:
			(void) fclose(cfile);
			SetProtection(buf, orig_protections | RWFLAGS) ;
			return(-1);
		}
	}
	else
	{
		fprintf(stderr, "Can't open .netrc file '%s'\n", buf) ;
	}
}

static int
token()
{
	char *cp;
	int c;
	struct toktab *t;

	if (feof(cfile))
		return (0);
	while ((c = getc(cfile)) != EOF &&
	    (c == '\n' || c == '\t' || c == ' ' || c == ','))
		continue;
	if (c == EOF)
		return (0);
	cp = tokval;
	if (c == '"') {
		while ((c = getc(cfile)) != EOF && c != '"') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	} else {
		*cp++ = c;
		while ((c = getc(cfile)) != EOF
		    && c != '\n' && c != '\t' && c != ' ' && c != ',') {
			if (c == '\\')
				c = getc(cfile);
			*cp++ = c;
		}
	}
	*cp = 0;
	if (tokval[0] == 0)
		return (0);
	for (t = toktab; t->tokstr; t++)
		if (!strcmp(t->tokstr, tokval))
			return (t->tval);
	return (ID);
}
