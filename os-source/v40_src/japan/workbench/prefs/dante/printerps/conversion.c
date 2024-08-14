
#include <exec/types.h>
#include <stdio.h>

#include <clib/dos_protos.h>

#include <pragmas/dos_pragmas.h>

#include "pe_custom.h"


/*****************************************************************************/


LONG Power (LONG n, LONG p)
{
    LONG r = n;

    while (--p)
	r *= n;
    return (r);
}


/*****************************************************************************/


BOOL CheckNumber(STRPTR buffer, LONG *longvar, LONG *decvar, LONG maxdec,
                 BOOL exiting)
{
LONG total, dec, numdec;
BOOL decimal;
BOOL inrange;
BYTE digit;
int  sign;

    decimal = FALSE;
    sign    = -1;			/* gather negative number */

    if (*buffer == '-')
    {
	sign = 1;
	buffer++;
    }
    else if (*buffer == '+')
    {
	buffer++;
    }

    inrange = TRUE;
    total   = 0;
    dec     = 0;
    numdec  = 0;
    while (*buffer)
    {
	if (*buffer == '.')
	{
	    if (decimal)
	    {
		inrange = FALSE;
		break;
	    }
	    decimal = TRUE;
	}

	/* must be a digit */
	if (((digit = *buffer - '0') < 0 || digit > 9) && *buffer != '.')
	{
	    inrange = FALSE;
	    break;
	}

	if ((total < -214748364) || ((total == -214748364) && (digit > 8)))
	{
	    inrange = FALSE;
	    break;
	}
	else
	{
	    if (*buffer != '.')
	    {
		if (decimal)
		{
		    dec = (dec * 10) - digit;
		    numdec++;
		}
		else
		{
		    total = (total * 10) - digit;
		}
	    }
	    buffer++;
	}

	if (exiting && (numdec >= maxdec))
	    break;
    }

    if (exiting && (numdec < maxdec))
	dec *= Power (10, (maxdec - numdec));

    *longvar = total * sign;
    *decvar  = dec * (-1);

    return(inrange);
}


/*****************************************************************************/


STRPTR FromMille(EdDataPtr ed, ULONG mp, LONG numdec)
{
ULONG num    = 0;
ULONG remain = 0;
UBYTE buffer[32];

    switch (ed->ed_CurrentSystem)
    {
	/* Centimeters (28346.45669291 millipoints per centimeter) */
	case 0: num = (mp*1000) / 28346456;
                remain = (mp*1000 / 283464) - ((mp*1000 / 28346456) * 100);
                if (remain == 100)
                {
                    num++;
                    remain = 0;
                }
                break;

	/* Inches (72 points per inch) */
	case 1: num = mp / 72000;
                remain = ((mp / 72) - ((mp / 72000) * 1000)) / 10;
                break;

	/* Points */
	case 2: num = mp / 1000;
                remain = (mp / 10) - ((mp / 1000) * 100);
                break;
    }
    sprintf(buffer, "%%ld.%%0%ldld", numdec);
    sprintf(ed->ed_NumBuffer, buffer, num, remain);

    return (ed->ed_NumBuffer);
}


/*****************************************************************************/


STRPTR FromMilleNC(EdDataPtr ed, ULONG mp, LONG numdec)
{
ULONG remain;
UBYTE buffer[32];

    remain = (mp / 10) - ((mp / 1000) * 100);

    sprintf(buffer, "%%ld.%%0%ldld", numdec);
    sprintf(ed->ed_NumBuffer, buffer, mp / 1000, remain);

    return (ed->ed_NumBuffer);
}


/*****************************************************************************/


#define NUM(x) (x-'0')
#define ASC(x) (x+'0')

/* multiplies two string containing numbers. The "answer" string should be
 * at least twice as long as either of the two numbers to be multiplied. The
 * parameters and answer are in base 10, one digit per byte, stored in ASCII
 * for easier debugging. length of "one" and "two" are limited to 50 each.
 */
VOID StrMult(STRPTR one, STRPTR two, STRPTR answer)
{
WORD  i,j,k,l;
UWORD onelen,twolen;
UWORD carry;
UWORD num;
UWORD len;
char  ch;
char  temp[100];

    onelen    = strlen(one);
    twolen    = strlen(two);
    l         = 0;

    for (i = 0; i <= onelen+twolen; i++)
        answer[i] = ASC(0);
    answer[i] = 0;

    for (i = onelen-1; i >= 0; i--)
    {
        k     = 0;
        carry = 0;
        for (j = twolen-1; j >= 0; j--)
        {
            num       = NUM(two[j]) * NUM(one[i]) + carry;
            carry     = num / 10;
            temp[k++] = ASC(num % 10);
        }
        temp[k++] = ASC(carry);
        temp[k]   = 0;

        k     = l;
        carry = 0;
        while (temp[k-l])
        {
            num         = NUM(answer[k]) + NUM(temp[k-l]) + carry;
            carry       = num / 10;
            answer[k++] = ASC(num % 10);
        }
        answer[k] = ASC(carry);
        l++;
    }

    if (len = strlen(answer))
    {
        while (answer[len-1] == ASC(0))
            len--;
        answer[len] = 0;
    }

    if (len)
    {
        for (i = 0; i < (len / 2); i++)
        {
            ch = answer[i];
            answer[i] = answer[len-i-1];
            answer[len-i-1] = ch;
        }
    }
    else
    {
        answer[0] = ASC(0);
        answer[1] = 0;
    }
}


/*****************************************************************************/


ULONG ToMille(EdDataPtr ed, STRPTR value)
{
ULONG num, remain;
ULONG retval = 0;
char  conv[50], answer[100];
UWORD numdec,i,j,round;

    CheckNumber (value, &num, &remain, 0, FALSE);
    switch (ed->ed_CurrentSystem)
    {
	/* Centimeters */
	case 0: i      = 0;
                j      = 0;
                numdec = 8;   /* .45669291 */
                while (value[i])
                {
                    if (value[i] != '.')
                    {
                        conv[j] = value[i];
                        if (j != i)   /* means we got a '.' */
                            numdec++;
                        j++;
                    }

                    i++;
                }
                conv[j] = 0;

                StrMult(conv,"2834645669291",answer);  /* 28346.45669291 millipoints/cm */

                round = 0;
                if (numdec > 0)
                    if (NUM(answer[strlen(answer)-numdec]) >= 5)
                        round = 1;

                answer[strlen(answer)-numdec] = 0;
                StrToLong(answer,&retval);
                retval += round;
                break;

	/* Inches */
	case 1: retval = (num * 72000) + (remain * 720);
                break;

	/* Points */
	case 2: retval = (num * 1000) + (remain * 10);
                break;
    }

    return (retval);
}


/*****************************************************************************/


ULONG ToMilleNC(EdDataPtr ed, STRPTR value)
{
ULONG num, remain;

    CheckNumber(value, &num, &remain, 0, FALSE);

    return ( (num * 1000) + (remain * 10) );
}
