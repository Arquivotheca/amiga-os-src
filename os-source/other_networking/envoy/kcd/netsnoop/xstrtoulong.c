/* xStrToULong.  Converts a hex string to a ULONG.
   Library type of function.  Snarl. 
   Inputs:
         STRPTR - string of form $nnnnnnnn, 0xnnnnnnnn,
                  Xnnnnnnnn, or xnnnnnnnn.
                  There must be 1 to 8 valid hex digits
                  after the introductory character(s).
                  String MUST be null-terminated!
                  String won't be altered.
         ULONG *- pointer to ULONG in which you want the result.
                  ULong will be altered most of the time, whether
                  this function succeeds or fails.

   Returns:
         TRUE if conversion succeeded.  ULONG contains converted number.
         FALSE otherwise (number too large, not hex, whatever)
               ULONG contains garbage.
*/

#include <exec/types.h>

/* Prototype */
BOOL xStrToULong(STRPTR str, ULONG *result);


BOOL xStrToULong(STRPTR str, ULONG *result) {
    STRPTR s = str, tmp;
    int len=0, shifter;

    /* Fail if NULL args were given */
    if (!str || !result)
        return(FALSE);

    /* look at str[1] */
    s++;

    /* position s to point at the first hex digit */    
    switch (*str) {
        case '0':
            if ((*s != 'x') && (*s != 'X'))
                return(FALSE);
            s++;
            break;
        case '$':
        case 'x':
        case 'X':
            break;
        default:
            return(FALSE);
            break;
    }

    /* find out how many hex digits we have */
    tmp = s;
    while (*tmp++)
        len++;

    /* fail if we have too many or too few */
    if ((len > 8) || (len == 0))
        return(FALSE);

    /* zap out whatever was in *result */
    *result = 0UL;

    /* turn string into a number */
    while (*s) {
        shifter = (--len) * 4;
        switch (*s++) {
            case '0':
                break;
            case '1':
                *result += (1UL<<shifter);
                break;
            case '2':
                *result += (2UL * (1UL<<shifter));
                break;
            case '3':
                *result += (3UL * (1UL<<shifter));
                break;
            case '4':
                *result += (4UL * (1UL<<shifter));
                break;
            case '5':
                *result += (5UL * (1UL<<shifter));
                break;
            case '6':
                *result += (6UL * (1UL<<shifter));
                break;
            case '7':
                *result += (7UL * (1UL<<shifter));
                break;
            case '8':
                *result += (8UL * (1UL<<shifter));
                break;
            case '9':
                *result += (9UL * (1UL<<shifter));
                break;
            case 'a':
            case 'A':
                *result += (10UL * (1UL<<shifter));
                break;
            case 'b':
            case 'B':
                *result += (11UL * (1UL<<shifter));
                break;
            case 'c':
            case 'C':
                *result += (12UL * (1UL<<shifter));
                break;
            case 'd':
            case 'D':
                *result += (13UL * (1UL<<shifter));
                break;
            case 'e':
            case 'E':
                *result += (14UL * (1UL<<shifter));
                break;
            case 'f':
            case 'F':
                *result += (15UL * (1UL<<shifter));
                break;
            default:
                /* fail if we find a bad digit */
                return(FALSE);
                break;
        }
    }
    return(TRUE);
}

