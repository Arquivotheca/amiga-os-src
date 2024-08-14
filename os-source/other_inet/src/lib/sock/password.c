/****** socket/getpass  *************************************************
*
*	NAME
*		getpass - Get a password entry from the user 
*
*	SYNOPSIS
*		password = getpass( prompt )
*
*		char *getpass( char * )
*
*	FUNCTION
*		Gets a password string from the user. 
*
*
*	INPUTS
*		prompt - A string which will prompt the user.
*
*	RESULT
*		password - A pointer to the string.
*
*	NOTES
*		Control-X clears the line and starts over.
*
*	BUGS
*		(1) There are currently no checks on the length (max == 32) of the
*		    string entered by the user. 
*
********************************************************************
*
* Simple passwd system for the Amiga.  The password encoding scheme
* needs improvement, but due to US DOD export requirements and the
* AT&T patent on the Unix scheme the obvious choice isn't available.
*
*/

#include <sys/param.h>
#include <pwd.h>
#include <stdio.h>

#define OSIZE	12

int	_pw_stayopen;

/*
** Read a password from the User.
*/

#ifdef AZTEC_C
#include <string.h>
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
#ifdef AZTEC_C
	struct sgttyb sg;
#endif
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


/****** socket/getlogin *************************************************
*
*	NAME
*		getlogin - gets the current login name
*
*	SYNOPSIS
*		name = getlogin()
*
*		char * getlogin( void )
*
*	FUNCTION
*		Returns a pointer to the current login name. In the Amiga's case
*		this function always returns NULL.
*
*
*	INPUTS
*		none
*
*
*	RESULT
*		pointer to a string
*
*
*	NOTES
*		As currently implemented, this function always returns a NULL.
*		The correct procedure is to call getlogin() and if it returns
*		NULL, call getpwuid().
*
********************************************************************
*
* get login name.  All we do here is a getuid(), then getpwuid().  Unix
* does this differently, but we ain't Unix either.
*/

char *
getlogin()
{
	return 0;
}

/****** socket/getpwuid *************************************************
*
*	NAME
*		getpwuid - Fill in a password structure with a line from from the
*		           password file using the user ID as the key.
*
*	SYNOPSIS
*		passwd = getpwuid( uid )
*
*		struct passwd *getpwuid( int ) 
*
*	FUNCTION
*		Getpwuid returns a pointer to a passwd structure. The passwd
*		structure fields are filled in from the fields contained in
*		one line of the passwd file (inet:db/passwd)
*
*	INPUTS
*		uid - an integer representing the current user id
*
*	RESULT
*		passwd - A pointer to a passwd struct with it's elements filled in
*		         from the passwd file.
*
*	NOTES
*
*			#include <sys/types.h>
*			
*			struct passwd {
*				char	*pw_name;
*				char	*pw_dir;
*				char	*pw_passwd;
*				char	*pw_gecos;
*				uid_t	pw_uid, pw_gid;
*				char	*pw_shell;		 
*				char	*pw_comment;	
*			};
*
*   
********************************************************************
*
* get pw entry by uid   
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

/****** socket/getpwname *************************************************
*
*	NAME
*		getpwname - Fill in a password structure with a line from from the
*		            password file using the user name as the key.
*
*	SYNOPSIS
*		passwd = getpwname( name )
*
*		struct passwd *getpwname( char * ) 
*
*	FUNCTION
*		Getpwname returns a pointer to a passwd structure. The passwd
*		structure fields are filled in from the fields contained in
*		one line of the passwd file (inet:db/passwd)
*
*	INPUTS
*		name - a pointer to the user name
*
*	RESULT
*		passwd - A pointer to a passwd struct with it's elements filled in
*		         from the passwd file.
*
*	NOTES
*
*			#include <sys/types.h>
*			
*			struct passwd {
*				char	*pw_name;
*				char	*pw_dir;
*				char	*pw_passwd;
*				char	*pw_gecos;
*				uid_t	pw_uid, pw_gid;
*				char	*pw_shell;		  
*				char	*pw_comment;	
*			};
*
*   
********************************************************************
*
* get passwd by name
*
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

/****** socket/getpwent *************************************************
*
*	NAME
*		getpwent - Read the next line in the passwd file.
*
*	SYNOPSIS
*		passwd = getpwent()
*
*		struct passwd *getpwent( void ) ;
*
*	FUNCTION
*		getpwent read the next line of the passwd file, filling the
*		fields in the passwd structure with the fields the line.
*
*
*	INPUTS
*		None
*
*
*	RESULT
*		passwd - a pointer to a filled in passwd structure if successful,
*		         NULL upon failure.
*	SEE ALSO
*
*   
********************************************************************
*
*
*/


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

/****** socket/setpwent *************************************************
*
*	NAME
*		setpwent - Opens the passwd file.
*
*	SYNOPSIS
*		success = setpwent()
*
*		int setpwent( void )
*
*	FUNCTION
*		if the file is already open the file is rewound. Otherwise the
*		file is opened.
*
*	INPUTS
*		none
*
*	RESULT
*		success - integer 0 (zero) upon success.  (-1) upon failure.
*
*	SEE ALSO
*		endpwent()
*   
********************************************************************
*
*
*/

int
setpwent()
{
	if(!fp && !(fp = fopen(PWFILE, "r"))){
		return -1;
	}

	return 0;
}


/****** socket/endpwent *************************************************
*
*	NAME
*		endpwent - Closes the passwd file.
*
*	SYNOPSIS
*		success = endpwent()
*
*		int endpwent( void )
*
*	FUNCTION
*		endpwent closes the passwd file.
*
*	INPUTS
*		none
*
*
*	RESULT
*		success - integer 0 (zero) upon success.  (-1) upon failure.
*
*
*	NOTES
*
*
*	SEE ALSO
*		setpwent()
*   
********************************************************************
*
*
*/

int
endpwent()
{
	fclose(fp);
	fp = 0;
	return 0;
}

/****** socket/crypt *************************************************
*
*	NAME
*		crypt - encryption of password
*
*	SYNOPSIS
*		crypt_string = crypt( password, username )
*
*		char *crypt( char *, char * )
*
*	FUNCTION
*		Encrypt a password, using the the user's name in the mix so as to
*		provide a unique passwd even if two users should select the same
*       passwd.
*
*	INPUTS
*		password - pointer to the user's password entry.
*		username - the user's login name.
*
*	RESULT
*		Pointer to the encrypted result string.  This string is always
*		eleven (11) characters long.
*
*   
********************************************************************
*
* function used to encode passwords.  passwd string output is a function
* of uname since we want the passwd string to be different even when two users
* choose the same password.  We always output a 11 char string.
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
