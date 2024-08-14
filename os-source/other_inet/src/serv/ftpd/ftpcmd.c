#ifndef lint
char yysccsid[] = "@(#)yaccpar 1.00 (Berkeley) 1989/11/22";
#endif
#line 26 "ftpcmd.y"

#ifndef lint
static char sccsid[] = "@(#)ftpcmd.y	5.13 (Berkeley) 11/30/88";
#endif /* not lint */

#include <sys/types.h>
#include <sys/socket.h>
#ifdef AMIGA
#include <sys/time.h>
#endif

#include <netinet/in.h>

#include <arpa/ftp.h>

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <pwd.h>
#include <setjmp.h>
#include <syslog.h>

extern	struct sockaddr_in data_dest;
extern	int logged_in;
extern	struct passwd *pw;
extern	int guest;
extern	int logging;
extern	int type;
extern	int form;
extern	int debug;
extern	int timeout;
extern  int pdata;
extern	char hostname[];
extern	char *globerr;
extern	int usedefault;
extern	int unique;
extern  int transflag;
extern  char tmpline[];
char	**glob();

static	int cmd_type;
static	int cmd_form;
static	int cmd_bytesz;
char cbuf[512];
char *fromname;

char	*index();
short yylhs[] = {                                        -1,
    0,    0,    0,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    2,    3,    4,   11,    5,   12,   12,
   12,    6,    6,    6,    6,    6,    6,    6,    6,    7,
    7,    7,    8,    8,    8,   10,   13,    9,
};
short yylen[] = {                                         2,
    0,    2,    2,    4,    4,    4,    2,    4,    4,    4,
    4,    5,    5,    5,    3,    5,    3,    5,    5,    4,
    2,    3,    5,    2,    4,    2,    5,    5,    3,    3,
    5,    2,    2,    5,    1,    1,    1,   11,    1,    1,
    1,    1,    3,    1,    3,    1,    1,    3,    2,    1,
    1,    1,    1,    1,    1,    1,    1,    0,
};
short yydefred[] = {                                      1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
   58,   58,   58,    0,   58,    0,    0,   58,   58,   58,
   58,    0,    0,   58,   58,   58,   58,   58,    2,    3,
   33,    0,    0,   32,    0,    7,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   21,    0,    0,    0,    0,
    0,   24,   26,    0,    0,    0,    0,    0,   35,    0,
   36,    0,    0,    0,    0,    0,   46,    0,    0,   50,
   52,   51,    0,   54,   55,   53,    0,    0,    0,    0,
    0,    0,   57,    0,   56,    0,    0,   22,    0,   17,
    0,   15,    0,    0,    0,   29,   30,    0,    4,    5,
    0,    6,    0,    0,    0,   37,   49,    8,    9,   10,
    0,    0,    0,   11,    0,   20,    0,    0,    0,    0,
   25,    0,    0,    0,    0,   41,   39,   40,   43,   45,
   48,   12,   13,   14,   34,   19,   23,   18,   16,   27,
   28,   31,    0,    0,    0,    0,    0,    0,    0,   38,
};
short yydgoto[] = {                                       1,
   29,   30,   60,   62,   64,   69,   73,   77,   40,   84,
  107,  129,   85,
};
short yysindex[] = {                                      0,
 -256, -267, -248, -210, -202, -194, -199, -192, -189, -188,
    0,    0,    0, -186,    0, -185, -184,    0,    0,    0,
    0, -264, -183,    0,    0,    0,    0,    0,    0,    0,
    0, -190, -182,    0, -181,    0, -226, -187, -247, -180,
 -178, -176, -179, -174, -175,    0, -173, -237, -222, -207,
 -172,    0,    0, -171, -170, -169, -168, -166,    0, -165,
    0, -164, -167, -163, -161, -160,    0, -234, -159,    0,
    0,    0, -158,    0,    0,    0, -157, -175, -175, -175,
 -156, -175,    0, -155,    0, -175, -175,    0, -175,    0,
 -175,    0, -154, -175, -175,    0,    0, -175,    0,    0,
 -153,    0, -198, -198, -152,    0,    0,    0,    0,    0,
 -151, -148, -147,    0, -146,    0, -145, -144, -143, -142,
    0, -141, -140, -139, -138,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0, -137, -136, -135, -134, -133, -132, -131,    0,
};
short yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0, -129, -127,    0, -126,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
};
short yygindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,  -11,  -22,
  -20,  -16,    0,
};
#define	YYTABLESIZE		144
short yytable[] = {                                       2,
   41,   42,   31,   44,   51,   52,   47,   48,   49,   50,
   74,   75,   54,   55,   56,   57,   58,    3,    4,   76,
   32,    5,    6,    7,    8,    9,   10,   11,   12,   13,
   65,   87,   88,   66,  105,   67,   68,   14,  106,   15,
   16,   17,   18,   19,   20,   21,   89,   90,   22,   23,
   24,   25,   26,   27,   28,  111,  112,  113,   33,  115,
  126,   91,   92,  117,  118,  127,  119,   34,  120,  128,
   36,  122,  123,   70,   35,  124,   37,   71,   72,   38,
   39,   59,   43,   45,  131,   46,   53,  130,   78,   61,
   79,   63,   80,   81,   82,   86,   83,   94,   95,   93,
   96,   97,   98,  101,   99,  100,  102,  103,  104,    0,
  108,  109,  110,  114,  116,  121,    0,    0,  132,  125,
  106,  133,  134,  135,  136,  137,  138,  139,  140,  141,
  142,    0,  143,    0,  145,  144,  147,  146,  149,  148,
   42,  150,   44,   47,
};
short yycheck[] = {                                     256,
   12,   13,  270,   15,  269,  270,   18,   19,   20,   21,
  258,  259,   24,   25,   26,   27,   28,  274,  275,  267,
  269,  278,  279,  280,  281,  282,  283,  284,  285,  286,
  257,  269,  270,  260,  269,  262,  263,  294,  273,  296,
  297,  298,  299,  300,  301,  302,  269,  270,  305,  306,
  307,  308,  309,  310,  311,   78,   79,   80,  269,   82,
  259,  269,  270,   86,   87,  264,   89,  270,   91,  268,
  270,   94,   95,  261,  269,   98,  269,  265,  266,  269,
  269,  272,  269,  269,  105,  270,  270,  104,  269,  272,
  269,  273,  269,  273,  269,  269,  272,  269,  269,  272,
  270,  270,  269,  271,  270,  270,  270,  269,  269,   -1,
  270,  270,  270,  270,  270,  270,   -1,   -1,  270,  273,
  273,  270,  270,  270,  270,  270,  270,  270,  270,  270,
  270,   -1,  271,   -1,  271,  273,  271,  273,  271,  273,
  270,  273,  270,  270,
};
#define A 257
#define B 258
#define C 259
#define E 260
#define F 261
#define I 262
#define L 263
#define N 264
#define P 265
#define R 266
#define S 267
#define T 268
#define SP 269
#define CRLF 270
#define COMMA 271
#define STRING 272
#define NUMBER 273
#define USER 274
#define PASS 275
#define ACCT 276
#define REIN 277
#define QUIT 278
#define PORT 279
#define PASV 280
#define TYPE 281
#define STRU 282
#define MODE 283
#define RETR 284
#define STOR 285
#define APPE 286
#define MLFL 287
#define MAIL 288
#define MSND 289
#define MSOM 290
#define MSAM 291
#define MRSQ 292
#define MRCP 293
#define ALLO 294
#define REST 295
#define RNFR 296
#define RNTO 297
#define ABOR 298
#define DELE 299
#define CWD 300
#define LIST 301
#define NLST 302
#define SITE 303
#define STAT 304
#define HELP 305
#define NOOP 306
#define XMKD 307
#define XRMD 308
#define XPWD 309
#define XCUP 310
#define STOU 311
#define LEXERR 312
#define YYERRCODE 256
#line 219 "y.tab.c"
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 312
#if YYDEBUG
char *yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"A","B","C","E","F","I","L","N",
"P","R","S","T","SP","CRLF","COMMA","STRING","NUMBER","USER","PASS","ACCT",
"REIN","QUIT","PORT","PASV","TYPE","STRU","MODE","RETR","STOR","APPE","MLFL",
"MAIL","MSND","MSOM","MSAM","MRSQ","MRCP","ALLO","REST","RNFR","RNTO","ABOR",
"DELE","CWD","LIST","NLST","SITE","STAT","HELP","NOOP","XMKD","XRMD","XPWD",
"XCUP","STOU","LEXERR",
};
char *yyrule[] = {
"$accept : cmd_list",
"cmd_list :",
"cmd_list : cmd_list cmd",
"cmd_list : cmd_list rcmd",
"cmd : USER SP username CRLF",
"cmd : PASS SP password CRLF",
"cmd : PORT SP host_port CRLF",
"cmd : PASV CRLF",
"cmd : TYPE SP type_code CRLF",
"cmd : STRU SP struct_code CRLF",
"cmd : MODE SP mode_code CRLF",
"cmd : ALLO SP NUMBER CRLF",
"cmd : RETR check_login SP pathname CRLF",
"cmd : STOR check_login SP pathname CRLF",
"cmd : APPE check_login SP pathname CRLF",
"cmd : NLST check_login CRLF",
"cmd : NLST check_login SP pathname CRLF",
"cmd : LIST check_login CRLF",
"cmd : LIST check_login SP pathname CRLF",
"cmd : DELE check_login SP pathname CRLF",
"cmd : RNTO SP pathname CRLF",
"cmd : ABOR CRLF",
"cmd : CWD check_login CRLF",
"cmd : CWD check_login SP pathname CRLF",
"cmd : HELP CRLF",
"cmd : HELP SP STRING CRLF",
"cmd : NOOP CRLF",
"cmd : XMKD check_login SP pathname CRLF",
"cmd : XRMD check_login SP pathname CRLF",
"cmd : XPWD check_login CRLF",
"cmd : XCUP check_login CRLF",
"cmd : STOU check_login SP pathname CRLF",
"cmd : QUIT CRLF",
"cmd : error CRLF",
"rcmd : RNFR check_login SP pathname CRLF",
"username : STRING",
"password : STRING",
"byte_size : NUMBER",
"host_port : NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER COMMA NUMBER",
"form_code : N",
"form_code : T",
"form_code : C",
"type_code : A",
"type_code : A SP form_code",
"type_code : E",
"type_code : E SP form_code",
"type_code : I",
"type_code : L",
"type_code : L SP byte_size",
"type_code : L byte_size",
"struct_code : F",
"struct_code : R",
"struct_code : P",
"mode_code : S",
"mode_code : B",
"mode_code : C",
"pathname : pathstring",
"pathstring : STRING",
"check_login :",
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
#line 502 "ftpcmd.y"

extern jmp_buf errcatch;

#define	CMD	0	/* beginning of command */
#define	ARGS	1	/* expect miscellaneous arguments */
#define	STR1	2	/* expect SP followed by STRING */
#define	STR2	3	/* expect STRING */
#define	OSTR	4	/* optional STRING */

struct tab {
	char	*name;
	short	token;
	short	state;
	short	implemented;	/* 1 if command is implemented */
	char	*help;
};

struct tab cmdtab[] = {		/* In order defined in RFC 765 */
	{ "USER", USER, STR1, 1,	"<sp> username" },
	{ "PASS", PASS, STR1, 1,	"<sp> password" },
	{ "ACCT", ACCT, STR1, 0,	"(specify account)" },
	{ "REIN", REIN, ARGS, 0,	"(reinitialize server state)" },
	{ "QUIT", QUIT, ARGS, 1,	"(terminate service)", },
	{ "PORT", PORT, ARGS, 1,	"<sp> b0, b1, b2, b3, b4" },
	{ "PASV", PASV, ARGS, 1,	"(set server in passive mode)" },
	{ "TYPE", TYPE, ARGS, 1,	"<sp> [ A | E | I | L ]" },
	{ "STRU", STRU, ARGS, 1,	"(specify file structure)" },
	{ "MODE", MODE, ARGS, 1,	"(specify transfer mode)" },
	{ "RETR", RETR, STR1, 1,	"<sp> file-name" },
	{ "STOR", STOR, STR1, 1,	"<sp> file-name" },
	{ "APPE", APPE, STR1, 1,	"<sp> file-name" },
	{ "MLFL", MLFL, OSTR, 0,	"(mail file)" },
	{ "MAIL", MAIL, OSTR, 0,	"(mail to user)" },
	{ "MSND", MSND, OSTR, 0,	"(mail send to terminal)" },
	{ "MSOM", MSOM, OSTR, 0,	"(mail send to terminal or mailbox)" },
	{ "MSAM", MSAM, OSTR, 0,	"(mail send to terminal and mailbox)" },
	{ "MRSQ", MRSQ, OSTR, 0,	"(mail recipient scheme question)" },
	{ "MRCP", MRCP, STR1, 0,	"(mail recipient)" },
	{ "ALLO", ALLO, ARGS, 1,	"allocate storage (vacuously)" },
	{ "REST", REST, STR1, 0,	"(restart command)" },
	{ "RNFR", RNFR, STR1, 1,	"<sp> file-name" },
	{ "RNTO", RNTO, STR1, 1,	"<sp> file-name" },
	{ "ABOR", ABOR, ARGS, 1,	"(abort operation)" },
	{ "DELE", DELE, STR1, 1,	"<sp> file-name" },
	{ "CWD",  CWD,  OSTR, 1,	"[ <sp> directory-name]" },
	{ "XCWD", CWD,	OSTR, 1,	"[ <sp> directory-name ]" },
	{ "LIST", LIST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "NLST", NLST, OSTR, 1,	"[ <sp> path-name ]" },
	{ "SITE", SITE, STR1, 0,	"(get site parameters)" },
	{ "STAT", STAT, OSTR, 0,	"(get server status)" },
	{ "HELP", HELP, OSTR, 1,	"[ <sp> <string> ]" },
	{ "NOOP", NOOP, ARGS, 1,	"" },
	{ "MKD",  XMKD, STR1, 1,	"<sp> path-name" },
	{ "XMKD", XMKD, STR1, 1,	"<sp> path-name" },
	{ "RMD",  XRMD, STR1, 1,	"<sp> path-name" },
	{ "XRMD", XRMD, STR1, 1,	"<sp> path-name" },
	{ "PWD",  XPWD, ARGS, 1,	"(return current directory)" },
	{ "XPWD", XPWD, ARGS, 1,	"(return current directory)" },
	{ "CDUP", XCUP, ARGS, 1,	"(change to parent directory)" },
	{ "XCUP", XCUP, ARGS, 1,	"(change to parent directory)" },
	{ "STOU", STOU, STR1, 1,	"<sp> file-name" },
	{ NULL,   0,    0,    0,	0 }
};

struct tab *
lookup(cmd)
	char *cmd;
{
	register struct tab *p;

	for (p = cmdtab; p->name != NULL; p++)
		if (strcmp(cmd, p->name) == 0)
			return (p);
	return (0);
}

#include <arpa/telnet.h>

/*
 * getline - a hacked up version of fgets to ignore TELNET escape codes.
 */
char *
getline(s, n, iop)
	char *s;
	register FILE *iop;
{
	register c;
	register char *cs;

	cs = s;
/* tmpline may contain saved command from urgent mode interruption */
	for (c = 0; tmpline[c] != '\0' && --n > 0; ++c) {
		*cs++ = tmpline[c];
		if (tmpline[c] == '\n') {
			*cs++ = '\0';
			if (debug) {
				syslog(LOG_DEBUG, "FTPD: command: %s", s);
			}
			tmpline[0] = '\0';
			return(s);
		}
		if (c == 0) {
			tmpline[0] = '\0';
		}
	}
	while (--n > 0 && (c = getc(iop)) != EOF) {
		while ((0377&c) == IAC) {
			switch (0377&(c = getc(iop))) {
			case WILL:
			case WONT:
				c = getc(iop);
				printf("%c%c%c", IAC, WONT, 0377&c);
				(void) fflush(stdout);
				break;
			case DO:
			case DONT:
				c = getc(iop);
				printf("%c%c%c", IAC, DONT, 0377&c);
				(void) fflush(stdout);
				break;
			default:
				break;
			}
			c = getc(iop); /* try next character */
		}
		*cs++ = 0377&c;
		if ((0377&c) == '\n')
			break;
	}
	if (c == EOF && cs == s)
		return (NULL);
	*cs++ = '\0';
	if (debug) {
		syslog(LOG_DEBUG, "FTPD: command: %s", s);
	}
	return (s);
}

static int
toolong()
{
	time_t now;
	extern char *ctime();
	extern time_t time();

	reply(421,
	  "Timeout (%d seconds): closing control connection.", timeout);
	(void) time(&now);
	if (logging) {
		syslog(LOG_INFO,
			"FTPD: User %s timed out after %d seconds at %s",
			(pw ? pw -> pw_name : "unknown"), timeout, ctime(&now));
	}
	dologout(1);
}

yylex()
{
	static int cpos, state;
	register char *cp;
	register struct tab *p;
	int n;
	char c, *strpbrk();

	for (;;) {
		switch (state) {

		case CMD:
			(void) signal(SIGALRM, toolong);
			(void) alarm((unsigned) timeout);
			if (getline(cbuf, sizeof(cbuf)-1, stdin) == NULL) {
				reply(221, "You could at least say goodbye.");
				dologout(0);
			}
			(void) alarm(0);
			if ((cp = index(cbuf, '\r'))) {
				*cp++ = '\n'; *cp = '\0';
			}
			if ((cp = strpbrk(cbuf, " \n")))
				cpos = cp - cbuf;
			if (cpos == 0) {
				cpos = 4;
			}
			c = cbuf[cpos];
			cbuf[cpos] = '\0';
			upper(cbuf);
			p = lookup(cbuf);
			cbuf[cpos] = c;
			if (p != 0) {
				if (p->implemented == 0) {
					nack(p->name);
					longjmp(errcatch,0);
					/* NOTREACHED */
				}
				state = p->state;
				yylval = (int) p->name;
				return (p->token);
			}
			break;

		case OSTR:
			if (cbuf[cpos] == '\n') {
				state = CMD;
				return (CRLF);
			}
			/* FALL THRU */

		case STR1:
			if (cbuf[cpos] == ' ') {
				cpos++;
				state = STR2;
				return (SP);
			}
			break;

		case STR2:
			cp = &cbuf[cpos];
			n = strlen(cp);
			cpos += n - 1;
			/*
			 * Make sure the string is nonempty and \n terminated.
			 */
			if (n > 1 && cbuf[cpos] == '\n') {
				cbuf[cpos] = '\0';
				yylval = copy(cp);
				cbuf[cpos] = '\n';
				state = ARGS;
				return (STRING);
			}
			break;

		case ARGS:
			if (isdigit(cbuf[cpos])) {
				cp = &cbuf[cpos];
				while (isdigit(cbuf[++cpos]))
					;
				c = cbuf[cpos];
				cbuf[cpos] = '\0';
				yylval = atoi(cp);
				cbuf[cpos] = c;
				return (NUMBER);
			}
			switch (cbuf[cpos++]) {

			case '\n':
				state = CMD;
				return (CRLF);

			case ' ':
				return (SP);

			case ',':
				return (COMMA);

			case 'A':
			case 'a':
				return (A);

			case 'B':
			case 'b':
				return (B);

			case 'C':
			case 'c':
				return (C);

			case 'E':
			case 'e':
				return (E);

			case 'F':
			case 'f':
				return (F);

			case 'I':
			case 'i':
				return (I);

			case 'L':
			case 'l':
				return (L);

			case 'N':
			case 'n':
				return (N);

			case 'P':
			case 'p':
				return (P);

			case 'R':
			case 'r':
				return (R);

			case 'S':
			case 's':
				return (S);

			case 'T':
			case 't':
				return (T);

			}
			break;

		default:
			fatal("Unknown state in scanner.");
		}
		yyerror((char *) 0);
		state = CMD;
		longjmp(errcatch,0);
	}
}

upper(s)
	register char *s;
{
	while (*s != '\0') {
		if (islower(*s))
			*s = toupper(*s);
		s++;
	}
}

copy(s)
	char *s;
{
	char *p;
	extern char *malloc(), *strcpy();

	p = malloc((unsigned) strlen(s) + 1);
	if (p == NULL)
		fatal("Ran out of memory.");
	(void) strcpy(p, s);
	return ((int)p);
}

help(s)
	char *s;
{
	register struct tab *c;
	register int width, NCMDS;

	width = 0, NCMDS = 0;
	for (c = cmdtab; c->name != NULL; c++) {
		int len = strlen(c->name) + 1;

		if (len > width)
			width = len;
		NCMDS++;
	}
	width = (width + 8) &~ 7;
	if (s == 0) {
		register int i, j, w;
		int columns, lines;

		lreply(214,
	  "The following commands are recognized (* =>'s unimplemented).");
		columns = 76 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			printf("   ");
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				printf("%s%c", c->name,
					c->implemented ? ' ' : '*');
				if (c + lines >= &cmdtab[NCMDS])
					break;
				w = strlen(c->name) + 1;
				while (w < width) {
					putchar(' ');
					w++;
				}
			}
			printf("\r\n");
		}
		(void) fflush(stdout);
		reply(214, "Direct comments to ftp-bugs@%s.", hostname);
		return;
	}
	upper(s);
	c = lookup(s);
	if (c == (struct tab *)0) {
		reply(502, "Unknown command %s.", s);
		return;
	}
	if (c->implemented)
		reply(214, "Syntax: %s %s", c->name, c->help);
	else
		reply(214, "%-*s\t%s; unimplemented.", width, c->name, c->help);
}
#line 720 "y.tab.c"
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
#line 97 "ftpcmd.y"
 {
			fromname = (char *) 0;
		}
break;
case 4:
#line 104 "ftpcmd.y"
 {
			extern struct passwd *sgetpwnam();

			logged_in = 0;
			if (strcmp((char *) yyvsp[-1], "ftp") == 0 ||
			  strcmp((char *) yyvsp[-1], "anonymous") == 0) {
				if ((pw = sgetpwnam("ftp")) != NULL) {
					guest = 1;
					reply(331,
				  "Guest login ok, send ident as password.");
				}
				else {
					reply(530, "User %s unknown.", yyvsp[-1]);
				}
			} else if (checkuser((char *) yyvsp[-1])) {
				guest = 0;
				pw = sgetpwnam((char *) yyvsp[-1]);
				if (pw == NULL) {
					reply(530, "User %s unknown.", yyvsp[-1]);
				}
				else {
				    reply(331, "Password required for %s.", yyvsp[-1]);
				}
			} else {
				reply(530, "User %s access denied.", yyvsp[-1]);
			}
			free((char *) yyvsp[-1]);
		}
break;
case 5:
#line 133 "ftpcmd.y"
 {
			pass((char *) yyvsp[-1]);
			free((char *) yyvsp[-1]);
		}
break;
case 6:
#line 138 "ftpcmd.y"
 {
			usedefault = 0;
			if (pdata > 0) {
				(void) close(pdata);
			}
			pdata = -1;
			reply(200, "PORT command successful.");
		}
break;
case 7:
#line 147 "ftpcmd.y"
 {
			passive();
		}
break;
case 8:
#line 151 "ftpcmd.y"
 {
			switch (cmd_type) {

			case TYPE_A:
				if (cmd_form == FORM_N) {
					reply(200, "Type set to A.");
					type = cmd_type;
					form = cmd_form;
				} else
					reply(504, "Form must be N.");
				break;

			case TYPE_E:
				reply(504, "Type E not implemented.");
				break;

			case TYPE_I:
				reply(200, "Type set to I.");
				type = cmd_type;
				break;

			case TYPE_L:
				if (cmd_bytesz == 8) {
					reply(200,
					    "Type set to L (byte size 8).");
					type = cmd_type;
				} else
					reply(504, "Byte size must be 8.");
			}
		}
break;
case 9:
#line 182 "ftpcmd.y"
 {
			switch (yyvsp[-1]) {

			case STRU_F:
				reply(200, "STRU F ok.");
				break;

			default:
				reply(504, "Unimplemented STRU type.");
			}
		}
break;
case 10:
#line 194 "ftpcmd.y"
 {
			switch (yyvsp[-1]) {

			case MODE_S:
				reply(200, "MODE S ok.");
				break;

			default:
				reply(502, "Unimplemented MODE type.");
			}
		}
break;
case 11:
#line 206 "ftpcmd.y"
 {
			reply(202, "ALLO command ignored.");
		}
break;
case 12:
#line 210 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				retrieve((char *) 0, (char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 13:
#line 217 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				store((char *) yyvsp[-1], "w");
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 14:
#line 224 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				store((char *) yyvsp[-1], "a");
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 15:
#line 231 "ftpcmd.y"
 {
			if (yyvsp[-1])
				retrieve("/bin/ls -k -X 1", "");
		}
break;
case 16:
#line 236 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				retrieve("/bin/ls -k -X 1 %s", (char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 17:
#line 243 "ftpcmd.y"
 {
			if (yyvsp[-1])
				retrieve("/bin/ls -l", "");
		}
break;
case 18:
#line 248 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				retrieve("/bin/ls -l %s", (char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 19:
#line 255 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				delete((char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 20:
#line 262 "ftpcmd.y"
 {
			if (fromname) {
				renamecmd(fromname, (char *) yyvsp[-1]);
				free(fromname);
				fromname = (char *) 0;
			} else {
				reply(503, "Bad sequence of commands.");
			}
			free((char *) yyvsp[-1]);
		}
break;
case 21:
#line 273 "ftpcmd.y"
 {
			reply(225, "ABOR command successful.");
		}
break;
case 22:
#line 277 "ftpcmd.y"
 {
			if (yyvsp[-1])
				cwd(pw->pw_dir);
		}
break;
case 23:
#line 282 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				cwd((char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 24:
#line 289 "ftpcmd.y"
 {
			help((char *) 0);
		}
break;
case 25:
#line 293 "ftpcmd.y"
 {
			help((char *) yyvsp[-1]);
		}
break;
case 26:
#line 297 "ftpcmd.y"
 {
			reply(200, "NOOP command successful.");
		}
break;
case 27:
#line 301 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				makedir((char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 28:
#line 308 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL)
				removedir((char *) yyvsp[-1]);
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 29:
#line 315 "ftpcmd.y"
 {
			if (yyvsp[-1])
				pwd();
		}
break;
case 30:
#line 320 "ftpcmd.y"
 {
			if (yyvsp[-1])
#ifdef AMIGA
				cwd("/");
#else
				cwd("..");
#endif
		}
break;
case 31:
#line 329 "ftpcmd.y"
 {
			if (yyvsp[-3] && yyvsp[-1] != NULL) {
				unique++;
				store((char *) yyvsp[-1], "w");
				unique = 0;
			}
			if (yyvsp[-1] != NULL)
				free((char *) yyvsp[-1]);
		}
break;
case 32:
#line 339 "ftpcmd.y"
 {
			reply(221, "Goodbye.");
			dologout(0);
		}
break;
case 33:
#line 344 "ftpcmd.y"
 {
			yyerrok;
		}
break;
case 34:
#line 350 "ftpcmd.y"
 {
			char *renamefrom();

			if (yyvsp[-3] && yyvsp[-1]) {
				fromname = renamefrom((char *) yyvsp[-1]);
				if (fromname == (char *) 0 && yyvsp[-1]) {
					free((char *) yyvsp[-1]);
				}
			}
		}
break;
case 38:
#line 373 "ftpcmd.y"
 {
			register char *a, *p;

			a = (char *)&data_dest.sin_addr;
			a[0] = yyvsp[-10]; a[1] = yyvsp[-8]; a[2] = yyvsp[-6]; a[3] = yyvsp[-4];
			p = (char *)&data_dest.sin_port;
			p[0] = yyvsp[-2]; p[1] = yyvsp[0];
			data_dest.sin_family = AF_INET;
		}
break;
case 39:
#line 385 "ftpcmd.y"
 {
		yyval  = FORM_N;
	}
break;
case 40:
#line 389 "ftpcmd.y"
 {
		yyval  = FORM_T;
	}
break;
case 41:
#line 393 "ftpcmd.y"
 {
		yyval  = FORM_C;
	}
break;
case 42:
#line 399 "ftpcmd.y"
 {
		cmd_type = TYPE_A;
		cmd_form = FORM_N;
	}
break;
case 43:
#line 404 "ftpcmd.y"
 {
		cmd_type = TYPE_A;
		cmd_form = yyvsp[0];
	}
break;
case 44:
#line 409 "ftpcmd.y"
 {
		cmd_type = TYPE_E;
		cmd_form = FORM_N;
	}
break;
case 45:
#line 414 "ftpcmd.y"
 {
		cmd_type = TYPE_E;
		cmd_form = yyvsp[0];
	}
break;
case 46:
#line 419 "ftpcmd.y"
 {
		cmd_type = TYPE_I;
	}
break;
case 47:
#line 423 "ftpcmd.y"
 {
		cmd_type = TYPE_L;
		cmd_bytesz = 8;
	}
break;
case 48:
#line 428 "ftpcmd.y"
 {
		cmd_type = TYPE_L;
		cmd_bytesz = yyvsp[0];
	}
break;
case 49:
#line 434 "ftpcmd.y"
 {
		cmd_type = TYPE_L;
		cmd_bytesz = yyvsp[0];
	}
break;
case 50:
#line 441 "ftpcmd.y"
 {
		yyval  = STRU_F;
	}
break;
case 51:
#line 445 "ftpcmd.y"
 {
		yyval  = STRU_R;
	}
break;
case 52:
#line 449 "ftpcmd.y"
 {
		yyval  = STRU_P;
	}
break;
case 53:
#line 455 "ftpcmd.y"
 {
		yyval  = MODE_S;
	}
break;
case 54:
#line 459 "ftpcmd.y"
 {
		yyval  = MODE_B;
	}
break;
case 55:
#line 463 "ftpcmd.y"
 {
		yyval  = MODE_C;
	}
break;
case 56:
#line 469 "ftpcmd.y"
 {
		/*
	 * Problem: this production is used for all pathname
	 * processing, but only gives a 550 error reply.
	 * This is a valid reply in some cases but not in others.
	 */
		if (yyvsp[0] && strncmp((char *) yyvsp[0], "~", 1) == 0) {
			yyval  = (int)*glob((char *) yyvsp[0]);
			if (globerr != NULL) {
				reply(550, globerr);
				yyval  = NULL;
			}
			free((char *) yyvsp[0]);
		} else
			yyval  = yyvsp[0];
	}
break;
case 58:
#line 491 "ftpcmd.y"
 {
		if (logged_in)
			yyval  = 1;
		else {
			reply(530, "Please login with USER and PASS.");
			yyval  = 0;
		}
	}
break;
#line 1330 "y.tab.c"
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
