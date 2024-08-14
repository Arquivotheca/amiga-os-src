/*
 *  jcctest.c
 *
 */
#include <exec/types.h>
#include <clib/exec_protos.h>

#include "/jcc_protos.h"
#include "/jcc_pragmas.h"

#include "stdio.h"

struct Library *JCCBase = NULL;

void main(void)
{
	struct JConversionCodeSet *jcc = NULL;
	ULONG whatinputcode = 0, whatoutputcode = 0;
	UBYTE firstbyte = 0;
	ULONG esc = 0;
	ULONG hankata = 0;
	LONG ret;

	if (JCCBase = OpenLibrary("jcc.library", 0L))
	{
		printf("JCCBase at $%lx\n", JCCBase);
		if (jcc = AllocConversionHandle(JCC_WhatInputCode, CTYPE_NEWJIS,
						JCC_WhatOutputCode, CTYPE_EUC,
						TAG_DONE))
		{
			ClearConversionHandle(jcc);
			SetJConversionAttrs(jcc, JCC_WhatInputCode, CTYPE_NECJIS,
					JCC_WhatOutputCode, CTYPE_SJIS,
					JCC_FirstByte, 0xa0,
					JCC_ESC, 0x1b4b0000,
					JCC_HanKata, 0xb4b5b6b7,
					TAG_DONE);
			ret = GetJConversionAttrs(jcc, JCC_WhatInputCode, &whatinputcode,
					JCC_WhatOutputCode, &whatoutputcode,
					JCC_FirstByte, &firstbyte,
					JCC_ESC, &esc,
					JCC_HanKata, &hankata,
					TAG_DONE);

			printf("ret = %ld\n", ret);
			printf("whatinputcode = %ld\n", whatinputcode);
			printf("whatoutputcode = %ld\n", whatoutputcode);
			printf("firstbyte = 0x%lx\n", firstbyte);
			printf("esc = 0x%lx\n", esc);
			printf("hankata = 0x%lx\n", hankata);

			FreeConversionHandle(jcc);
		}
		else
			printf("Alloc error\n");
		CloseLibrary(JCCBase);
	}
	else printf("jcc.library can not open...\n");
}

