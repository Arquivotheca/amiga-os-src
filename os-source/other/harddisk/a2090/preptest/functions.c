#include "exec/types.h"

#define STR_LEN (64-1)
#define EOF (-1)

strlen(s)
register char *s;
{
    int i = 0;

    while(*s++) i++;
    return(i);
}


int isdigit(c)
char c;
{
    if ((c >= '0') && (c <= '9'))return (c - '0');
    else return(-1);
}


int sscanf(s, nptr)	/* hack version original by Dale Luck */
char *s;		/* modified by Eric Cotton 	      */
int *nptr;
{
    int c;
    int num = 0;

    if(isdigit(*s) < 0)return(FALSE);	/* no input */
    while((c = isdigit(*s++)) >= 0)
    {
	num = num*10 + c;
    }
    *nptr = num;
    return(TRUE);	/* ok input */
}


char *gets(s)	/* hack version */
char *s;
{
    char *pt = s;
    int c, i;

    for(i = 0; i < STR_LEN; i++)
    {
	c = getchar();
	if((c == EOF) || (c == '\n'))
	{
	    *pt = '\0';
	    if((c == EOF) || (pt == s))
		return(0);
	    else
		return(s);
	}
	else *(pt++) = (char)c;
    }
    *pt = '\0';
    return(s);
}

