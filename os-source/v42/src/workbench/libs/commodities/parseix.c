
#include <devices/inputevent.h>
#include <string.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include "commoditiesbase.h"
#include "parseix.h"


/*****************************************************************************/


struct Keyword
{
    STRPTR word;
    LONG   id;
};


/*****************************************************************************/


/* table of classes */
static const struct Keyword Classes[] =
{
    {"DISKINSERTED",  IECLASS_DISKINSERTED},
    {"DISKREMOVED",   IECLASS_DISKREMOVED},
    {"EVENT",         IECLASS_EVENT},
    {"NEWPOINTERPOS", IECLASS_NEWPOINTERPOS},
    {"NEWPREFS",      IECLASS_NEWPREFS},
    {"POINTERPOS",    IECLASS_POINTERPOS},
    {"RAWKEY",        IECLASS_RAWKEY},
    {"RAWMOUSE",      IECLASS_RAWMOUSE},
    {"TIMER",         IECLASS_TIMER}
};
#define NUMCLASSES (sizeof(Classes) / sizeof(struct Keyword))


/*****************************************************************************/


/* table of qualifiers */
static const struct Keyword Qualifiers[] =
{
    {"CAPSLOCK",      IEQUALIFIER_CAPSLOCK},
    {"CAPS_LOCK",     IEQUALIFIER_CAPSLOCK},
    {"CONTROL",       IEQUALIFIER_CONTROL},
    {"CTRL",          IEQUALIFIER_CONTROL},
    {"LALT",          IEQUALIFIER_LALT},
    {"LAMIGA",        IEQUALIFIER_LCOMMAND},
    {"LBUTTON",       IEQUALIFIER_LEFTBUTTON},
    {"LCOMMAND",      IEQUALIFIER_LCOMMAND},
    {"LEFTBUTTON",    IEQUALIFIER_LEFTBUTTON},
    {"LEFT_ALT",      IEQUALIFIER_LALT},
    {"LEFT_AMIGA",    IEQUALIFIER_LCOMMAND},
    {"LEFT_BUTTON",   IEQUALIFIER_LEFTBUTTON},
    {"LEFT_COMMAND",  IEQUALIFIER_LCOMMAND},
    {"LEFT_SHIFT",    IEQUALIFIER_LSHIFT},
    {"LSHIFT",        IEQUALIFIER_LSHIFT},
    {"MBUTTON",       IEQUALIFIER_MIDBUTTON},
    {"MIDBUTTON",     IEQUALIFIER_MIDBUTTON},
    {"MIDDLEBUTTON",  IEQUALIFIER_MIDBUTTON},
    {"MIDDLE_BUTTON", IEQUALIFIER_MIDBUTTON},
    {"NUMERICPAD",    IEQUALIFIER_NUMERICPAD},
    {"NUMERIC_PAD",   IEQUALIFIER_NUMERICPAD},
    {"NUMPAD",        IEQUALIFIER_NUMERICPAD},
    {"NUM_PAD",       IEQUALIFIER_NUMERICPAD},
    {"RALT",          IEQUALIFIER_RALT},
    {"RAMIGA",        IEQUALIFIER_RCOMMAND},
    {"RBUTTON",       IEQUALIFIER_RBUTTON},
    {"RCOMMAND",      IEQUALIFIER_RCOMMAND},
    {"RELATIVEMOUSE", IEQUALIFIER_RELATIVEMOUSE},
    {"REPEAT",        IEQUALIFIER_REPEAT},
    {"RIGHTBUTTON",   IEQUALIFIER_RBUTTON},
    {"RIGHT_ALT",     IEQUALIFIER_RALT},
    {"RIGHT_AMIGA",   IEQUALIFIER_RCOMMAND},
    {"RIGHT_BUTTON",  IEQUALIFIER_RBUTTON},
    {"RIGHT_COMMAND", IEQUALIFIER_RCOMMAND},
    {"RIGHT_SHIFT",   IEQUALIFIER_RSHIFT},
    {"RSHIFT",        IEQUALIFIER_RSHIFT}
};
#define NUMQUALS (sizeof(Qualifiers) / sizeof(struct Keyword))


/*****************************************************************************/


/* Qualifier Synonyms */
static const struct Keyword QualifierSynonyms[] =
{
    {"ALT",   IXSYM_ALT},
    {"CAPS",  IXSYM_CAPS},
    {"SHIFT", IXSYM_SHIFT}
};
#define NUMSYNS (sizeof(QualifierSynonyms) / sizeof(struct Keyword))


/*****************************************************************************/


/* upstroke qualifier */
static const struct Keyword UpPfxs[] =
{
   {"UPSTROKE", 1}
};
#define NUMUPFX (sizeof(UpPfxs) / sizeof(struct Keyword))


/*****************************************************************************/


static const struct Keyword HiKeys[] =
{
    {"BACKSPACE",   0x41},
    {"BREAK",       0x6e},
    {"COMMA",       0x38},
    {"CURSOR_DOWN", 0x4d},
    {"CURSOR_LEFT", 0x4f},
    {"CURSOR_RIGHT",0x4e},
    {"CURSOR_UP",   0x4c},
    {"DEL",         0x46},
    {"DELETE",      0x46},
    {"DOWN",        0x4d},
    {"END",         0x71},
    {"ENTER",       0x43},
    {"ESC",         0x45},
    {"ESCAPE",      0x45},
    {"F1",          0x50},
    {"F10",         0x59},
    {"F11",         0x4b},
    {"F12",         0x6f},
    {"F2",          0x51},
    {"F3",          0x52},
    {"F4",          0x53},
    {"F5",          0x54},
    {"F6",          0x55},
    {"F7",          0x56},
    {"F8",          0x57},
    {"F9",          0x58},
    {"HELP",        0x5f},
    {"HOME",        0x70},
    {"INSERT",      0x47},
    {"LEFT",        0x4f},
    {"PAGE_DOWN",   0x49},
    {"PAGE_UP",     0x48},
    {"PAUSE",       0x6e},
    {"RETURN",      0x44},
    {"RIGHT",       0x4e},
    {"SPACE",       0x40},
    {"SPACEBAR",    0x40},
    {"TAB",         0x42},
    {"UP",          0x4c}
};
#define NUMHIKEYS (sizeof(HiKeys) / sizeof(struct Keyword))


/*****************************************************************************/


static const struct Keyword MouseKeys[] =
{
    {"MOUSE_LEFTPRESS",   IECODE_LBUTTON},
    {"MOUSE_MIDDLEPRESS", IECODE_MBUTTON},
    {"MOUSE_RIGHTPRESS",  IECODE_RBUTTON}
};
#define NUMMOUSEKEYS (sizeof(MouseKeys) / sizeof(struct Keyword))


/*****************************************************************************/


static const struct Keyword NumericKeys[] =
{
    {"(", 0x5a},
    {")", 0x5b},
    {"*", 0x5d},
    {"+", 0x5e},
    {"-", 0x4a},
    {".", 0x3c},
    {"/", 0x5c},
    {"0", 0x0f},
    {"1", 0x1d},
    {"2", 0x1e},
    {"3", 0x1f},
    {"4", 0x2d},
    {"5", 0x2e},
    {"6", 0x2f},
    {"7", 0x3d},
    {"8", 0x3e},
    {"9", 0x3f}
};
#define NUMNUMERICKEYS (sizeof(NumericKeys) / sizeof(struct Keyword))


/*****************************************************************************/


#define NOCARE_PREFIX   '-'
#define NOMATCH         -1
#define PARSEBUFLEN	257


/*****************************************************************************/


static WORD BinSearch(struct CxLib *CxBase, STRPTR token, UWORD tokenLen,
                      const struct Keyword *keywords, WORD numKeywords)
{
WORD l,r,x;
WORD comp;

    l = 0;
    r = numKeywords-1;

    do
    {
        x = (l+r) / 2;

        comp = Strnicmp(token,keywords[x].word,tokenLen);
        if (comp < 0)
            r = x-1;
        else if (comp > 0)
            l = x+1;
        else if (strlen(keywords[x].word) != tokenLen)
            r = x-1;
        else
            return((WORD)keywords[x].id);
    }
    while (l <= r);

    return(NOMATCH);
}


/*****************************************************************************/


static BOOL IsSeparator(char ch)
{
    return((BOOL) ((ch == ' ') || (ch == '\t') || (ch == ',') || (ch == '\n')));
}


/*****************************************************************************/


static STRPTR GetToken(STRPTR buf, UWORD *resultLen)
{
STRPTR result;

    while (IsSeparator(*buf))
        buf++;

    result = NULL;
    *resultLen = 0;

    if (*buf)
    {
        result = buf;
        while (!IsSeparator(*buf) && (*buf))
        {
            buf++;
            *resultLen = *resultLen+1;
        }
    }

    return(result);
}


/*****************************************************************************/


LONG ASM ParseIXLVO(REG(a0) STRPTR string,
                    REG(a1) struct InputXpression *ix,
                    REG(a6) struct CxLib *CxBase)
{
   /* Returns:  0 if OK
    *          -1 if tokens after end (code spec)
    *          -2 if null string 'string'
    */

STRPTR            token;
UWORD             tokenLen;
LONG              cc;
WORD              qualadd;
LONG              dontcare;
struct InputEvent ievent;

    ix->ix_Class     = IECLASS_RAWKEY;
    ix->ix_Code      = 0;
    ix->ix_CodeMask  = 0xFFFF;
    ix->ix_Qualifier = 0;
    ix->ix_QualMask  = 0x73FF;   /* note no RELATIVEMOUSE */
    ix->ix_QualSame  = 0;

    if (!string || (string[0] == 0))
    {
        ix->ix_Code = 0xFFFF;     /* nothing will ever match this */
        return (-2);
    }

    /* check for a class specificier */

    if (token = GetToken(string,&tokenLen))
    {
        if ((cc = BinSearch(CxBase,token,tokenLen,Classes,NUMCLASSES)) != NOMATCH)
        {
            ix->ix_Class = cc;
            token = GetToken(&token[tokenLen],&tokenLen);
        }

        /* check for qualifiers and synonyms */

        while (token)
        {
            /* check "don't care" prefix: e.g. -shift */
            if (dontcare = (token[0] == NOCARE_PREFIX))
            {
                token++;
                tokenLen--;
            }

            if ((cc = BinSearch(CxBase,token,tokenLen,Qualifiers,NUMQUALS)) != NOMATCH)
            {
                if (dontcare)
                    ix->ix_QualMask &= ~cc;
                else
                    ix->ix_Qualifier |= cc;
            }
            else if ((cc = BinSearch(CxBase,token,tokenLen,QualifierSynonyms,NUMSYNS)) != NOMATCH)
            {
                ix->ix_QualSame |= cc;
                switch (cc)
                {
                    case IXSYM_SHIFT: qualadd = IXSYM_SHIFTMASK;
                                      break;

                    case IXSYM_CAPS : qualadd = IXSYM_CAPSMASK;
                                      break;

                    case IXSYM_ALT  : qualadd = IXSYM_ALTMASK;
                                      break;
                }

                if (dontcare)
                    ix->ix_QualMask &= ~qualadd;
                else
                    ix->ix_Qualifier |= qualadd;
            }
            else
            {
                if (dontcare)
                {
                    /* We noted the "don't care" prefix above, but it wasn't
                     * attached to a qualifier or synonym, so we'd better
                     * back up so we can catch it below, when we test uppfxs
                     */
                    token--;
                    tokenLen++;
                }
                break;   /* try function code, below */
            }
            token = GetToken(&token[tokenLen],&tokenLen);
        }

        /* check IECODE_UP_PREFIX */

        if (token)
        {
            if (dontcare = (token[0] == NOCARE_PREFIX))
            {
                token++;
                tokenLen--;
            }

            if ((cc = BinSearch(CxBase,token,tokenLen,UpPfxs,NUMUPFX)) != NOMATCH)
            {
                if (dontcare)
                {
                    ix->ix_CodeMask &= ~IECODE_UP_PREFIX;
                }
                else
                {
                    ix->ix_Code |= IECODE_UP_PREFIX;
                }
                token = GetToken(&token[tokenLen],&tokenLen);
            }
        }

        if (token)
        {
            /* need a mousecode now */
            if (ix->ix_Class == IECLASS_RAWMOUSE)
            {
                if ((cc = BinSearch(CxBase,token,tokenLen,MouseKeys,NUMMOUSEKEYS)) != NOMATCH)
                {
                    ix->ix_Code |= cc;
                    token = GetToken(&token[tokenLen],&tokenLen);
                }
            }
            /* or a keycode */
            else if (ix->ix_Class == IECLASS_RAWKEY)
            {
                if ((cc = BinSearch(CxBase,token,tokenLen,HiKeys,NUMHIKEYS)) != NOMATCH)
                {
                    ix->ix_Code |= cc;
                    token = GetToken(&token[tokenLen],&tokenLen);
                }
                else if (strlen(token) == 1)      /* invert the keycode */
                {
                    if (ix->ix_Qualifier & IEQUALIFIER_NUMERICPAD)
                    {
                        cc= BinSearch(CxBase,token,tokenLen,NumericKeys,NUMNUMERICKEYS);
                        if (cc != NOMATCH)
                        {
                            ix->ix_Code |= cc;
                        }
                    }
                    else if (InvertKeyMap((ULONG)token[0],&ievent,NULL))
                    {
                        ix->ix_Code      |= ievent.ie_Code;
                        ix->ix_Qualifier |= ievent.ie_Qualifier;
                        /* kludge to get numeric pad keys */
                    }
                    token = GetToken(&token[tokenLen],&tokenLen);
                }
            }
        }
    }

    return ((LONG)token? -1: 0);   /* if I got here by NULL, it's OK   */
}
