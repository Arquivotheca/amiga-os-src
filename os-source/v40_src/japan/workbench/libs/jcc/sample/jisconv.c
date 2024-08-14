/******************************************************************************
*
* DESCRIPTION:
*  1) Supports SHIFT-JIS, EUC, NEW-JIS, OLD-JIS, and NEC-JIS for both input
*     and output.
*  2) Automatically detects input file's kanji code. There is only one thing
*     which will make this fail: a SHIFT-JIS file which only contains half-
*     size katakana (remedy: insert a kanji anywhere in the file).
*  3) Will automatically convert half-size katakana in SHIFT-JIS and EUC into
*     full-size equivalents. Note that half-size katakana includes other
*     symbols such as a Japanese period, Japanese comma, center dot, etc.
*  4) Supports conversion between the same code (i.e., EUC -> EUC,
*     SHIFT-JIS -> SHIFT-JIS, etc.).
*     This useful as a filter for converting half-size katakana to full-size
*     equivalents.
*
* OPERATION:
*  1) The UNIX-style command-line is "[-kanji-code-flag] [infile] [outfile]".
*  2) The [kanji-code-flag], which determines the outfile's kanji code,
*     can be 1 of 5 possible values:
*
* 	-j  =  New-JIS (.new)
* 	-o  =  Old-JIS (.old)
* 	-n  =  NEC-JIS (.nec)
* 	-e  =  EUC (.euc)
* 	-s  =  Shift-JIS (.sjs)
*
*     The default kanji code flag (if the user doesn't select one) is -s.
*  3) The [infile] field is always required (you cannot redirect stdin).
*  4) The [outfile] field is optional. If it is omitted, the qoutfile's name
*     will be the same as that of the infile's name plus a file extension
*     (listed under 2). Otherwise, it will be whatever the user types.
*
* MORE INFORMATION FOR CODESET:
*  See "jiscode.doc"
*
*******************************************************************************/
#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>

#include <clib/exec_protos.h>
#include <clib/dos_protos.h>

#include "/jcc_protos.h"
#include "/jcc_pragmas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void __regargs __chkabort(void){ }	/* CTL-C disbale */

void makeext(UBYTE *ext, LONG *extcnt, UBYTE *name);
void ConvertFile(struct JConversionCodeSet *jcc, BPTR inp, BPTR outp,
		UBYTE *inbuf, UBYTE *outbuf, LONG incode, LONG outcode, LONG hzflag);
LONG DetectCodeType(struct JConversionCodeSet *jcc, BPTR inp, UBYTE *inbuf);

struct Library *JCCBase = NULL;

int inbuf_max = 64;
int outbuf_max = 192;	/* need at least 3 times of input buffer size */

#define CTYPE_HANKAKU	1
#define CTYPE_ZENKAKU	2

void main(int argc,char **argv)
{
	BPTR inp = NULL, outp = NULL;
	char infilename[80], outfilename[80], extension[40];
	int c = 0;
	LONG incode = CTYPE_UNKNOWN, outcode = CTYPE_UNKNOWN, hzcode = CTYPE_UNKNOWN;
	int exit_val = RETURN_OK;
	UBYTE *inbuf = NULL, *outbuf = NULL;
	struct JConversionCodeSet *jcc = NULL;
	LONG extcnt = 0;

	if ((argc == 0) || ((argc == 2) && (argv[1][0] == '?')))
	{
		printf("\nUSAGE: jisconv [kanji-code output flag] [infile] [outfile]\n");
		printf(
		"  -j  =  New-JIS (.new)\n"
		"  -o  =  Old-JIS (.old)\n"
		"  -n  =  NEC-JIS (.nec)\n"
		"  -e  =  EUC (.euc)\n"
		"  -s  =  Shift-JIS (.sjs)\n"
		"  -h  =  Hankaku (.han)\n"
		"  -z  =  Zenkaku (.zen)\n");
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
					makeext(&extension[extcnt],&extcnt,"new");
					break;
				case 'o':
					outcode = CTYPE_OLDJIS;
					makeext(&extension[extcnt],&extcnt,"old");
					break;
				case 'n':
					outcode = CTYPE_NECJIS;
					makeext(&extension[extcnt],&extcnt,"nec");
					break;
				case 'e':
					outcode = CTYPE_EUC;
					makeext(&extension[extcnt],&extcnt,"euc");
					break;
				case 's':
					outcode = CTYPE_SJIS;
					makeext(&extension[extcnt],&extcnt,"sjs");
					break;
				case 'h':
					hzcode = CTYPE_HANKAKU;
					makeext(&extension[extcnt],&extcnt,"han");
					break;
				case 'z':
					hzcode = CTYPE_ZENKAKU;
					makeext(&extension[extcnt],&extcnt,"zen");
					break;
				default:
					break;
			}
		}
	}

	if (outcode == CTYPE_UNKNOWN)
	{
		outcode = CTYPE_EUC;
		if (hzcode == 0)
			sprintf(extension, ".euchan");
		if (hzcode == 1)
			sprintf(extension, ".euczen");
	}

	if (argc == 1)
	{
		strcpy(infilename, *argv);
		sprintf(outfilename, "%s%s", *argv, extension);
	}
	else if (argc > 1)
	{
		strcpy(infilename, *argv);
		strcpy(outfilename, *++argv);
	}

	if (argc > 2)
	{
		inbuf_max = atoi(*++argv);
		outbuf_max = inbuf_max * 3;
	}

	if ((JCCBase = OpenLibrary("jcc.library", 0L)) == NULL)
	{
		printf("\nCannot open jcc.library.\n");
		exit_val = RETURN_FAIL;
		goto END;
	}

	if ((inp = Open(infilename, MODE_OLDFILE)) == NULL)
	{
		printf("\nCannot open %s file.\n",infilename);
		exit_val = RETURN_FAIL;
		goto END;
	}


	if ((inbuf = (UBYTE *)AllocVec(inbuf_max + MEM_BLOCKSIZE, MEMF_CLEAR)) == NULL)
	{
		printf("\nCannot allocate input buffer.\n");
		exit_val = RETURN_FAIL;
		goto END;
	}

	if ((jcc = AllocConversionHandle(TAG_DONE)) == NULL)
	{
		printf("\nCannot allocate conversion handle.\n");
		exit_val = RETURN_FAIL;
		goto END;
	}

	incode = DetectCodeType(jcc, inp, inbuf);
	if (incode != CTYPE_EUC
	 && incode != CTYPE_SJIS
	 && incode != CTYPE_NEWJIS
	 && incode != CTYPE_OLDJIS
	 && incode != CTYPE_NECJIS)
	{
		printf("\nNo KANJI code detected in %s file. code = %d\n", infilename, incode);
		exit_val = RETURN_FAIL;
		goto END;
	}

	switch (incode)
	{
		case CTYPE_EUC:
			printf("\nDetected EUC in %s file.\n", infilename);
			break;
		case CTYPE_SJIS:
			printf("\nDetected S-JIS in %s file.\n", infilename);
			break;
		case CTYPE_NEWJIS:
			printf("\nDetected NEW-JIS in %s file.\n", infilename);
			break;
		case CTYPE_OLDJIS:
			printf("\nDetected OLD-JIS in %s file.\n", infilename);
			break;
		case CTYPE_NECJIS:
			printf("\nDetected NEC-JIS in %s file.\n", infilename);
			break;
	}

	/* Rewind start of file -- I could not use just OFFSET_BEGINNING. */
	Seek(inp, 0, OFFSET_BEGINNING);

	if ((outp = Open(outfilename, MODE_NEWFILE)) == NULL)
	{
		printf("\nCannot open %s file.\n", outfilename);
		exit_val = RETURN_FAIL;
		goto END;
	}

	if ((outbuf = (UBYTE *)AllocVec(outbuf_max + MEM_BLOCKSIZE, MEMF_CLEAR)) == NULL)
	{
		printf("\nCannot allocate output buffer.\n");
		exit_val = RETURN_FAIL;
		goto END;
	}

	ConvertFile(jcc, inp, outp, inbuf, outbuf, incode, outcode, hzcode);

END:
	if (jcc)
		FreeConversionHandle(jcc);
	if (inp)
		Close(inp);
	if (outp)
		Close(outp);
	if (inbuf)
		FreeVec((void *)inbuf);
	if (outbuf)
		FreeVec((void *)outbuf);
	if (JCCBase)
		CloseLibrary(JCCBase);

	exit(exit_val);
}

void makeext(UBYTE *ext, LONG *extcnt, UBYTE *name)
{
	if (*extcnt == 0)
	{
		*ext++ = '.';
		sprintf(ext, name);
		*extcnt += 4;
	}
	else
	{
		sprintf(ext, name);
		*extcnt += 3;
	}
}

void ConvertFile(struct JConversionCodeSet *jcc, BPTR in, BPTR out,
			 UBYTE *inbuf, UBYTE *outbuf, LONG incode, LONG outcode, LONG hzcode)
{
	LONG WriteCount;
	ULONG textfilter = TRUE;

	SetJConversionAttrs(jcc, JCC_ShiftedIn, FALSE, TAG_DONE);

	while (1)
	{
		memset(inbuf, 0x00, inbuf_max + MEM_BLOCKSIZE);
		memset(outbuf, 0x00, outbuf_max + MEM_BLOCKSIZE);

		if (Read(in, inbuf, inbuf_max) == 0)
			break;

		if (hzcode == CTYPE_HANKAKU)
			WriteCount = ZenToHan(jcc, inbuf, outbuf, incode, outcode, -1);
		else if (hzcode == CTYPE_ZENKAKU)
			WriteCount = HanToZen(jcc, inbuf, outbuf, incode, outcode, -1);
		else
		{
			WriteCount = JStringConvert(jcc, inbuf, outbuf, incode, outcode, -1,
//									JCT_EUCHanKata, TRUE,
//									JCT_SJISHanKata, TRUE,
//									JCT_UDHanKata, 0x24,
									JCT_TextFilter, textfilter,
									TAG_DONE);
		}

//		printf("wlen = %d\n", WriteCount);

		if (Write(out, outbuf, WriteCount) < WriteCount)
			break;
	}

	/* PC has end-of-file "0x1a". */
	if (outcode == CTYPE_SJIS && textfilter)
	{
		char c = 0x1a;
		Write(out, &c, 1);
	}

	/* Well, we have to put the "JIS kanji out" at end of file,
	 * if it's in "JIS kanji in" mode.
	 * But, standard text file has "0x0a" at end of file,
	 * That case "JStringConvert" function takes care of this process.
	 * I don't think we need this process here.
	 */
/*
	if (outcode == CTYPE_NEWJIS || outcode == CTYPE_OLDJIS || outcode == CTYPE_NECJIS)
	{
		LONG shiftedin;

		GetJConversionAttrs(jcc, JCC_ShiftedIn, &shiftedin, TAG_DONE);
		printf("shiftedin = %d",shiftedin);
		if (shiftedin)
			if ((WriteCount = KanjiOut(outcode, outbuf)) > 1)
				Write(out, outbuf, WriteCount);
	}
*/
}

LONG DetectCodeType(struct JConversionCodeSet *jcc, BPTR in, UBYTE *inbuf)
{
	LONG whatcode = CTYPE_UNKNOWN;
	LONG whatcode_save = CTYPE_UNKNOWN;
	LONG ReadCount;

	while (1)
	{
		memset(inbuf, 0x00, inbuf_max + MEM_BLOCKSIZE);

		if ((ReadCount = Read(in, inbuf, inbuf_max)) == 0)
			break;

		whatcode = IdentifyCodeSet(jcc, inbuf, -1);

		if (whatcode == CTYPE_BINARY)
			break;

		if (whatcode != CTYPE_CONTINUE
		 && whatcode != CTYPE_ASCII)
			if (whatcode_save != CTYPE_SJIS)
				whatcode_save = whatcode;

//		printf("whatcode = %d\n", whatcode);
	}

//	printf("whatcode_save = %d\n", whatcode_save);

	if (whatcode != CTYPE_BINARY)
		if (whatcode_save != CTYPE_UNKNOWN)
			whatcode = whatcode_save;

	return whatcode;
}

