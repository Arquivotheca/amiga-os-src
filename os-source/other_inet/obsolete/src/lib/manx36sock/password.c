/*
** Simple passwd system for the Amiga.  The password encoding scheme
** needs improvement, but due to US DOD export requirements and the
** AT&T patent on the Unix scheme the obvious choice isn't available.
*/

#include <sys/param.h>
#include <pwd.h>
#include <stdio.h>
#include <bstr.h>

#define OSIZE	12

int	_pw_stayopen;

/*
** Read a password from the User.
*/

#ifdef AZTEC_C
#include <sgtty.h>
#endif

#ifdef LATTICE
#include <fcntl.h>
#endif

extern struct passwd *getpwent();
extern struct passwd *getpwuid();
extern struct passwd *getpwnam();

char *
getpass(prompt)
	char *prompt;
{
	static char buf[32];
	char *pass = buf;
	struct sgttyb sg;
	char *cp;

	if(prompt){
		printf("%s",prompt);
	}
#ifdef AZTEC_C
	sg.sg_flags = RAW;
	ioctl(1, TIOCSETP, &sg);
#endif
#ifdef LATTICE
	iomode(1,1);
#endif
	*pass = '\0'; cp = pass;
	do {
		*cp = getchar();
		if (*cp == EOF) {
#ifdef AZTEC_C
			sg.sg_flags = 0;
			ioctl(1, TIOCSETP, &sg);
#endif
#ifdef LATTICE
			iomode(1,0);
#endif
		} else if (*cp == '\r') {
				*cp = '\0';
				printf("\n");
				break;
		} else if (*cp == '\030') {
				*pass = '\0';
				cp = pass;
		} else if (*cp == '\010') {
				if (cp > pass){
					cp--;
				}
		} else {
			cp++;
		}
	} while (1);
#ifdef AZTEC_C
	sg.sg_flags = 0;
	ioctl(1, TIOCSETP, &sg);
#endif
#ifdef LATTICE
	iomode(1,0);
#endif
	return buf;
}

/*
** get login name.  All we do here is a getuid(), then getpwuid().  Unix
** does this differently, but we ain't Unix either.
*/
char *
getlogin()
{
	return 0;
}

/*
** get pw entry by uid
*/
struct passwd *
getpwuid(uid)
	int	uid;
{
	register struct passwd *pw;

	setpwent();
	do {
		pw = getpwent();
	} while(pw && (pw->pw_uid != uid));
	endpwent();

	return pw;
}

/*
** get pw entry by name
*/
struct passwd *
getpwnam(name)
	char	*name;
{
	register struct passwd *pw;

	setpwent();
	do {
		pw = getpwent();
	} while(pw && strcasecmp(pw->pw_name, name) != 0);
	endpwent();

	return pw;
}

static FILE *fp;

/*
** entry fmt	user|passwd|uid|gid[,gid]|name|dir|shell
*/
static char *
field(s, buf, buflen)
	register char *s, *buf;
	register int buflen;
{
	*buf = 0;
	while(*s && --buflen >= 0){
		if(*s == '|'){
			break;
		}
		*buf++ = *s++;
	}
	*buf++ = 0;

	while(*s && (*s++ != '|')){
		/* skip to next separator or null */
	}

	return s;
}

struct passwd *
getpwent()
{
	static char name[64], user[32], dir[MAXPATHLEN];
	static char shell[MAXPATHLEN], pass[OSIZE];
	char buf[128], uid[8], gid[8];
	static struct passwd pwd;
	register char *s;

	bzero(&pwd, sizeof(pwd));
	if(!fp){
		return 0;
	}
	if(fgets(buf, sizeof(buf), fp) != NULL){
		if(s = index(buf, '\n')){
			*s = 0;
		}
		s = field(buf, user, sizeof(user));
		s = field(s, pass, sizeof(pass));
		s = field(s, uid, sizeof(uid));
		s = field(s, gid, sizeof(gid));	
		s = field(s, name, sizeof(name));
		s = field(s, dir, sizeof(dir));	
		s = field(s, shell, sizeof(shell));

		pwd.pw_uid = atoi(uid);
		pwd.pw_gid = atoi(gid);
		pwd.pw_name = user;
		pwd.pw_passwd = pass;
		pwd.pw_gecos = name;
		pwd.pw_shell = shell;	
		pwd.pw_dir = dir;
		return &pwd;
	}

	return NULL;
}

int
setpwent()
{
	if(!fp && !(fp = fopen(PWFILE, "r"))){
		return -1;
	}

	return 0;
}

int
endpwent()
{
	fclose(fp);
	fp = 0;
	return 0;
}

/*
** function used to encode passwords.  passwd string output is a function
** of uname since we want the passwd string to be different even when two users
** choose the same password.  We always output a 10 char string.
*/

char *
crypt(s, uname)
	register unsigned char *uname, *s;
{
	static char obuf[OSIZE];
	unsigned int buf[OSIZE];
	register int i, k;

	for(i = 0; i < OSIZE; i++){
		buf[i] = 'A' + (*s? *s++:i) + (*uname? *uname++:i);
	}

	for(i = 0; i < OSIZE; i++){
		for(k = 0; k < OSIZE; k++){
			buf[i] += buf[OSIZE-k-1];
			buf[i] %= 53;
		}
		obuf[i] = buf[i] + 'A';
	}

	obuf[OSIZE-1] = 0;
	return obuf;
}
