#include <exec/types.h>
#include "ecc.h"
#include "gf64.def"


#ifndef	GF_MACS
/* just for reference */
/* ==================================================================== */

GF GF_Add (GF a, GF b)
{
        return (GF)(b ^ a);
}
/* -------------------------------------------------------------------- */
GF GF_Mul (GF a, GF b)
{
	return MulTab[a][b];
}
/* ------------------------------------------------------------------- */
GF	GF_Inv(GF a)
{
	return Inv[a];
}
#endif