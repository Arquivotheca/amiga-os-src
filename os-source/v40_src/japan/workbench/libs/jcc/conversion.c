/*
 *  conversion.c - conversion library functions...
 *
 */
#include <exec/types.h>
#include <exec/libraries.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <dos.h>
#include <stdio.h>
#include <string.h>

#include "jcc.h"
#include "jccbase.h"

#define JCCBase		((struct JCCLibrary *)getreg(REG_A6))
#define UtilityBase	(JCCBase->jl_UtilityBase)

/**********    debug macros     **********/
#define MYDEBUG  0
void kprintf(UBYTE *fmt,...);
#define bug kprintf
#if MYDEBUG
#define D(x) (x);
#else
#define D(x) ;
#endif /* MYDEBUG */
/********** end of debug macros **********/

extern VOID __saveds __asm
han2zen(register __a0 LONG *one,
        register __a1 LONG *two,
        register __d0 LONG nigori,
        register __d1 LONG maru);

LONG __saveds __asm
KanjiIn(register __d0 LONG outcode,
        register __a0 UBYTE *outbuf)
{
	LONG wlen;

	switch(outcode)
	{
		case CTYPE_NEWJIS:
			*outbuf++ = ESC;
			*outbuf++ = '$';
			*outbuf   = 'B';
			wlen = 3;
			break;
		case CTYPE_OLDJIS:
			*outbuf++ = ESC;
			*outbuf++ = '$';
			*outbuf   = '@';
			wlen = 3;
			break;
		case CTYPE_NECJIS:
			*outbuf++ = ESC;
			*outbuf   = 'K';
			wlen = 2;
			break;
		default:
			wlen = 0;
			break;
	}
	return wlen;
}

LONG __saveds __asm
KanjiOut(register __d0 LONG outcode,
         register __a0 UBYTE *outbuf)
{
	LONG wlen;
	switch(outcode)
	{
		case CTYPE_NEWJIS:
		case CTYPE_OLDJIS:
			*outbuf++ = ESC;
			*outbuf++ = '(';
			*outbuf   = 'J';
			wlen = 3;
			break;
		case CTYPE_NECJIS:
			*outbuf++ = ESC;
			*outbuf   = 'H';
			wlen = 2;
			break;
		default:
			wlen = 0;
			break;
	}
	return wlen;
}

LONG __saveds __asm
writebuf(register __d0 LONG p1,
         register __d1 LONG p2,
         register __a0 UBYTE *outbuf)
{
	*outbuf++ = p1;
	*outbuf   = p2;

	return (2);
}

int __saveds __asm
isodd(register __d0 int number)
{
	return((number % 2) ? 1 : 0);
}

/* Shift-JIS to JIS */
VOID __saveds __asm
LIBEightToSeven(register __a0 LONG *p1, register __a1 LONG *p2)
{
	LONG temp;

	temp = *p2;
	if ((*p2 >= 0x40) && (*p2 <= 0x9e))
		*p2 -= 0x1f;
	else if ((*p2 >= 0x9f) && (*p2 <= 0xfc))
		*p2 -= 0x7e;
	if ((temp > 0x7f) && (temp <= 0x9e))
		(*p2)--;
	if ((*p1 >= 0x81) && (*p1 <= 0x9f))
	{
		if ((temp >= 0x40) && (temp <= 0x9e))
			*p1 = ((*p1 - 0x71) * 2) + 1;
		else if ((temp >= 0x9f) && (temp <= 0xfc))
			*p1 = (*p1 - 0x70) * 2;
	}
	else if ((*p1 >= 0xe0) && (*p1 <= 0xef))
	{
		if ((temp >= 0x40) && (temp <= 0x9e))
			*p1 = ((*p1 - 0xb1) * 2) + 1;
		else if ((temp >= 0x9f) && (temp <= 0xfc))
			*p1 = (*p1 - 0xb0) * 2;
	}
}

/* JIS to Shift-JIS */
VOID __saveds __asm
LIBSevenToEight(register __a0 LONG *p1, register __a1 LONG *p2)
{
	if (isodd(*p1))
		*p2 += 31;
	else
		*p2 += 126;
	if ((*p2 >= 127) && (*p2 < 158))
		(*p2)++;
	if ((*p1 >= 33) && (*p1 <= 94))
	{
		if (isodd(*p1))
			*p1 = ((*p1 - 1) / 2) + 113;
		else if (!isodd(*p1))
			*p1 = (*p1 / 2) + 112;
	}
	else if ((*p1 >= 95) && (*p1 <= 126))
	{
		if (isodd(*p1))
			*p1 = ((*p1 - 1) / 2) + 177;
		else if (!isodd(*p1))
			*p1 = (*p1 / 2) + 176;
	}
}

VOID __saveds __asm
eight2seven(register __a0 LONG *p1, register __a1 LONG *p2)
{
	/*
	 * PC98 "GAIJI" kluge. Mostly for "KEISEN" (line).
	 * PC98 is using strange code for "KEISEN", which 1st byte is "0x86".
	 * When we detect this code, just replace 2byte JIS code of "SPACE".
	 */
#if 0
	if (*p1 > 0x84 && *p1 < 0x88)
	{
		*p1 = 0x21;
		*p2 = 0x21;
		return;
	}
#endif

	LIBEightToSeven(p1, p2);
}

VOID __saveds __asm
seven2eight(register __a0 LONG *p1, register __a1 LONG *p2)
{
	/*
	 * PC98 "GAIJI" kluge. Mostly for "KEISEN" (line).
	 * PC98 is using strange code for "KEISEN", which 1st byte is "0x86".
	 * So that when we detect JIS standard "KEISEN" code,
	 * just replace 2byte S-JIS code of "SPACE" to avoid showing
	 * incorrect display in the other machine.
	 */
#if 0
	if (*p1 > 0x27 && *p1 < 0x30)
	{
		*p1 = 0x81;
		*p2 = 0x40;
		return;
	}
#endif

	LIBSevenToEight(p1, p2);
}

VOID __saveds __asm
pop(register __a0 LONG *p, register __a1 UBYTE *jcc_b)
{
	*p = *jcc_b;
	*jcc_b = 0x00;
}

/* JIS to JIS */
LONG __saveds __asm
seven2seven(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
    	    register __d0 LONG length,
            register __d1 ULONG skipescseq)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("seven2seven:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_JIS_JIS;
		}
		if (jcc->jcc_ESC[0] == ESC)
		{
			pop(&p1, &(jcc->jcc_ESC[0]));
			if (jcc->jcc_ESC[1] == '$'
			 || jcc->jcc_ESC[1] == '(')
			{
				pop(&p2, &(jcc->jcc_ESC[1]));
				goto ESC_THIRD_JIS_JIS;
			}
			goto ESC_SECOND_JIS_JIS;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case ESC:
				if (rcnt >= rlen)
				{
					if (length == -1)	/* continuous string */
					{
						jcc->jcc_ESC[0] = ESC;
						break;
					}
					else			/* single string convert */
					{
						if (jcc->jcc_ShiftedIn)
						{
							jcc->jcc_ShiftedIn = FALSE;
							if (!skipescseq)
								wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
						}
						break;
					}
				}
ESC_SECOND_JIS_JIS:
				p2 = inbuf[rcnt++];
				if      (p2 == 'K')
					jcc->jcc_ShiftedIn = TRUE;
				else if (p2 == 'H')
					jcc->jcc_ShiftedIn = FALSE;
				else if ((p2 == '$') || (p2 == '('))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_ESC[0] = p1;
							jcc->jcc_ESC[1] = p2;
							break;
						}
						else			/* single string convert */
						{
							if (jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = FALSE;
								if (!skipescseq)
									wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							break;
						}
					}
ESC_THIRD_JIS_JIS:
					rcnt++;		/* junk */
					if (p2 == '$')
						jcc->jcc_ShiftedIn = TRUE;
					if (p2 == '(')
						jcc->jcc_ShiftedIn = FALSE;
				}
				if (jcc->jcc_ShiftedIn)
				{
					if (!skipescseq)
						wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
				}
				else
				{
					if (!skipescseq)
						wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
				}
				break;

			case LF:
				if (jcc->jcc_ShiftedIn)
				{
					jcc->jcc_ShiftedIn = FALSE;
					if (!skipescseq)
						wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
				}
				outbuf[wcnt++] = p1;
				break;
			case FF:
				break;
			default:
				if (jcc->jcc_ShiftedIn)
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
						{
							if (jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = FALSE;
								if (!skipescseq)
									wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							break;
						}
					}
SECONDBYTE_JIS_JIS:
					p2 = inbuf[rcnt++];
					wcnt += writebuf(p1, p2, &outbuf[wcnt]);
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* JIS to EUC */
LONG __saveds __asm
seven2euc(register __a0 struct JConversionCodeSet *jcc,
          register __a1 UBYTE *inbuf,
          register __a2 UBYTE *outbuf,
          register __d0 LONG length)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("seven2euc:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_JIS_EUC;
		}
		if (jcc->jcc_ESC[0] == ESC)
		{
			if (jcc->jcc_ESC[1] == '$'
			 || jcc->jcc_ESC[1] == '(')
			{
				pop(&p1, &(jcc->jcc_ESC[0]));
				pop(&p2, &(jcc->jcc_ESC[1]));
				goto ESC_THIRD_JIS_EUC;
			}
			pop(&p1, &(jcc->jcc_ESC[0]));
			goto ESC_SECOND_JIS_EUC;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case ESC:
				if (rcnt >= rlen)
				{
					if (length == -1)	/* continuous string */
					{
						jcc->jcc_ESC[0] = ESC;
						break;
					}
					else			/* single string convert */
						break;
				}
ESC_SECOND_JIS_EUC:
				p2 = inbuf[rcnt++];
				if      (p2 == 'K')
					jcc->jcc_ShiftedIn = TRUE;
				else if (p2 == 'H')
					jcc->jcc_ShiftedIn = FALSE;
				else if ((p2 == '$') || (p2 == '('))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_ESC[0] = p1;
							jcc->jcc_ESC[1] = p2;
							break;
						}
						else			/* single string convert */
							break;
					}
ESC_THIRD_JIS_EUC:
					rcnt++;		/* junk */
					if (p2 == '$')
						jcc->jcc_ShiftedIn = TRUE;
					if (p2 == '(')
						jcc->jcc_ShiftedIn = FALSE;
				}
				break;
			case LF:
				if (jcc->jcc_ShiftedIn)
				{
					jcc->jcc_ShiftedIn = FALSE;
				}
				outbuf[wcnt++] = p1;
				break;
			case FF:
				break;
			default:
				if (jcc->jcc_ShiftedIn)
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
SECONDBYTE_JIS_EUC:
					p2 = inbuf[rcnt++];
					wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* JIS to Shift-JIS */
LONG __saveds __asm
seven2shift(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
            register __d0 LONG length)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("seven2shift:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_JIS_SJS;
		}
		if (jcc->jcc_ESC[0] == ESC)
		{
			if (jcc->jcc_ESC[1] == '$'
			 || jcc->jcc_ESC[1] == '(')
			{
				pop(&p1, &(jcc->jcc_ESC[0]));
				pop(&p2, &(jcc->jcc_ESC[1]));
				goto ESC_THIRD_JIS_SJS;
			}
			pop(&p1, &(jcc->jcc_ESC[0]));
			goto ESC_SECOND_JIS_SJS;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case ESC:
				if (rcnt >= rlen)
				{
					if (length == -1)	/* continuous string */
					{
						jcc->jcc_ESC[0] = ESC;
						break;
					}
					else			/* single string convert */
						break;
				}
ESC_SECOND_JIS_SJS:
				p2 = inbuf[rcnt++];
				if      (p2 == 'K')
					jcc->jcc_ShiftedIn = TRUE;
				else if (p2 == 'H')
					jcc->jcc_ShiftedIn = FALSE;
				else if ((p2 == '$') || (p2 == '('))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_ESC[0] = p1;
							jcc->jcc_ESC[1] = p2;
							break;
						}
						else			/* single string convert */
							break;
					}
ESC_THIRD_JIS_SJS:
					rcnt++;		/* junk */
					if (p2 == '$')
						jcc->jcc_ShiftedIn = TRUE;
					if (p2 == '(')
						jcc->jcc_ShiftedIn = FALSE;
				}
				break;
			case FF:
				break;
			default:
				if (jcc->jcc_ShiftedIn)
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
SECONDBYTE_JIS_SJS:
					p2 = inbuf[rcnt++];
					seven2eight(&p1, &p2);
					wcnt += writebuf(p1, p2, &outbuf[wcnt]);
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* EUC to JIS */
LONG __saveds __asm
euc2seven(register __a0 struct JConversionCodeSet *jcc,
          register __a1 UBYTE *inbuf,
          register __a2 UBYTE *outbuf,
          register __d0 LONG length,
          register __d1 ULONG skipescseq)
{
	LONG p1, p2, p3, p4;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;
	LONG nigori = FALSE, maru = FALSE;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("euc2seven:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_EUC_JIS;
		}
		if (jcc->jcc_HanKata[0])
		{
			pop(&p1, &(jcc->jcc_HanKata[0]));
			if (jcc->jcc_HanKata[1])
			{
				pop(&p2, &(jcc->jcc_HanKata[1]));
				if (jcc->jcc_HanKata[2])
				{
					pop(&p3, &(jcc->jcc_HanKata[2]));
					goto HANKATA_FORTHBYTE_EUC_JIS;
				}
				goto HANKATA_THIRDBYTE_EUC_JIS;
			}
			goto HANKATA_SECONDBYTE_EUC_JIS;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case LF:
				if (jcc->jcc_ShiftedIn)
				{
					jcc->jcc_ShiftedIn = FALSE;
					if (!skipescseq)
						wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
				}
				outbuf[wcnt++] = p1;
				break;
			case FF:
				break;
			default:
				if (IS_EUC_BYTE(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
						{
							if (jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = FALSE;
								if (!skipescseq)
									wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							break;
						}
					}
SECONDBYTE_EUC_JIS:
					p2 = inbuf[rcnt++];
					if (IS_EUC_BYTE(p2))
					{
						p1 -= 0x80;
						p2 -= 0x80;
						if (!jcc->jcc_ShiftedIn)
						{
							jcc->jcc_ShiftedIn = TRUE;
							if (!skipescseq)
								wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
						}
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
				}
				else if (p1 == 0x8e)	/* EUC half size 1st byte */
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_HanKata[0] = p1;
							break;
						}
						else			/* single string convert */
						{
							if (jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = FALSE;
								if (!skipescseq)
									wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							break;
						}
					}
HANKATA_SECONDBYTE_EUC_JIS:
					p2 = inbuf[rcnt++];
					if (IS_HANKATA(p2))
					{
						if (rcnt >= rlen)
						{
							if (length == -1)	/* continuous string */
							{
								jcc->jcc_HanKata[0] = p1;
								jcc->jcc_HanKata[1] = p2;
								break;
							}
							else			/* single string convert */
							{
								p1 = p2;
								han2zen(&p1, &p2, 0, 0);
								eight2seven(&p1, &p2);
								if (!jcc->jcc_ShiftedIn)
								{
									jcc->jcc_ShiftedIn = TRUE;
									if (!skipescseq)
										wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
								}
								wcnt += writebuf(p1, p2, &outbuf[wcnt]);
								if (jcc->jcc_ShiftedIn)
								{
									jcc->jcc_ShiftedIn = FALSE;
									if (!skipescseq)
										wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
								}
								break;
							}
						}
HANKATA_THIRDBYTE_EUC_JIS:
						p3 = inbuf[rcnt++];
						if (p3 == 0x8e)	/* EUC half size 1st byte */
						{
							if (rcnt >= rlen)
							{
								if (length == -1)	/* continuous string */
								{
									jcc->jcc_HanKata[0] = p1;
									jcc->jcc_HanKata[1] = p2;
									jcc->jcc_HanKata[2] = p3;
									break;
								}
								else
								{
									p1 = p2;
									han2zen(&p1, &p2, 0, 0);
									eight2seven(&p1, &p2);
									if (!jcc->jcc_ShiftedIn)
									{
										jcc->jcc_ShiftedIn = TRUE;
										if (!skipescseq)
											wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
									}
									wcnt += writebuf(p1, p2, &outbuf[wcnt]);
									if (jcc->jcc_ShiftedIn)
									{
										jcc->jcc_ShiftedIn = FALSE;
										if (!skipescseq)
											wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
									}
									break;
								}
							}
HANKATA_FORTHBYTE_EUC_JIS:
							nigori = maru = FALSE;
							p4 = inbuf[rcnt++];

							p1 = p2;
							if (IS_NIGORI(p1, p4))	/* Next is dakuten */
								nigori = TRUE;
							else if (IS_MARU(p1, p4))	/* Next is maru */
								maru = TRUE;
							else	/* just HANKATA */
							{
								han2zen(&p1, &p2, nigori, maru);
								eight2seven(&p1, &p2);
								if (!jcc->jcc_ShiftedIn)
								{
									jcc->jcc_ShiftedIn = TRUE;
									if (!skipescseq)
										wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
								}
								wcnt += writebuf(p1, p2, &outbuf[wcnt]);

								rcnt--;
								p1 = p3;
								goto HANKATA_SECONDBYTE_EUC_JIS;
							}
						}
						else
							rcnt--;

						p1 = p2;
						han2zen(&p1, &p2, nigori, maru);
						eight2seven(&p1, &p2);
						if (!jcc->jcc_ShiftedIn)
						{
							jcc->jcc_ShiftedIn = TRUE;
							if (!skipescseq)
								wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
						}
					}
					wcnt += writebuf(p1, p2, &outbuf[wcnt]);
				}
				else
				{
					if (jcc->jcc_ShiftedIn)
					{
						jcc->jcc_ShiftedIn = FALSE;
						if (!skipescseq)
							wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
					}
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* EUC to EUC */
LONG __saveds __asm
euc2euc(register __a0 struct JConversionCodeSet *jcc,
        register __a1 UBYTE *inbuf,
        register __a2 UBYTE *outbuf,
        register __d0 LONG length,
        register __d1 ULONG hankatatype,
        register __d2 UBYTE udhankata,
        register __d3 ULONG textfilter)
{
	LONG p1, p2, p3, p4;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;
	LONG nigori = FALSE, maru = FALSE;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("euc2euc:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_EUC_EUC;
		}
		if (jcc->jcc_HanKata[0])
		{
			pop(&p1, &(jcc->jcc_HanKata[0]));
			if (jcc->jcc_HanKata[1])
			{
				pop(&p2, &(jcc->jcc_HanKata[1]));
				if (jcc->jcc_HanKata[2])
				{
					pop(&p3, &(jcc->jcc_HanKata[2]));
					goto HANKATA_FORTHBYTE_EUC_EUC;
				}
				goto HANKATA_THIRDBYTE_EUC_EUC;
			}
			goto HANKATA_SECONDBYTE_EUC_EUC;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case CR:
				if (textfilter)	/* remove 0x0d */
					break;
				else
					outbuf[wcnt++] = CR;
				break;
			case LF:
				outbuf[wcnt++] = LF;
				break;
			case FF:
				break;
			default:
				if (IS_EUC_BYTE(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
SECONDBYTE_EUC_EUC:
					p2 = inbuf[rcnt++];
					if (IS_EUC_BYTE(p2))
					{
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
				}
				else if (p1 == 0x8e)	/* EUC half size 1st byte */
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_HanKata[0] = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
HANKATA_SECONDBYTE_EUC_EUC:
					p2 = inbuf[rcnt++];
					if (IS_HANKATA(p2))
					{
						if (hankatatype == JCT_EUCHanKata)
						{
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
							continue;
						}
						else if (hankatatype == JCT_UDHanKata)
						{
							outbuf[wcnt++] = udhankata;
							continue;
						}
						else
						{
							if (rcnt >= rlen)
							{
								if (length == -1)	/* continuous string */
								{
									jcc->jcc_HanKata[0] = p1;
									jcc->jcc_HanKata[1] = p2;
									break;
								}
								else			/* single string convert */
								{
									p1 = p2;
									han2zen(&p1, &p2, 0, 0);
									eight2seven(&p1, &p2);
									wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
									break;
								}
							}
						}
HANKATA_THIRDBYTE_EUC_EUC:
						p3 = inbuf[rcnt++];
						if (p3 == 0x8e)	/* EUC half size 1st byte */
						{
							if (rcnt >= rlen)
							{
								if (length == -1)	/* continuous string */
								{
									jcc->jcc_HanKata[0] = p1;
									jcc->jcc_HanKata[1] = p2;
									jcc->jcc_HanKata[2] = p3;
									break;
								}
								else
								{
									p1 = p2;
									han2zen(&p1, &p2, 0, 0);
									eight2seven(&p1, &p2);
									wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
									break;
								}
							}
HANKATA_FORTHBYTE_EUC_EUC:
							nigori = maru = FALSE;
							p4 = inbuf[rcnt++];

							p1 = p2;
							if (IS_NIGORI(p1, p4))	/* Next is dakuten */
								nigori = TRUE;
							else if (IS_MARU(p1, p4))	/* Next is maru */
								maru = TRUE;
							else	/* just HANKATA */
							{
								han2zen(&p1, &p2, nigori, maru);
								eight2seven(&p1, &p2);
								wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);

								rcnt--;
								p1 = p3;
								goto HANKATA_SECONDBYTE_EUC_EUC;
							}
						}
						else
							rcnt--;

						p1 = p2;
						han2zen(&p1, &p2, nigori, maru);
						eight2seven(&p1, &p2);
						p1 += 0x80;
						p2 += 0x80;
					}
					wcnt += writebuf(p1, p2, &outbuf[wcnt]);
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* EUC to Shift-JIS */
LONG __saveds __asm
euc2shift(register __a0 struct JConversionCodeSet *jcc,
          register __a1 UBYTE *inbuf,
          register __a2 UBYTE *outbuf,
          register __d0 LONG length,
          register __d1 ULONG hankatatype,
          register __d2 UBYTE udhankata,
          register __d3 LONG textfilter)
{
	LONG p1, p2, p3, p4;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;
	LONG nigori = FALSE, maru = FALSE;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("euc2shift:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_EUC_SJIS;
		}
		if (jcc->jcc_HanKata[0])
		{
			pop(&p1, &(jcc->jcc_HanKata[0]));
			if (jcc->jcc_HanKata[1])
			{
				pop(&p2, &(jcc->jcc_HanKata[1]));
				if (jcc->jcc_HanKata[2])
				{
					pop(&p3, &(jcc->jcc_HanKata[2]));
					goto HANKATA_FORTHBYTE_EUC_SJIS;
				}
				goto HANKATA_THIRDBYTE_EUC_SJIS;
			}
			goto HANKATA_SECONDBYTE_EUC_SJIS;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case LF:
				if (textfilter)	/* add 0x0d */
					outbuf[wcnt++] = CR;
				outbuf[wcnt++] = LF;
				break;
			case FF:
				break;
			default:
				if (IS_EUC_BYTE(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
SECONDBYTE_EUC_SJIS:
					p2 = inbuf[rcnt++];
					if (IS_EUC_BYTE(p2))
					{
						p1 -= 0x80;
						p2 -= 0x80;
						seven2eight(&p1, &p2);
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
					else	/* for DOS/V irlegal '0xfa56' code, I have to put original data. */
					{
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
				}
				else if (p1 == 0x8e)	/* EUC half size 1st byte */
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_HanKata[0] = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
HANKATA_SECONDBYTE_EUC_SJIS:
					p2 = inbuf[rcnt++];
					if (IS_HANKATA(p2))
					{
						if (hankatatype == JCT_SJISHanKata)
						{
							outbuf[wcnt++] = p2;
							continue;
						}
						else if (hankatatype == JCT_UDHanKata)
						{
							outbuf[wcnt++] = udhankata;
							continue;
						}
						else
						{
							if (rcnt >= rlen)
							{
								if (length == -1)	/* continuous string */
								{
									jcc->jcc_HanKata[0] = p1;
									jcc->jcc_HanKata[1] = p2;
									break;
								}
								else			/* single string convert */
								{
									p1 = p2;
									han2zen(&p1, &p2, 0, 0);
									wcnt += writebuf(p1, p2, &outbuf[wcnt]);
									break;
								}
							}
						}
HANKATA_THIRDBYTE_EUC_SJIS:
						p3 = inbuf[rcnt++];
						if (p3 == 0x8e)	/* EUC half size 1st byte */
						{
							if (rcnt >= rlen)
							{
								if (length == -1)	/* continuous string */
								{
									jcc->jcc_HanKata[0] = p1;
									jcc->jcc_HanKata[1] = p2;
									jcc->jcc_HanKata[2] = p3;
									break;
								}
								else
								{
									p1 = p2;
									han2zen(&p1, &p2, 0, 0);
									wcnt += writebuf(p1, p2, &outbuf[wcnt]);
									break;
								}
							}
HANKATA_FORTHBYTE_EUC_SJIS:
							nigori = maru = FALSE;
							p4 = inbuf[rcnt++];

							p1 = p2;
							if (IS_NIGORI(p1, p4))	/* Next is dakuten */
								nigori = TRUE;
							else if (IS_MARU(p1, p4))	/* Next is maru */
								maru = TRUE;
							else	/* just HANKATA */
							{
								han2zen(&p1, &p2, 0, 0);
								wcnt += writebuf(p1, p2, &outbuf[wcnt]);

								rcnt--;
								p1 = p3;
								goto HANKATA_SECONDBYTE_EUC_SJIS;
							}
						}
						else
							rcnt--;

						p1 = p2;
						han2zen(&p1, &p2, nigori, maru);
					}
					wcnt += writebuf(p1, p2, &outbuf[wcnt]);
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* Shift-JIS to JIS */
LONG __saveds __asm
shift2seven(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
            register __d0 LONG length,
            register __d1 ULONG skipescseq)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;
	LONG nigori = FALSE, maru = FALSE;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("shift2seven:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_SJIS_JIS;
		}
		if (jcc->jcc_HanKata[0])
		{
			pop(&p1, &(jcc->jcc_HanKata[0]));
			goto HANKATA_SECONDBYTE_SJIS_JIS;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case CR:
			case LF:
				if (jcc->jcc_ShiftedIn)
				{
					jcc->jcc_ShiftedIn = FALSE;
					if (!skipescseq)
						wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
				}
				outbuf[wcnt++] = LF;
				break;
			case NULL:
			case FF:
				break;
			default:
				if (IS_SJIS_BYTE1(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
						{
							if (jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = FALSE;
								if (!skipescseq)
									wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							break;
						}
					}
SECONDBYTE_SJIS_JIS:
					p2 = inbuf[rcnt++];
					if (IS_SJIS_BYTE2(p2))
					{
						eight2seven(&p1, &p2);
						if (!jcc->jcc_ShiftedIn)
						{
							jcc->jcc_ShiftedIn = TRUE;
							if (!skipescseq)
								wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
						}
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
				}
				else if (IS_HANKATA(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_HanKata[0] = p1;
							break;
						}
						else			/* single string convert */
						{
							han2zen(&p1, &p2, 0, 0);
							eight2seven(&p1, &p2);
							if (!jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = TRUE;
								if (!skipescseq)
									wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
							if (jcc->jcc_ShiftedIn)
							{
								jcc->jcc_ShiftedIn = FALSE;
								if (!skipescseq)
									wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
							}
							break;
						}
					}
					else
					{
HANKATA_SECONDBYTE_SJIS_JIS:
						nigori = maru = FALSE;
						p2 = inbuf[rcnt++];

						if (IS_NIGORI(p1, p2))	/* Next is dakuten */
							nigori = TRUE;
						else if (IS_MARU(p1, p2))	/* Next is maru */
							maru = TRUE;
						else
							rcnt--;

						han2zen(&p1, &p2, nigori, maru);
					}
					eight2seven(&p1, &p2);
					if (!jcc->jcc_ShiftedIn)
					{
						jcc->jcc_ShiftedIn = TRUE;
						if (!skipescseq)
							wcnt += KanjiIn(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
					}
					wcnt += writebuf(p1, p2, &outbuf[wcnt]);
				}
				else
				{
					if (jcc->jcc_ShiftedIn)
					{
						jcc->jcc_ShiftedIn = FALSE;
						if (!skipescseq)
							wcnt += KanjiOut(jcc->jcc_WhatOutputCode, &outbuf[wcnt]);
					}
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* Shift-JIS to EUC */
LONG __saveds __asm
shift2euc(register __a0 struct JConversionCodeSet *jcc,
          register __a1 UBYTE *inbuf,
          register __a2 UBYTE *outbuf,
          register __d0 LONG length,
          register __d1 ULONG hankatatype,
          register __d2 UBYTE udhankata,
          register __d3 ULONG textfilter)

{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;
	LONG nigori = FALSE, maru = FALSE;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("shift2euc:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_SJIS_EUC;
		}
		if (jcc->jcc_HanKata[0])
		{
			pop(&p1, &(jcc->jcc_HanKata[0]));
			goto HANKATA_SECONDBYTE_SJIS_EUC;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case CR:
				if (textfilter)	/* remove 0x0d */
					break;
				else
					outbuf[wcnt++] = CR;
				break;
			case LF:
				outbuf[wcnt++] = LF;
				break;
			case NULL:
			case FF:
			case PCEOF:
				break;
			default:
				if (IS_SJIS_BYTE1(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
SECONDBYTE_SJIS_EUC:
					p2 = inbuf[rcnt++];
					if (IS_SJIS_BYTE2(p2))
					{
						eight2seven(&p1, &p2);
						wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
					}
					else	/* for DOS/V irlegal '0xfa56' code, I have to put original data. */
					{
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
				}
				else if (IS_HANKATA(p1))
				{
					if (rcnt >= rlen)
					{
						if (hankatatype == JCT_EUCHanKata)
							wcnt += writebuf(0x8e, p1, &outbuf[wcnt]);
						else if (hankatatype == JCT_UDHanKata)
							outbuf[wcnt++] = udhankata;
						else
						{
							if (length == -1)	/* continuous string */
							{
								jcc->jcc_HanKata[0] = p1;
								break;
							}
							else			/* single string convert */
							{
								han2zen(&p1, &p2, 0, 0);
								eight2seven(&p1, &p2);
								wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
							}
						}
					}
					else
					{
HANKATA_SECONDBYTE_SJIS_EUC:
						if (hankatatype == JCT_EUCHanKata)
							wcnt += writebuf(0x8e, p1, &outbuf[wcnt]);
						else if (hankatatype == JCT_UDHanKata)
							outbuf[wcnt++] = udhankata;
						else
						{
							nigori = maru = FALSE;
							p2 = inbuf[rcnt++];

							if (IS_NIGORI(p1, p2))	/* Next is dakuten */
								nigori = TRUE;
							else if (IS_MARU(p1, p2))	/* Next is maru */
								maru = TRUE;
							else
								rcnt--;

							han2zen(&p1, &p2, nigori, maru);
							eight2seven(&p1, &p2);
							wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
						}
					}
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

/* Shift-JIS to Shift-JIS */
LONG __saveds __asm
shift2shift(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
            register __d0 LONG length,
            register __d1 ULONG hankatatype,
            register __d2 UBYTE udhankata,
            register __d3 ULONG textfilter)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen;
	LONG nigori = FALSE, maru = FALSE;

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;

	D(bug("shift2shift:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_SJIS_SJIS;
		}
		if (jcc->jcc_HanKata[0])
		{
			pop(&p1, &(jcc->jcc_HanKata[0]));
			goto HANKATA_SECONDBYTE_SJIS_SJIS;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case LF:
				if (textfilter)	/* add 0x0d */
					outbuf[wcnt++] = CR;
				outbuf[wcnt++] = LF;
				break;
			case NULL:
			case FF:
			case PCEOF:
				break;
			default:
				if (IS_SJIS_BYTE1(p1))
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* single string convert */
							break;
					}
SECONDBYTE_SJIS_SJIS:
					p2 = inbuf[rcnt++];
					if (IS_SJIS_BYTE2(p2))
					{
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
					}
				}
				else if (IS_HANKATA(p1))
				{
					if (rcnt >= rlen)
					{
						if (hankatatype == JCT_SJISHanKata)
							outbuf[wcnt++] = p1;
						else if (hankatatype == JCT_UDHanKata)
							outbuf[wcnt++] = udhankata;
						else
						{
							if (length == -1)	/* continuous string */
							{
								jcc->jcc_HanKata[0] = p1;
								break;
							}
							else			/* single string convert */
							{
								han2zen(&p1, &p2, 0, 0);
								wcnt += writebuf(p1, p2, &outbuf[wcnt]);
							}
						}
					}
					else
					{
HANKATA_SECONDBYTE_SJIS_SJIS:
						if (hankatatype == JCT_SJISHanKata)
							outbuf[wcnt++] = p1;
						else if (hankatatype == JCT_UDHanKata)
							outbuf[wcnt++] = udhankata;
						else
						{
							nigori = maru = FALSE;
							p2 = inbuf[rcnt++];

							if (IS_NIGORI(p1, p2))	/* Next is dakuten */
								nigori = TRUE;
							else if (IS_MARU(p1, p2))	/* Next is maru */
								maru = TRUE;
							else
								rcnt--;

							han2zen(&p1, &p2, nigori, maru);
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
						}
					}
				}
				else
				{
					outbuf[wcnt++] = p1;
				}
				break;
		}
	}
	return wcnt;
}

LONG __saveds __asm
LIBJStringConvertA(register __a0 struct JConversionCodeSet *jcc,
                   register __a1 UBYTE *inbuf,
                   register __a2 UBYTE *outbuf,
                   register __d0 LONG incode,
                   register __d1 LONG outcode,
                   register __d2 LONG length,
                   register __a3 struct TagItem *taglist)
{
	LONG wcnt = -1;
	LONG hankatatype = 0;

	BOOL euchankata = GetTagData(JCT_EUCHanKata, FALSE ,taglist);
	BOOL sjishankata = GetTagData(JCT_SJISHanKata, FALSE ,taglist);
	UBYTE udhankata = GetTagData(JCT_UDHanKata, 0x00 ,taglist);
	BOOL textfilter = GetTagData(JCT_TextFilter, FALSE ,taglist);
	BOOL skipescseq = GetTagData(JCT_SkipESCSeq, FALSE ,taglist);

	if (udhankata)
	{
		hankatatype = JCT_UDHanKata;
	}
	else
	{
		if (outcode == CTYPE_EUC)
			if (euchankata)
				hankatatype = JCT_EUCHanKata;
		if (outcode == CTYPE_SJIS)
			if (sjishankata)
				hankatatype = JCT_SJISHanKata;
	}

	jcc->jcc_WhatInputCode = incode;
	jcc->jcc_WhatOutputCode = outcode;

	switch(incode)
	{
		case CTYPE_NEWJIS:
		case CTYPE_OLDJIS:
		case CTYPE_NECJIS:
			switch(outcode)
			{
				case CTYPE_NEWJIS:
				case CTYPE_OLDJIS:
				case CTYPE_NECJIS:
					wcnt = seven2seven(jcc, inbuf, outbuf, length, skipescseq);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_EUC:
					wcnt = seven2euc(jcc, inbuf, outbuf, length);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_SJIS:
					wcnt = seven2shift(jcc, inbuf, outbuf, length);
					outbuf[wcnt] = 0x00;
					break;
				deault:
					break;
			}
			break;
		case CTYPE_EUC:
			switch(outcode)
			{
				case CTYPE_NEWJIS:
				case CTYPE_OLDJIS:
				case CTYPE_NECJIS:
					wcnt = euc2seven(jcc, inbuf, outbuf, length, skipescseq);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_EUC:
					wcnt = euc2euc(jcc, inbuf, outbuf, length, hankatatype, udhankata, textfilter);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_SJIS:
					wcnt = euc2shift(jcc, inbuf, outbuf, length, hankatatype, udhankata, textfilter);
					outbuf[wcnt] = 0x00;
					break;
				default:
					break;
			}
			break;
		case CTYPE_SJIS:
			switch(outcode)
			{
				case CTYPE_NEWJIS:
				case CTYPE_OLDJIS:
				case CTYPE_NECJIS:
					wcnt = shift2seven(jcc, inbuf, outbuf, length, skipescseq);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_EUC:
					wcnt = shift2euc(jcc, inbuf, outbuf, length, hankatatype, udhankata, textfilter);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_SJIS:
					wcnt = shift2shift(jcc, inbuf, outbuf, length, hankatatype, udhankata, textfilter);
					outbuf[wcnt] = 0x00;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return wcnt;
}
