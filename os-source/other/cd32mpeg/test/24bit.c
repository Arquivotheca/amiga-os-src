/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <exec/types.h>
#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

extern struct Library *SysBase;

/*
 * We'll define the "ConvertColor" macro here to do fixed point arithmetic
 * that'll convert from YCrCb to RGB using:
 *	R = L + 1.40200*Cr;
 *	G = L - 0.34414*Cb - 0.71414*Cr
 *	B = L + 1.77200*Cb;
 *
 * We'll use fixed point by adding two extra bits after the decimal.
 */

#define BITS	8
#define ONE     ((int) 1)
#define CONST_SCALE	(ONE << BITS)
#define ROUND_FACTOR	(ONE << (BITS-1))

/* Macro to convert integer to fixed. */
#define UP(x)	(((int)(x)) << BITS)

/* Macro to convert fixed to integer (with rounding). */
#define DOWN(x)	(((x) + ROUND_FACTOR) >> BITS)

/* Macro to convert a float to a fixed */
#define FIX(x)  ((int) ((x)*CONST_SCALE + 0.5))

#define CLAMP(ll,x,ul)	( ((x)<(ll)) ?(ll):( ((x)>(ul)) ?(ul):(x)))

static int *Cb_r_tab, *Cr_g_tab, *Cb_g_tab, *Cr_b_tab;

/*
 *--------------------------------------------------------------
 *
 * InitColorDither --
 *
 *	To get rid of the multiply and other conversions in color
 *	dither, we use a lookup table.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	The lookup tables are initialized.
 *
 *--------------------------------------------------------------
 */

void
InitColorDither(void)
{
    int CR, CB, i;

    Cr_b_tab = (int *)AllocVec(256*sizeof(int),0);
    Cr_g_tab = (int *)AllocVec(256*sizeof(int),0);
    Cb_g_tab = (int *)AllocVec(256*sizeof(int),0);
    Cb_r_tab = (int *)AllocVec(256*sizeof(int),0);

    for (i=0; i<256; i++) {
	CB = CR = i;

	CB -= 128; CR -= 128;

	Cb_r_tab[i] = FIX(1.40200) * CB;
	Cr_g_tab[i] = -FIX(0.34414) * CR;
	Cb_g_tab[i] = -FIX(0.71414) * CB;
	Cr_b_tab[i] = FIX(1.77200) * CR;
    }
}

void EndColorDither(void)
{
    FreeVec(Cr_b_tab);
    FreeVec(Cr_g_tab);
    FreeVec(Cb_g_tab);
    FreeVec(Cb_r_tab);
}


/*
 *--------------------------------------------------------------
 *
 * ColorDitherImage --
 *
 *	Converts image into 24 bit color.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	None.
 *
 *--------------------------------------------------------------
 */

void
ColorDitherImage(lum, cr, cb, red, green, blue, cols, rows)
  unsigned char *lum;
  unsigned char *cr;
  unsigned char *cb;
  unsigned char *red;
  unsigned char *green;
  unsigned char *blue;
  int cols, rows;

{
    int L, CR, CB;
    unsigned char *r_row1, *r_row2;
    unsigned char *g_row1, *g_row2;
    unsigned char *b_row1, *b_row2;

    unsigned char *lum2;
    int x, y;
    int cb_r;
    int cr_g;
    int cb_g;
    int cr_b;

    r_row1 = red;
    r_row2 = r_row1 + cols;

    g_row1 = green;
    g_row2 = g_row1 + cols;

    b_row1 = blue;
    b_row2 = b_row1 + cols;

    lum2 = lum + cols;
    for (y=0; y<rows; y+=2) {
	for (x=0; x<cols; x+=2) {
	    int R, G, B;

	    CR = *cr++;
	    CB = *cb++;
	    cb_r = Cb_r_tab[CB];
	    cr_g = Cr_g_tab[CR];
	    cb_g = Cb_g_tab[CB];
	    cr_b = Cr_b_tab[CR];

	    L = *lum++;
	    L = UP(L);
	    R = L + cb_r;
	    G = L + cr_g + cb_g;
	    B = L + cr_b;
	    *r_row1++ = CLAMP(0,R,UP(255)) >> BITS;
	    *g_row1++ = CLAMP(0,G,UP(255)) >> BITS;
	    *b_row1++ = CLAMP(0,B,UP(255)) >> BITS;

	    L = *lum++;
	    L = UP(L);
	    R = L + cb_r;
	    G = L + cr_g + cb_g;
	    B = L + cr_b;
	    *r_row1++ = CLAMP(0,R,UP(255)) >> BITS;
	    *g_row1++ = CLAMP(0,G,UP(255)) >> BITS;
	    *b_row1++ = CLAMP(0,B,UP(255)) >> BITS;

	    /*
	     * Now, do second row.
	     */
	    L = *lum2++;
	    L = UP(L);
	    R = L + cb_r;
	    G = L + cr_g + cb_g;
	    B = L + cr_b;
	    *r_row2++ = CLAMP(0,R,UP(255)) >> BITS;
	    *g_row2++ = CLAMP(0,G,UP(255)) >> BITS;
	    *b_row2++ = CLAMP(0,B,UP(255)) >> BITS;

	    L = *lum2++;
	    L = UP(L);
	    R = L + cb_r;
	    G = L + cr_g + cb_g;
	    B = L + cr_b;

	    *r_row2++ = CLAMP(0,R,UP(255)) >> BITS;
	    *g_row2++ = CLAMP(0,G,UP(255)) >> BITS;
	    *b_row2++ = CLAMP(0,B,UP(255)) >> BITS;
	}
	lum += cols;
	lum2 += cols;
	r_row1 += cols;
	r_row2 += cols;
	g_row1 += cols;
	g_row2 += cols;
	b_row1 += cols;
	b_row2 += cols;
    }
}



