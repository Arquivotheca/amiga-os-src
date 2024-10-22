
/* Character Output Processing
 *
 * This is the heart of the text processing in the printer driver. This
 * function can only return data to be output, it can not actually output
 * anything. Also, the size of the buffer that it can return stuff in is small
 * so buffer management routines are used to save PostScript code that is
 * generated. In cases where more code is generated than can be returned, the
 * code returned from this routine may not correspond to the actual characters
 * passed in. If data is still left in these buffers when the printer is
 * closed it is flushed.
 */

#include <exec/types.h>
#include <devices/prtbase.h>
#include <devices/printer.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include <prefs/prefhdr.h>
#include <prefs/printerps.h>

#include "driver.h"
#include "textbuf.h"
#include "dospecial.h"
#include "convfunc.h"


/*****************************************************************************/


extern struct PrinterPSPrefs  ps_pref;
extern struct PrinterData    *PD;

extern STRPTR  outbuf1;
extern STRPTR  outbuf2;
extern BOOLEAN initflg;
extern BOOLEAN outflg;
extern BOOLEAN any_outflg;
extern LONG    curcol;
extern LONG    curloc;
extern LONG    maxloc;
extern LONG    maxtloc;
extern BOOLEAN beginline;
extern LONG    toppage;
extern BOOLEAN forced_page;
extern BOOLEAN underline;
extern BOOLEAN shadow;
extern LONG    line_offset;
extern BOOLEAN script;

STRPTR optr;

static STRPTR pshdr = "%!";
LONG          pshd_cnt;


/*****************************************************************************/


static VOID OctalStr(STRPTR ptr, LONG num)
{
    *(ptr++) = '\\';

    ptr += 3;

    *(ptr--) = 0;
    *(ptr--) = (num & 0x07) + '0';
    num >>= 3;
    *(ptr--) = (num & 0x07) + '0';
    num >>= 3;
    *ptr = (num & 0x03) + '0';
}


/*****************************************************************************/


LONG __saveds __stdargs DriverConvert(char *buf, LONG c, LONG flag)
{
    if (ps_pref.ps_DriverMode == DM_PASSTHROUGH)
    {
        any_outflg = TRUE;
        *buf = c;
        return (1);
    }

    /* Handle initial empty lines, this is to fix problems
     * with sending the postscript header when there is nothing
     * but blank lines to output
     */
    if (c == '\n' && initflg)
    {
        curloc -= (ps_pref.ps_FontPointSize + ps_pref.ps_Leading);

        any_outflg = TRUE;
        outflg = TRUE;

        if (curloc < maxtloc)
            curloc = toppage;

        return (0);
    }

    /* If this is the beginning of an escape sequence just pass it back */
    if (c == 0x1b || c == 0x9b)
        return (-1);

    /* Look for auto pass through mode */
    if (pshd_cnt != -1 && pshd_cnt < 2)
    {
        if (pshdr[pshd_cnt] == c)
        {
            pshd_cnt++;

            if (pshd_cnt == 2)
            {
                ps_pref.ps_DriverMode = DM_PASSTHROUGH;
                strcpy(buf,pshdr);
                return (2);
            }

            return (0);
        }
    }

    ProcChar(c);

    if (bufferHasBytes)   /* only call this if there is something to drain */
        return(PSDrain(buf));

    return(0);
}


/*****************************************************************************/


VOID ProcChar(char c)
{
LONG        i, j;
char        tmp1[20], tmp2[20];
STRPTR      ptr;
static LONG bs_cnt;
const  LONG tab_units[] = {18,36,72};

    if (pshd_cnt != -1)
    {
        j = pshd_cnt;
        pshd_cnt = -1;

        for (i=0; i<j; i++)
            ProcChar(pshdr[i]);
    }

    if (initflg)
    {
        initflg = FALSE;
        bs_cnt  = 0;
        optr    = outbuf2;
    }

    if (bs_cnt)
    {
        if (c == '\b')
        {
            ++bs_cnt;
            return;
        }
        else
        {
            j = optr - outbuf2;

            if (j >= bs_cnt)
                j = bs_cnt;

            ptr = outbuf1;
            *(ptr++) = '(';

            for (i=0;i<j;i++)
            {
                --optr;

                if (*optr == '\\' || *optr == ')' || *optr == '(')
                {
                    *(ptr++) = '\\';
                    *(ptr++) = *optr;
                    continue;
                }

                if (!(*optr & 0x80) && isprint(*optr))
                {
                    *(ptr++) = *optr;
                }
                else
                {
                    OctalStr(ptr,*optr);
                    ptr += 4;
                }
            }

            for (;i<bs_cnt;i++)
                *(ptr++) = ' ';

            strcpy(ptr,") BS\n");

            optr = outbuf2;

            bs_cnt = 0;
        }
    }

    if (!c || (c == '\r') || (c == '\n') || (c == '\f') || (c == 27) ||
        (c == '\b') || ((c =='\t') && (ps_pref.ps_Tab > TAB_8)))
    {
        *optr = 0;

        any_outflg = TRUE;

        if (c == '\n')
            outflg = TRUE;

        if (*outbuf2 || c == '\t')
        {
            if (beginline || script)
            {
                i = curloc - ps_pref.ps_FontPointSize;

                if (line_offset)
                    i += (line_offset * ps_pref.ps_FontPointSize) >> 1;

                FixedPointToStr(i,tmp2);

                if (beginline)
                {
                    FixedPointToStr(ps_pref.ps_LeftMargin,tmp1);
                    strcat(outbuf1,tmp1);
                    strcat(outbuf1," ");
                    strcat(outbuf1,tmp2);
                    strcat(outbuf1," M\n");
                }
                else
                {
                    strcat(outbuf1,tmp2);
                    strcat(outbuf1," YM\n");
                }

                beginline = FALSE;
                script = FALSE;
            }

            if (*outbuf2)
            {
                strcat(outbuf1,"(");

                /* Point ptr at the end of outbuf1 */
                for (ptr=outbuf1;*ptr;ptr++);

                /* Copy the output string */
                for (optr=outbuf2;*optr;optr++)
                {
                    if (*optr == '\\' || *optr == ')' || *optr == '(')
                    {
                        *(ptr++) = '\\';
                        *(ptr++) = *optr;
                        continue;
                    }

                    if (!(*optr & 0x80) && isprint(*optr))
                    {
                        *(ptr++) = *optr;
                    }
                    else
                    {
                        OctalStr(ptr,*optr);
                        ptr += 4;
                    }
                }

                strcpy(ptr,") ");
                for (;*ptr;++ptr);

                if (underline)
                {
                    strcpy(ptr,"U ");
                    for (;*ptr;++ptr);
                }

                if (shadow)
                    strcpy(ptr,"S");

                strcpy(ptr,"S\n");

                outflg = TRUE;
                any_outflg = TRUE;
            }

            if (c == '\b')
            {
                bs_cnt = 1;
                return;
            }
        }
        else
        {
            /* BS, There is nothing in the last string buffer so ignore it */
            if (c == '\b')
               return;
        }

        optr = outbuf2;
    }

    if (outbuf1[0])
    {
        PSWrite(outbuf1);
        outbuf1[0] = 0;
    }

    switch (c)
    {
        case '\r': beginline = TRUE;
                   curcol    = 0;
                   break;

        case '\n': curloc -= (ps_pref.ps_FontPointSize + ps_pref.ps_Leading);
                   CheckNewPage(FALSE,TRUE);
                   beginline = TRUE;
                   curcol = 0;
                   break;

        case '\t': if (ps_pref.ps_Tab <= TAB_8)
                   {
                       if (ps_pref.ps_Tab == TAB_4)
                           j = 4;
                       else
                           j = 8;

                       i = j - (curcol % j);
                       curcol += i;

                       while (i--)
                           *(optr++) = ' ';
                   }
                   else
                   {
                       sprintf(tmp1,"%ld T\n",tab_units[ps_pref.ps_Tab - TAB_QUART]);
                       PSWrite(tmp1);
                   }
                   break;

        case 0   : break;

        case '\f': if (!outflg && forced_page)
                   {
                       forced_page = FALSE;
                   }
                   else
                   {
                       /* New Page */
                       CheckNewPage(TRUE,TRUE);
                   }
                   break;

        default  : (*optr++) = c;
                   curcol++;
                   break;
    }
}


/*****************************************************************************/


VOID CheckNewPage(BOOL forced, BOOL directWrite)
{
    if (forced || (curloc < maxtloc))
    {
        if (directWrite)
            PSWrite("SP\n");
        else
            strcat(outbuf1,"SP\n");

        curloc      = toppage;
        outflg      = FALSE;
        beginline   = TRUE;
        curcol      = 0;
        forced_page = FALSE;

        MarginClip(directWrite);
    }
}


/*****************************************************************************/


/* Create a clipping path so that text rendering is limited to within the left
 * and right margins. Don't bother with the top/bottom margins, the formatting
 * code takes care of that
 */
VOID MarginClip(BOOL directWrite)
{
char temp[200];
char num[30];

    strcpy(temp,"initclip\n");
    strcpy(temp,"newpath\n");
    FixedPointToStr(ps_pref.ps_LeftMargin,num);
    strcat(temp,num);
    strcat(temp," ");
    FixedPointToStr(ps_pref.ps_PaperHeight,num);
    strcat(temp,num);
    strcat(temp," moveto\n");

    FixedPointToStr(ps_pref.ps_PaperWidth - ps_pref.ps_RightMargin,num);
    strcat(temp,num);
    strcat(temp," ");
    FixedPointToStr(ps_pref.ps_PaperHeight,num);
    strcat(temp,num);
    strcat(temp," lineto\n");

    FixedPointToStr(ps_pref.ps_PaperWidth - ps_pref.ps_RightMargin,num);
    strcat(temp,num);
    strcat(temp," 0 lineto\n");

    FixedPointToStr(ps_pref.ps_LeftMargin,num);
    strcat(temp,num);
    strcat(temp," 0 lineto\n");

    strcat(temp,"closepath\n");
    strcat(temp,"clip\n");

    if (directWrite)
        PSWrite(temp);
    else
        strcat(outbuf1,temp);
}
