	TTL    '$Header: ch1.asm,v 36.0 89/05/25 16:46:24 eric Exp $'
**********************************************************************
*								     *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.	     *
*   No part of this program may be reproduced, transmitted,	     *
*   transcribed, stored in retrieval system, or translated into	     *
*   any language or computer language, in any form or by any	     *
*   means, electronic, mechanical, magnetic, optical, chemical,	     *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030				     *
*								     *
**********************************************************************
*
*   CH1:  SWISS KeyMap #1	(A2000)
*
*   Source Control
*   --------------
*   $Header: ch1.asm,v 36.0 89/05/25 16:46:24 eric Exp $
*
*   $Locker:  $
*
*   $Log:	ch1.asm,v $
*   Revision 36.0  89/05/25  16:46:24  eric
*   Initial Submission
*   
*   
*
**********************************************************************

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"devices/keymap.i"

CH1Node:
	DC.L	0,0		; ln_Succ, ln_Pred
	DC.B	0,0		; ln_Type, ln_Pri
	DC.L	CH1Name		; ln_Name
	DC.L	CH1LowMapType
	DC.L	CH1LowMap
	DC.L	CH1LCapsable
	DC.L	CH1LRepeatable
	DC.L	CH1HighMapType
	DC.L	CH1HighMap
	DC.L	CH1HCapsable
	DC.L	CH1HRepeatable


*------ Key Translation Table ------------------------------------------------
*
*
*   00	01  02	03  04	05  06	07  08	09  0A	0B  0C	0D  5A  5B  5C  5D
*	  10  11  12  13  14  15  16  17  18  19  1A  1B    3D	3E  3F  4A
*	   20  21  22  23  24  25  26  27  28  29  2A  2B   2D	2E  2F  5E
*	 30  31	 32  33	 34  35	 36  37	 38  39	 3A	    1D  1E  1F  43
*							    0F      3C
*
*   `~	1+  2"	3*  4c	5%  6&	7/  8(	9) 0=  '?  ^`  \|   [{  ]}  /   *
*	  qQ  wW  eE  rR  tT  zZ  uU  iI  oO  pP  eu  "!    7   8   9   -
*	   aA  sS  dD  fF  gG  hH  jJ  kK  lL  eo  aa  $L   4   5   6   +
*	     yY	 xX  cC	 vV  bB	 nN  mM	 ,;  .:	 -_	    1   2   3   enter
*							    0       .
*
*-----------------------------------------------------------------------------
*
*	40	Space
*	41	Backspace
*	42	Tab
*	43	Enter
*	44	Return
*	45	Escape
*	46	Delete
*	4A	Numeric Pad -
*	4C	Cursor Up
*	4D	Cursor Down
*	4E	Cursor Forward
*	4F	Cursor Backward
*	
*	50-59	Function keys F1-F10
*	5A	Numeric Pad [{	(A2000)
*	5B	Numeric Pad ]}  (A2000)
*	5C	Numeric Pad /	(A2000)
*	5D	Numeric Pad *	(A2000)
*	5E	Numeric Pad +	(A2000)
*	5F	Help
*	
*	60	Left Shift
*	61	Right Shift
*	62	Caps Lock
*	63	Control
*	64	Left Alt
*	65	Right Alt
*	66	Left Amiga
*	67	Right Amiga
*	
*	68	Left Mouse Button (not converted)
*	69	Right Mouse Button (not converted)
*	6A	Middle Mouse Button (not converted)
*	
*---------------------------------------------------------------------


CH1LCapsable:
	DC.B	%00000000	; 07 - 00
	DC.B	%00000000	; 0F - 08
	DC.B	%11111111	; 17 - 10
	DC.B	%00000011	; 1F - 18
	DC.B	%11111111	; 27 - 20
	DC.B	%00000001	; 2F - 28
	DC.B	%11111110	; 37 - 30
	DC.B	%00000000	; 3F - 38

CH1HCapsable:
	DC.B	%00000000	; 47 - 40
	DC.B	%00000000	; 4F - 48
	DC.B	%00000000	; 57 - 50
	DC.B	%00000000	; 5F - 58
	DC.B	%00000000	; 67 - 60
	DC.B	%00000000	; 6F - 68
	DC.B	%00000000	; 77 - 70


CH1LRepeatable:
	DC.B	%11111111	; 07 - 00
	DC.B	%10111111	; 0F - 08
	DC.B	%11111111	; 17 - 10
	DC.B	%11101111	; 1F - 18
	DC.B	%11111111	; 27 - 20
	DC.B	%11101111	; 2F - 28
	DC.B	%11111111	; 37 - 30
	DC.B	%11110111	; 3F - 38

CH1HRepeatable:
	DC.B	%01000111	; 47 - 40
	DC.B	%11110100	; 4F - 48
	DC.B	%11111111	; 57 - 50
	DC.B	%01111111	; 5F - 58
	DC.B	%00000000	; 67 - 60
	DC.B	%00000000	; 6F - 68
	DC.B	%00000000	; 77 - 70



CH1LowMapType:
	DC.B	KCF_DEAD+KC_VANILLA		; $00
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_SHIFT+KCF_ALT

	DC.B	KCF_SHIFT+KCF_ALT		; $08
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL

	DC.B	KC_VANILLA			; $10
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA

	DC.B	KCF_DEAD+KC_VANILLA		; $18
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL

	DC.B	KCF_DEAD+KC_VANILLA		; $20
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA

	DC.B	KC_VANILLA			; $28
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL

	DC.B	KCF_SHIFT+KCF_ALT		; $30
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KC_VANILLA

	DC.B	KCF_SHIFT+KCF_ALT		; $38
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL


CH1HighMapType:
	DC.B	KCF_DEAD+KCF_ALT		; $40
	DC.B	KC_NOQUAL
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KC_NOQUAL
	DC.B	KCF_CONTROL
	DC.B	KCF_ALT
	DC.B	KC_NOQUAL
	DC.B	KCF_NOP

	DC.B	KCF_NOP				; $48
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KCF_NOP
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT

	DC.B	KCF_STRING+KCF_SHIFT		; $50
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT

	DC.B	KCF_STRING+KCF_SHIFT		; $58
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_SHIFT+KCF_CONTROL
	DC.B	KCF_SHIFT+KCF_CONTROL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KCF_STRING

	DC.B	KCF_NOP				; $60
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP

	DC.B	KCF_NOP				; $68
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP

	DC.B	KCF_NOP				; $70
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP



CH1LowMap:
	DC.L	key00		; `, dead ~ / Vanilla			$00
	DC.B	'!',$B9,'+','1'	; 1, +, super 1, !
	DC.B	'@',$B2,'"','2'	; 2, ", super 2, @ / Vanilla
	DC.B	'#',$B3,'*','3'	; 3, *, super 3, #
	DC.B	$B0,$A2,$E7,'4'	; 4, c, cents, degree
	DC.B	$A7,$BC,'%','5'	; 5, %, 1/4, squiggle thing
	DC.B	'^',$BD,'&','6'	; 6, &, 1/2, ^ / Vanilla
	DC.B	'&',$BE,'/','7'	; 7, /, 3/4, &

	DC.B	'*',$B7,'(','8'	; 8, (, bullet, *			$08
	DC.B	'(',$AB,')','9'	; 9, ), <<, (
	DC.B	')',$BB,'=','0'	; 0, =, >>, )
	DC.B	'_','-','?',$27	; ', ?, -, _ / Vanilla
	DC.L	key0C		; dead ^, dead `, =, dead '
	DC.B	'|','\','|','\'	; \, | / Vanilla
	DC.L	0
	DC.B	0,0,0,'0'	; 0

	DC.B	$C5,$E5,'Q','q'	; q, Q, dot a, dot A / Vanilla		$10
	DC.B	$B0,$B0,'W','w'	; w, W, dot, dot / Vanilla
	DC.L	key12		; e, E, (c), (c) / Vanilla
	DC.B	$AE,$AE,'R','r'	; r, R, (r), (r) / Vanilla
	DC.B	$DE,$FE,'T','t'	; t, T, thorn, THORN / Vanilla
	DC.B	$A5,$A4,'Z','z' ; z, Z, IMS, Yen / Vanilla
	DC.L	key16		; u, U, micro, micro / Vanilla
	DC.L	key17		; i, I, inverted !, | / Vanilla

	DC.L	key18		; o, O, slash o, slash O  / Vanilla	$18
	DC.B	$B6,$B6,'P','p'	; p, P, paragraph, paragraph / Vanilla
	DC.B	'{','[',$FC,$E8	; grave-e, umlaut-u, [, { / Vanilla
	DC.L	key1B		; dead ", dead ~, ], } / Vanilla
	DC.L	0		; NOP
	DC.B	0,0,0,'1'	; 1
	DC.B	0,0,0,'2'	; 2
	DC.B	0,0,0,'3'	; 3

	DC.L	key20		; a, A, ae, AE  / Vanilla		$20
	DC.B	$A7,$DF,'S','s'	; s, S, sharp s, section / Vanilla
	DC.B	$D0,$F0,'D','d'	; d, D, bar d, bar D / Vanilla
	DC.L	key23		; f, F, dead ' / Vanilla
	DC.L	key24		; g, G, dead ` / Vanilla
	DC.L	key25		; h, H, dead ^ / Vanilla
	DC.L	key26		; j, J, dead ~ / Vanilla
	DC.L	key27		; k, K, dead " / Vanilla

	DC.B	$A3,$A3,'L','l'	; l, L, pound, pound / Vanilla		$28
	DC.B	':',';',$F6,$E9	; aigu-e, umlaut-o, ;, :
	DC.B	'"',$27,$E4,$E0	; grave-a, umlaut-a, ', "
	DC.B	0,0,$A3,'$'	; $, pound
	DC.L	0		; NOP
	DC.B	0,0,0,'4'	; 4
	DC.B	0,0,0,'5'	; 5
	DC.B	0,0,0,'6'	; 6

	DC.B	$BB,$AB,'>','<'	; <, >, <<, >>				$30
	DC.L    key31           ; y, Y, IMS, Yen / Vanilla
	DC.B	$F7,$D7,'X','x'	; x, X, times, divide / Vanilla
	DC.B	$C7,$E7,'C','c'	; c, C, c cedilla, C cedilla / Vanilla
	DC.B	$AA,$AA,'V','v'	; v, V, female ordinal, / Vanilla
	DC.B	$BA,$BA,'B','b'	; b, B, male ordinal / Vanilla
	DC.L	key36		; n, N, SHY, overbar / Vanilla
	DC.B	$BF,$B8,'M','m'	; m, M, cedilla, inverted ? / Vanilla

	DC.B	'<',',',';',','	; ,, ;, ,, <				$38
	DC.B	'>','.',':','.'	; ., :, ., >
	DC.B	'?','/','_','-'	; -, _, /, ?
	DC.L	0		; NOP
	DC.B	0,0,0,'.'	; .
	DC.B	0,0,0,'7'	; 7
	DC.B	0,0,0,'8'	; 8
	DC.B	0,0,0,'9'	; 9


CH1HighMap:
	DC.L	key40							$40
	DC.B	0,0,0,$08
	DC.L	key42
	DC.B	0,0,0,$0D
	DC.B	0,0,$0A,$0D
	DC.B	0,0,$9B,$1B
	DC.B	0,0,0,$7F
	DC.L	0

	DC.L	0							$48
	DC.L	0
	DC.B	0,0,0,'-'
	DC.L	0
	DC.L	key4C
	DC.L	key4D
	DC.L	key4E
	DC.L	key4F

	DC.L	key50							$50
	DC.L	key51
	DC.L	key52
	DC.L	key53
	DC.L	key54
	DC.L	key55
	DC.L	key56
	DC.L	key57

	DC.L	key58							$58
	DC.L	key59
	DC.B	$1B,$1B,'{','['	; numeric pad [, {
	DC.B	$1D,$1D,'}',']'	; numeric pad ], }
	DC.B	0,0,0,'/'	; numeric pad /
	DC.B	0,0,0,'*'	; numeric pad *
	DC.B	0,0,0,'+'	; numeric pad +
	DC.L	key5F		

	DC.L	0							$60
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0

	DC.L	0							$68
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0

	DC.L	0							$70
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0


;------ possible dead keys

key00:
	DC.B	0,'`',DPF_DEAD,4		; `, dead ~
	DC.B	0,'`',0,'~'			; `, ~
	DC.B	0,$60,0,$7E,0,$E0,$FE		; control translation

key0C:
	DC.B	DPF_DEAD,3,DPF_DEAD,2		; dead ^, dead `
	DC.B	0,'=',DPF_DEAD,1		; =, dead '

key1B:
	DC.B	DPF_DEAD,5,0,'!'		; dead umlaut, !
	DC.B	0,']',0,'}'			; ], }
	DC.B	0,$1D,0,$1D,0,$9D,0,$9D		; control translation

key23:
	DC.B	0,'f',0,'F'			; f, F
	DC.B	DPF_DEAD,1,DPF_DEAD,1		; dead ', dead '
	DC.B	0,$06,0,$06,0,$86,0,$86		; control translation

key24:
	DC.B	0,'g',0,'G'			; g, G
	DC.B	DPF_DEAD,2,DPF_DEAD,2		; dead `, dead `
	DC.B	0,$07,0,$07,0,$87,0,$87		; control translation

key25:
	DC.B	0,'h',0,'H'			; h, H
	DC.B	DPF_DEAD,3,DPF_DEAD,3		; dead ^, dead ^
	DC.B	0,$08,0,$08,0,$88,0,$88		; control translation
key26:
	DC.B	0,'j',0,'J'			; j, J
	DC.B	DPF_DEAD,4,DPF_DEAD,4		; dead ~, dead ~
	DC.B	0,$0A,0,$0A,0,$8A,0,$8A		; control translation

key27:
	DC.B	0,'k',0,'K'			; k, K
	DC.B	DPF_DEAD,5,DPF_DEAD,5		; dead ", dead "
	DC.B	0,$0B,0,$0B,0,$8B,0,$8B		; control translation


;------ deadable keys (modified by dead keys)

key12:						; e, E, (c), (c)
	DC.B	DPF_MOD,key12u-key12,DPF_MOD,key12s-key12
	DC.B	0,$A9,0,$A9
	DC.B	0,$05,0,$05,0,$85,0,$85	; control translation
key12u:
	DC.B	'e',$E9,$E8,$EA,'e',$EB
key12s:
	DC.B	'E',$C9,$C8,$CA,'E',$CB

key16:						; u, U, micro, micro
	DC.B	DPF_MOD,key16u-key16,DPF_MOD,key16s-key16
	DC.B	0,$B5,0,$B5
	DC.B	0,$15,0,$15,0,$95,0,$95	; control translation
key16u:
	DC.B	'u',$FA,$F9,$FB,'u',$FC
key16s:
	DC.B	'U',$DA,$D9,$DB,'U',$DC

key17:						; i, I, inverted !, broken |
	DC.B	DPF_MOD,key17u-key17,DPF_MOD,key17s-key17
	DC.B	0,$A1,0,$A6
	DC.B	0,$09,0,$09,0,$89,0,$89	; control translation
key17u:
	DC.B	'i',$ED,$EC,$EE,'i',$EF
key17s:
	DC.B	'I',$CD,$CC,$CE,'I',$CF

key18:						; o, O, bar o, bar O
	DC.B	DPF_MOD,key18u-key18,DPF_MOD,key18s-key18
	DC.B	0,$F8,0,$D8
	DC.B	0,$0F,0,$0F,0,$8F,0,$8F	; control translation
key18u:
	DC.B	'o',$F3,$F2,$F4,$F5,$F6
key18s:
	DC.B	'O',$D3,$D2,$D4,$D5,$D6

key20:						; a, A, ae, AE
	DC.B	DPF_MOD,key20u-key20,DPF_MOD,key20s-key20
	DC.B	0,$E6,0,$C6
	DC.B	0,$01,0,$01,0,$81,0,$81	; control translation
key20u:
	DC.B	'a',$E1,$E0,$E2,$E3,$E4
key20s:
	DC.B	'A',$C1,$C0,$C2,$C3,$C4

key31:						; y, Y, IMS, yen
	DC.B	DPF_MOD,key31u-key31,DPF_MOD,key31s-key31
	DC.B	0,$B1,0,$AC
	DC.B	0,$19,0,$19,0,$99,0,$99	; control translation
key31u:
	DC.B	'y',$FD,'y','y','y',$FF
key31s:
	DC.B	'Y',$DD,'Y','Y','Y','Y'

key36:						; n, N, SHY, overbar
	DC.B	DPF_MOD,key36u-key36,DPF_MOD,key36s-key36
	DC.B	0,$AD,0,$AF
	DC.B	0,$0E,0,$0E,0,$8E,0,$8E	; control translation
key36u:
	DC.B	'n','n','n','n',$F1,'n'
key36s:
	DC.B	'N','N','N','N',$D1,'N'

key40:						; space,,NBSP, and accents
	DC.B	DPF_MOD,key40d-key40,0,$A0
key40d:
	DC.B	' ',$B4,'`','^','~',$A8


;------ string keys 

key42:
	DC.B	key42ue-key42us,key42us-key42
	DC.B	key42se-key42ss,key42ss-key42
key42us:
	DC.B	$09
key42ue:
key42ss:
	DC.B	$9B,'Z'
key42se:

key4C:
	DC.B	key4Cue-key4Cus,key4Cus-key4C
	DC.B	key4Cse-key4Css,key4Css-key4C
key4Cus:
	DC.B	$9B,'A'
key4Cue:
key4Css:
	DC.B	$9B,'T'
key4Cse:

key4D:
	DC.B	key4Due-key4Dus,key4Dus-key4D
	DC.B	key4Dse-key4Dss,key4Dss-key4D
key4Dus:
	DC.B	$9B,'B'
key4Due:
key4Dss:
	DC.B	$9B,'S'
key4Dse:

key4E:
	DC.B	key4Eue-key4Eus,key4Eus-key4E
	DC.B	key4Ese-key4Ess,key4Ess-key4E
key4Eus:
	DC.B	$9B,'C'
key4Eue:
key4Ess:
	DC.B	$9B,' ','@'
key4Ese:

key4F:
	DC.B	key4Fue-key4Fus,key4Fus-key4F
	DC.B	key4Fse-key4Fss,key4Fss-key4F
key4Fus:
	DC.B	$9B,'D'
key4Fue:
key4Fss:
	DC.B	$9B,' ','A'
key4Fse:

key50:
	DC.B	key50ue-key50us,key50us-key50
	DC.B	key50se-key50ss,key50ss-key50
key50us:
	DC.B	$9B,'0~'
key50ue:
key50ss:
	DC.B	$9B,'10~'
key50se:

key51:
	DC.B	key51ue-key51us,key51us-key51
	DC.B	key51se-key51ss,key51ss-key51
key51us:
	DC.B	$9B,'1~'
key51ue:
key51ss:
	DC.B	$9B,'11~'
key51se:

key52:
	DC.B	key52ue-key52us,key52us-key52
	DC.B	key52se-key52ss,key52ss-key52
key52us:
	DC.B	$9B,'2~'
key52ue:
key52ss:
	DC.B	$9B,'12~'
key52se:

key53:
	DC.B	key53ue-key53us,key53us-key53
	DC.B	key53se-key53ss,key53ss-key53
key53us:
	DC.B	$9B,'3~'
key53ue:
key53ss:
	DC.B	$9B,'13~'
key53se:

key54:
	DC.B	key54ue-key54us,key54us-key54
	DC.B	key54se-key54ss,key54ss-key54
key54us:
	DC.B	$9B,'4~'
key54ue:
key54ss:
	DC.B	$9B,'14~'
key54se:

key55:
	DC.B	key55ue-key55us,key55us-key55
	DC.B	key55se-key55ss,key55ss-key55
key55us:
	DC.B	$9B,'5~'
key55ue:
key55ss:
	DC.B	$9B,'15~'
key55se:

key56:
	DC.B	key56ue-key56us,key56us-key56
	DC.B	key56se-key56ss,key56ss-key56
key56us:
	DC.B	$9B,'6~'
key56ue:
key56ss:
	DC.B	$9B,'16~'
key56se:

key57:
	DC.B	key57ue-key57us,key57us-key57
	DC.B	key57se-key57ss,key57ss-key57
key57us:
	DC.B	$9B,'7~'
key57ue:
key57ss:
	DC.B	$9B,'17~'
key57se:

key58:
	DC.B	key58ue-key58us,key58us-key58
	DC.B	key58se-key58ss,key58ss-key58
key58us:
	DC.B	$9B,'8~'
key58ue:
key58ss:
	DC.B	$9B,'18~'
key58se:

key59:
	DC.B	key59ue-key59us,key59us-key59
	DC.B	key59se-key59ss,key59ss-key59
key59us:
	DC.B	$9B,'9~'
key59ue:
key59ss:
	DC.B	$9B,'19~'
key59se:

key5F:
	DC.B	key5Fe-key5Fs,key5Fs-key5F
key5Fs:
	DC.B	$9B,'?~'
key5Fe:


CH1Name:
	DC.B	'ch1',0
kmEnd

		END

