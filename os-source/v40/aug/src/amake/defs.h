/* defs 4.6 83/07/01 */
#include <stdio.h>
#include <ctype.h>

#include <string.h>
#include <exec/types.h>
#include <dos.h>
#include <libraries/dos.h>
/* #include <proto/all.h> */
#include	<pragmas/rexxsyslib_pragmas.h>

typedef ULONG	TIMETYPE;

#define	MAXNAMLEN	FMSIZE
#define	index		strchr
#define	rindex		strrchr

/* define FSTATIC to be static on systems with C compilers
** supporting file-static; otherwise define it to be null
*/

#define FSTATIC 	static

#define	NOFILE		(8)

#define NO 			(0)
#define YES 		(1)

#define unequal		strcmp
#define HASHSIZE	(1021)
#define NLEFTS		(512)
#define NCHARS 		(500)
#define NINTS  		(250)
#define INMAX 		(3500)
#define OUTMAX 		(3500)
#define QBUFMAX 	(2500)
#define MAXDIR 		(10)

#define ALLDEPS  	(1)
#define SOMEDEPS 	(2)

#define META 		(01)
#define TERMINAL 	(02)

#define ALLOC(x)	(struct x *) ckalloc(sizeof(struct x))

struct nameblock {
	struct	nameblock	*nxtnameblock;
			char		*namep;
	struct	lineblock	*linep;
			int			 done		:3;
			int			 septype	:3;
			TIMETYPE	 modtime;
};

struct lineblock {
	struct	lineblock	*nxtlineblock;
	struct	depblock	*depp;
	struct	shblock		*shp;
};

struct depblock {
	struct	depblock	*nxtdepblock;
	struct	nameblock	*depname;
};

struct shblock {
	struct	shblock		*nxtshblock;
			char		*shbp;
};

struct varblock {
	struct	varblock	*nxtvarblock;
	char				*varname;
	char				*varval;
	int					 noreset	:1;
	int					 used		:1;
};

struct pattern {
	struct	pattern		*nxtpattern;
	char				*patval;
};

struct	dirhdr	{
	struct	dirhdr		*nxtopendir;
	BPTR				 dirfc;
	char				*dirn;
};

struct chain {
	struct	chain		*nextp;
			char		*datap;
};

char					*copys(),
						*concat(),
						*subst();

int						*ckalloc();

struct	nameblock		*srchname(),
						*makename();
		TIMETYPE		 exists();

extern	char			*prompt;
extern	char			*filename;
extern	char			 junkname[ ];
extern	char			 funny[128];

extern	int				 sigivalue;
extern	int				 sigqvalue;
extern	int				 waitpid;
extern	int				 dbgflag;
extern	int				 prtrflag;
extern	int				 silflag;
extern	int				 noexflag;
extern	int				 keepgoing;
extern	int				 noruleflag;
extern	int				 touchflag;
extern	int				 questflag;
extern	int				 ndocoms;
extern	int				 ignerr;
extern	int				 okdel;
extern	int				 inarglist;
extern	int				 nopdir;
            	
extern struct nameblock *mainname ;
extern struct nameblock *firstname;

extern struct lineblock *sufflist;

extern struct varblock *firstvar;

extern struct dirhdr *firstod;

extern struct pattern *firstpat;

