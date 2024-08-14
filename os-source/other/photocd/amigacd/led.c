/*** led.c ******************************************************************
 *
 *  $Id: led.c,v 1.1 94/02/18 15:56:23 jjszucs Exp $
 *
 *  Photo CD Player for Amiga CD
 *  LED Module
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright © 1994 Commodore-Amiga, Inc.
 *
 *****************************************************************************/

/*
$Log:	led.c,v $
 * Revision 1.1  94/02/18  15:56:23  jjszucs
 * Initial revision
 * 
*/

/****************************************************************************
 *                                                                          *
 *  Includes                                                                *
 *                                                                          *
 ****************************************************************************/

/* ANSI includes */
#include <stdio.h>

/* Amiga includes */
#include <exec/types.h>

#include <graphics/gfx.h>

#include <intuition/intuition.h>

#include <clib/exec_protos.h>
#include <clib/graphics_protos.h>
#include <clib/intuition_protos.h>
#include <clib/debug_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/graphics_pragmas.h>
#include <pragmas/intuition_pragmas.h>

/* Local includes */
#include "global.h"
#include "interface.h"
#include "glyphs.h"

/****************************************************************************
 *                                                                          *
 *  ipow()   -   integer power                                              *
 *                                                                          *
 ****************************************************************************/

int ipow(int base, int exponent)
{

    int i, result;

    result=1;
    for (i=0;i<exponent;i++) {
        result=result*base;
    }

    return result;

}

/****************************************************************************
 *                                                                          *
 *  clearLED()   -   clear LED display                                      *
 *                                                                          *
 ****************************************************************************/

BOOL clearLED(struct appContext *appContext, struct RastPort *rastport,
    UWORD x, UWORD y, USHORT nDigits, BOOL firstNarrow)
{

    UWORD segX, segY;
    UWORD nDigit;

    /* Begin at first digit */
    segX=x;
    segY=y;

    /* Loop through digits */
    for (nDigit=0;nDigit<nDigits;nDigit++) {

        /* If on first narrow digit ... */
        if (firstNarrow && nDigit==0) {

            /* Put narrow LED segment glyph */
            putGlyph(appContext, GLYPH_LEDSEGNARROW, rastport,
                segX, segY);

            /* Advance to next character */
            segX+=LED_DIGIT_NARROW_SPACE;

        /* ... else ... */
        } else {

            /* Put LED segment glyph */
            putGlyph(appContext, GLYPH_LEDSEG, rastport,
                segX, segY);

            /* Advance to next character */
            segX+=LED_DIGIT_SPACE;

        }

    }

    /* Success */
    return TRUE;

}

/****************************************************************************
 *                                                                          *
 *  displayLEDNumber()   -   display value in LED display                   *
 *                                                                          *
 ****************************************************************************/

BOOL displayLEDNumber(struct appContext *appContext, struct RastPort *rastport,
    UWORD value, UWORD x, UWORD y, USHORT nDigits, BOOL firstNarrow)
{

    UWORD segX, segY;
    UWORD nDigit;
    UBYTE pad;
    UWORD placeValue;
    UWORD digitValue;

    /* Begin at first digit */
    segX=x;
    segY=y;

    /* Begin padded with blank digits */
    pad=TRUE;

    /* Compute place value of first digit */
    placeValue=ipow(10, nDigits-1);

    /* Loop through digits */
    for (nDigit=0;nDigit<nDigits;nDigit++) {

        /* Compute digit value */
        digitValue=value/placeValue;

        /* Subtract digit value except for -1 */
        if (value!=Largest(value)) {
            value=value%placeValue;
        }

        /* If last digit or digit value is non-zero ... */
        if (digitValue || nDigit==nDigits-1) {

            /* Clear padding flag */
            pad=FALSE;

        }

        /* If on first narrow digit ... */
        if (firstNarrow & nDigit==0) {

            /* Put narrow segment */
            putGlyph(appContext,
                (digitValue && value!=Largest(value)) ?
                    GLYPH_LED1NARROW : GLYPH_LEDSEGNARROW,
                rastport,
                segX, segY);

            /* Update position */
            segX+=LED_DIGIT_NARROW_SPACE;

        /* ... else ... */
        } else {

            /* Put normal segment, digit, or dash */
            putGlyph(appContext,
                pad ? GLYPH_LEDSEG :
                    (value==Largest(value) ?
                        GLYPH_LEDDIGITDASH : GLYPH_LED0+digitValue),
                rastport,
                segX, segY);

            /* Update position */
            segX+=LED_DIGIT_SPACE;

        }

        /* Compute next place value */
        placeValue=placeValue/10;

    }

    /* Success */
    return TRUE;

}

BOOL displayLEDSymbol(
    struct appContext *appContext, struct RastPort *rastport,
    UBYTE symbol, UWORD x, UWORD y,
    BOOL visible)
{

    UWORD   glyphID;

    /* Look-up glyph for symbol */
    switch (symbol) {

        /* '/' */
        case '/':
            glyphID=visible?GLYPH_LEDSLASH:GLYPH_LEDSLASHSEG;
            break;

        /* Other characters ... */
        default:
            /* Fail */
            return FALSE;

    }

    /* Put glyph */
    putGlyph(appContext,
        glyphID,
        rastport,
        x, y);

}
