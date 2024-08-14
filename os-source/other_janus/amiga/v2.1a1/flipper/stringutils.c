#include "string.h"

int atoi(s)
register char *s;
{
    register int total = 0;
    register unsigned digit;
    register minus = 0;

    while (isspace(*s))
        s++;
    if (*s == '-') {
        s++;
        minus = 1;
    }
    while ((digit = *s++ - '0') < 10) {
        int temp = total;
        if((digit < 0) || (digit > 9))
            break;
        /* total *= 10; */
        total <<= 2;    /* total *= 4 */
        total += temp;  /* total = total * 5 */
        total <<= 1;    /* total = total * 10 */
        total += digit;
    }
    return(minus ? -total : total);
}

int				/* <0 for <, 0 for ==, >0 for > */
strcmp(s1, s2)
const char *s1;
const char *s2;
{
    register const char *scan1;
    register const char *scan2;

    scan1 = s1;
    scan2 = s2;
    while (*scan1 != '\0' && *scan1 == *scan2) {
        scan1++;
        scan2++;
    }

	/*
	 * The following case analysis is necessary so that characters
	 * which look negative collate low against normal characters but
	 * high against the end-of-string NUL.
	 */
    if (*scan1 == '\0' && *scan2 == '\0')
        return(0);
    else if (*scan1 == '\0')
        return(-1);
    else if (*scan2 == '\0')
        return(1);
    else
        return(*scan1 - *scan2);
}

int				/* <0 for <, 0 for ==, >0 for > */
strncmp(s1, s2, n)
const char *s1;
const char *s2;
unsigned long n;
{
    register const char *scan1;
    register const char *scan2;
    register unsigned long count;

    scan1 = s1;
    scan2 = s2;
    count = n;
    while ((count > 0) && *scan1 != '\0' && *scan1 == *scan2) {
        scan1++;
        scan2++;
        count--;
    }
    if (count <= 0)
        return(0);
    /*
     * The following case analysis is necessary so that characters
     * which look negative collate low against normal characters but
     * high against the end-of-string NUL.
     */
    if (*scan1 == '\0' && *scan2 == '\0')
        return(0);
    else if (*scan1 == '\0')
        return(-1);
    else if (*scan2 == '\0')
        return(1);
    else
        return(*scan1 - *scan2);
}

int				/* <0 for <, 0 for ==, >0 for > */
stricmp(s1, s2)
const char *s1;
const char *s2;
{
    register const char *scan1;
    register const char *scan2;

    scan1 = s1;
    scan2 = s2;
    while (*scan1 != '\0' && (toupper(*scan1) == toupper(*scan2))) {
        scan1++;
        scan2++;
    }

	/*
	 * The following case analysis is necessary so that characters
	 * which look negative collate low against normal characters but
	 * high against the end-of-string NUL.
	 */
    if (*scan1 == '\0' && *scan2 == '\0')
        return(0);
    else if (*scan1 == '\0')
        return(-1);
    else if (*scan2 == '\0')
        return(1);
    else
        return(toupper(*scan1) - toupper(*scan2));
}

int				/* <0 for <, 0 for ==, >0 for > */
strnicmp(s1, s2, n)
const char *s1;
const char *s2;
unsigned long n;
{
    register const char *scan1;
    register const char *scan2;
    register unsigned long count;

    scan1 = s1;
    scan2 = s2;
    count = n;
    while ((count > 0) && *scan1 != '\0' && 
        (toupper(*scan1) == toupper(*scan2))) {
        scan1++;
        scan2++;
        count--;
    }
    if (count <= 0)
        return(0);
    /*
     * The following case analysis is necessary so that characters
     * which look negative collate low against normal characters but
     * high against the end-of-string NUL.
     */
    if (*scan1 == '\0' && *scan2 == '\0')
        return(0);
    else if (*scan1 == '\0')
        return(-1);
    else if (*scan2 == '\0')
        return(1);
    else
        return(toupper(*scan1) - toupper(*scan2));
}

/*
 * strlen - length of string (not including NUL)
 */
unsigned long
strlen(s)
const char *s;
{
    register const char *scan;
    register unsigned long count;

    count = 0;
    scan = s;
    while (*scan++ != '\0')
        count++;
    return(count);
}
