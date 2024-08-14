#ifndef lint
char yysccsid[] = "@(#)yaccpar 1.00 (Berkeley) 1989/11/22";
#endif
#line 17 "nfsmgr.y"
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


short yylhs[] = {                                        -1,
    0,    0,    1,    1,    1,    1,    1,    1,    1,    1,
    9,    3,   10,   10,   11,    2,    2,   12,    6,    6,
    6,    6,    6,    8,    8,    7,    7,   13,   13,   14,
   14,   14,   14,   14,   14,   14,   14,   15,   15,    4,
    4,    4,    5,    5,   16,   16,   17,   17,   17,   17,
   17,
};
short yylen[] = {                                         2,
    2,    3,    1,    1,    1,    1,    1,    1,    1,    1,
    2,    2,    3,    2,    1,    2,    2,    1,    1,    2,
    3,    2,    3,    2,    2,    2,    2,    2,    1,    3,
    3,    3,    3,    3,    3,    1,    1,    1,    3,    1,
    2,    3,    1,    2,    2,    1,    1,    3,    3,    3,
    3,
};
short yydefred[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    3,    4,    5,    6,    7,    8,    9,   10,   15,   12,
    0,   16,   17,   18,    0,   44,    0,    0,   27,   26,
   25,   24,   11,    0,    1,    0,    0,    0,    0,    0,
    0,   46,   23,   21,    2,    0,    0,   36,    0,    0,
    0,    0,   37,    0,   29,    0,    0,    0,    0,   45,
    0,    0,    0,    0,    0,    0,   28,    0,   48,   49,
   50,   51,   31,   30,   34,   35,   32,   33,    0,   39,
};
short yydgoto[] = {                                       9,
   10,   11,   12,   13,   14,   15,   16,   17,   18,   20,
   21,   25,   54,   55,   69,   41,   42,
};
short yysindex[] = {                                   -250,
 -242, -260, -238, -238, -243, -230, -222, -221, -250, -233,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
 -238,    0,    0,    0, -231,    0, -228, -227,    0,    0,
    0,    0,    0, -229,    0, -266, -226, -225, -224, -223,
 -231,    0,    0,    0,    0, -220, -219,    0, -218, -217,
 -216, -215,    0, -266,    0, -212, -212, -212, -211,    0,
 -210, -209, -208, -207, -206, -205,    0, -214,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -203,    0,
};
short yyrindex[] = {                                      0,
    0,    0, -204, -202, -201,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -200,    0, -199, -198,    0,    0,
    0,    0,    0,    0,    0, -197, -245,    0,    0,    0,
 -196,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0, -195,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0, -241,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
   39,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   -2,    0,    1,  -53,    0,   13,
};
#define	YYTABLESIZE		83
short yytable[] = {                                      46,
   47,   26,   48,   70,   71,   49,    1,    2,    3,    4,
    5,    6,    7,   22,   50,   51,   52,   53,   36,   23,
   27,   28,    8,   47,   47,   47,   47,   38,   38,   38,
   38,   19,   47,   29,   30,   24,   38,   37,   38,   39,
   40,   31,   32,   33,   35,   43,   44,   34,   45,   56,
   57,   58,   59,   60,   67,   61,   62,   63,   64,   65,
   66,   68,   79,   72,   73,   74,   75,   76,   77,   78,
   80,    0,    0,   40,    0,   43,   19,   41,   22,   20,
   14,   42,   13,
};
short yycheck[] = {                                     266,
  267,    4,  269,   57,   58,  272,  257,  258,  259,  260,
  261,  262,  263,  274,  281,  282,  283,  284,   21,  280,
  264,  265,  273,  269,  270,  271,  272,  269,  270,  271,
  272,  274,  278,  264,  265,  274,  278,  269,  270,  271,
  272,  264,  265,  265,  278,  274,  274,    9,  278,  276,
  276,  276,  276,   41,   54,  276,  276,  276,  276,  276,
  276,  274,  277,  275,  275,  275,  275,  275,  275,  275,
  274,   -1,   -1,  278,   -1,  278,  278,  278,  278,  278,
  278,  278,  278,
};
#define MOUNT 257
#define UMOUNT 258
#define EXPORT 259
#define UEXPORT 260
#define STATS 261
#define KILL 262
#define QUIT 263
#define SERVER 264
#define CLIENT 265
#define TIMEOUT 266
#define RETRY 267
#define RETRIES 268
#define READONLY 269
#define WRITE 270
#define READWRITE 271
#define CACHESIZE 272
#define REFRESH 273
#define STRING 274
#define NUMBER 275
#define EQ 276
#define AT 277
#define EOL 278
#define SLASH 279
#define ALL 280
#define PORT 281
#define READSIZE 282
#define WRITESIZE 283
#define CASE 284
#define SPACE 285
#define COMMENT 286
#define YYERRCODE 256
#line 153 "y.tab.c"
#define YYFINAL 9
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 286
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"MOUNT","UMOUNT","EXPORT",
"UEXPORT","STATS","KILL","QUIT","SERVER","CLIENT","TIMEOUT","RETRY","RETRIES",
"READONLY","WRITE","READWRITE","CACHESIZE","REFRESH","STRING","NUMBER","EQ",
"AT","EOL","SLASH","ALL","PORT","READSIZE","WRITESIZE","CASE","SPACE","COMMENT",
};
char *yyrule[] = {
"$accept : line",
"line : command EOL",
"line : line command EOL",
"command : umount",
"command : mount",
"command : export",
"command : uexport",
"command : stats",
"command : kill",
"command : quit",
"command : refresh",
"refresh : REFRESH CLIENT",
"mount : MOUNT mountargs",
"mountargs : path volume moptions",
"mountargs : path volume",
"path : STRING",
"umount : UMOUNT STRING",
"umount : UMOUNT ALL",
"volume : STRING",
"stats : STATS",
"stats : STATS CLIENT",
"stats : STATS CLIENT STRING",
"stats : STATS SERVER",
"stats : STATS SERVER STRING",
"quit : QUIT CLIENT",
"quit : QUIT SERVER",
"kill : KILL CLIENT",
"kill : KILL SERVER",
"moptions : moptions one_m_option",
"moptions : one_m_option",
"one_m_option : RETRY EQ NUMBER",
"one_m_option : TIMEOUT EQ NUMBER",
"one_m_option : READSIZE EQ NUMBER",
"one_m_option : WRITESIZE EQ NUMBER",
"one_m_option : CACHESIZE EQ NUMBER",
"one_m_option : PORT EQ NUMBER",
"one_m_option : READONLY",
"one_m_option : CASE",
"user : STRING",
"user : STRING AT STRING",
"export : EXPORT",
"export : EXPORT volume",
"export : EXPORT volume eoptions",
"uexport : UEXPORT",
"uexport : UEXPORT volume",
"eoptions : eoptions one_e_option",
"eoptions : one_e_option",
"one_e_option : READONLY",
"one_e_option : READONLY EQ user",
"one_e_option : WRITE EQ user",
"one_e_option : READWRITE EQ user",
"one_e_option : CACHESIZE EQ NUMBER",
};
#endif
#ifndef YYSTYPE
typedef int YYSTYPE;
#endif
#define yyclearin (yychar=(-1))
#define yyerrok (yyerrflag=0)
#ifndef YYSTACKSIZE
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 300
#endif
#endif
int yybackup;
int yydebug;
int yynerrs;
int yyerrflag;
int yychar;
short *yyssp;
YYSTYPE *yyvsp;
YYSTYPE yyval;
YYSTYPE yylval;
#define yystacksize YYSTACKSIZE
short yyss[YYSTACKSIZE];
YYSTYPE yyvs[YYSTACKSIZE];
#line 338 "nfsmgr.y"

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

#line 362 "y.tab.c"
#define YYACCEPT goto yyaccept
#define YYERROR goto yyerrlab
int
yyparse()
{
    register int yym, yyn, yystate;
#if YYDEBUG
    register char *yys;
    extern char *getenv();

    if (yys = getenv("YYDEBUG"))
    {
        yyn = *yys;
        if (yyn == '0')
            yydebug = 0;
        else if (yyn >= '1' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yybackup = 0;
    yynerrs = 0;
    yyerrflag = 0;
    yychar = (-1);

    yyssp = yyss;
    yyvsp = yyvs;
    *yyssp = yystate = 0;

yyloop:
    if (yyn = yydefred[yystate]) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, reading %d (%s)\n", yystate,
                    yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("yydebug: state %d, shifting to state %d\n",
                    yystate, yytable[yyn]);
#endif
        if (yyssp >= yyss + yystacksize - 1)
        {
            goto yyoverflow;
        }
        *++yyssp = yystate = yytable[yyn];
        *++yyvsp = yylval;
        yychar = (-1);
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;
yynewerror:
    yyerror("syntax error");
yyerrlab:
    ++yynerrs;
yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yyssp]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: state %d, error recovery shifting\
 to state %d\n", *yyssp, yytable[yyn]);
#endif
                if (yyssp >= yyss + yystacksize - 1)
                {
                    goto yyoverflow;
                }
                *++yyssp = yystate = yytable[yyn];
                *++yyvsp = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("yydebug: error recovery discarding state %d\n",
                            *yyssp);
#endif
                if (yyssp <= yyss) goto yyabort;
                --yyssp;
                --yyvsp;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("yydebug: state %d, error recovery discards token %d (%s)\n",
                    yystate, yychar, yys);
        }
#endif
        yychar = (-1);
        goto yyloop;
    }
yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("yydebug: state %d, reducing by rule %d (%s)\n",
                yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    yyval = yyvsp[1-yym];
    switch (yyn)
    {
case 2:
#line 69 "nfsmgr.y"
{ bzero(&nm,sizeof(nm)); }
break;
case 11:
#line 87 "nfsmgr.y"
{
			refreshclient();
		}
break;
case 12:
#line 93 "nfsmgr.y"
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
break;
case 15:
#line 148 "nfsmgr.y"
{
			char *p;

			p = strchr(yyvsp[0], ':');
			if(p == 0 || p[1] == 0){
				printf("illegal path spec %s\n", yyvsp[0]);
				YYERROR;
			}
			*p++ = 0;
			strncpy(host, yyvsp[0], sizeof(host)-1);
			host[sizeof(host) - 1] = 0;

			strncpy(path, p, sizeof(path)-1);
			path[sizeof(path) - 1] = 0;
			free(yyvsp[0]);

		}
break;
case 16:
#line 168 "nfsmgr.y"
{
			nu.nu_msg.opcode = NFSC_UMOUNT;
			strncpy(nu.nu_volume, yyvsp[0], sizeof(nu.nu_volume)-1);
			free(yyvsp[0]);
			nu.nu_volume[sizeof(nu.nu_volume)-1] = 0;

			client(&nu);
			nfs_error(&nu);
		}
break;
case 17:
#line 178 "nfsmgr.y"
{
			nu.nu_msg.opcode = NFSC_UMOUNT;
			strcpy(nu.nu_volume, "all");
			client(&nu);
			nfs_error(&nu);
		}
break;
case 18:
#line 187 "nfsmgr.y"
{ 
			char *p;

			strncpy(volume, yyvsp[0], sizeof(volume)-1);	
			volume[sizeof(volume)-1] = 0;
			free(yyvsp[0]);
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
break;
case 19:
#line 207 "nfsmgr.y"
{
			serverstats("all");
			clientstats("all");
		}
break;
case 20:
#line 212 "nfsmgr.y"
{
			clientstats("all");
		}
break;
case 21:
#line 216 "nfsmgr.y"
{
			clientstats(yyvsp[0]);
		}
break;
case 22:
#line 220 "nfsmgr.y"
{
			serverstats("all");
		}
break;
case 23:
#line 224 "nfsmgr.y"
{
			serverstats(yyvsp[0]);
		}
break;
case 24:
#line 231 "nfsmgr.y"
{
			struct MsgPort *clientp;

			clientp = FindClient(0);
			if(!clientp){
				fprintf(stderr, "nfsmgr: could not quit client - client is not running\n");
			} else {
				Signal(clientp->mp_SigTask, SIGBREAKF_CTRL_D);
			}
		}
break;
case 25:
#line 242 "nfsmgr.y"
{
			struct MsgPort *serverp;

			serverp = FindServer(0);
			if(!serverp){
				fprintf(stderr, "nfsmgr: could not quit server - server is not running\n");
			} else {
				Signal(serverp->mp_SigTask, SIGBREAKF_CTRL_D);
			}
		}
break;
case 26:
#line 256 "nfsmgr.y"
{
			struct MsgPort *clientp;

			clientp = FindClient(0);
			if(!clientp){
				fprintf(stderr, "nfsmgr: could not kill client - client is not running\n");
			} else {
				Signal(clientp->mp_SigTask, SIGBREAKF_CTRL_C);
			}
		}
break;
case 27:
#line 267 "nfsmgr.y"
{
			struct MsgPort *serverp;

			serverp = FindServer(0);
			if(!serverp){
				fprintf(stderr, "nfsmgr: could not kill server - server is not running\n");
			} else {
				Signal(serverp->mp_SigTask, SIGBREAKF_CTRL_C);
			}
		}
break;
case 30:
#line 286 "nfsmgr.y"
{	
			nm.nm_retry.tv_sec = yyvsp[0]/10; 
			nm.nm_retry.tv_usec = (yyvsp[0]%10)*1000000/10;
		}
break;
case 31:
#line 291 "nfsmgr.y"
{	
			nm.nm_total.tv_sec = yyvsp[0]/10; 
			nm.nm_total.tv_usec = (yyvsp[0]%10)*1000000/10;
		}
break;
case 32:
#line 296 "nfsmgr.y"
{	nm.nm_rdsize = yyvsp[0];	}
break;
case 33:
#line 298 "nfsmgr.y"
{	nm.nm_wrsize = yyvsp[0];	}
break;
case 34:
#line 300 "nfsmgr.y"
{	nm.nm_cachesize = yyvsp[0];	}
break;
case 35:
#line 302 "nfsmgr.y"
{	nm.nm_port = yyvsp[0];	}
break;
case 36:
#line 304 "nfsmgr.y"
{	nm.nm_readonly = 1;	}
break;
case 37:
#line 306 "nfsmgr.y"
{	nm.nm_case = 1;		}
break;
#line 738 "y.tab.c"
    }
    yyssp -= yym;
    yystate = *yyssp;
    yyvsp -= yym;
    if (yybackup)
    {
        yybackup = 0;
        goto yyloop;
    }
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#ifdef YYDEBUG
        if (yydebug)
            printf("yydebug: after reduction, shifting from state 0 to\
 state %d\n", YYFINAL);
#endif
        yystate = YYFINAL;
        *++yyssp = YYFINAL;
        *++yyvsp = yyval;
        if (yychar < 0)
        {
            if ((yychar = yylex()) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("yydebug: state %d, reading %d (%s)\n",
                        YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#ifdef YYDEBUG
    if (yydebug)
        printf("yydebug: after reduction, shifting from state %d \
to state %d\n", *yyssp, yystate);
#endif
    if (yyssp >= yyss + yystacksize - 1)
    {
        goto yyoverflow;
    }
    *++yyssp = yystate;
    *++yyvsp = yyval;
    goto yyloop;
yyoverflow:
    yyerror("yacc stack overflow");
yyabort:
    return (1);
yyaccept:
    return (0);
}
