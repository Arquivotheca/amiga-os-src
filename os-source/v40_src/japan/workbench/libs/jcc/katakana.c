/*
 *  Katakana.c - conversion functions half and full size for Katakana
 *
 */
#include <exec/types.h>
#include <exec/libraries.h>

#include <clib/exec_protos.h>
#include <pragmas/exec_pragmas.h>

#include <stdio.h>
#include <string.h>

#include "jcc.h"
//#include "jccbase.h"

//#define JCCBase		((struct JCCLibrary *)getreg(REG_A6))

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

extern LONG __saveds __asm
shift2euc(register __a0 struct JConversionCodeSet *jcc,
          register __a1 UBYTE *inbuf,
          register __a2 UBYTE *outbuf,
          register __d0 LONG length,
          register __d1 ULONG hankatatype,
          register __d2 UBYTE udhankata,
          register __d3 ULONG textfilter);

extern LONG __saveds __asm
shift2shift(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
            register __d0 LONG length,
            register __d1 ULONG hankatatype,
            register __d2 UBYTE udhankata,
            register __d3 ULONG textfilter);

extern LONG __saveds __asm
euc2euc(register __a0 struct JConversionCodeSet *jcc,
        register __a1 UBYTE *inbuf,
        register __a2 UBYTE *outbuf,
        register __d0 LONG length,
        register __d1 ULONG textfilter);

extern LONG __saveds __asm
euc2shift(register __a0 struct JConversionCodeSet *jcc,
          register __a1 UBYTE *inbuf,
          register __a2 UBYTE *outbuf,
          register __d0 LONG length,
          register __d1 LONG textfilter);

extern VOID __saveds __asm
pop(register __a0 LONG *p,
    register __a1 UBYTE *jcc_b);

extern LONG __saveds __asm
writebuf(register __d0 LONG p1,
         register __d1 LONG p2,
         register __a0 UBYTE *outbuf);

extern VOID __saveds __asm
eight2seven(register __a0 LONG *p1,
            register __a1 LONG *p2);

extern VOID __saveds __asm
seven2eight(register __a0 LONG *p1,
            register __a1 LONG *p2);

/* Shift-JIS hankaku to Shift-JIS zenkaku */
VOID __saveds __asm
han2zen(register __a0 LONG *one,
        register __a1 LONG *two,
        register __d0 LONG nigori,
        register __d1 LONG maru)
{
	switch (*one)
	{
		case 0xa1:	/* Japanese period */
			*one = 0x81;
			*two = 0x42;
			break;
		case 0xa2:	/* kagikakko left up */
			*one = 0x81;
			*two = 0x75;
			break;
		case 0xa3:      /* kagikakko right down */
			*one = 0x81;
			*two = 0x76;
			break;
		case 0xa4:     /* Japanese comma */
			*one = 0x81;
			*two = 0x41;
			break;
		case 0xa5:      /* center dot */
			*one = 0x81;
			*two = 0x45;
			break;
		case 0xa6:	/* wo */
			*one = 0x83;
			*two = 0x92;
			break;
		case 0xa7:	/* lower a */
			*one = 0x83;
			*two = 0x40;
			break;
		case 0xa8:	/* lower i */
			*one = 0x83;
			*two = 0x42;
			break;
		case 0xa9:	/* lower u */
			*one = 0x83;
			*two = 0x44;
			break;
		case 0xaa:	/* lower e */
			*one = 0x83;
			*two = 0x46;
			break;
		case 0xab:	/* lower o */
			*one = 0x83;
			*two = 0x48;
			break;
		case 0xac:	/* lower ya */
			*one = 0x83;
			*two = 0x83;
			break;
		case 0xad:	/* lower yu */
			*one = 0x83;
			*two = 0x85;
			break;
		case 0xae:	/* lower yo */
			*one = 0x83;
			*two = 0x87;
			break;
		case 0xaf:	/* lower tu */
			*one = 0x83;
			*two = 0x62;
			break;
		case 0xb0:	/* nobashi - */
			*one = 0x81;
			*two = 0x5b;
			break;
		case 0xb1:	/* a */
			*one = 0x83;
			*two = 0x41;
			break;
		case 0xb2:	/* i */
			*one = 0x83;
			*two = 0x43;
			break;
		case 0xb3:	/* u */
			*one = 0x83;
			*two = 0x45;
			break;
		case 0xb4:	/* e */
			*one = 0x83;
			*two = 0x47;
			break;
		case 0xb5:	/* o */
			*one = 0x83;
			*two = 0x49;
			break;
		case 0xb6:	/* ka */
			*one = 0x83;
			*two = 0x4a;
			break;
		case 0xb7:	/* ki */
			*one = 0x83;
			*two = 0x4c;
			break;
		case 0xb8:	/* ku */
			*one = 0x83;
			*two = 0x4e;
			break;
		case 0xb9:	/* ke */
			*one = 0x83;
			*two = 0x50;
			break;
		case 0xba:	/* ko */
			*one = 0x83;
			*two = 0x52;
			break;
		case 0xbb:	/* sa */
			*one = 0x83;
			*two = 0x54;
			break;
		case 0xbc:	/* si */
			*one = 0x83;
			*two = 0x56;
			break;
		case 0xbd:	/* su */
			*one = 0x83;
			*two = 0x58;
			break;
		case 0xbe:	/* se */
			*one = 0x83;
			*two = 0x5a;
			break;
		case 0xbf:	/* so */
			*one = 0x83;
			*two = 0x5c;
			break;
		case 0xc0:	/* ta */
			*one = 0x83;
			*two = 0x5e;
			break;
		case 0xc1:	/* ti */
			*one = 0x83;
			*two = 0x60;
			break;
		case 0xc2:	/* tu */
			*one = 0x83;
			*two = 0x63;
			break;
		case 0xc3:	/* te */
			*one = 0x83;
			*two = 0x65;
			break;
		case 0xc4:	/* to */
			*one = 0x83;
			*two = 0x67;
			break;
		case 0xc5:	/* na */
			*one = 0x83;
			*two = 0x69;
			break;
		case 0xc6:	/* ni */
			*one = 0x83;
			*two = 0x6a;
			break;
		case 0xc7:	/* nu */
			*one = 0x83;
			*two = 0x6b;
			break;
		case 0xc8:	/* ne */
			*one = 0x83;
			*two = 0x6c;
			break;
		case 0xc9:	/* no */
			*one = 0x83;
			*two = 0x6d;
			break;
		case 0xca:	/* ha */
			*one = 0x83;
			*two = 0x6e;
			break;
		case 0xcb:	/* hi */
			*one = 0x83;
			*two = 0x71;
			break;
		case 0xcc:	/* fu */
			*one = 0x83;
			*two = 0x74;
			break;
		case 0xcd:	/* he */
			*one = 0x83;
			*two = 0x77;
			break;
		case 0xce:	/* ho */
			*one = 0x83;
			*two = 0x7a;
			break;
		case 0xcf:	/* ma */
			*one = 0x83;
			*two = 0x7d;
			break;
		case 0xd0:	/* mi */
			*one = 0x83;
			*two = 0x7e;
			break;
		case 0xd1:	/* mu */
			*one = 0x83;
			*two = 0x80;
			break;
		case 0xd2:	/* me */
			*one = 0x83;
			*two = 0x81;
			break;
		case 0xd3:	/* mo */
			*one = 0x83;
			*two = 0x82;
			break;
		case 0xd4:	/* ya */
			*one = 0x83;
			*two = 0x84;
			break;
		case 0xd5:	/* yu */
			*one = 0x83;
			*two = 0x86;
			break;
		case 0xd6:	/* yo */
			*one = 0x83;
			*two = 0x88;
			break;
		case 0xd7:	/* ra */
			*one = 0x83;
			*two = 0x89;
			break;
		case 0xd8:	/* ri */
			*one = 0x83;
			*two = 0x8a;
			break;
		case 0xd9:	/* ru */
			*one = 0x83;
			*two = 0x8b;
			break;
		case 0xda:	/* re */
			*one = 0x83;
			*two = 0x8c;
			break;
		case 0xdb:	/* ro */
			*one = 0x83;
			*two = 0x8d;
			break;
		case 0xdc:	/* wa */
			*one = 0x83;
			*two = 0x8f;
			break;
		case 0xdd:	/* n */
			*one = 0x83;
			*two = 0x93;
			break;
		case 0xde:	/* dakuten */
			*one = 0x81;
			*two = 0x4a;
			break;
		case 0xdf:	/* maru */
			*one = 0x81;
			*two = 0x4b;
			break;
	}
	if (nigori)
	{
		if (((*two >= 0x4a) && (*two <= 0x67))	/* from "ka" to "to" */
		 || ((*two >= 0x6e) && (*two <= 0x7a)))	/* from "ha" to "ho" */
			(*two)++;
		else if ((*one == 0x83) && (*two == 0x45)) /* "u" */
			*two = 0x94;	/* "bu" */
	}
	else if ((maru)
		 && ((*two >= 0x6e) && (*two <= 0x7a)))	/* from "ha" to "ho" */
		*two += 2;
}

/* Shift-JIS zenkaku to Shift-JIS hankaku */
LONG __saveds __asm
zen2han(register __d0 LONG one,
        register __d1 LONG two,
        register __a0 UBYTE *outbuf)
{
	LONG wlen;

	/* Error..., this is not Shift-JIS Katakana 1st byte */
	if (!IS_SJIS_KBYTE1(one))
		return(0);

	if (one == 0x81)
	{
		switch (two)
		{
		case 0x41:	/* Japanese comma */
			*outbuf = 0xa4;
			wlen = 1;
			break;
		case 0x42:	/* Japanese period */
			*outbuf = 0xa1;
			wlen = 1;
			break;
		case 0x45:      /* center dot */
			*outbuf = 0xa5;
			wlen = 1;
			break;
		case 0x5b:	/* nobashi - */
			*outbuf = 0xb0;
			wlen = 1;
			break;
		case 0x75:	/* kagikakko left up */
			*outbuf = 0xa2;
			wlen = 1;
			break;
		case 0x76:      /* kagikakko right down */
			*outbuf = 0xa3;
			wlen = 1;
			break;
		default:
			wlen = 0;
			break;
		}
	}

	if (one == 0x83)
	{
		switch (two)
		{
		case 0x40:	/* lower a */
			*outbuf = 0xa7;
			wlen = 1;
			break;
		case 0x41:	/* a */
			*outbuf = 0xb1;
			wlen = 1;
			break;
		case 0x42:	/* lower i */
			*outbuf = 0xa8;
			wlen = 1;
			break;
		case 0x43:	/* i */
			*outbuf = 0xb2;
			wlen = 1;
			break;
		case 0x44:	/* lower u */
			*outbuf = 0xa9;
			wlen = 1;
			break;
		case 0x45:	/* u */
			*outbuf = 0xb3;
			wlen = 1;
			break;
		case 0x46:	/* lower e */
			*outbuf = 0xaa;
			wlen = 1;
			break;
		case 0x47:	/* e */
			*outbuf = 0xb4;
			wlen = 1;
			break;
		case 0x48:	/* lower o */
			*outbuf = 0xab;
			wlen = 1;
			break;
		case 0x49:	/* o */
			*outbuf = 0xb5;
			wlen = 1;
			break;
		case 0x4a:	/* ka */
			*outbuf = 0xb6;
			wlen = 1;
			break;
		case 0x4b:	/* ga */
			*outbuf++ = 0xb6;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x4c:	/* ki */
			*outbuf = 0xb7;
			wlen = 1;
			break;
		case 0x4d:	/* gi */
			*outbuf++ = 0xb7;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x4e:	/* ku */
			*outbuf = 0xb8;
			wlen = 1;
			break;
		case 0x4f:	/* gu */
			*outbuf++ = 0xb8;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x50:	/* ke */
			*outbuf = 0xb9;
			wlen = 1;
			break;
		case 0x51:	/* ge */
			*outbuf++ = 0xb9;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x52:	/* ko */
			*outbuf = 0xba;
			wlen = 1;
			break;
		case 0x53:	/* go */
			*outbuf++ = 0xba;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x54:	/* sa */
			*outbuf = 0xbb;
			wlen = 1;
			break;
		case 0x55:	/* za */
			*outbuf++ = 0xbb;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x56:	/* si */
			*outbuf = 0xbc;
			wlen = 1;
			break;
		case 0x57:	/* zi */
			*outbuf++ = 0xbc;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x58:	/* su */
			*outbuf = 0xbd;
			wlen = 1;
			break;
		case 0x59:	/* zu */
			*outbuf++ = 0xbd;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x5a:	/* se */
			*outbuf = 0xbe;
			wlen = 1;
			break;
		case 0x5b:	/* ze */
			*outbuf++ = 0xbe;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x5c:	/* so */
			*outbuf = 0xbf;
			wlen = 1;
			break;
		case 0x5d:	/* zo */
			*outbuf++ = 0xbf;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x5e:	/* ta */
			*outbuf = 0xc0;
			wlen = 1;
			break;
		case 0x5f:	/* da */
			*outbuf++ = 0xc0;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x60:	/* ti */
			*outbuf = 0xc1;
			wlen = 1;
			break;
		case 0x61:	/* zi */
			*outbuf++ = 0xc1;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x62:	/* lower tu */
			*outbuf = 0xaf;
			wlen = 1;
			break;
		case 0x63:	/* tu */
			*outbuf = 0xc2;
			wlen = 1;
			break;
		case 0x64:	/* zu */
			*outbuf++ = 0xc2;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x65:	/* te */
			*outbuf = 0xc3;
			wlen = 1;
			break;
		case 0x66:	/* de */
			*outbuf++ = 0xc3;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x67:	/* to */
			*outbuf = 0xc4;
			wlen = 1;
			break;
		case 0x68:	/* do */
			*outbuf++ = 0xc4;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x69:	/* na */
			*outbuf = 0xc5;
			wlen = 1;
			break;
		case 0x6a:	/* ni */
			*outbuf = 0xc6;
			wlen = 1;
			break;
		case 0x6b:	/* nu */
			*outbuf = 0xc7;
			wlen = 1;
			break;
		case 0x6c:	/* ne */
			*outbuf = 0xc8;
			wlen = 1;
			break;
		case 0x6d:	/* no */
			*outbuf = 0xc9;
			wlen = 1;
			break;
		case 0x6e:	/* ha */
			*outbuf = 0xca;
			wlen = 1;
			break;
		case 0x6f:	/* ba */
			*outbuf++ = 0xca;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x70:	/* pa */
			*outbuf++ = 0xca;
			*outbuf   = 0xdf;
			wlen = 2;
			break;
		case 0x71:	/* hi */
			*outbuf = 0xcb;
			wlen = 1;
			break;
		case 0x72:	/* bi */
			*outbuf++ = 0xcb;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x73:	/* pi */
			*outbuf++ = 0xcb;
			*outbuf   = 0xdf;
			wlen = 2;
			break;
		case 0x74:	/* fu */
			*outbuf = 0xcc;
			wlen = 1;
			break;
		case 0x75:	/* bu */
			*outbuf++ = 0xcc;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x76:	/* pu */
			*outbuf++ = 0xcc;
			*outbuf   = 0xdf;
			wlen = 2;
			break;
		case 0x77:	/* he */
			*outbuf = 0xcd;
			wlen = 1;
			break;
		case 0x78:	/* be */
			*outbuf++ = 0xcd;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x79:	/* pe */
			*outbuf++ = 0xcd;
			*outbuf   = 0xdf;
			wlen = 2;
			break;
		case 0x7a:	/* ho */
			*outbuf = 0xce;
			wlen = 1;
			break;
		case 0x7b:	/* bo */
			*outbuf++ = 0xce;
			*outbuf   = 0xde;
			wlen = 2;
			break;
		case 0x7c:	/* po */
			*outbuf++ = 0xce;
			*outbuf   = 0xdf;
			wlen = 2;
			break;
		case 0x7d:	/* ma */
			*outbuf = 0xcf;
			wlen = 1;
			break;
		case 0x7e:	/* mi */
			*outbuf = 0xd0;
			wlen = 1;
			break;
		case 0x80:	/* mu */
			*outbuf = 0xd1;
			wlen = 1;
			break;
		case 0x81:	/* me */
			*outbuf = 0xd2;
			wlen = 1;
			break;
		case 0x82:	/* mo */
			*outbuf = 0xd3;
			wlen = 1;
			break;
		case 0x83:	/* lower ya */
			*outbuf = 0xac;
			wlen = 1;
			break;
		case 0x84:	/* ya */
			*outbuf = 0xd4;
			wlen = 1;
			break;
		case 0x85:	/* lower yu */
			*outbuf = 0xad;
			wlen = 1;
			break;
		case 0x86:	/* yu */
			*outbuf = 0xd5;
			wlen = 1;
			break;
		case 0x87:	/* lower yo */
			*outbuf = 0xae;
			wlen = 1;
			break;
		case 0x88:	/* yo */
			*outbuf = 0xd6;
			wlen = 1;
			break;
		case 0x89:	/* ra */
			*outbuf = 0xd7;
			wlen = 1;
			break;
		case 0x8a:	/* ri */
			*outbuf = 0xd8;
			wlen = 1;
			break;
		case 0x8b:	/* ru */
			*outbuf = 0xd9;
			wlen = 1;
			break;
		case 0x8c:	/* re */
			*outbuf = 0xda;
			wlen = 1;
			break;
		case 0x8d:	/* ro */
			*outbuf = 0xdb;
			wlen = 1;
			break;
		case 0x8e:	/* lower wa - not exist */
			*outbuf = 0xdc;
			wlen = 1;
			break;
		case 0x8f:	/* wa */
			*outbuf = 0xdc;
			wlen = 1;
			break;
		case 0x90:	/* old i - not exist */
			*outbuf = 0xb2;
			wlen = 1;
			break;
		case 0x91:	/* old e - not exist */
			*outbuf = 0xb4;
			wlen = 1;
			break;
		case 0x92:	/* wo */
			*outbuf = 0xa6;
			wlen = 1;
			break;
		case 0x93:	/* n */
			*outbuf = 0xdd;
			wlen = 1;
			break;
		case 0x94:	/* lower ka - not exist */
			*outbuf = 0xb6;
			wlen = 1;
			break;
		case 0x95:	/* lower ke - not exist */
			*outbuf = 0xb9;
			wlen = 1;
			break;
		default:
			wlen = 0;
			break;
		}
	}

	return wlen;
}

/* EUC zenkaku to EUC hankaku */
LONG __saveds __asm
euczen2euchan(register __a0 struct JConversionCodeSet *jcc,
              register __a1 UBYTE *inbuf,
              register __a2 UBYTE *outbuf,
              register __d0 LONG length)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen = strlen(inbuf);
	LONG maxlen = length;

	if (maxlen == -1)
		maxlen = rlen;

	D(bug("euczen2euchan:rlen = %ld\n", rlen));

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
			goto HANKATA_SECONDBYTE_EUC_EUC;
		}
	}

	while (rcnt < rlen && rcnt < maxlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case FF:
				break;
			default:
				if (IS_EUC_BYTE(p1))
				{
					if (rcnt >= rlen || rcnt >= maxlen)
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
					if (IS_EUC_KBYTE1(p1))	/* EUC Katakana 1st byte */
					{
						if (IS_EUC_KBYTE2(p2))	/* EUC Katakana 2nd byte */
						{
							LONG tmpi = 0;
							LONG c1 = p1, c2 = p2;
							p1 -= 0x80;
							p2 -= 0x80;
							seven2eight(&p1, &p2);
							tmpi = zen2han(p1, p2, &outbuf[wcnt]);
							if (tmpi == 0)	/* could not convert, put original */
							{
								wcnt += writebuf(c1, c2, &outbuf[wcnt]);
							}
							else if (tmpi == 1)	/* hankata and no dakuten, no maru */
							{
								c1 = outbuf[wcnt];
								wcnt += writebuf(0x8e, c1, &outbuf[wcnt]);
							}
							else if (tmpi == 2)	/* hankata and dakuten, maru */
							{
								c1 = outbuf[wcnt];
								c2 = outbuf[wcnt+1];
								wcnt += writebuf(0x8e, c1, &outbuf[wcnt]);
								wcnt += writebuf(0x8e, c2, &outbuf[wcnt]);
							}
						}
						else if (IS_EUC_BYTE(p2))	/* EUC 2nd byte */
						{
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
						}
					}
					else	/* EUC 1st byte */
					{
						if (IS_EUC_BYTE(p2))	/* EUC 2nd byte */
						{
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
						}
					}
				}
				else if (p1 == 0x8e)	/* EUC Hankaku Katakana 1st byte */
				{
					if (rcnt >= rlen || rcnt >= maxlen)
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
					if (IS_HANKATA(p2))	/* EUC Hankaku Katakana 2nd byte */
					{
						wcnt += writebuf(p1, p2, &outbuf[wcnt]);
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

/* EUC zenkaku to Shift-JIS hankaku */
LONG __saveds __asm
euczen2shifthan(register __a0 struct JConversionCodeSet *jcc,
                register __a1 UBYTE *inbuf,
                register __a2 UBYTE *outbuf,
                register __d0 LONG length)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen = strlen(inbuf);
	LONG maxlen = length;

	if (maxlen == -1)
		maxlen = rlen;

	D(bug("euczen2shifthan:rlen = %ld\n", rlen));

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
			goto HANKATA_SECONDBYTE_EUC_SJIS;
		}
	}

	while (rcnt < rlen && rcnt < maxlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
			case FF:
				break;
			default:
				if (IS_EUC_BYTE(p1))
				{
					if (rcnt >= rlen || rcnt >= maxlen)
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
					if (IS_EUC_KBYTE1(p1))	/* EUC Katakana 1st byte */
					{
						if (IS_EUC_KBYTE2(p2))	/* EUC Katakana 2nd byte */
						{
							LONG tmpi = 0;
							p1 -= 0x80;
							p2 -= 0x80;
							seven2eight(&p1, &p2);
							tmpi = zen2han(p1, p2, &outbuf[wcnt]);
							if (tmpi == 0)	/* could not convert, put original */
								wcnt += writebuf(p1, p2, &outbuf[wcnt]);
							else
								wcnt += tmpi;
						}
						else if (IS_EUC_BYTE(p2))	/* EUC 2nd byte */
						{
							p1 -= 0x80;
							p2 -= 0x80;
							seven2eight(&p1, &p2);
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
						}
					}
					else	/* EUC 1st byte */
					{
						if (IS_EUC_BYTE(p2))	/* EUC 2nd byte */
						{
							p1 -= 0x80;
							p2 -= 0x80;
							seven2eight(&p1, &p2);
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
						}
					}
				}
				else if (p1 == 0x8e)	/* EUC Hankaku Katakana 1st byte */
				{
					if (rcnt >= rlen || rcnt >= maxlen)
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
					if (IS_HANKATA(p2))	/* EUC Hankaku Katakana 2nd byte */
					{
						outbuf[wcnt++] = p2;
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

/* Shift-JIS zenkaku to EUC hankaku */
LONG __saveds __asm
shiftzen2euchan(register __a0 struct JConversionCodeSet *jcc,
                register __a1 UBYTE *inbuf,
                register __a2 UBYTE *outbuf,
                register __d0 LONG length)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen = strlen(inbuf);
	LONG maxlen = length;

	if (maxlen == -1)
		maxlen = rlen;

	D(bug("shiftzen2shifthan:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_SJIS_EUC;
		}
	}

	while (rcnt < rlen && rcnt < maxlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
#if 0
			case CR:
			case LF:
				outbuf[wcnt++] = LF;
				break;
#endif
			case NULL:
			case FF:
				break;
			default:
				if (IS_SJIS_BYTE1(p1))	/* Shift-JIS 1st byte */
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* assume continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* assume single string convert */
							break;
					}
SECONDBYTE_SJIS_EUC:
					p2 = inbuf[rcnt++];
					if (IS_SJIS_KBYTE1(p1))	/* Shift-JIS Katakana 1st byte */
					{
						if (IS_SJIS_KBYTE2(p2))	/* Shift-JIS Katakana 2nd byte */
						{
							LONG tmpi = 0;
							LONG c1 = p1, c2 = p2;
							tmpi = zen2han(p1, p2, &outbuf[wcnt]);
							if (tmpi == 0)	/* could not convert, put original */
							{
								eight2seven(&c1, &c2);
								wcnt += writebuf(c1+0x80, c2+0x80, &outbuf[wcnt]);
							}
							else if (tmpi == 1)	/* hankata and no dakuten, no maru */
							{
								c1 = outbuf[wcnt];
								wcnt += writebuf(0x8e, c1, &outbuf[wcnt]);
							}
							else if (tmpi == 2)	/* hankata and dakuten, maru */
							{
								c1 = outbuf[wcnt];
								c2 = outbuf[wcnt+1];
								wcnt += writebuf(0x8e, c1, &outbuf[wcnt]);
								wcnt += writebuf(0x8e, c2, &outbuf[wcnt]);
							}
						}
						else if (IS_SJIS_BYTE2(p2))	/* Shift-JIS 2nd byte */
						{
							eight2seven(&p1, &p2);
							wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
						}
					}
					else	/* Shift-JIS 1st byte */
					{
						if (IS_SJIS_BYTE2(p2))	/* Shift-JIS 2nd byte */
						{
							eight2seven(&p1, &p2);
							wcnt += writebuf(p1+0x80, p2+0x80, &outbuf[wcnt]);
						}
					}
				}
				else if (IS_HANKATA(p1))	/* Shift-JIS Hankaku Katakana */
				{
					wcnt += writebuf(0x8e, p1, &outbuf[wcnt]);
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

/* Shift-JIS zenkaku to Shift-JIS hankaku */
LONG __saveds __asm
shiftzen2shifthan(register __a0 struct JConversionCodeSet *jcc,
                  register __a1 UBYTE *inbuf,
                  register __a2 UBYTE *outbuf,
                  register __d0 LONG length)
{
	LONG p1, p2;
	LONG rcnt = 0;
	LONG wcnt = 0;
	LONG rlen = strlen(inbuf);
	LONG maxlen = length;

	if (maxlen == -1)
		maxlen = rlen;

	D(bug("shiftzen2shifthan:rlen = %ld\n", rlen));

	if (length == -1)
	{
		if (jcc->jcc_FirstByte)
		{
			pop(&p1, &jcc->jcc_FirstByte);
			goto SECONDBYTE_SJIS_SJIS;
		}
	}

	while (rcnt < rlen && rcnt < maxlen)
	{
		p1 = inbuf[rcnt++];
		switch (p1)
		{
#if 0
			case CR:
			case LF:
				outbuf[wcnt++] = LF;
				break;
#endif
			case NULL:
			case FF:
				break;
			default:
				if (IS_SJIS_BYTE1(p1))	/* Shift-JIS 1st byte */
				{
					if (rcnt >= rlen)
					{
						if (length == -1)	/* assume continuous string */
						{
							jcc->jcc_FirstByte = p1;
							break;
						}
						else			/* assume single string convert */
							break;
					}
SECONDBYTE_SJIS_SJIS:
					p2 = inbuf[rcnt++];
					if (IS_SJIS_KBYTE1(p1))	/* Shift-JIS Katakana 1st byte */
					{
						if (IS_SJIS_KBYTE2(p2))	/* Shift-JIS Katakana 2nd byte */
						{
							LONG tmpi = 0;
							tmpi = zen2han(p1, p2, &outbuf[wcnt]);
							if (tmpi == 0)	/* could not convert, put original */
								wcnt += writebuf(p1, p2, &outbuf[wcnt]);
							else
								wcnt += tmpi;
						}
						else if (IS_SJIS_BYTE2(p2))	/* Shift-JIS 2nd byte */
						{
							wcnt += writebuf(p1, p2, &outbuf[wcnt]);
						}
					}
					else	/* Shift-JIS 1st byte */
					{
						if (IS_SJIS_BYTE2(p2))	/* Shift-JIS 2nd byte */
						{
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

/* half size to full size */
LONG __saveds __asm
LIBHanToZen(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
            register __d0 LONG incode,
            register __d1 LONG outcode,
            register __d2 LONG length)
{
	LONG wcnt = -1;

	jcc->jcc_WhatInputCode = incode;
	jcc->jcc_WhatOutputCode = outcode;

	switch(incode)
	{
		case CTYPE_EUC:
			switch(outcode)
			{
				case CTYPE_EUC:
					wcnt = euc2euc(jcc, inbuf, outbuf, length, 0);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_SJIS:
					wcnt = euc2shift(jcc, inbuf, outbuf, length, 0);
					outbuf[wcnt] = 0x00;
					break;
				default:
					break;
			}
			break;
		case CTYPE_SJIS:
			switch(outcode)
			{
				case CTYPE_EUC:
					wcnt = shift2euc(jcc, inbuf, outbuf, length, 0, 0, 0);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_SJIS:
					wcnt = shift2shift(jcc, inbuf, outbuf, length, 0, 0, 0);
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

/* full size to half size */
LONG __saveds __asm
LIBZenToHan(register __a0 struct JConversionCodeSet *jcc,
            register __a1 UBYTE *inbuf,
            register __a2 UBYTE *outbuf,
            register __d0 LONG incode,
            register __d1 LONG outcode,
            register __d2 LONG length)
{
	LONG wcnt = -1;

	jcc->jcc_WhatInputCode = incode;
	jcc->jcc_WhatOutputCode = outcode;

	switch(incode)
	{
		case CTYPE_EUC:
			switch(outcode)
			{
				case CTYPE_SJIS:
					wcnt = euczen2shifthan(jcc, inbuf, outbuf, length);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_EUC:
					wcnt = euczen2euchan(jcc, inbuf, outbuf, length);
					outbuf[wcnt] = 0x00;
					break;
				default:
					break;
			}
			break;
		case CTYPE_SJIS:
			switch(outcode)
			{
				case CTYPE_SJIS:
					wcnt = shiftzen2shifthan(jcc, inbuf, outbuf, length);
					outbuf[wcnt] = 0x00;
					break;
				case CTYPE_EUC:
					wcnt = shiftzen2euchan(jcc, inbuf, outbuf, length);
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
