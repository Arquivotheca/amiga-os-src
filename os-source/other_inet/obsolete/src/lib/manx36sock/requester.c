/*
 * Put up a requester.  Text may be up to 3 lines long.  Two buttons
 * may be specified.  proc returns 0 or 1 corresponding to a click
 * over one of the buttons.
 */

#include <intuition/intuition.h>
#ifdef LATTICE
#include <proto/exec.h>
#include <proto/intuition.h>
#endif

#include <bstr.h>

#define REQITEXT(p) {0, 0, JAM1, 7, 3, NULL, (UBYTE *) p, NULL}

#define NUM_LINES 3
static struct IntuiText text[NUM_LINES] = {
	REQITEXT(""),
	REQITEXT(""),
	REQITEXT("")
};
static struct IntuiText pos = REQITEXT("");
static struct IntuiText neg = REQITEXT("");
static UBYTE buf[256];

struct IntuitionBase *IntuitionBase;

static
font_width()
{
	struct Preferences pref;

	GetPrefs(&pref, (long)sizeof(pref));

	return ((pref.FontHeight == TOPAZ_SIXTY) ? 10 : 8);
}

requester(positive, negative, fmt, a, b, c, d, e, f, g, h)
	UBYTE	*positive, *negative;
	char	*fmt;
	unsigned a, b, c, d, e, f, g, h;
{
	int rtn, openedintuition, len, line, width;
	register UBYTE *p;

	openedintuition = 0;
  	if(IntuitionBase == (struct IntuitionBase *)0L){
		IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", 0L);
		if(IntuitionBase == 0){
			return -1;
		}
		openedintuition++;
    	}

	width = font_width();
	pos.IText = positive; neg.IText = negative;

	sprintf(buf, fmt, a, b, c, d, e, f, g, h);

	for(line = 0, p = buf; p && *p && (line < NUM_LINES); line++){
		text[line].IText = p;
		p = (UBYTE *)index(text[line].IText, '\n');
		if(p){
			*p++ = 0;
		}
		len = strlen(text[line].IText);
		if(len > 36){
			len = 36;
			text[line].IText[len] = 0;
		}
		text[line].TopEdge = 7 + line*10;
		text[line].LeftEdge = 4 + ((36 - len)/2)*width; 
		text[line].NextText = &text[line+1];
	}
	text[line - 1].NextText = 0;

	rtn = AutoRequest(0L, text, 	positive ? &pos : NULL, 
				    	negative ? &neg : NULL, 
					0L, 0L, (long)(width * 40), 72L);

	if(openedintuition){
		CloseLibrary(IntuitionBase);
		IntuitionBase = (struct IntuitionBase *)NULL;
	}

	return rtn;
}

