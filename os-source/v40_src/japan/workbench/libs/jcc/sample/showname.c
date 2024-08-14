/*
 *  showname.c
 *
 */
#include <exec/types.h>
#include <libraries/dos.h>

#include <clib/exec_protos.h>

#include "/jcc_protos.h"
#include "/jcc_pragmas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void __regargs __chkabort(void){ }	/* CTL-C disbale */

static UBYTE *jname[] =
{
	"·¬ÅÄ¹áŒK“c",
	"ŒK“c·¬ÅÄ¹á"
};

struct Library *JCCBase = NULL;


void main(int argc,char **argv)
{
	int c = 0;
	LONG incode, outcode = CTYPE_UNKNOWN;
	int exit_val = RETURN_OK;
	UBYTE outbuf[40];
	struct JConversionCodeSet *jcc = NULL;
	LONG jcnt = 0;
	LONG i = 0;
	LONG WriteCount;
	LONG count = 7;

	if ((argc == 0) || ((argc == 2) && (argv[1][0] == '?')))
	{
		printf("\nUSAGE: showname [kanji-code output flag] [checking byte numbers]\n");
		printf(
		"  -j  =  New-JIS\n"
		"  -o  =  Old-JIS\n"
		"  -n  =  NEC-JIS\n"
		"  -e  =  EUC\n"
		"  -s  =  Shift-JIS\n");

		exit(0L);
	}

	while ((--argc > 0) && ((*++argv)[0] == '-'))
	{
		while (c = *++argv[0])
		{
			switch (c)
			{
				case 'j':
					outcode = CTYPE_NEWJIS;
					break;
				case 'o':
					outcode = CTYPE_OLDJIS;
					break;
				case 'n':
					outcode = CTYPE_NECJIS;
					break;
				case 'e':
					outcode = CTYPE_EUC;
					break;
				case 's':
					outcode = CTYPE_SJIS;
					break;
				default:
					break;
			}
		}
	}

	if (argc == 1)
	{
		count = atoi(argv[0]);
        }

	if (outcode == CTYPE_UNKNOWN)
	{
		outcode = CTYPE_EUC;
	}

	if ((JCCBase = OpenLibrary("jcc.library", 0L)) == NULL)
	{
		printf("\nCannot open jcc.library.\n");
		exit_val = RETURN_FAIL;
		goto END;
	}

	if ((jcc = AllocConversionHandle(TAG_DONE)) == NULL)
	{
		printf("\nCannot allocate conversion handle.\n");
		exit_val = RETURN_FAIL;
		goto END;
	}

	while (jcnt < 2)
	{
		for (i = 0; jname[jcnt][i] != 0x00; i++)
			 printf("%02x ",jname[jcnt][i]);

		incode = IdentifyCodeSet(jcc, jname[jcnt], count);
		if (incode != CTYPE_EUC
		 && incode != CTYPE_SJIS
		 && incode != CTYPE_NEWJIS
		 && incode != CTYPE_OLDJIS
		 && incode != CTYPE_NECJIS)
		{
			printf(" / No KANJI code detected. code = %d\n", incode);
			continue;
		}

		switch (incode)
		{
			case CTYPE_EUC:
				printf("/ %d bytes checked. Detected EUC.\n", count);
				break;
			case CTYPE_SJIS:
				printf("/ %d bytes checked. Detected S-JIS.\n", count);
				break;
			case CTYPE_NEWJIS:
				printf("/ %d bytes checked. Detected NEW-JIS.\n", count);
				break;
			case CTYPE_OLDJIS:
				printf("/ %d bytes checked. Detected OLD-JIS.\n", count);
				break;
			case CTYPE_NECJIS:
				printf("/ %d bytes checked. Detected NEC-JIS.\n", count);
				break;
		}

		SetJConversionAttrs(jcc, JCC_ShiftedIn, FALSE, TAG_DONE);

		WriteCount = JStringConvert(jcc, jname[jcnt], outbuf, incode, outcode, count, TAG_DONE);

		for (i = 0; i < WriteCount; i++) printf("%02x ",outbuf[i]);

		switch (outcode)
		{
			case CTYPE_EUC:
				printf("/ Converted to EUC.\n");
				break;
			case CTYPE_SJIS:
				printf("/ Converted to S-JIS.\n");
				break;
			case CTYPE_NEWJIS:
				printf("/ Converted to NEW-JIS.\n");
				break;
			case CTYPE_OLDJIS:
				printf("/ Converted to OLD-JIS.\n");
				break;
			case CTYPE_NECJIS:
				printf("/ Converted to NEC-JIS.\n");
				break;
		}
		jcnt++;
	}

END:
	if (jcc)
		FreeConversionHandle(jcc);
	if (JCCBase)
		CloseLibrary(JCCBase);

	exit(exit_val);
}
