/****************************************************\
* 															        *
*  Amiga Control Character Jump Table and Routines   *
* 			      												  *
\****************************************************/

#include "exec/types.h"
#include "st.h"
#include "ascii.h"
#include "menu.h"

#define APASS *strptr++ = c; /* pass ctrl-char on to console device */

extern unsigned char c, *strptr, *inputptr;
extern UBYTE beep, cr_expand_in;
extern UBYTE lf_expand_in;

pass() /* pass ctrl-char on to console device */
{
	APASS
}

aenq() /* E enquiry */
{
	APASS; /* transmit answer back message */
}

abell() /* G sound bell */
{
	if (beep!=NOFLASH) APASS
	VTBeep(beep!=SILENT); /* ring bell (maybe) */
}

alf() /* J linefeed */
{
	APASS /* console device does this */
	if (lf_expand_in) { /* if expand incomming lf to lf/cr */
		c = RETURN;
		APASS
	}
}

acr() /* M carriage return */
{
	APASS /* console device does this */
	if (cr_expand_in) { /* if expand incomming cr to cr/lf */
		c = LINEFEED;
		APASS
	}
}

adle() /* P */
{
	APASS /* pass through */
}

/* array of ptrs to ctrl-char functions */
int (*amiga_functions[])() = {
	pass, pass, pass, pass,		/* @ A B C */
	pass, aenq, pass, abell,	/* D E F G */
	pass, pass, alf, pass,		/* H I J K */
	pass, acr, pass, pass,		/* L M N O */
	adle, pass, pass, pass,		/* P Q R S */
	pass, pass, pass, pass,		/* T U V W */
	pass, pass, pass, pass,		/* X Y Z [ */
	pass, pass, pass, pass		/* \ ] ~ ? */
	};
