#define isascii(c)	(((unsigned) (c) & 0xFF) < 0200)

/* For all the following functions the parameter must be ASCII */

#define _between(a,b,c)	((unsigned) ((b) - (a)) < (c) - (a))

#define isupper(c)	_between('A', c, 'Z')
#define islower(c)	_between('a', c, 'z')
#define isdigit(c)	_between('0', c, '9')
#define isprint(c)	_between(' ', c, '~')

/* The others are problematic as the parameter may only be evaluated once */

static _c_;		/* used to store the evaluated parameter */

#define isalpha(c)	(isupper(_c_ = (c)) || islower(_c_))
#define isalnum(c)	(isalpha(c) || isdigit(_c_))
#define _isblank(c)	((_c_ = (c)) == ' ' || _c_ == '\t')
#define isspace(c)	(_isblank(c) || _c_=='\r' || _c_=='\n' || _c_=='\f')
#define iscntrl(c)	((_c_ = (c)) == 0177 || _c_ < ' ')

#define toupper(c) (isupper(c) ? (c) : ( (c - 'a') + 'A'))
#define tolower(c) (islower(c) ? (c) : ( (c - 'A') + 'a'))

int atoi(char *);
int strcmp(const char *, const char *);
int stricmp(char *, char *);
int strncmp(char *, char *, unsigned long );
int strnicmp(char *, char *, unsigned long );
unsigned long strlen(char *);
