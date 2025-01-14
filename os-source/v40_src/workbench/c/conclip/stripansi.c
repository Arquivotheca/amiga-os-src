

#include <exec/types.h>


/*****************************************************************************/


#define STATE_VANILLA	0
#define STATE_ESC	1
#define STATE_ESCI	2
#define STATE_CSI	3
#define STATE_STRING	4


VOID StripANSI(STRPTR ansiBuffer, LONG length)
{
char *source;
char  c, state;
LONG  i;

    source = ansiBuffer;
    state  = STATE_VANILLA;

    for (i = 0; i < length; i++)
    {
	c = *source++;
	switch (state)
	{
            case STATE_VANILLA: if (c == 0x1b)
                                {
                                    state = STATE_ESC;
                                }
                                else if (c == 0x9b)
                                {
                                    state = STATE_CSI;
                                }
                                else if ((c == 0x90) || ((c >= 0x9d) && (c <= 0x9f)))
                                {
                                    state = STATE_STRING;
                                }
                                else if (c)
                                {
                                    *ansiBuffer++ = c;
                                }
                                break;

            case STATE_ESC    : if (c == 0x5b)
                                {
                                    state = STATE_CSI;
                                }
                                else if ((c == 0x50) || ((c >= 0x5d) && (c <= 0x5f)))
                                {
                                    state = STATE_STRING;
                                }
                                else if ((c >= '0') && (c <= '~'))
                                {
                                    state = STATE_VANILLA;
                                }
                                else
                                {
                                    state = STATE_ESCI;
                                }
                                break;

            case STATE_ESCI   : if ((c >= '0') && (c <= '~'))
                                {
                                    state = STATE_VANILLA;
                                }
                                break;

            case STATE_CSI    : if ((c >= '@') && (c <= '~'))
                                {
                                    state = STATE_VANILLA;
                                }
                                break;

            case STATE_STRING : if (c == 0x9c)
                                {
                                    state = STATE_VANILLA;
                                }
                                break;
	}
    }
    *ansiBuffer = '\0';
}
