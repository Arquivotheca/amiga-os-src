/****** CALib.library/StrToDouble *********************************************

    NAME
        StrToDouble -- Convert a string to a double float

    SYNOPSIS
        double StrToDouble(char *s, long *plen)

    FUNCTION
        Converts the given string to a double. Optionally
        supplies the number of characters consumed. Will
        stop on a non-numeric character. Skips over leading
        whitespace.

    INPUTS
        s       - string to convert
        plen    - pointer to long that will receive the length of
                  the string connverted, or NULL. This includes any
                  leading whitespace skipped. If pointer is NULL, will
                  be ignored.

    RESULTS
        Returns the double result. If no number was encountered, will
        return a zero.

    BUGS
        Error checking needs to be improved.

    SEE ALSO

******************************************************************************/

#include <exec/types.h>
#include <math.h>
#include <ctype.h>

#ifdef FAST
#include <m68881.h>
#endif

double StrToDouble(char *s, long *plen)
{
    char *p = s;
    double d = 0.0, e = 0.0, point = 0.1;
    BOOL done = FALSE;      // completed task?
    BOOL decimal = FALSE;   // encountered decimal
    BOOL scinote = FALSE;   // scientific notation mode
    BOOL mantissa_negative = FALSE; // mantissa sign
    BOOL exponent_negative = FALSE; // exponent sign
    BOOL mantissa_sign_seen = FALSE;
    BOOL exponent_sign_seen = FALSE;

    // skip over whitespace
    while (isspace(*p))
        ++p;

    while (!done)
    {
        if (*p)
        {
            if (isdigit(*p))
            {
                if (!decimal && !scinote)
                    d = d * 10.0 + (double) (*p - '0');

                else if (decimal && !scinote)
                {
                    d = d + point * (double) (*p - '0');
                    point *= 0.1;
                }

                else if (scinote)
                    e = e * 10.0 + (double) (*p - '0');
            }

            else if (*p == '.')
                decimal = TRUE;

            else if (*p == 'e' || *p == 'E')
            {
                scinote = TRUE;
                decimal = FALSE;
            }

            else if (*p == '+')
            {
                if (scinote)
                {
                    if (exponent_sign_seen)
                        done = TRUE;
                    else
                        exponent_sign_seen = TRUE;
                }
                else
                {
                    if (mantissa_sign_seen)
                        done = TRUE;
                    else
                        mantissa_sign_seen = TRUE;
                }
            }

            else if (*p == '-')
            {
                if (scinote)
                {
                    if (exponent_sign_seen)
                        done = TRUE;
                    else
                    {
                        exponent_sign_seen = TRUE;
                        exponent_negative = TRUE;
                    }
                }
                else
                {
                    if (mantissa_sign_seen)
                        done = TRUE;
                    else
                    {
                        mantissa_sign_seen = TRUE;
                        mantissa_negative = TRUE;
                    }
                }
            }

            else
                done = TRUE;

            if (!done)
                ++p;
        }
        else
            done = TRUE;
    }

    // at this point d is the mantissa and e is the exponent
    if (mantissa_negative) d = -d;
    if (exponent_negative) e = -e;

    if (d && e) // no point if either is zero
        d *= pow(10.0, e);

    // now d contains the result. Compute the bytes used
    if (plen)
        *plen = (ULONG) p - (ULONG) s;

    return d;
}
