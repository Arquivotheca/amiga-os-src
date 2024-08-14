/*
    Copyright (c) 1989 Mitchell/Ware Systems, Inc.

    May be used by Commodore-Amiga, Inc. for in-house purposes only.
    No part of this program, or any parts or modifications thereof, either
    in source-code or object-code, or library form, may be used by
    Commodore, or any of its employees, either indepently, or in conjunction
    with Commordore, as, or as a part of , any public-domain or commercial
    product, program, or any type of software, teaching aid, etc. without
    the prior written permission of Mitchell/Ware Systems.

    This copyright notice must not be removed from the document of which
    it is attached.
*/
/*****************************************************************************
    EdTools.c

    Mitchell/Ware Systems       Version 1.03                 7-17-89

    Line Edit Routines and such.

    ________________________________________________________________


    RPLineEd *LEInit( window, font, exfunct, helpfunct )

            - Init and allocate RPLineEd struct

    void LEDInit( r )

            - Delete RPLineEd structure

    char *LineEd( r, x, y, string, chpos )

            - Edit!!! Returns upon certain keys or inputevents. If the
              event is a nonsequiter, the IntuiMessage is NOT removed from
              the queue! Therefore, one MUST check the error status.

            - x, y  - used if either or both are non-zero else defaults
                      to former.

            - chpos - is used if non-negative else defaults to former.

            - returns - a newly allocated string or NULL. Does not modify
                        old string. Always check r->error! Nonsequiters
                        are left in queue!

*****************************************************************************/

#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <string.h>
#include <ctype.h>
#include "Tools.h"
#include "ascii.h"
#include "raw.h"

struct RPLineEd *LEInit(win, font, exfun, helpfun)
struct Window *win;
struct TextFont *font;
void (*exfun)(), (*helpfun)();
{
    struct RPLineEd *r = NULL;

    if (r = calloc(1, sizeof(*r)))
    {
        r->win = win;
        if (!(r->font = font))
        {
            if (!(r->font = win->RPort->Font))
                fuckup("LEInit: No Findum Font!");
        }
        r->curheight = r->font->tf_YSize;
        r->curbase = r->font->tf_Baseline;
        r->exfunct = exfun;
        r->helpfunct = helpfun;
        r->mode = JAM2;
        r->fore = 1;
        r->back = 0;
        r->spacing = 2; /* might be changed... */
    }
    return r;
}

void LEDInit(r)
struct RPLineEd *r;
{
    free(r);
}

/* Display Cursor -
    NOTE: Characters are positioned based on their baseline being the
          origin. Therefore, we must look at that to determine where to
          place our cursor!
*/
void _disp_cur(r)   /* display cursor */
struct RPLineEd *r;
{
    register struct RastPort *rp = r->win->RPort;
    register x, y, yy;

    SetDrMd(rp, COMPLEMENT);

    /* We shall now calculate where the cursor is to appear!
    */
    x = r->startx + r->xpos;
    y = r->starty - r->curbase;

    /* And now, for the final act of drawing it!
    */
    Move(rp, x-1, y);                   Draw(rp, x+1, y);
    Move(rp, x, y);                     Draw(rp, x, yy = y + r->curheight - 1);
    Move(rp, x-1, yy);                  Draw(rp, x+1, yy);
 /* Move(rp, x-1, yy = y + r->curbase); Draw(rp, x+1, yy); */

    /* And reset the draw mode
    */
    SetDrMd(rp, r->mode);
}

BOOL _is_cur_visible(r)
struct RPLineEd *r;
{
    register x, y;

    x = r->startx + r->xpos;
    y = r->starty;

    if (x < r->leftm || x >= r->win->Width - r->rightm)
        return FALSE;

    if (y < r->topm || y >= r->win->Height - r->botm)
        return FALSE;

    return TRUE;
}

char *_finish_up(r) /* clean up & return an allocated version of new string */
struct RPLineEd *r;
{
    char *str;

    str = calloc(1, strlen(r->buf)+1);
    strcpy(str, r->buf);
    return str;
}

void _raw_process(r)
struct RPLineEd *r;
{
    register struct RastPort *rp = r->win->RPort;
    register char *p;

    switch (r->chCode)
    {
    case LTARROW:
        if (r->chpos > 0)
        {
            _disp_cur(r);   /* undisplay cursor */
            --r->chpos;
            r->xpos = TextLength(rp, r->buf, r->chpos);
            _disp_cur(r);   /* redisplay cursor */

            if (!_is_cur_visible(r))
                r->error = ed_hidden;
        }
        break;

    case RTARROW:
        if (r->chpos < strlen(r->buf))
        {
            _disp_cur(r);   /* undisplay cursor */
            ++r->chpos;
            r->xpos = TextLength(rp, r->buf, r->chpos);
            _disp_cur(r);   /* redisplay cursor */

            if (!_is_cur_visible(r))
                r->error = ed_hidden;
        }
        break;

    case F10: /* INSERT for now! Insert a space */
        strins(r->buf + r->chpos, " ");
        _disp_cur(r);   /* undisplay cursor */

        Move(rp, r->startx, r->starty);
        Text(rp, r->buf, strlen(r->buf));
        _disp_cur(r);   /* redisplay cursor */

        if (strlen(r->buf) > sizeof(r->buf) - 2)
            r->error = ed_overflow;

        break;

    case DEL:   /* Delete a character */
        if (r->chpos < strlen(r->buf))
        {
            p = r->buf + r->chpos;
            movmem(p+1, p, strlen(p));

            _disp_cur(r);   /* undisplay cursor */
            Move(rp, r->startx, r->starty);
            Text(rp, r->buf, strlen(r->buf));
            Text(rp, " ", 1); /* to erase the very last character */
            _disp_cur(r);   /* redisplay cursor */
        }
        break;

    case BS:   /* BackSpace Delete a character */
        if (r->chpos)
        {
            --r->chpos;
            p = r->buf + r->chpos;
            movmem(p+1, p, strlen(p));

            _disp_cur(r);   /* undisplay cursor */
            Move(rp, r->startx, r->starty);
            Text(rp, r->buf, strlen(r->buf));
            Text(rp, " ", 1); /* to erase the very last character */
            r->xpos = TextLength(rp, r->buf, r->chpos);
            _disp_cur(r);   /* redisplay cursor */
        }
        break;

    case HELP:
        if (r->helpfunct)
            (*r->helpfunct)(r);
        break;

    default:
        r->error = ed_raw;
        break;
    }
}

void _vanilla_process(r)
struct RPLineEd *r;
{
    register struct RastPort *rp = r->win->RPort;
    char c[2];

    if (iscntrl(r->chCode) || r->chCode >= DEL)
        _raw_process(r);
    else
    {
        _disp_cur(r);   /* undisplay cursor */

        c[0] = r->chCode;
        c[1] = '\0';
        strins(r->buf + r->chpos, c);
        ++r->chpos;
        r->xpos = TextLength(rp, r->buf, r->chpos);

        Move(rp, r->startx, r->starty);
        Text(rp, r->buf, strlen(r->buf));
        _disp_cur(r);   /* redisplay cursor */

        if (strlen(r->buf) >= sizeof(r->buf)-2) /* if overflow */
            r->error = ed_overflow;

        else if (!_is_cur_visible(r))
            r->error = ed_hidden;
    }
}

char *LineEd(r, x, y, str, chpos)
struct RPLineEd *r;
short x, y;
char *str;
short chpos;
{
    register struct RastPort *rp = r->win->RPort;
    register struct IntuiMessage *m;
    register char *sret = NULL;

    r->error = ed_ok;
    SetDrMd(rp, r->mode);
    SetAPen(rp, r->fore);
    SetBPen(rp, r->back);
    SetFont(rp, r->font);
    strcpy(r->buf, str);

    if (x || y)
    {
        r->startx = x;
        r->starty = y;
    }

    Move(rp, r->startx, r->starty);
    Text(rp, r->buf, strlen(r->buf));
    if (chpos >= 0)
        r->chpos = chpos;
    r->xpos = TextLength(rp, r->buf, r->chpos);
    _disp_cur(r);  /* turn on cursor */
    if (_is_cur_visible(r))
    {
        for (;r->error == ed_ok ;)
        {
            m = WaitPort(r->win->UserPort);
            switch(m->Class)
            {
            case INTUITICKS: /* We trap any timer operation. */
            case VANILLAKEY:
            case RAWKEY:
            case MOUSEMOVE: /* We trap these. We must allow user to move mouse about. */
                break;

            default:
                r->error = ed_nonsequiter;
                sret = _finish_up(r);
                break;
            }
            if (r->error != ed_ok)
                break;

            m = GetMsg(r->win->UserPort);
            r->chClass = m->Class;
            r->chCode = m->Code;
            r->chQualifier = m->Qualifier;
            r->chX = m->MouseX;
            r->chY = m->MouseY;
            ReplyMsg(m);

            switch(r->chClass)
            {
            case VANILLAKEY:
                _vanilla_process(r);
                break;

         /* case RAWKEY:        Note: RAWKEY does not function along with VANILLAKEY!!! Use Keymap mws1.
                _raw_process(r);
                break; */
            }
            if (r->error != ed_ok)
                sret = _finish_up(r);
        }
    }
    else /* cursor is not visible */
    {
        r->error = ed_hidden;
        sret = NULL;
    }
    _disp_cur(r);   /* turn off cursor */
    return sret;
}