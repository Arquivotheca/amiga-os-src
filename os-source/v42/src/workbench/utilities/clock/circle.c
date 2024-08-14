
#include <exec/types.h>


/*****************************************************************************/


/* table of sin or cos for 0-90 degrees expressed as fractions of 65535 */

static UWORD sincos_table[] =
{
        0,  1144,  2287,  3430,  4571,  5712,  6850,  7987,  9121, 10252,
    11380, 12505, 13625, 14742, 15854, 16962, 18064, 19161, 20251, 21336,
    22414, 23486, 24550, 25607, 26655, 27696, 28729, 29752, 30767, 31772,
    32768, 33753, 34728, 35693, 36647, 37589, 38521, 39440, 40347, 41243,
    42125, 42995, 43851, 44695, 45524, 46340, 47142, 47929, 48702, 49460,
    50203, 50930, 51642, 52339, 53019, 53683, 54331, 54962, 55577, 56174,
    56755, 57318, 57864, 58392, 58902, 59395, 59869, 60325, 60763, 61182,
    61583, 61965, 62327, 62671, 62996, 63302, 63588, 63855, 64103, 64331,
    64539, 64728, 64897, 65047, 65176, 65286, 65375, 65445, 65495, 65525,
    65535
};


/*****************************************************************************/


/* returns integer representing mult*sin(theta) */

LONG sine(LONG mult, LONG theta)
{
LONG quadrant = 0;

    while (theta > 360)
        theta -= 360;

    while (theta < 0)
        theta += 360;

    while (theta > 90)
    {
	++quadrant;
	theta -= 90;
    }

    if (quadrant & 1)
        theta = (90 - theta);

    mult *= (LONG)sincos_table[theta];
    mult = (mult >> 16);

    if (quadrant > 1)
        mult = 0 - mult;

    return (mult);
}


/*****************************************************************************/


/* returns integer representing mult*cos(theta) */

LONG cosine(LONG mult, LONG theta)
{
LONG quadrant = 0;

    while (theta > 360)
        theta -= 360;

    while (theta < 0)
        theta += 360;

    while (theta > 90)
    {
	quadrant++;
	theta -= 90;
    }

    if (!(quadrant & 1))
        theta = (90 - theta);

    mult *= (LONG)sincos_table[theta];
    mult = (mult >> 16);

    if ((quadrant == 1) || (quadrant == 2))
        mult = 0 - mult;

    return (mult);
}
