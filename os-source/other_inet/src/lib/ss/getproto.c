/* -----------------------------------------------------------------------
 * getproto.c 
 *
 * $Locker:  $
 *
 * $Id: getproto.c,v 1.4 92/07/21 16:25:54 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Log:	getproto.c,v $
 * Revision 1.4  92/07/21  16:25:54  bj
 * details. 
 * 4.0 version.
 * 
 * Revision 1.2  91/08/07  13:21:08  bj
 * rcs header added.
 * 
 *
 * $Header: AS225:src/lib/ss/RCS/getproto.c,v 1.4 92/07/21 16:25:54 bj Exp $
 *
 *------------------------------------------------------------------------
 */
#include "sslib.h"

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define any(cp,match)	strpbrk(cp,match)

BPTR protof =NULL;
UBYTE proto_stayopen = (UBYTE)0;

/****** socket.library/setprotoent ******************************************
*
*   NAME
*	setprotoent -- Open or rewind the protocols file.
*
*   SYNOPSIS
*	setprotoent( flag )
*
*	void setprotoent( int );
*	                  D1
*
*   FUNCTION
*	This function is rarely useful.
*
*	Opens the protocols file if necessary.
*	Rewinds the protocols file if it is open.
*
*   INPUTS
*	flag		- if 'flag' is 1, calls to getprotobyname() and
*			  getprotobynumber() will not close the file
*			  between calls.  You must close the file with an
*			  endprotoent().  Once set, 'flag' cannot be reset
*			  except by calling endprotoent().
*   RESULT
*	None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	getprotoent(), endprotoent(), getprotobyname(), getprotobynumber()
*
******************************************************************************
*
*/


void __saveds __asm setprotoent( register __d1 int flag )
{
	if (protof == NULL)
		protof = Open(PROTOFILE, MODE_OLDFILE );
	else
		Seek(protof,0L,OFFSET_BEGINNING);

	proto_stayopen |= (UBYTE)flag;
}

/****** socket.library/endprotoent ******************************************
*
*   NAME
*	endprotoent -- Closes the protocols file.
*
*   SYNOPSIS
*	endprotoent( )
*
*	void endprotoent( void );
*
*   FUNCTION
*	Closes the protocols file if it is open.  This function is needed
*	only if getprotoent() or setprotoent() have been called.
*
*   INPUTS
*	None.
*
*   RESULT
*	None.
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*	getprotoent(), setprotoent(), getprotobyname(), getprotobynumber()
*
******************************************************************************
*
*/

void __saveds endprotoent( void )
{
	if(protof != NULL){
		Close(protof);
		protof = NULL;
		proto_stayopen = (UBYTE)0;
	}
}

/****** socket.library/getprotoent ******************************************
*
*   NAME
*	getprotoent -- Get a protocol entry from the protocols file.
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	proto = getprotoent ( )
*
*	struct protoent *getprotoent ( void );
*
*   FUNCTION
*	There is normally no reason to use this function.  It is used
*	internally by getprotobyname() and getprotobynumber().  It is
*	provided only for Un*x compatibility.
*
*	Opens the protocols file if necessary.  Returns the next entry
*	in the file in a protoent structure.
*
*	struct	protoent {
*		char    *p_name;        \* official protocol name *\
*		char    **p_aliases;    \* alias list *\
*		int     p_proto;        \* protocol # *\
*	};
*
*
*   INPUTS
*	None.
*
*   RESULT
*	proto		- NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The protoent structure is returned in a buffer that will be
*	overwritten on the next call to getproto*()
*
*   BUGS
*
*   SEE ALSO
*	getprotobyname(), getprotobynumber(), setprotoent(), endprotoent()
*
******************************************************************************
*
*/

struct protoent * __saveds getprotoent( void )
{
	char *p, *cp, **q;
    static struct protoent proto;
    static char line[128];
    static char*proto_aliases[MAXALIASES];

	/* if we haven't opened the protocols file, open it */
	if(protof == NULL){
		setprotoent(0);
		if(protof == NULL)
			return(NULL);
	}

again:
	if ((p = FGets(protof, line, 127)) == NULL)
		return (NULL);

	/* check for comment */
	if (*p == '#')
		goto again;

	/* check for end of line or start of comment */
	cp = any(p, "#\n");
	if (cp == NULL)
		goto again;
	*cp = '\0';

	proto.p_name = p;
	cp = any(p, " \t");
	if (cp == NULL)
		goto again;
	*cp++ = '\0';
	while (*cp == ' ' || *cp == '\t')
		cp++;
	p = any(cp, " \t");
	if (p != NULL)
		*p++ = '\0';

	proto.p_proto = atoi(cp);
	q = proto.p_aliases = proto_aliases;
	if (p != NULL) {
		cp = p;
		while (cp && *cp) {
			if (*cp == ' ' || *cp == '\t') {
				cp++;
				continue;
			}
			if (q < &proto_aliases[MAXALIASES-1])
				*q++ = cp;
			cp = any(cp, " \t");
			if (cp != NULL)
				*cp++ = '\0';
		}
	}
	*q = NULL;
	return (&proto);
}


/****** socket.library/getprotobyname ******************************************
*
*   NAME
*	getprotobyname -- find a protocol entry by name
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	proto = getprotobyname( name )
*	D0                      A0
*
*	struct protoent *getprotobyname( char * );
*
*   FUNCTION
*	Opens the protocols file if necessary.  Returns the entry
*	with a matching name in a protoent structure.
*
*	struct	protoent {
*		char    *p_name;        \* official protocol name *\
*		char    **p_aliases;    \* alias list *\
*		int     p_proto;        \* protocol # *\
*	};
*
*
*   INPUTS
*	name		- name of prototype to return.
*
*   RESULT
*	proto		- pointer to struct protoent for protocol 'name.'
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The protoent structure is returned in a buffer that will be
*	overwritten on the next call to getproto*()
*
*   BUGS
*
*   SEE ALSO
*	getprotobynumber()
*
******************************************************************************
*
*/

struct protoent * __saveds __asm getprotobyname(
	register __a0 char *name)
{
	struct protoent *p;
	char **cp;

	setprotoent(0);

	while (p = getprotoent()) {
		if (strcmp(p->p_name, name) == 0)
			break;
		for (cp = p->p_aliases; *cp != 0; cp++)
			if (strcmp(*cp, name) == 0)
				goto found;
	}
found:
	if(!proto_stayopen)
		endprotoent();
	return (p);
}


/****** socket.library/getprotobynumber ******************************************
*
*   NAME
*	getprotobynumber -- find a protocol entry by number
*
*   SYNOPSIS
*	#include <netdb.h>
*
*	proto = getprotobynumber( proto )
*	D0                        D0
*
*	struct protoent *getprotobynumber( int );
*
*   FUNCTION
*	Opens the protocols file if necessary.  Returns the entry
*	with a matching protocol number in a protoent structure.
*
*	struct	protoent {
*		char    *p_name;        \* official protocol name *\
*		char    **p_aliases;    \* alias list *\
*		int     p_proto;        \* protocol # *\
*	};
*
*
*   INPUTS
*	proto		- number of prototype to return.
*
*   RESULT
*	proto		- pointer to struct protoent for protocol 'number.'
*			  NULL on EOF or failure to open protocols file.
*
*   EXAMPLE
*
*   NOTES
*	The protoent structure is returned in a buffer that will be
*	overwritten on the next call to getproto*()
*
*   BUGS
*
*   SEE ALSO
*	getprotobyname()
*
******************************************************************************
*
*/

struct protoent * __saveds __asm getprotobynumber(
	register __d0 int proto)
{
	struct protoent *p;

	setprotoent(0);

	while (p = getprotoent())
		if (p->p_proto == proto)
			break;

	if(!proto_stayopen)
		endprotoent();
	return (p);
}
