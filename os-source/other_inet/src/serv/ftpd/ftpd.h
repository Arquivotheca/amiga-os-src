#define UNIX_COMPAT 1
#include <ss/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/exec.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/dos_pragmas.h>
extern struct Library *DOSBase;
extern struct ExecBase *SysBase;

#define index(a,b)	strchr(a,b)
#define rindex(a,b)	strrchr(a,b)
#define strcasecmp(a,b)	stricmp(a,b)
#define strncasecmp(a,b,n)	strnicmp(a,b,n)

/* unix emulation functions */
void syslog(int pri, ...);
void sleep(int);
int inherit(char *);
// char *basename(char *p);
int stat(char *, struct stat *buf);
int fstat(int, struct stat *);
// int chmod(register char *name, int);
int chdir(char *);
char *getcwd(char *, int);
char *getwd(char *);
char *crypt(char *, char *);
int read(int, char *, unsigned int);
int write(int, char *, unsigned int);
void clean_sock(void);
int init_sock(void);


/* ftpd.c */
char *sgetsave(char *);
struct passwd *sgetpwnam(char *name);
void user(char *name);
int checkuser(char *name);
void end_login(void);
void pass(char *);
void retrieve(char *, char *);
void store(char *, char *, int);
FILE *getdatasock(char *mode);
FILE *dataconn(char *, off_t, char *);
void send_data(FILE *instr, FILE *outstr, off_t blksize);
int receive_data(FILE *instr, FILE *outstr);
void statfilecmd(char *filename);
void statcmd(void);
void fatal(char *s);
void ack(char *s);
void nack(char *s);
void yyerror(char *s);
void delete(char *name);
void cwd(char *path);
void makedir(char *name);
void removedir(char *name);
void pwd(void);
char *renamefrom(char *name);
void renamecmd(char *, char *);
void dolog(struct sockaddr_in *sin);
void dologout(int status);
void myoob();
void passive(void);
char *gunique(char *local);
void perror_reply(int, char *);
void send_file_list(char *);
void reply();
void lreply();

void Dprintf(char *fmt, ...);

/* ftpcmd.y */
struct tab *lookup(register struct tab *p, char *cmd);
char *getline(char *s, int n, register FILE *iop);
short yylex(void);
void upper(register char *s);
char *copy(char *s);
void help(struct tab *ctab, char *s);
void sizecmd(char *filename);
int yyparse(void);

/* logwtmp.c */
void logwtmp(char *, char *, char *);

/* popen.c */
FILE *ftpd_popen(char *, char *);
void ftpd_pclose(FILE *);

/* glob.c */
char **ftpglob(register char *v);
