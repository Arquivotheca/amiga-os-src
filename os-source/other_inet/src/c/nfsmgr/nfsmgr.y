/* -----------------------------------------------------------------------
 * nfsmgr.y  SAS
 *
 * $Locker:  $
 *
 * $Id: nfsmgr.y,v 1.4 92/12/07 16:43:06 bj Exp $
 *
 * $Revision: 1.4 $
 *
 * $Header: AS225:src/c/nfsmgr/RCS/nfsmgr.y,v 1.4 92/12/07 16:43:06 bj Exp $
 *
 *------------------------------------------------------------------------
 */


%{
/* grammar for NFS manager 
 */

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>

#include <rpc/types.h>
#include <nfs/nfs.h>
#include <nfs/clmsg.h>

/* shared library stuff */
#include <ss/socket.h>

static char host[MAXHOSTNAMELEN], path[NFS_MAXPATHLEN], volume[MAXVOL];

static struct nfs_mount_req nm;
static struct nfs_umount_req nu;

void yyerror( char * );
struct MsgPort *FindClient( int );
struct MsgPort *FindServer( int );
void client( struct Message * );
int nfs_error( struct nfs_req * );
int myungetc( char );
char mygetchar( void );
void refreshclient( void );
void serverstats( char *volume );
void clientstats( char *volume );
void printstats( struct nfs_stats_req * );


%}

%token MOUNT UMOUNT EXPORT UEXPORT STATS KILL QUIT SERVER CLIENT
%token TIMEOUT RETRY RETRIES READONLY WRITE READWRITE CACHESIZE REFRESH
%token STRING NUMBER EQ AT EOL SLASH ALL PORT READSIZE WRITESIZE CASE
%token SPACE COMMENT		/* dummy - don't use in grammar */

%start line
%%

line:	command EOL 
	| line command EOL
	{ bzero(&nm,sizeof(nm)); }
	;


command:
	  umount 
	| mount 
	| export 
	| uexport 
	| stats 
	| kill 
	| quit
	| refresh
	;


refresh:	
	REFRESH CLIENT
		{
			refreshclient();
		}
	;

mount:	MOUNT mountargs
	{
		struct hostent *hp, *gethostbyname();
		char *p, *inet_ntoa();
		struct in_addr addr;
		
		nm.nm_msg.opcode = NFSC_MOUNT;
		if(inet_addr(p = host) == -1){ /* must translate to addr */
			hp = gethostbyname(p);
			if(hp == NULL){
				fprintf(stderr, 
				    "Can't find address for host \"%s\"\n", p);
				YYERROR;
			}
			bcopy(hp->h_addr, &addr, sizeof(addr));
			p = inet_ntoa(addr.s_addr);
		}
		strcpy(nm.nm_host, p);
		strcpy(nm.nm_path, path);
		strcpy(nm.nm_volume, volume);

		if(nm.nm_rdsize <= 0 || nm.nm_rdsize > NFS_MAXDATA){
			nm.nm_rdsize = NFS_MAXDATA;
		}
		if(nm.nm_wrsize <= 0 || nm.nm_wrsize > NFS_MAXDATA){
			nm.nm_wrsize = NFS_MAXDATA;
		}
		if(nm.nm_port == 0){
			nm.nm_port = NFS_PORT;
		}
		if(nm.nm_cachesize == 0){
			nm.nm_cachesize = 4*NFS_MAXDATA;
		}
		if(nm.nm_retry.tv_sec==0 && nm.nm_retry.tv_usec==0){
			nm.nm_retry.tv_sec = 2;
			nm.nm_retry.tv_usec = 0;
		}
		if(nm.nm_total.tv_sec==0 && nm.nm_total.tv_usec==0){
			nm.nm_total.tv_sec = 10;
			nm.nm_total.tv_usec = 0;
		}
		if(nm.nm_volume[0] == 0){
			strcpy(nm.nm_volume, "nfs");
		}
		client(&nm);
		nfs_error(&nm);
	}
	;

mountargs:
	  path volume moptions
	| path volume
	;

path:
	STRING 
		{
			char *p;

			p = strchr($1, ':');
			if(p == 0 || p[1] == 0){
				printf("illegal path spec %s\n", $1);
				YYERROR;
			}
			*p++ = 0;
			strncpy(host, $1, sizeof(host)-1);
			host[sizeof(host) - 1] = 0;

			strncpy(path, p, sizeof(path)-1);
			path[sizeof(path) - 1] = 0;
			free($1);

		}
	;

umount:	UMOUNT STRING 
		{
			nu.nu_msg.opcode = NFSC_UMOUNT;
			strncpy(nu.nu_volume, $2, sizeof(nu.nu_volume)-1);
			free($2);
			nu.nu_volume[sizeof(nu.nu_volume)-1] = 0;

			client(&nu);
			nfs_error(&nu);
		}
	| UMOUNT ALL
		{
			nu.nu_msg.opcode = NFSC_UMOUNT;
			strcpy(nu.nu_volume, "all");
			client(&nu);
			nfs_error(&nu);
		}
	;

volume:	STRING
		{ 
			char *p;

			strncpy(volume, $1, sizeof(volume)-1);	
			volume[sizeof(volume)-1] = 0;
			free($1);
			p = strchr(volume, ':');
			if(p){
				if(p[1] != '\0'){
					printf("illegal volume name %s\n",
						volume);
					YYERROR;
				}
				*p = '\0';
			}
		}
	;

stats:	
	  STATS 
		{
			serverstats("all");
			clientstats("all");
		}
	| STATS CLIENT
		{
			clientstats("all");
		}
	| STATS CLIENT STRING 
		{
			clientstats($3);
		}
	| STATS SERVER 
		{
			serverstats("all");
		}
	| STATS SERVER STRING
		{
			serverstats($3);
		}
	;

quit:	
	  QUIT CLIENT
		{
			struct MsgPort *clientp;

			clientp = FindClient(0);
			if(!clientp){
				fprintf(stderr, "nfsmgr: could not quit client - client is not running\n");
			} else {
				Signal(clientp->mp_SigTask, SIGBREAKF_CTRL_D);
			}
		}
	| QUIT SERVER 
		{
			struct MsgPort *serverp;

			serverp = FindServer(0);
			if(!serverp){
				fprintf(stderr, "nfsmgr: could not quit server - server is not running\n");
			} else {
				Signal(serverp->mp_SigTask, SIGBREAKF_CTRL_D);
			}
		}
	;

kill:	
	  KILL CLIENT
		{
			struct MsgPort *clientp;

			clientp = FindClient(0);
			if(!clientp){
				fprintf(stderr, "nfsmgr: could not kill client - client is not running\n");
			} else {
				Signal(clientp->mp_SigTask, SIGBREAKF_CTRL_C);
			}
		}
	| KILL SERVER 
		{
			struct MsgPort *serverp;

			serverp = FindServer(0);
			if(!serverp){
				fprintf(stderr, "nfsmgr: could not kill server - server is not running\n");
			} else {
				Signal(serverp->mp_SigTask, SIGBREAKF_CTRL_C);
			}
		}
	;

moptions:
	moptions one_m_option 
	| one_m_option
	;

one_m_option:
	  RETRY EQ NUMBER 
		{	
			nm.nm_retry.tv_sec = $3/10; 
			nm.nm_retry.tv_usec = ($3%10)*1000000/10;
		}
	| TIMEOUT EQ NUMBER 
		{	
			nm.nm_total.tv_sec = $3/10; 
			nm.nm_total.tv_usec = ($3%10)*1000000/10;
		}
	| READSIZE EQ NUMBER
		{	nm.nm_rdsize = $3;	}
	| WRITESIZE EQ NUMBER
		{	nm.nm_wrsize = $3;	}
	| CACHESIZE EQ NUMBER
		{	nm.nm_cachesize = $3;	}
	| PORT EQ NUMBER
		{	nm.nm_port = $3;	}
	| READONLY 
		{	nm.nm_readonly = 1;	}
	| CASE
		{	nm.nm_case = 1;		}
	;

user:	STRING 
	| STRING AT STRING
	;

export:
	EXPORT 
	| EXPORT volume 
	| EXPORT volume eoptions
	;

uexport:
	UEXPORT 
	| UEXPORT volume
	;

eoptions:
	eoptions one_e_option 
	| one_e_option
	;

one_e_option:
	READONLY 
	| READONLY EQ user 
	| WRITE EQ user 
	| READWRITE EQ user 
	| CACHESIZE EQ NUMBER
	;
	
%%

static struct {
	char	*name;
	int	token;
} tokens[] = {
	{"mount",	MOUNT}, 	{"unmount",	UMOUNT},
	{"export",	EXPORT}, 	{"unexport",	UEXPORT},
	{"stats",	STATS},		{"kill", 	KILL},
	{"quit", 	QUIT},		{"maxwrite",	WRITESIZE},
	{"client", 	CLIENT},	{"timeout",	TIMEOUT},
	{"retry",	RETRY},		{"retries",	RETRIES},
	{"readonly",	READONLY},	{"cachesize", 	CACHESIZE},
	{"write",	WRITE},		{"readwrite",	READWRITE},
	{"all",		ALL}, 		{"port",	PORT},
	{"maxread",	READSIZE},	{"refresh",	REFRESH},
	{"umount",	UMOUNT},	{"case",	CASE},
#ifdef NFS_SERVER
	{"server", 	SERVER}
#else
	{0, 		0}
#endif
};
#define NUMTOKEN (sizeof(tokens)/sizeof(tokens[0]))

/*
 * return next token.  Token defaults to type STRING unless we find
 * that it exists in the token table, or is one of the few special
 * characters, eg = @ . / that we look for explicitly.  Tokens are generally
 * delimited by whitespace, though we allow the special chars to 
 * delimit (to allow the usual rick@ameristar convention of user/host naming)
 *
 * The scanner works very simply - gather characters up to next whitespace
 * or special character.  Check token table, if fail then try to convert it
 * to number if fail then return STRING.
 */
yylex()
{
	static char sbuf[64];
	int tok;
	char c, *p;

	p = sbuf; 
	do {
		tok = STRING;
		c = mygetchar();
		switch(c){
		case EOF:	tok = EOF; 	break;
		case ' ':
		case '\t':	tok = SPACE; 	break;
		case '\r':
		case '\n':	tok = EOL; 	break;
		case '=':	tok = EQ; 	break;
		case '@':	tok = AT; 	break;
		case '\'':
			while( (c=mygetchar()) != '\'' ) 
				if( p < &sbuf[sizeof(sbuf)-2] )
					*p++ = c;	
			break;
		case '#':
			if( p == sbuf )
			{
				while(((c = mygetchar()) != '\n') && ( c != EOF))
					;
				if( c == EOF )
				{
					tok = EOF ;
					break ;
				}
				p = sbuf ;
				continue ;
			}

		default:
			if(p < &sbuf[sizeof(sbuf)-2])
				*p++ = c;
		}
	} while(tok == STRING || (tok == SPACE && p == sbuf));

	*p = 0;
	if(p != sbuf){		/* we gathered chars then saw delim */
		int	i;

		myungetc(c);
		*p++ = 0;
		for(i = 0; i < NUMTOKEN && tokens[i].name ; i++){
			if(stricmp(tokens[i].name, sbuf) == 0){
				return tokens[i].token;
			}
		}

		for(p = sbuf; *p; p++){
			if(!(*p >= '0' && *p <= '9')){
				break;
			}
		}

		if(*p){
			char	*cp;
			
			cp = malloc(strlen(sbuf)+1);
			strcpy(cp, sbuf);
			yylval = (int)cp; tok = STRING;
		} else {
			yylval = atoi(sbuf); tok = NUMBER;
		}
	}

	return tok;
}

