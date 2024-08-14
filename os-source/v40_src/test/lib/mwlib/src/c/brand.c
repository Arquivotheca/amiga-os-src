/****** MWLib/c/BRand *********************************************************

    NAME
        BRand -- Fixed Random number generator

    VERSION
        1.00    28 Mar 1991 - Inception

    SYNOPSIS
        void BSeed(ULONG seed)
        ULONG BRand(void)

    FUNCTION
        Generates Random Numbers, and returns the result. The sequence
        of numbers it generates is rather large, about 2^(32 * AR_SIZE).

    INPUTS
        seed    - 32-bit seed to seed the generator.

    RETURNS
        A 32-bit, unsigned, random number.

    SEE ALSO
        ARand()

******************************************************************************/

#include "exec/types.h"

#define AR_SIZE 11

static ULONG ar[AR_SIZE] ={
    2108806353,
    1664990704,
    1842421811,
    1421573258,
    149670437,
    582382964,
    380961575,
    636372782,
    1333026745,
    981656632,
    1778907227,
}, o=0; i1=1; i2=2;

void BSeed(ULONG seed)
{
    ULONG i;

    for (srand(seed), i = 0; i < AR_SIZE; ++i)
        ar[i] = rand();
}

ULONG BRand()
{
    if (++o >= AR_SIZE) o = 0;
    if (++i1 >= AR_SIZE) i1 = 0;
    if (++i2 >= AR_SIZE) i2 = 0;

    return ar[o] = ar[i1] + ar[i2];
}
