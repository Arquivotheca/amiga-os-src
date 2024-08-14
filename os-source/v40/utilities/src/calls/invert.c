
#include <stdio.h>

/* states */
#define START 0
#define SPACES 1
#define NNAME  2

#define MAXLINESIZE	164
#define MAXNAMESIZE	32
#define MAXDEPTH	24

struct callnode {
    int	  cn_level;
    char *cn_name[MAXNAMESIZE];
    };

struct callnode ctrace[MAXDEPTH];

int	cdepth = -2;
int	clevel	= -2;

char linebuf[MAXLINESIZE];
char namebuf[MAXNAMESIZE];

main(argc, argv)
int argc;
char *argv[];
{
    int lev;

    if (argc < 2)
    {
	fprintf(stdout, "usage: %s <functionname>\n", argv[0]);
	exit (1);
    }

    printf("seeking paths to %s\n", argv[1]);

    while (gets(linebuf))
    {
	if (linebuf[0] == '\0')
	{	/* new top level */
	    /* reset */
	    cdepth = -1;
	    clevel = -2;
	}
	else
	{
	    lev = levelName(namebuf, linebuf);
	    if (lev == 2)
	    {
		printf(" !! level 2 error: <%s>\n", linebuf );
	    }

	    if (lev > clevel)
	    {
		/* more depth	*/
		cdepth++;
	    }
	    else if (lev < clevel)
	    {
		/* decrement depth */
		cdepth = backup(lev);
	    }
	    ctrace[cdepth].cn_level = clevel = lev;
	    strcpy(ctrace[cdepth].cn_name, namebuf);

	    /** dump(); **/

	    if (strcmp(namebuf, argv[1]) == 0)
	    {
#if FULLTRACE
		dump_ctrace();
#else
		dump_caller();
#endif
	    }
	}
    }
}

dump()
{
    int i;
    for (i = 0; i <= cdepth; ++i)
    {
	printf("%d ", ctrace[i].cn_level);
    }
    printf(" %s\n", ctrace[cdepth].cn_name);
}

backup(lev)
{
    int i = cdepth;
    while (i > 0 && (ctrace[i-1].cn_level >= lev))
    {
	i--;
    }
    return (i);
}


dump_ctrace()
{
    int i;

    puts("");
    for (i = 0; i <= cdepth; ++i)
    {
	printf("\t%s\n", ctrace[i].cn_name);
	/** 
	printf("%s	-- lev: %d\n", ctrace[i].cn_name, ctrace[i].cn_level);
	**/
    }
}

dump_caller()
{
    int i;

    /* printf("dump caller, cdepth %d ", cdepth ); */

    if ( cdepth > 0 )
	printf("\t%s\n", ctrace[cdepth - 1 ].cn_name);
    else 
	printf(" no callers\n" );
}

/* returns number of spaces before the 'calls' file string
 * returns -1 if failure
 * put name found in string into 'name'
 */

levelName(name, str)
char *name;
char *str;
{
    char  c;
    int count = 0;
    int state = START;

    while (c = *str++)
    {
	switch (state)
	{
	case START:
	    switch (c)
	    {
	    case ' ':	/* no tabs expected */
		count++;
		state = SPACES;
		break;
	    case '\n':
		fprintf(stdout, "unexpected newline (start) \n");
		return (-1);
	    default:	/* haven't finished with line number yet */
		count++;
	    }
	    break;

	case SPACES:
	    switch (c)
	    {
	    case ' ':
		count++;
		break;
	    case '\n':
		fprintf(stdout, "unexpected newline (spaces)\n");
		return (-1);
	    default:	/* beginning name part */
		*name++ = c;	/* copy first character */
		state = NNAME;
	    }
	    break;

	case NNAME:
	    switch (c)
	    {
	    case '\n':
	    case ' ':
		*name = '\0';
		return (count);
	    default:
		*name++ = c;
	    }
	    break;
	}
    }

    /* end of line */
    if (state == NNAME)
    {
	*name = '\0';
    }
    else
    {
	fprintf(stdout, "unexpected endof file\n");
	count  = -1;
    }
    return (count);
}
