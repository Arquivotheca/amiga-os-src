/*
** getservent(), getservbyport(), getservbyname(), setservent(),
** endservent()
**
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <exec/types.h>

#define NULLFP	(FILE *)NULL
#define NULLSERV (struct servent *)NULL
#define NULLCHAR (char *)NULL

static FILE	*servfp;
static 		keepopen;

#define MAXALIAS 10

static char *
getnext(oldpos, bufmax)
register char **oldpos;
register char *bufmax;
{
	register char *op, *pos;

	pos = *oldpos;
	for(pos = *oldpos; pos < bufmax; pos++){
		if(!isspace(*pos))
			break;
	}

	if(pos >= bufmax){
		return(NULLCHAR);
	}

	for(op = pos; op < bufmax; op++){
		if(isspace(*op)){
			break;
		}
	}

	*op++ = 0; *oldpos = op;

	return(pos);
}

struct servent *
getservent( )
{
	static char *aliases[MAXALIAS+1], line[256];
	char *pos, *op, *bufmax, *sp, *index();
	static struct servent s;
	int	i ;

	aliases[0] = aliases[1] = NULLCHAR;

	if(servfp == NULLFP){
		(VOID)setservent(0);
		if(servfp == NULLFP)
			return(NULLSERV);
	}

	do {
		/*
		** get next line & initialize scanner.
		*/
		
		if(!fgets(line, sizeof(line), servfp)){
			return(NULLSERV);
		}
		bufmax = &line[strlen(line)]; pos = line;

		/*
		** Determine whether this is blank line, comment only
		** line, or a service definition.  If service definition,
		** then record official service name in aliases[0].
		*/
		op = getnext(&pos, bufmax);
		if(op == NULLCHAR || *op == '#'){
			continue;
		} else {
			aliases[0] = op;
		}

		/*
		** Grab port/proto tuple.
		*/
		op = getnext(&pos, bufmax);
#ifdef LATTICE
		if(op == NULLCHAR || (sp = (char *)strchr(op, '/')) == NULLCHAR){
#else
		if(op == NULLCHAR || (sp = index(op, '/')) == NULLCHAR){
#endif
			return NULLSERV;
		} else {
			*sp++ = '\0';
			s.s_port = htons(atoi(op));
			s.s_proto = sp;
		}

		/*
		** Construct aliases vector for this service.  Eat 
		** comments for dinner.
		*/
		for(i = 1; i < MAXALIAS; i++){
			op = getnext(&pos, bufmax);
			if(op == NULLCHAR || *op == '#'){
				break;
			}

			aliases[i] = op;
		}
		aliases[i] = NULLCHAR;

		/*
		** If we made it this far, we've matched a line.
		*/
		break;

	} while(1);

	s.s_aliases = &aliases[1];
	s.s_name = aliases[0];

	return(&s);
}

struct servent *
getservbyname(name, proto)
char *name;
char *proto;
{
	register struct servent *sp;
	int	match, i;

	setservent(FALSE);

	for(match = 0; (sp = getservent()) != NULLSERV;){
		if(strcmp(sp->s_name, name) == 0){
			match++;
		} else {
			for(i = 0; sp->s_aliases[i] != NULLCHAR; i++)
				if(strcmp(sp->s_aliases[i], name) == 0){
					match++;
					break;
				}
		}

		if(match && strcmp(proto, sp->s_proto)==0)
			break;
	}

	endservent();
	return(sp);
}

setservent(stayopen)
int stayopen;
{
	keepopen = stayopen;
	if(servfp != NULLFP){
		rewind(servfp);
	} else {
		if((servfp = fopen(SERVFILE, "r")) == NULLFP)
			return(-1);
	}

	return(0);
}

endservent( )
{
	if(servfp != NULLFP && !keepopen){
		fclose(servfp);
		servfp = NULLFP;
	}
	return(0);
}

struct servent *
getservbyport( port, proto)
u_short port;
char *proto;
{
	register struct servent *sp;

	setservent(FALSE);

	for(; (sp = getservent()) != NULLSERV;){
		if((proto == NULLCHAR || strcmp(proto, sp->s_proto) == 0) &&
			port == sp->s_port){
			break;
		}
	}

	endservent();
	return(sp);
}
