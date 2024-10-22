/* $Header: test.c,v 36.2 89/12/17 19:57:54 kodiak Exp $ */
#include	"exec/types.h"
#include	"exec/libraries.h"
#include	"devices/inputevent.h"
#include	"dos/dos.h"

struct Library *KeymapBase = 0;

#define	BUFFERLEN	80
unsigned char buffer[BUFFERLEN];
unsigned char buffer2[BUFFERLEN];

#define	STIMSIZE	2	/* one dead key, one key */
unsigned char rBuffer[STIMSIZE*2];

unsigned char deadQuals[] = {
    IEQUALIFIER_LSHIFT|IEQUALIFIER_LALT, IEQUALIFIER_LALT, 0 };

unsigned char deadCodes[] = {
    0, 0x23, 0x24, 0x25, 0x26, 0x27 };

unsigned short *trigger;

EndGame(code, format, arg1, arg2)
{
    if (KeymapBase)
	CloseLibrary(KeymapBase);
    if (code != 0)
	printf(format, arg1, arg2);
    exit(code);
}

ReportChar(code)
unsigned char code;
{
    if (code & 0x80)
	if (code & 0x60)
	    printf(" |%lc", code-0x80);
	else
	    printf(" ~%lc", code-0x80+'@');
    else
	if (code & 0x60)
	    printf("  %lc", code);
	else
	    printf(" ^%lc", code+'@');
    printf(" $%02lx", code);
}

main()
{
    unsigned char dead1CodeI, dead1Code, dead1QualI, dead1Qual;
    unsigned short code, qual;
    struct InputEvent event, event2;
    unsigned char stimulus[STIMSIZE*2], *s, *r;
    int actual, actual2, i;

    KeymapBase = (struct Library *) OpenLibrary("keymap.library", 0);
    if (KeymapBase == 0) {
	EndGame(20, "** OpenLibrary(\"keymap.library\") failed\n");
    }

    event.ie_NextEvent = event2.ie_NextEvent = 0;
    event.ie_Class = event2.ie_Class = IECLASS_RAWKEY;
    event.ie_SubClass = event2.ie_SubClass = 0;
    event.ie_Prev2DownCode = event2.ie_Prev2DownCode = 0;
    event.ie_Prev2DownQual = event2.ie_Prev2DownQual = 0;

    /* check for reversability */
    printf("code -> key -> code\n");
    trigger = (unsigned short *) 0xc0;
    for (code = 0; code < 256; code++) {
	buffer[0] = code;
	*trigger = code;
	actual = MapANSI(buffer, 1, rBuffer, STIMSIZE, 0);
	switch (actual) {
	    case -2:
		printf("MapANSI internal");
		goto reportChar;
	    case -1:
		printf("MapANSI overflow");
		goto reportChar;
	    case 0:
		printf("MapANSI ungeneratable");
		goto reportChar;
	    case 1:
		event.ie_Prev1DownCode = 0;
		event.ie_Prev1DownQual = 0;
		event.ie_Code = rBuffer[0];
		event.ie_Qualifier = rBuffer[1];
		goto testChar;
	    case 2:
		event.ie_Prev1DownCode = rBuffer[0];
		event.ie_Prev1DownQual = rBuffer[1];
		event.ie_Code = rBuffer[2];
		event.ie_Qualifier = rBuffer[3];
testChar:
		actual = MapRawKey(&event, buffer, BUFFERLEN, 0);
		if ((actual != 1) || (buffer[0] != code)) {
		    printf("MapANSI not reversable");
		    for (i = 0; i < actual; i++)
			ReportChar(buffer[i]);
		    printf(" from");
reportChar:
		    ReportChar(code);
		    printf("\n");
		}
	}
	if (SetSignal(0, 0) & SIGBREAKF_CTRL_C)
	    EndGame(1, "<Control-C>\n");
    }

    printf("key -> code -> key\n");
    trigger = (unsigned short *) 0xc2;
    for (dead1QualI = 0; dead1QualI < sizeof(deadQuals); dead1QualI++) {
	dead1Qual = deadQuals[dead1QualI];
	event.ie_Prev1DownQual = dead1Qual;
	stimulus[1] = dead1Qual;
	for (dead1CodeI = 0; dead1CodeI < sizeof(deadCodes); dead1CodeI++) {
	    dead1Code = deadCodes[dead1CodeI];
	    event.ie_Prev1DownCode = dead1Code;
	    stimulus[0] = dead1Code;
	    for (qual = 0x00; qual < 0x20; qual += 2) {
		event.ie_Qualifier = qual;
		stimulus[3] = qual;
		printf(" 0x%lx 0x%lx 0x%lx 0x%lx        \r",
			dead1Code, dead1Qual, code, qual);
		for (code = 0; code < 256; code++) {
		    event.ie_Code = code;
		    stimulus[2] = code;
		    actual = MapRawKey(&event, buffer, BUFFERLEN, 0);
		    if (actual > 0) {
			*trigger = code;
			actual2 = MapANSI(buffer, actual, rBuffer, STIMSIZE, 0);
			switch (actual2) {
			    case -2:
				printf("MapANSI internal");
				goto reportCode;
			    case -1:
				printf("MapANSI overflow");
				goto reportCode;
			    case 0:
				printf("MapANSI ungeneratable");
				goto reportCode;
			    case 1:
				event2.ie_Prev1DownCode = 0;
				event2.ie_Prev1DownQual = 0;
				event2.ie_Code = rBuffer[0];
				event2.ie_Qualifier = rBuffer[1];
				goto testCode;
			    case 2:
				event2.ie_Prev1DownCode = rBuffer[0];
				event2.ie_Prev1DownQual = rBuffer[1];
				event2.ie_Code = rBuffer[2];
				event2.ie_Qualifier = rBuffer[3];
testCode:
				r = rBuffer;
				s = stimulus + ((STIMSIZE-actual2)*2);

				for (i = actual2*2; i > 0; i--)
				    if (*s++ != *r++)
					goto reCheckCode;
				break;

reCheckCode:
				actual2 = MapRawKey(&event2, buffer2,
					BUFFERLEN, 0);
				if (actual == actual2) {
				    actual = 1;
				    for (i = 0; i < actual2; i++)
					if (buffer[i] != buffer2[i]) {
					    printf("i: %ld", i);
					    ReportChar(buffer[i]);
					    printf(" !=");
					    ReportChar(buffer2[i]);
					    actual = 0;
					    printf("...\n");
					}
				    if (actual)
					break;
				}
				printf("MapANSI not reversable, generated");
				for (i = 0; i < STIMSIZE*2; i++)
				    printf(" 0x%lx", rBuffer[i]);
				printf("\ninstead of");
reportCode:
				printf(" 0x%lx 0x%lx 0x%lx 0x%lx",
					dead1Code, dead1Qual, code, qual);
				for (i = 0; i < actual; i++)
				    ReportChar(buffer[i]);
				printf("\n");
			}
			if (SetSignal(0, 0) & SIGBREAKF_CTRL_C)
			    EndGame(1, "<Control-C>\n");
		    }
		}
	    }
	}
    }
}
