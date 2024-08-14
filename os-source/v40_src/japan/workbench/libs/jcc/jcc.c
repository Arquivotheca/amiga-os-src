/*
 *  jcc.c - These are the actual library functions...
 *
 */
#include <exec/types.h>
#include <exec/libraries.h>
#include <exec/memory.h>

#include <clib/exec_protos.h>
#include <clib/utility_protos.h>

#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>

#include <dos.h>
#include <string.h>

#include "jcc.h"
#include "jccbase.h"

#define JCCBase		((struct JCCLibrary *)getreg(REG_A6))
#define SysBase		(JCCBase->jl_SysBase)
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

/*
******* jcc.library/AllocConversionHandleA ***********************************
*
*   NAME
*	AllocConversionHandleA -- allocate JConversionCodeSet structure
*	AllocConversionHandle -- varargs stub for AllocConversionHandleA().
*
*   SYNOPSYS
*	jcc = AllocConversionHandleA(taglist);
*	D0                           A0
*
*	struct JConversionCodeSet *AllocConversionHandleA(struct TagItem *);
*
*	jcc = AllocConversionHandle(firstTag, ...);
*
*	struct JConversionCodeSet *AllocConversionHandle(Tag, ...);
*
*   FUNCTION
*	Allocate JConversionCodeSet structure.
*
*   INPUTS
*	taglist - pointer to TagItem list.
*
*   TAGS
*	JCC_WhatInputCode (LONG) - input code type
*	JCC_WhatOutputCode (LONG) - output code type
*	JCC_FirstByte (BYTE) - first byte of double-byte
*	JCC_ESC (LONG) - escape sequece for JIS (New-JIS, Old-JIS and
*	                 NEC-JIS) code
*	JCC_HanKata (LONG) - single-byte KATAKANA
*	JCC_ShiftedIn (BOOL) - TRUE: in double-byte JIS mode
*	                       FALSE: in single-byte ASCII mode
*
*   CODESETS
*	CTYPE_EUC - EUC is Japanese code for Amiga and UNIX.
*	CTYPE_SJIS - Shift-JIS is most common Japanese code for personal
*	             computer including MS-DOS machine.
*	CTYPE_NEWJIS - New Japanese Industrial Standard code.
*	CTYPE_OLDJIS - Old Japanese Industrial Standard code.
*	CTYPE_NECJIS - NEC Japanese Industrial Standard code.
*                    
*   RESULT
*	jcc - an allocated JConversionCodeSet data structure, or NULL on
*	      failure.
*
*   SEE ALSO
*	FreeConversionHandle()
*
******************************************************************************
*/
struct JConversionCodeSet * __saveds __asm
LIBAllocConversionHandleA(register __a0 struct TagItem *taglist)
{
	struct JConversionCodeSet *jcc = NULL;
	LONG whatinputcode = GetTagData(JCC_WhatInputCode, -1 ,taglist);
	LONG whatoutputcode = GetTagData(JCC_WhatOutputCode, -1 ,taglist);
	UBYTE firstbyte = GetTagData(JCC_FirstByte, 0xff ,taglist);
	BOOL shiftedin = GetTagData(JCC_ShiftedIn, -1 ,taglist);
	union x
	{
		ULONG esc_l;
		UBYTE esc_c[4];
	} x;
	union y
	{
		ULONG hankata_l;
		UBYTE hankata_c[4];
	} y;
	x.esc_l = GetTagData(JCC_ESC, 0xffffffff ,taglist);
	y.hankata_l = GetTagData(JCC_HanKata, 0xffffffff ,taglist);

	if (jcc = AllocMem(sizeof(struct JConversionCodeSet),
				MEMF_PUBLIC|MEMF_CLEAR))
	{
		if (whatinputcode != -1)
			jcc->jcc_WhatInputCode = whatinputcode;
		if (whatoutputcode != -1)
			jcc->jcc_WhatOutputCode = whatoutputcode;
		if (firstbyte != 0xff)
			jcc->jcc_FirstByte = firstbyte;
		if (x.esc_l != 0xffffffff)
		{
			int i;
			for (i = 0; i < 4; i++)
				jcc->jcc_ESC[i] = x.esc_c[i];
		}
		if (y.hankata_l != 0xffffffff)
		{
			int i;
			for (i = 0; i < 4; i++)
				jcc->jcc_HanKata[i] = y.hankata_c[i];
		}
		if (shiftedin != -1)
			jcc->jcc_ShiftedIn = shiftedin;

#if 0
		D(bug("jcc_whatinput = %ld\n", jcc->jcc_WhatInputCode));
		D(bug("jcc_whatoutput = %ld\n", jcc->jcc_WhatOutputCode));
		D(bug("jcc_firstbyte = 0x%lx\n", jcc->jcc_FirstByte));
		D(bug("jcc_esc = 0x%02x\n", jcc->jcc_ESC[0]));
		D(bug("jcc_hankata = 0x%02x\n", jcc->jcc_HanKata[0]));
		D(bug("jcc_shiftedin = %ld\n", jcc->jcc_ShiftedIn));
#endif
		return(jcc);
	}
	else
	{
		return(NULL);
	}
}

/*
******* jcc.library/FreeConversionHandle *************************************
*
*   NAME
*	FreeConversionHandle -- free JConversionCodeSet structure
*
*   SYNOPSYS
*	FreeConversionHandleA(jcc);
*	                      A0
*
*	VOID FreeConversionHandleA(struct JConversionCodeSet *);
*
*   FUNCTION
*	Free JConversionCodeSet structure that was allocated by
*	AllocConversionHandle().
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*
*   SEE ALSO
*	AllocConversionHandle()
*
******************************************************************************
*/
void __saveds __asm
LIBFreeConversionHandle(register __a0 struct JConversionCodeSet *jcc)
{
	if (jcc) FreeMem(jcc, sizeof(struct JConversionCodeSet));
	jcc = NULL;
}

LONG __saveds __asm
getpt(register __a0 void *v)
{
	return((LONG)v);
}

/*
******* jcc.library/GetJConversionAttrsA *************************************
*
*   NAME
*	GetJConversionAttrsA -- get the attributes of a JConversionCodeSet
*	                        data.
*	GetJConversionAttrs -- varargs stub for GetJConversionAttrsA().
*
*   SYNOPSYS
*	numProcessed = GetJConversionAttrsA(jcc, taglist);
*	D0                                 A0   A1
*
*	LONG GetJConversionAttrsA(struct JConversionCodeSet *,
*	                         struct TagItem *);
*
*	numProcessed = GetJConversionAttrs(jcc, firstTag, ...);
*
*	LONG GetJConversionAttrs(struct JConversionCodeSet *, Tag, ...);
*
*   FUNCTION
*	Retrieve the attributes of the JConversionCodeSet data, according to
*	the attributes chosen in the tag list.  For each entry in the tag
*	list, ti_Tag identifies the attribute, and ti_Data is a pointer to
*	the long variable where you wish the result to be stored.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function returns 0.
*	taglist - pointer to TagItem list.
*
*   TAGS
*	See AllocConversionHandle().
*
*   RESULT
*	numProcessed - the number of attributes successfully filled in.
*
*   SEE ALSO
*	SetJConversionAttrs()
*
******************************************************************************
*/
LONG __saveds __asm
LIBGetJConversionAttrsA(register __a0 struct JConversionCodeSet *jcc,
                        register __a1 struct TagItem *taglist)
{
	LONG numproc = 0;
	struct TagItem *tstate = taglist;
	struct TagItem *tag;

	D(bug("taglist = $%lx\n", taglist));

	if (!jcc) return(0);

	while (tag = NextTagItem(&tstate))
	{
		LONG pt;

		switch (tag->ti_Tag)
		{
		case JCC_WhatInputCode:
			pt = getpt(&jcc->jcc_WhatInputCode);
			*((LONG *)(tag->ti_Data)) = *((LONG *)pt);
			numproc++;
			break;
		case JCC_WhatOutputCode:
			pt = getpt(&jcc->jcc_WhatOutputCode);
			*((LONG *)(tag->ti_Data)) = *((LONG *)pt);
			numproc++;
			break;
		case JCC_FirstByte:
			pt = getpt(&jcc->jcc_FirstByte);
			*((LONG *)(tag->ti_Data)) = *((LONG *)pt);
			numproc++;
			break;
		case JCC_ESC:
			pt = getpt(&jcc->jcc_ESC);
			*((LONG *)(tag->ti_Data)) = *((LONG *)pt);
			numproc++;
			break;
		case JCC_HanKata:
			pt = getpt(&jcc->jcc_HanKata);
			*((LONG *)(tag->ti_Data)) = *((LONG *)pt);
			numproc++;
			break;
		case JCC_ShiftedIn:
			pt = getpt(&jcc->jcc_ShiftedIn);
			*((LONG *)(tag->ti_Data)) = *((LONG *)pt);
			numproc++;
			break;

		}
	}
	return(numproc);
}

/*
******* jcc.library/SetJConversionAttrsA *************************************
*
*   NAME
*	SetJConversionAttrsA -- set the attributes of a JConversionCodeSet
*	                        data.
*	SetJConversionAttrs -- varargs stub for SetJConversionAttrsA().
*
*   SYNOPSYS
*	SetJConversionAttrsA(jcc, taglist);
*	                    A0   A1
*
*	VOID SetJConversionAttrsA(struct JConversionCodeSet *,
*	                         struct TagItem *);
*
*	SetJConversionAttrs(jcc, firstTag, ...);
*
*	VOID SetJConversionAttrs(struct JConversionCodeSet *, Tag, ...);
*
*   FUNCTION
*	Change the attributes of the JapaneseConversionCodeSet data, according
*	to the attributes chosen in the tag list. If an attribute is not
*	provided in the tag list, its value remains unchanged.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function does nothing.
*	taglist - pointer to TagItem list.
*
*   TAGS
*	See AllocConversionHandle().
*
*   SEE ALSO
*	GetJConversionAttrs()
*
******************************************************************************
*/
VOID __saveds __asm
LIBSetJConversionAttrsA(register __a0 struct JConversionCodeSet *jcc,
                        register __a1 struct TagItem *taglist)
{
	LONG whatinputcode = GetTagData(JCC_WhatInputCode, -1 ,taglist);
	LONG whatoutputcode = GetTagData(JCC_WhatOutputCode, -1 ,taglist);
	UBYTE firstbyte = GetTagData(JCC_FirstByte, 0xff ,taglist);
	BOOL shiftedin = GetTagData(JCC_ShiftedIn, -1 ,taglist);
	union x
	{
		ULONG esc_l;
		UBYTE esc_c[4];
	} x;
	union y
	{
		ULONG hankata_l;
		UBYTE hankata_c[4];
	} y;
	x.esc_l = GetTagData(JCC_ESC, 0xffffffff ,taglist);
	y.hankata_l = GetTagData(JCC_HanKata, 0xffffffff ,taglist);

	if (!jcc) return;

	if (whatinputcode != -1)
		jcc->jcc_WhatInputCode = whatinputcode;
	if (whatoutputcode != -1)
		jcc->jcc_WhatOutputCode = whatoutputcode;
	if (firstbyte != 0xff)
		jcc->jcc_FirstByte = firstbyte;
	if (x.esc_l != 0xffffffff)
	{
		int i;
		for (i = 0; i < 4; i++)
			jcc->jcc_ESC[i] = x.esc_c[i];
	}
	if (y.hankata_l != 0xffffffff)
	{
		int i;
		for (i = 0; i < 4; i++)
			jcc->jcc_HanKata[i] = y.hankata_c[i];
	}
	if (shiftedin != -1)
		jcc->jcc_ShiftedIn = shiftedin;
#if 0
	D(bug("jcc_whatinput = %ld\n", jcc->jcc_WhatInputCode));
	D(bug("jcc_whatoutput = %ld\n", jcc->jcc_WhatOutputCode));
	D(bug("jcc_firstbyte = 0x%lx\n", jcc->jcc_FirstByte));
	D(bug("jcc_esc = 0x%02x\n", jcc->jcc_ESC[0]));
	D(bug("jcc_hankata = 0x%02x\n", jcc->jcc_HanKata[0]));
	D(bug("jcc_shiftedin = %ld\n", jcc->jcc_ShiftedIn));
#endif
}

extern VOID __saveds __asm
pop(register __a0 LONG *p,
    register __a1 UBYTE *jcc_b);

/*
******* jcc.library/IdentifyCodeSet ******************************************
*
*   NAME
*	IdentifyCodeSet -- Detect code set
*
*   SYNOPSYS
*	codeset = IdentifyCodeSet(jcc, inbuf, length);
*	D0                        A0   A1     D0
*
*	LONG IdentifyCodeSet(struct JConversionCodeSet *jcc, UBYTE *inbuf,
*	                     ULONG length);
*
*   FUNCTION
*	Determine code set in input buffer string.
*
*	The length parameter specifies how many characters to use for
*	detection, or if the length is specified as -1 then the string
*	are parsed until NULL is encountered and the function supposes
*	the string is continuous.
*
*	- User's responsibility to determine how many bytes enough to feed.
*	- User's responsibility to determine what kinds of string to feed.
*	- Assume never combine Japanese charcater and Europian character
*	  for input string. Especially, it's diffecult to determine Shift-JIS
*	  and ASCII including Europian character.
*	- When you get "CTYPE_BINARY", you don't need to feed more strings.
*	  In the other way, as long as returns the other codes than
*	  "CTYPE_BINARY", you might doubt the answer is wrong.
*	- New-JIS, Old-JIS and NEC-JIS can't determine untill get escape code
*	  in string. If end of string has part of escape code, return
*	  "CTYPE_CONTINUE". When you get "CTYPE_NEW" or "CTYPE_OLD" or
*	  "CTYPE_NEC", if you think string does not have binary data, you don't
*	  need to feed more strings.
*	- Shift-JIS and EUC are needed to check first byte and second byte.
*	  But still can't determine with only few bytes, because 25 % of EUC
*	  code has same code with Shift-JIS like "e0c0". And also all of EUC
*	  single-byte KATAKANA is located in Shift-JIS. This function returns
*	  "CTYPE_EUC" in these cases. While returnig "CTYPE_ASCII"
*	  and "CTYPE_EUC", you had better feed more strings.
*	  When you get "CTYPE_SJIS", if you think string does not have binary
*	  data, you don't need to feed more strings.
*	  The function returns "CTYPE_SJIS", only when get the character which
*	  is not "EUC". If user does not give reasonal string, this routine
*	  might give wrong answer. If end of string has first byte Shift-JIS,
*	  returns "CTYPE_CONTINUE".
*	- In any cases, while returning "CTYPE_ASCII" and "CTYPE_CONTINUE",
*	  please feed more strings.
*
*   INPUTS
*	jcc - pointer to JConversionCodeSet.
*	      if it is NULL, this function returns -1.
*	inbuf - input string to be determined.
*	length - how many characters to use for detection, or if the length
*	         is specified as -1 then the string are parsed until NULL
*	         is encountered and the function supposes the string is
*	         continuous.
*
*   RESULTS
*	-1:            / jcc is NULL
*	CTYPE_UNKNOWN  / Can't determine
*	CTYPE_ASCII    / ASCII code only (include Europian)
*	CTYPE_BINARY   / Include binary data
*	CTYPE_EUC      / EUC code
*	CTYPE_SJIS     / Shift JIS code
*	CTYPE_NEWJIS   / New JIS code
*	CTYPE_OLDJIS   / Old JIS code
*	CTYPE_NECJIS   / NEC JIS code
*	CTYPE_CONTINUE / Give continue string
*
*   SEE ALSO
*	See sample "jisconv.c"
*	More information for code set is in "jiscode.doc"
*
******************************************************************************
*/
LONG __saveds __asm
LIBIdentifyCodeSet(register __a0 struct JConversionCodeSet *jcc,
                   register __a1 UBYTE *inbuf,
                   register __d0 LONG length)
{
	LONG p1, p2, p3;
	LONG rcnt = 0;
	LONG rlen;
	LONG whatcode = CTYPE_ASCII;
	LONG whatcode_save = CTYPE_UNKNOWN;

	if (!jcc) return(-1);

	if (length == -1)
		rlen = strlen(inbuf);
	else
		rlen = length;
 
	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE;
		}
		if (jcc->jcc_ESC[0] == ESC)
		{
			pop(&p1, &(jcc->jcc_ESC[0]));
			if (jcc->jcc_ESC[1] == '$'
			 || jcc->jcc_ESC[1] == '(')
			{
				pop(&p2, &(jcc->jcc_ESC[1]));
				goto ESC_3;
			}
			goto ESC_2;
		}
	}

	while (rcnt < rlen)
	{
		p1 = inbuf[rcnt++];
		if (p1 == ESC)
		{
			if (rcnt >= rlen)
			{
				if (length == -1)
				{
					jcc->jcc_ESC[0] = p1;
					whatcode = CTYPE_CONTINUE;
					break;
				}
				else
					break;
			}
ESC_2:
			p2 = inbuf[rcnt++];
			if (p2 == '$')
			{
				if (rcnt >= rlen)
				{
					if (length == -1)
					{
						jcc->jcc_ESC[0] = p1;
						jcc->jcc_ESC[1] = p2;
						whatcode = CTYPE_CONTINUE;
						break;
					}
					else
						break;
				}
ESC_3:
				p3 = inbuf[rcnt++];
				if (p3 == 'B')
				{
					whatcode_save = CTYPE_NEWJIS;
					continue;
				}
				else if (p3 == '@')
				{
					whatcode_save = CTYPE_OLDJIS;
					continue;
				}
			}
			else if (p2 == 'K')
			{
				whatcode_save = CTYPE_NECJIS;
				continue;
			}
		}
		else if ((p1 >= 0x81) && (p1 <= 0xfe))
		{
			if (rcnt >= rlen)
			{
				if (length == -1)
				{
					if (whatcode_save != CTYPE_SJIS)
					{
						whatcode = CTYPE_CONTINUE;
						jcc->jcc_FirstByte = p1;
					}
					break;
				}
				else
					break;
			}
SECONDBYTE:
			p2 = inbuf[rcnt++];

			if (IS_NOT_EUC(p1, p2))
			{
				whatcode_save = CTYPE_SJIS;
				continue;
			}
			else if (IS_EUC_BYTE(p1) && IS_EUC_BYTE(p2))
			{
				if (whatcode_save != CTYPE_SJIS)
					whatcode_save = CTYPE_EUC;
				continue;
			}
			else if (((p1 == 0x8e)) && IS_HANKATA(p2))
			{
				if (whatcode_save != CTYPE_SJIS)
					whatcode_save = CTYPE_EUC;
				continue;
			}
		}
		/* 0x1a is at tail of PC98 text file */
		else if ((p1 >= 0x00) && (p1 < 0x08)
		      || (p1 >  0x0d) && (p1 < 0x1a))
		{
			whatcode = CTYPE_BINARY;
			break;
		}
	}

	D(bug("whatcode,save = %02ld,%02ld\n",whatcode,whatcode_save));

	if (whatcode == CTYPE_ASCII)
		if (whatcode_save != CTYPE_UNKNOWN)
			whatcode = whatcode_save;

	return whatcode;
}
