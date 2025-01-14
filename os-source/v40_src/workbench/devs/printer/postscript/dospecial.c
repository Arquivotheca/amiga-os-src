
/* This file contains the routines used to process special
 * commands like font changes, bold, underline, etc.
 */

#include <exec/types.h>
#include <devices/prtbase.h>
#include <devices/printer.h>
#include <prefs/printerps.h>
#include <stdio.h>
#include <string.h>

#include "driver.h"
#include "textbuf.h"
#include "convfunc.h"
#include "dospecial.h"


/*****************************************************************************/


extern struct PrinterPSPrefs  ps_pref;
extern struct PrinterData    *PD;

extern LONG    curcol;
extern LONG    curloc;
extern BOOLEAN beginline;
extern LONG    TopPage;

LONG    line_offset;
LONG    toppage;
LONG    maxloc;
LONG    maxtloc;
BOOLEAN bold;
BOOLEAN italic;
BOOLEAN underline;
BOOLEAN shadow;
BOOLEAN script;

/* This is a table of flags indicating which fonts have been defined.  As
 * each font is encountered the PostScript code needed to create the font is
 * generated and one of these flags is set so that it does not need to be
 * done again.
 */
UBYTE fontdefs[MAX_FONTS];


/*****************************************************************************/


static const STRPTR fonts[] =
{
    "Courier",
    "Courier-Bold",
    "Courier-Oblique",
    "Courier-BoldOblique",

    "Times-Roman",
    "Times-Bold",
    "Times-Italic",
    "Times-BoldItalic",

    "Helvetica",
    "Helvetica-Bold",
    "Helvetica-Oblique",
    "Helvetica-BoldOblique",

    "Helvetica-Narrow",
    "Helvetica-Narrow-Bold",
    "Helvetica-Narrow-Oblique",
    "Helvetica-Narrow-BoldOblique",

    "AvantGarde-Book",
    "AvantGarde-Demi",
    "AvantGarde-BookOblique",
    "AvantGarde-DemiOblique",

    "Bookman-Light",
    "Bookman-Demi",
    "Bookman-LightItalic",
    "Bookman-DemiItalic",

    "NewCenturySchlbk-Roman",
    "NewCenturySchlbk-Bold",
    "NewCenturySchlbk-Italic",
    "NewCenturySchlbk-BoldItalic",

    "Palatino-Roman",
    "Palatino-Bold",
    "Palatino-Italic",
    "Palatino-BoldItalic",

    "ZapfChancery-MediumItalic",
    "ZapfChancery-MediumItalic",
    "ZapfChancery-MediumItalic",
    "ZapfChancery-MediumItalic",
};

/* PostScript Routines to generate each type of font (normal,
 * compressed, expanded).
 */
static const STRPTR font_defs[] = {"FN","FC","FE"};

/* Color mapping table for foreground colors */
static const STRPTR grey_color[] =
{
    "0.0", /* Default: Black   */
    "0.1", /*          Red     */
    "0.2", /*          Green   */
    "0.3", /*          Blue    */
    "0.4", /*          Magenta */
    "0.5", /*          Cyan    */
    "0.6", /*          ????    */
    "0.8", /*          ????    */
};


/*****************************************************************************/


/* Converts the fixed point number 'value' to a string */
void FixedPointToStr(LONG value, STRPTR result)
{
LONG i;

    i      = value%1000;
    value /= 1000;

    /* If there is a fractional part... */
    if (i)
    {
        if (i < 0)
            i = -i;

        sprintf(result,"%ld.%03ld",value,i);

        /* Remove trailing 0's */
        i = strlen(result);
        while (result[--i] == '0')
            result[i] = 0;
    }
    else
    {
        /* There is no fractional points, so just use a simple printf */
        sprintf(result,"%ld",value);
    }
}


/*****************************************************************************/


VOID DoCmd(UWORD cmd, UBYTE *parms, BYTE *vline, BYTE *currentVMI)
{
char    tmpbuf[100];
char    tmp1[20];
LONG    i,j;
ULONG   it,bd;
BOOLEAN recalc;
BOOLEAN fontflg;

    /* If the current command sets these the font or page size will be
     * recalculated.
     */
    fontflg = FALSE;
    recalc  = FALSE;

    switch (cmd)
    {
        case aRIN : /* Initialize */

        case aRIS : /* Reset */
                    toppage = TopPage;
                    line_offset = 0;

        case aSGR0: /* Normal character set */
                    bold      = FALSE;
                    italic    = FALSE;
                    underline = FALSE;
                    shadow    = FALSE;
                    fontflg   = TRUE;

                    if (currentVMI)
                        *currentVMI = (216*(ps_pref.ps_FontPointSize + ps_pref.ps_Leading))/72000;

                    break;

        case aIND : /* Lf */
                    script = TRUE;

        case aNEL : /* Return, LF */
                    curloc -= (ps_pref.ps_FontPointSize + ps_pref.ps_Leading);
                    CheckNewPage(FALSE,TRUE);
                    if (cmd != aIND)
                    {
                        beginline = TRUE;
                        curcol    = 0;
                    }
                    break;

        case aRI  : /* Reverse, LF */
                    curloc += (ps_pref.ps_FontPointSize + ps_pref.ps_Leading);
                    script = TRUE;
                    curcol = 0;
                    break;

        case aSGR3: /* Italics on */
                    italic  = TRUE;
                    fontflg = TRUE;
                    break;

        case aSGR23: /* Italics off */
                     italic  = FALSE;
                     fontflg = TRUE;
                     break;

        case aSGR4: /* Underline on */
                    underline = TRUE;
                    break;

        case aSGR24: /* Underline off */
                     underline = FALSE;
                     break;

        case aSGR1: /* Boldface on */
                    bold    = TRUE;
                    fontflg = TRUE;
                    break;

        case aSGR22: /* Boldface off */
                     bold    = FALSE;
                     fontflg = TRUE;
                     break;

        case aSFC : /* Set foreground color */
                    i = parms[0] - 30;

                    if (i == 9)
                        i = 0;

                    if (i >= 0 && i <= 8)
                    {
                        PSWrite(grey_color[i]);
                        PSWrite(" setgray\n");
                    }
                    break;

        case aDEN6: /* Shadow print on */
                    shadow = TRUE;
                    break;

        case aDEN5: /* Shadow print off */
                    shadow = FALSE;
                    break;

        case aSHORP0: /* Normal pitch */
        case aSHORP1: /* Elite off */
        case aSHORP2: /* Elite on */
        case aSHORP3: /* Condensed fine off */
        case aSHORP5: /* Enlarged off */
                      ps_pref.ps_Pitch = PITCH_NORMAL;
                      PREF.PrintPitch  = ELITE;
                      fontflg          = TRUE;
                      break;

        case aSHORP4: /* Condensed fine on */
                      ps_pref.ps_Pitch = PITCH_COMPRESSED;
                      PREF.PrintPitch  = FINE;
                      fontflg          = TRUE;
                      break;

        case aSHORP6: /* Enlarged on */
                      ps_pref.ps_Pitch = PITCH_EXPANDED;
                      PREF.PrintPitch  = PICA;
                      fontflg          = TRUE;
                      break;

        case aVERP0: /* 1/8" line spacing */
                     PREF.PrintSpacing  = EIGHT_LPI;
                     ps_pref.ps_Leading = -(ps_pref.ps_FontPointSize/4);
                     *currentVMI        = (216*(ps_pref.ps_FontPointSize + ps_pref.ps_Leading))/72000;
                     recalc             = TRUE;
                     break;

        case aVERP1: /* 1/6" line spacing */
                     PREF.PrintSpacing  = SIX_LPI;
                     ps_pref.ps_Leading = 0;
                     *currentVMI        = (216 * ps_pref.ps_FontPointSize)/72000;
                     recalc             = TRUE;
                     break;

        case aLMS : /* Left Margin */
        case aRMS : /* Right Margin */
        case aSLRM: /* L&R Margins */

                    switch (cmd)
                    {
                        case aLMS: /* Left Margin */
                                   PREF.PrintLeftMargin = curcol+1;
                                   break;

                        case aRMS: /* Right Margin */
                                   PREF.PrintRightMargin = curcol+2;
                                   break;

                        case aSLRM: /* L&R Margins */
                                    PREF.PrintLeftMargin = parms[0];
                                    PREF.PrintRightMargin = parms[1];
                                    break;
                    }

                    switch (PREF.PrintPitch)
                    {
                        case ELITE: i = 12; break;
                        case FINE : i = 15; break;
                        case PICA : i = 10; break;
                    }

                    j                      = ps_pref.ps_LeftMargin;
                    ps_pref.ps_LeftMargin  = MARG_LEFT + ((PREF.PrintLeftMargin-1) * 72000)/i;
                    ps_pref.ps_RightMargin = ps_pref.ps_PaperWidth - ((PREF.PrintRightMargin) * 72000)/i;

                    if (j != ps_pref.ps_LeftMargin)
                    {
                        FixedPointToStr(ps_pref.ps_LeftMargin,tmpbuf);
                        PSWrite("/L ");
                        PSWrite(tmpbuf);
                        PSWrite(" def\n");
                    }

                    MarginClip(TRUE);
                    break;

        case aTMS : /* Top Margin */
                    ps_pref.ps_TopMargin = curloc;
                    toppage              = curloc;
                    recalc               = TRUE;
                    break;

        case aBMS : /* Bottom Margin */
                    ps_pref.ps_BottomMargin = curloc + ps_pref.ps_FontPointSize + ps_pref.ps_Leading;
                    recalc                  = TRUE;
                    break;

        case aSTBM: /* T&B Margins */
                    if (parms[0])
                    {
                        toppage = TopPage - ((parms[0]-1) *
                            (ps_pref.ps_Leading + ps_pref.ps_FontPointSize));

                        if (curloc > toppage)
                            curloc = toppage;
                    }

                    if (parms[1])
                    {
                        PREF.PaperLength = parms[1];
                        fontflg = TRUE;
                        recalc = TRUE;

                        ps_pref.ps_BottomMargin = ps_pref.ps_PaperHeight -
                            ps_pref.ps_TopMargin +
                            (parms[0]*(ps_pref.ps_Leading + ps_pref.ps_FontPointSize));
                    }
                    break;

        case aSLPP: /* Set form length to N */
                    if (parms[0])
                    {
                        PREF.PaperLength = parms[0];
                        fontflg = TRUE;
                        recalc = TRUE;

                        ps_pref.ps_BottomMargin =
                            ps_pref.ps_PaperHeight -
                            ps_pref.ps_TopMargin +
                            (parms[0]*(ps_pref.ps_Leading + ps_pref.ps_FontPointSize));
                    }
                    break;

        case aCAM : /* Clear Margins */
                    toppage = TopPage;
                    break;

        case aDEN2: /* NLQ on */
                    PREF.PrintQuality = LETTER;
                    fontflg = TRUE;
                    break;

        case aDEN1: /* NLQ off */
                    PREF.PrintQuality = DRAFT;
                    fontflg = TRUE;
                    break;

        case aSUS0: /* Normal the line */
                    line_offset = 0;
                    script = TRUE;
                    *vline = 0;
                    break;

        case aSUS2: /* Superscript on */
                    if (*vline < 1)
                    {
                        line_offset = 1;
                        script = TRUE;
                        *vline = 1;
                    }
                    break;

        case aSUS1: /* Superscript off */
                    if (*vline > 0)
                    {
                        line_offset = 0;
                        *vline = 0;
                        script = TRUE;
                    }
                    break;

        case aSUS4: /* Subscript on */
                    if (*vline > -1)
                    {
                        line_offset = -1;
                        script = TRUE;
                        *vline = -1;
                    }
                    break;

        case aSUS3: /* Subscript off */
                    if (*vline < 0)
                    {
                        line_offset = 0;
                        *vline = 0;
                        script = TRUE;
                    }
                    break;

        case aPLU: /* Partial line up */
                   ++line_offset;
                   ++(*vline);
                   script = TRUE;
                   break;

        case aPLD: /* Partial line down */
                   --line_offset;
                   --(*vline);
                   script = TRUE;
                   break;

        case aFNT0: /* US char set */
        case aFNT1: /* French char set */
        case aFNT2: /* German char set */
        case aFNT3: /* UK char set */
        case aFNT4: /* Danish char set */
        case aFNT5: /* Swedish char set */
        case aFNT6: /* Italian char set */
        case aFNT7: /* Spanish char set */
        case aFNT8: /* Japaneese char set */
                    ps_pref.ps_Font = cmd - aFNT0;  /* One of the FONT_XXX constants from <prefs/printerps.h> */
                    fontflg = TRUE;
                    break;

        case aFNT9 : /* Norwegian char set */
        case aFNT10: /* Danish II char set */
                     ps_pref.ps_Font = FONT_COURIER;
                     fontflg = TRUE;
                     break;

/*
        case aSBC   : /* Set background color */
        case aDEN4  : /* Doublestrike on */
        case aDEN3  : /* Doublestrike off */
        case aPROP2 : /* Proportional on */
        case aPROP1 : /* Proportional off */
        case aPROP0 : /* Proportional clear */
        case aTSS   : /* Set proportional offset */
        case aJFY5  : /* Auto left justify */
        case aJFY7  : /* Auto right justify */
        case aJFY6  : /* Auto full justify */
        case aJFY0  : /* Auto justify off */
        case aJFY3  : /* Letter space justify */
        case aJFY1  : /* Word fill(auto center) */
        case aPERF  : /* Perf skip N */
        case aPERF0 : /* Perf skip off */
        case aHTS   : /* Set horizontial tab */
        case aVTS   : /* Set vertical tab */
        case aTBC0  : /* Clear horizontal tab */
        case aTBC3  : /* Clear all horizontal tabs */
        case aTBC1  : /* Clear vertical tab */
        case aTBC4  : /* Clear all v tabs */
        case aTBCALL: /* Clear all tabs */
        case aTBSALL: /* Set default tabs */
        case aEXTEND: /* Extended commands */
        case aRAW   : /* Raw Characters */
*/
        default     : break;
    }

    if (recalc || fontflg)
    {
        maxloc = ps_pref.ps_BottomMargin;
        maxtloc= maxloc + ps_pref.ps_Leading + ps_pref.ps_FontPointSize;

        PREF.PaperLength = (ps_pref.ps_PaperHeight - ps_pref.ps_TopMargin -
                           ps_pref.ps_BottomMargin) / (ps_pref.ps_FontPointSize +
                           ps_pref.ps_Leading);

        CheckNewPage(FALSE,TRUE);
    }

    if (fontflg)
    {
        if (bold)
            bd = 1;
        else
            bd = 0;

        if (italic)
            it = 2;
        else
            it = 0;

        i = bd+it+(ps_pref.ps_Font * 4);
        j = i*3+ps_pref.ps_Pitch;

        if (!fontdefs[j])
        {
            FixedPointToStr(ps_pref.ps_FontPointSize,tmp1);
            sprintf(tmpbuf,"/F%ld %s /OF%ld /%s /OF%ld %s def\n",
                j,tmp1,j,fonts[i],j,font_defs[ps_pref.ps_Pitch]);
            PSWrite(tmpbuf);
        }

        sprintf(tmpbuf,"F%ld setfont\n",j);
        PSWrite(tmpbuf);

        /* Indicate that this font has been created */
        fontdefs[j] = 1;
    }
}


/*****************************************************************************/


/* This is the standard dospecial printer driver entry point */
LONG __saveds __stdargs DriverDoSpecial(UWORD *cmd, char *buf, BYTE *vline,
                                        BYTE *currentVMI, BYTE *crlfFlag,
                                        UBYTE *parms)
{
    if ((*cmd == aRIN) || (ps_pref.ps_DriverMode == DM_PASSTHROUGH))
        return (0);

    /* Clear any characters in the current line buffer */
    ProcChar(0);

    /* Process the special command */
    DoCmd(*cmd,parms,vline,currentVMI);

    return(0);
}
