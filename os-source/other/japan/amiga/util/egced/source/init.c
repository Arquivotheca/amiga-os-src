#ifndef lint
static char     rcsid[]="$Id: init.c,v 3.4.1.1 91/06/25 21:07:20 katogi GM Locker: katogi $";
#endif
/*
*
*     init.c  :   initialize functines
*
*        bfMemInit()    : memory intialize routine
*        bfFileInit()   : file intialize routine
*        vfCpyrt()      : show copyright routine
*        vfSetMonitor() : screen flame setting routine
*
*        Copyright(C) 1991 ERGOSOFT Corp. TOKIO Labo.
*        All Rights Reserved
*
*        Designed    by        H.Yanata  1991.Apr.04
*        Coded       by        H.Yanata  1991.Apr.04
*		 Modified    by        T.Koide   1992.09.14
*
*/


#include    <stdio.h>
#include    <string.h>

#include    "egced.h"
#include    "crt.h"

#ifndef    UNIX
#include    <stdlib.h>
#endif

#ifndef SEEK_END
#define SEEK_END 2
#endif
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif


/*------------------------------------------------------*/
/*    get memory                                        */
/*------------------------------------------------------*/
BOOL    bfMemInit(sDAT)
    sDATASET    *sDAT;        /* pointer to data buffer */
{
       REG        i, j;
static char    *cErr = "  ������������.�������[��J����.";

    CSRXY(3, 6); printf("�������m���...");
    /*---------    getting memory for data buffer control pointer ----------*/
    sDAT->sRecBuf = (sREC48**)calloc((sDAT->wIndexMax+ADDMAX),sizeof(sREC48*));

    /*---------- not enough memory ----------*/
    if (sDAT->sRecBuf == NULL){
        CSRXY(3, 7); puts(cErr);
        free(sDAT->sRecBuf);
        return    FALSE;
    } else{
        /*---------- fetting memory for data buffer ----------*/
        for (i = 0; i < sDAT->wIndexMax+ADDMAX; i++){
            sDAT->sRecBuf[i] = (sREC48*)calloc(1, sDAT->iDicLen);
            if (!((i + 1) % 1000)) fprintf(stdout, "#");
            /*----------    not enough memory    ----------*/
            if (sDAT->sRecBuf[i] == NULL){
                CSRXY(3, 7); puts(cErr);
                for (j = 0; j < i; j++)
                    free(sDAT->sRecBuf[j]);
                free(sDAT->sRecBuf);
                return    FALSE;
            }
        }
    }

	sDAT->IndexNum = (WORD *)calloc(sDAT->wIndexMax+ADDMAX,sizeof(WORD));
	if(sDAT->IndexNum == NULL) {
        CSRXY(3, 7); puts(cErr);
		free(sDAT->IndexNum);
		return FALSE;
	}

    /*----------    memory initialize success    ----------*/
    return        TRUE;
}

/*------------------------------------------------------------------*/
/*	check option, initialize data of file and make bakup			*/
/*------------------------------------------------------------------*/
BOOL	bfFileInit(argc, argv, sDAT)
	int			argc;
	char		**argv;
	sDATASET	*sDAT;			/*	pointer to data buffer		*/
{
	FILE	*fp_bak;			/*	for bak up					*/
	long	lFileStat;			/*	file length					*/
	UBYTE	ubBakup[80];		/*	bak up file name			*/
	UBYTE	ubTemp[100];			/*	temporary data buffer		*/
	REG		i;

	/*---------- usage ----------*/
	if (argc < 3){
		fprintf(stdout, "\n  usage : egced <file> [size] [option]\n\n");
		fprintf(stdout, "  [size]   = -1 --> 32byte data\n");
		fprintf(stdout, "             -2 --> 48byte data\n");
		fprintf(stdout, "             -3 --> 80byte data\n");
		fprintf(stdout, "  [option] = -b --> bakup\n");
		fprintf(stdout, "             -r --> read only\n");
		return	FALSE;
	}

	/*---------- set file name ---------*/
	strcpy(sDAT->ubFile, argv[1]);
	
	if ((sDAT->fp = fopen(sDAT->ubFile, "rb")) == NULL){
		fprintf(stderr, "\n  NO FILE  : %s\n", sDAT->ubFile);
		return	FALSE;
	}
	
	/*---------- get file length ----------*/
	fseek(sDAT->fp, 0L, SEEK_END);
	lFileStat = ftell(sDAT->fp);
	fseek(sDAT->fp, 0L, SEEK_SET);

	/*-----------datasize check-------------*/
	if(!strcmp(argv[2] ,"-1")) {
		sDAT->iKnjLen = KNJSIZ32;
		sDAT->iDicLen = DICSIZ32;
		sDAT->wIndexMax = (WORD)(lFileStat / DICSIZ32);
	} else if(!strcmp(argv[2] ,"-2")) {
		sDAT->iKnjLen = KNJSIZ48;
		sDAT->iDicLen = DICSIZ48;
		sDAT->wIndexMax = (WORD)(lFileStat / DICSIZ48);
	} else if(!strcmp(argv[2] ,"-3")) {
		sDAT->iKnjLen = KNJSIZ80;
		sDAT->iDicLen = DICSIZ80;
		sDAT->wIndexMax = (WORD)(lFileStat / DICSIZ80);
	} else {
		fprintf(stderr,"\n  NO DATASIZE OPTION: %s\n",argv[2]);
		return FALSE;
	}
    
	/*------------option check------------*/
	if (!strcmp(argv[3] ,"-r") || !strcmp(argv[4], "-r"))
		sDAT->iReadOnly = YES;
	else
		sDAT->iReadOnly = NO;

	if (!strcmp(argv[3] ,"-b") || !strcmp(argv[4], "-b")){
		fprintf(stdout, "  �������...\n\n");
		strcpy(ubBakup, "egcedbak");
		i = 0;
		while(argv[1][i] != '.') i++;
		strcat(ubBakup, sDAT->ubFile + i - 1);
		if ((fp_bak = fopen(ubBakup, "wb")) == NULL){
			fprintf(stderr, "\n  NO FILE  : %s\n", sDAT->ubFile);
			return	FALSE;
		}
		while (fread(ubTemp, sDAT->iDicLen, 1, sDAT->fp))
			fwrite(ubTemp, sDAT->iDicLen, 1, fp_bak);

		fclose(fp_bak);
	}
	fclose(sDAT->fp);

	/*---------file init sucsess-----*/
	return	TRUE;
}

/*------------------------------------------------------*/
/*    disp copyright message                            */
/*------------------------------------------------------*/

VOID    vfCpyrt()
{
#ifndef		UNIX
    CSRXY(3, 3);
    puts("ERGOSOFT  (R)  Sequential Dictionary Edit Utility Version 2.01");
#else
    CSRXY(3, 3);
    puts("ERGOSOFT  (R)  Sequential Dictionary Edit Utility Version 3.42");
#endif
    CSRXY(3, 4);
    puts("Copyright (C)  ERGOSOFT Corp 1991. All rights reserved.");
}

/*------------------------------------------------------*/
/*    main flame setting                                */
/*------------------------------------------------------*/
VOID    vfSetMonitor()
{
    REG        i;
#ifndef        PC9801
    static char    *ubTitleBar[] = {
        " Index ",
        "   �  �   ",
        "   �  ��   ",
        "  Status  ",
        "File  : ",
        "Size  : ",
        "Words : ",
        "Page  : ",
        " Control ",
        " Message ",
        "   �i����   ",
    };

    static char    *ubErgo[] = {
        "EGCED 3.42", "ERGOSOFT",
    };

    static char    *ubCtrl[] = {
        "CTRL + Q : PAGE UP",
        "CTRL + Z : PAGE DOWN",
        "[RET]    : COMMAND",
        "[TAB]    : HINSHI EDIT",
    };
    
    static char    *ubItem[] = {
        "��l�n��l�����������P���A�`�`��T�U��J�K�T�^�i�o�}����",
        "��������  ������    �  ���e������i�s�s�s�s�s�s�s�s�s",
        "�P����Q�������������������������s�������T�T�T�T�T�T�T�T�T",
    };
    static char        cVertFlm = '|';

    static char        cHorizLongFlm []= "-------------------------------------------------------------------------------";
    static char        cHorizShortFlm []= "------------------------";

    CSRXY(1,2); puts(cHorizLongFlm);
    CSRXY(1,17); puts(cHorizLongFlm);
    CSRXY(1,19); puts(cHorizLongFlm);
    CSRXY(55,6); puts(cHorizShortFlm);
    CSRXY(55,12); puts(cHorizShortFlm);
    for (i = 1; i < 15; i++){
        CSRXY(54, 2 + i);
        putchar(cVertFlm);
    }
    CSRXY(63, 2); puts(ubTitleBar[3]);
    CSRXY(63, 12); puts(ubTitleBar[8]);
    CSRXY(63, 6); puts(ubTitleBar[9]);
    NORMCSR();
    for (i = 0; i < 3; i++){
        CSRXY(3 , 20 + i);
        puts(ubItem[i]);
    }
    for (i = 0; i < 4; i++){
        CSRXY(57, 13 + i);
        puts(ubCtrl[i]);
    }
    for (i = 0; i < 3; i++){
        CSRXY(57 , 3 + i);
        puts(ubTitleBar[4 + i]);
    }
    CSRXY(68, 21); puts(ubErgo[0]);
    CSRXY(68, 22); puts(ubErgo[1]);
}
    
#else
	static UBYTE	*ubTitleBar[] = {
		" Index ",
		"   �  �   ",
		"   �  ��   ",
		" Status ",
		"File  : ",
		"Size  : ",
		"Words : ",
		"Page  : ",
		" Control ",
		" Message ",
		"   �i����   ",
	};

	static UBYTE	*ubErgo[] = {
		"EGCED 2.01", "ERGOSOFT",
	};

	static UBYTE	*ubCtrl[] = {
		"CTRL + Q : PAGE UP",
		"CTRL + Z : PAGE DOWN",
		"[RET]    : COMMAND",
		"[TAB]    : HINSHI EDIT",
	};
	
	static UBYTE	*ubItem[] = {
		"��l�n��l�����������P���A�`�`��T�U��J�K�T�^�i�o�}����",
		"��������  ������    �  ���e������i�s�s�s�s�s�s�s�s�s",
		"�P����Q�������������������������s�������T�T�T�T�T�T�T�T�T",
	};

	static UBYTE	*ubNoEdit[] = {
		"����P",
		"    �",
		"�����",
	};

	static	char	*ubBoxFlm[] = {
		"�", "�", "�", "�", "��", "��","�","�","�","�"
	};
	static char		cHorizLongFlm []= "������������������������������������������������������������������������������";
	static char		cHorizShortFlm []= "����������������������������";
	static char		cVertFlm[] = "��";
	
	for (i = 1; i < 15; i++){
		DrawText(53, 2 + i, cVertFlm, 2, EGLGREEN);
	}
	for (i = 1; i < 23; i++){
		DrawText(1, 2 + i, cVertFlm, 2, EGLGREEN);
		DrawText(79, 2 + i, cVertFlm, 2, EGLGREEN);
	}
	DrawText(53, 6, cHorizShortFlm, strlen(cHorizShortFlm), EGLGREEN);
	DrawText(53, 12, cHorizShortFlm, strlen(cHorizShortFlm), EGLGREEN);
	DrawText(1, 2, cHorizLongFlm, strlen(cHorizLongFlm), EGLGREEN);
	DrawText(1, 17, cHorizLongFlm, strlen(cHorizLongFlm), EGLGREEN);
	DrawText(1, 19, cHorizLongFlm, strlen(cHorizLongFlm), EGLGREEN);
	DrawText(1, 24, cHorizLongFlm, strlen(cHorizLongFlm), EGLGREEN);
	DrawText(53, 6, ubBoxFlm[6], strlen(ubBoxFlm[6]), EGLGREEN);
	DrawText(79, 6, ubBoxFlm[7], strlen(ubBoxFlm[7]), EGLGREEN);
	DrawText(53, 12, ubBoxFlm[6], strlen(ubBoxFlm[6]), EGLGREEN);
	DrawText(79, 12, ubBoxFlm[7], strlen(ubBoxFlm[7]), EGLGREEN);
	DrawText(79, 17, ubBoxFlm[7], strlen(ubBoxFlm[7]), EGLGREEN);
	DrawText(1, 17, ubBoxFlm[6], strlen(ubBoxFlm[6]), EGLGREEN);
	DrawText(79, 19, ubBoxFlm[7], strlen(ubBoxFlm[7]), EGLGREEN);
	DrawText(1, 19, ubBoxFlm[6], strlen(ubBoxFlm[6]), EGLGREEN);
	DrawText(1, 2, ubBoxFlm[0], strlen(ubBoxFlm[0]), EGLGREEN);
	DrawText(79, 2, ubBoxFlm[1], strlen(ubBoxFlm[1]), EGLGREEN);
	DrawText(1, 24, ubBoxFlm[2], strlen(ubBoxFlm[2]), EGLGREEN);
	DrawText(79, 24, ubBoxFlm[3], strlen(ubBoxFlm[3]), EGLGREEN);
	DrawText(53, 2, ubBoxFlm[9], strlen(ubBoxFlm[9]), EGLGREEN);
	DrawText(53, 17, ubBoxFlm[8], strlen(ubBoxFlm[8]), EGLGREEN);
	
	DrawText(63,2,ubTitleBar[3],strlen(ubTitleBar[3]),EGLWHITE);
	DrawText(64,12,ubTitleBar[8],strlen(ubTitleBar[8]),EGLWHITE);
	DrawText(64, 6, ubTitleBar[9],strlen(ubTitleBar[9]),EGLWHITE);
	
	for (i = 0; i < 3; i++)
	DrawText(3, 20 + i, ubItem[i], strlen(ubItem[i]), EGLWHITE);
	for (i = 0; i < 3; i++)
	DrawText(23, 20 + i, ubNoEdit[i], strlen(ubNoEdit[i]), EGLCYAN);
	for (i = 0; i < 4; i++)
	DrawText(56, 13 + i, ubCtrl[i], strlen(ubCtrl[i]), EGLCYAN);
	for (i = 0; i < 3; i++)
	DrawText(56, 3 + i, ubTitleBar[4 + i], strlen(ubTitleBar[4 + i]), EGLCYAN);

	for (i = 0; i < 12; i += 2){
		DrawText(65 + i, 20, ubBoxFlm[5], 2, EGLCYAN);
		DrawText(65 + i, 23, ubBoxFlm[5], 2, EGLCYAN);
	}

	for (i = 0; i < 3; i++){
		DrawText(65, 21 + i, ubBoxFlm[4], 2, EGLCYAN);
		DrawText(77, 21 + i, ubBoxFlm[4], 2, EGLCYAN);
	}

	DrawText(65, 20, ubBoxFlm[0], 2, EGLCYAN);
	DrawText(77, 20, ubBoxFlm[1], 2, EGLCYAN);
	DrawText(65, 23, ubBoxFlm[2], 2, EGLCYAN);
	DrawText(77, 23, ubBoxFlm[3], 2, EGLCYAN);
	DrawText(67, 21, ubErgo[0], strlen(ubErgo[0]), EGLCYAN);
	DrawText(67, 22, ubErgo[1], strlen(ubErgo[1]), EGLCYAN);
}

#endif

