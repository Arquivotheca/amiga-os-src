/* vers.c */

/* 
 *   $Header: HOG:Other/inet/src/c/vers/RCS/vers.c,v 1.1 90/11/02 18:11:47 bj Exp $
 */
 


#include <stdio.h>
#include <errno.h>
#include <exec/types.h>
#include <libraries/dos.h>
#include "vers.h"
#include "vers_rev.h"

#include <proto/dos.h>

char	v_string[] = VERSION_STRING;
char    ver[] = VERSTAG ;
char	buf[1024];
char    b2[512] ;

extern char *ctime();
extern long time();
int version( char *name ) ;
void printmsg( char *m ) ;
char *basenam( register char *p ) ;

main( int argc, char **argv)
{
	register char *bptr = b2 ;
	char *date;
	int len;
	long t;

	if(argc < 2)
		{
		printmsg("usage: vers program [program ...]\n");
		exit(RETURN_FAIL);
		}

	if(argc == 3 && strcmp(argv[1], "-v") == 0)
		{
		time(&t);
		date = ctime(&t);
		if(time && (len = strlen(date)) > 0)
			{
		    date[len-1] = '\0';
			}
			
		strcpy( bptr, "static char version[] = \"") ;
		strcat( bptr, v_string ) ;
		strcat( bptr, " " ) ;
		strcat( bptr, argv[2] ) ;
		strcat( bptr, " " ) ;
		strcat( bptr, date ) ;
		strcat( bptr, "\" ;\n") ;
		printmsg( bptr ) ;
		exit(RETURN_OK);
		}

	for(argc--, argv++; argc > 0; argc--)
		{
		version(*argv++);
		}

	exit(RETURN_OK);
}

int
version( char *fname )
{
	register int fd, cnt;
	register char *p, *bptr = b2 ;
	register long position;
	int noversion;

	if((fd = open(fname, 0)) < 0)
		{
		strcpy( bptr, "Can't open " ) ;
		strcat( bptr, fname ) ;
		strcat( bptr, "\n") ;
		printmsg( bptr ) ;
		return( RETURN_FAIL) ; 
		}

	for(position = 0; ; position += sizeof(buf) - sizeof(v_string))
		{
		if(lseek(fd, position, 0) < 0)
			{
			break ;
			}

		if((cnt = read( fd, buf, sizeof( buf ))) <= 0)
			{
			break;
			}

		for( p = buf ; cnt-- > 0 ; p++ )
			{
			if(*p==v_string[0] && *(p+1)==v_string[1] 
			   && strncmp(p, v_string, sizeof(v_string)-1) == 0)
				{
				break;
				}
			}
		if( cnt >= 0)
			{
			break;
			}
		}

	position += p - buf;
	noversion = 1;

	if(cnt >= 0 && !(lseek(fd, position, 0) < 0))
		{
		cnt = read(fd, buf, 256);
		if(cnt > 0)
			{
			buf[cnt - 1] = '\0';
			strcpy( bptr, &buf[sizeof(v_string)]) ;
			strcat( bptr, "\n") ;
			printmsg( bptr ) ;
			noversion = 0;
			}
		}

	if(noversion)
		{
		strcpy( bptr, basenam( fname )) ;
		strcat( bptr, " version string not found\n") ;
		printmsg( bptr ) ;
		}

	return( 1 ) ;
}

void
printmsg( char *msg )
{
	if( msg ) Write((BPTR)Output(), msg, (LONG)strlen(msg)) ;
}	


char 
*basenam( register char *p)
{
	register char *op = p ;

	while( *p )
		{
		if(*p == '/' || *p == ':')
			{
			op = ++p;
			} 
		else 
			{
			p++;
			}
		}
	return( op ) ;
}

