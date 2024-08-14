        TTL    '$Id: po.asm,v 38.1 92/04/07 11:18:34 eric Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1991, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore, 1200 Wilson Drive, West Chester, PA 19380	     *
*                                                                    *
**********************************************************************
*
*   PO:  PORTUGESE  (A2000)
*
*   Source Control
*   --------------
*   $Id: po.asm,v 38.1 92/04/07 11:18:34 eric Exp $
*
*   $Locker:  $
*
*   $Log:	po.asm,v $
*   Revision 38.1  92/04/07  11:18:34  eric
*   Internal name was incorrectly set to 'usa'.
*   Should be 'po'.
*   
*   Revision 38.0  92/04/07  11:16:30  eric
*   added to rcs
*   
*
**********************************************************************

*------ Included Files -----------------------------------------------

        INCLUDE         "exec/types.i"
        INCLUDE         "devices/keymap.i"

*---------------------------------------------------------------------

        DC.L    0,0     ; ln_Succ, ln_Pred
        DC.B    0,0     ; ln_Type, ln_Pri
        DC.L    POName  ; ln_Name
        DC.L    POLowMapType
        DC.L    POLowMap
        DC.L    POLCapsable
        DC.L    POLRepeatable
        DC.L    POHighMapType
        DC.L    POHighMap
        DC.L    POHCapsable
        DC.L    POHRepeatable


*------ Key Translation Table ----------------------------------------
*
*
*   00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  5A  5B  5C  5D
*         10  11  12  13  14  15  16  17  18  19  1A  1B    3D  3E  3F  4A
*          20  21  22  23  24  25  26  27  28  29  2A  2B   2D  2E  2F  5E
*        30  31  32  33  34  35  36  37  38  39  3A         1D  1E  1F  43
*                                                           0F      3C
*
*   `~  1!  2"  3S  4$  5%  6&  7/  8(  9)  0=  B?  '`  \|  [{  ]}  /   *
*         qQ  wW  eE  rR  tT  zZ  uU  iI  oO  pP  uU  +*    7   8   9   -
*          aA  sS  dD  fF  gG  hH  jJ  kK  lL  oO  aA  #^   4   5   6   +
*        <>  yY  xX  cC  vV  bB  nN  mM  ,;  .:  -_         1   2   3   enter
*                                                           0       .
*
*---------------------------------------------------------------------
*
*	40	Space
*	41	Backspace
*	42	Tab
*	43	Enter
*	44	Return
*	45	Escape
*	46	Delete
*	47	Insert		(101 keyboard)
*	48	Page Up		(101 keyboard)
*	49	Page Down	(101 keyboard)
*	4A	Numeric Pad -
*	4B	F11		(101 keyboard)
*	4C	Cursor Up
*	4D	Cursor Down
*	4E	Cursor Forward
*	4F	Cursor Backward
*	
*	50-59	Function keys F1-F10
*	5A	Numeric Pad (	(A2000)
*	5B	Numeric Pad )	(A2000)
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
*	6E	Pause/Break	(101 keyboard)
*	6F	F12		(101 keyboard)
*	70	Home		(101 keyboard)
*	71	End		(101 keyboard)
*
*		Currently Unused
*
*	0E 1C 2C 3B
*	6B 6C 6D
*
*		CDTV codes (reserved)
*	72-77
*
*---------------------------------------------------------------------

POLCapsable: ; NL means NULL (undefined) and RE means RESERVED
        DC.B    %00000000       ; 7\ 6& 5% 4$ 3s 2" 1! []  07 - 00
        DC.B    %00000000       ; 0N NL \| '` B? 0= 9) 8(  0F - 08
        DC.B    %11111111       ; iI uU zZ tT rR eE wW qQ  17 - 10
        DC.B    %00000111       ; 3N 2N 1N NL +* UU pP oO  1F - 18
        DC.B    %11111111       ; kK jJ hH gG fF dD sS aA  27 - 20
        DC.B    %00000111       ; 6N 5N 4N NL #' AA OO lL  2F - 28
        DC.B    %11111110       ; mM nN bB vV cC xX yY <>  37 - 30
        DC.B    %00000000       ; 9N 8N 7N .N NL -_ .: ,;  3F - 38

POHCapsable:
        DC.B    %00000000       ; 47 - 40
        DC.B    %00000000       ; 4F - 48
        DC.B    %00000000       ; 57 - 50
        DC.B    %00000000       ; 5F - 58
        DC.B    %00000000       ; 67 - 60
        DC.B    %00000000       ; 6F - 68
        DC.B    %00000000       ; 77 - 70


POLRepeatable:
        DC.B    %11111111       ; 7\ 6& 5% 4$ 3s 2" 1! []  07 - 00
        DC.B    %10111111       ; 0N NL \| '` B? 0= 9) 8(  0F - 08
        DC.B    %11111111       ; iI uU zZ tT rR eE wW qQ  17 - 10
        DC.B    %11101111       ; 3N 2N 1N NL +* UU pP oO  1F - 18
        DC.B    %11111111       ; kK jJ hH gG fF dD sS aA  27 - 20
        DC.B    %11101111       ; 6N 5N 4N NL #' AA OO lL  2F - 28
        DC.B    %11111111       ; mM nN bB vV cC xX yY <>  37 - 30
        DC.B    %11110111       ; 9N 8N 7N .N NL -_ .: ,;  3F - 38

POHRepeatable:
        DC.B    %01000111       ; 47 - 40
        DC.B    %11110100       ; 4F - 48
        DC.B    %11111111       ; 57 - 50
        DC.B    %01111111       ; 5F - 58
        DC.B    %00000000       ; 67 - 60
        DC.B    %00000000       ; 6F - 68
        DC.B    %00000000       ; 77 - 70

POLowMapType:
	DC.B	KC_VANILLA				; $00
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_SHIFT+KCF_ALT

	DC.B	KC_VANILLA				; $08
	DC.B	KC_VANILLA
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_VANILLA
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL

	DC.B	KC_VANILLA				; $10
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL

	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL	; $18
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KC_VANILLA
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL

	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL	; $20
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL

	DC.B	KC_VANILLA				; $28
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_DEAD+KCF_SHIFT
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL

	DC.B	KCF_SHIFT+KCF_ALT			; $30
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KC_VANILLA
	DC.B	KCF_DEAD+KCF_SHIFT+KCF_ALT+KCF_CONTROL
	DC.B	KC_VANILLA

	DC.B	KCF_SHIFT+KCF_ALT			; $38
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_NOP
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL


POHighMapType:
	DC.B	KCF_DEAD+KCF_ALT			; $40
	DC.B	KC_NOQUAL
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KC_NOQUAL
	DC.B	KCF_CONTROL
	DC.B	KCF_ALT
	DC.B	KC_NOQUAL
	DC.B	KCF_STRING+KCF_SHIFT

	DC.B	KCF_STRING+KCF_SHIFT			; $48
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KC_NOQUAL
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT

	DC.B	KCF_STRING+KCF_SHIFT			; $50
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT

	DC.B	KCF_STRING+KCF_SHIFT			; $58
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KCF_SHIFT+KCF_ALT
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KC_NOQUAL
	DC.B	KCF_STRING

	DC.B	KCF_NOP					; $60
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP

	DC.B	KCF_NOP					; $68
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_STRING+KCF_SHIFT

	DC.B	KCF_STRING+KCF_SHIFT			; $70
	DC.B	KCF_STRING+KCF_SHIFT
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP
	DC.B	KCF_NOP

POLowMap:
	DC.B	'~','`','|','\'		; \, |, `, ~			; $00
	DC.B	'!',$B9,'!','1'		; 1, !, super 1, !
	DC.B	'@',$B2,'"','2'		; 2, ", super 2, @
	DC.B	$A3,$B3,'#','3'		; 3, #, super 3, pound
	DC.B	$A7,$A2,'$','4'		; 4, $, cents, S
	DC.B	'%',$BC,'%','5'		; 5, %, 1/4, %
	DC.B	'^',$BD,'&','6'		; 6, &, 1/2, ^
	DC.B	'{',$BE,'/','7'		; 7, /, 3/4, {

	DC.B	'[',$B7,'(','8'		; 8, (, bullet, [		; $08
	DC.B	']',$AB,')','9'		; 9, ), <<, ]
	DC.B	'}',$BB,'=','0'		; 0, ), >>, )
	DC.B	'_','-','?',$27		; ', ?, -, _
	DC.B	'+','=',$BB,$AB		; <<, >>, =, +
	DC.B	'|','\','+','='		; =, +, \, |
	DC.L	0
	DC.B	0,0,0,'0'		; 0

	DC.B	$C5,$E5,'Q','q'		; q, Q, dot a, dot A		; $10
	DC.B	$B0,$B0,'W','w'		; w, W, dot, dot
	DC.L	key12			; e, E, (c), (c)
	DC.B	$AE,$AE,'R','r'		; r, R, (r), (r)
	DC.B	$DE,$FE,'T','t'		; t, T, thorn, THORN
	DC.L	key15			; y, Y, IMS, Yen
	DC.L	key16			; u, U, micro, micro
	DC.L	key17			; i, I, inverted !, broken |

	DC.L	key18			; o, O, slash o, slash O	; $18
	DC.B	$B6,$B6,'P','p'		; p, P, paragraph, paragraph
	DC.B	'-','[','*','+'		; +, *, -, {
	DC.L	key1B			; dead ', dead `, ], }
	DC.L	0			; KCF_NOP
	DC.B	0,0,0,'1'		; 1
	DC.B	0,0,0,'2'		; 2
	DC.B	0,0,0,'3'		; 3

	DC.L	key20			; a, A, ae, AE			; $20
	DC.B	$A7,$DF,'S','s'		; s, S, sharp s, section
	DC.B	$D0,$F0,'D','d'		; d, D, bar d, bar D
	DC.L	key23			; f, F, dead '
	DC.L	key24			; g, G, dead `
	DC.L	key25			; h, H, dead ^
	DC.L	key26			; j, J, dead ~
	DC.L	key27			; k, K, dead "

	DC.B	$A3,$A3,'L','l'		; l, L, pound, pound		; $28
	DC.B	':',';',$C7,$E7		; c cedilla, C cedilla, ;, :
	DC.B	'"',$27,$AA,$BA		; o, a, ', "
	DC.L	key2B			; dead ~, dead ^
	DC.L	0			; KCF_NOP
	DC.B	0,0,0,'4'		; 4
	DC.B	0,0,0,'5'		; 5
	DC.B	0,0,0,'6'		; 6

	DC.B	$BB,$AB,'>','<'		; <, >, <<, >>			; $30
	DC.B	$AC,$B1,'Z','z'		; z, Z, +/-, not / Vanilla
	DC.B	$F7,$D7,'X','x'		; x, X, times, divide / Vanilla
	DC.B	$C7,$E7,'C','c'		; c, C, c cedilla, C cedilla / Vanilla
	DC.B	$AA,$AA,'V','v'		; v, V, female ordinal, / Vanilla
	DC.B	$BA,$BA,'B','b'		; b, B, male ordinal / Vanilla
	DC.L	key36			; n, N, SHY, overbar / Accents
	DC.B	$BF,$B8,'M','m'		; m, M, cedilla, inverted ? / Vanilla

	DC.B	'<',',',';',','		; ,, ;, ,, <			; $38
	DC.B	'>','.',':','.'		; ., >
	DC.B	'?','/','-','_'		; /, ?
	DC.L	0			; KCF_NOP
	DC.B	0,0,0,'.'		; .
	DC.B	0,0,0,'7'		; 7
	DC.B	0,0,0,'8'		; 8
	DC.B	0,0,0,'9'		; 9


POHighMap:
	DC.L	key40			;SPACE				; $40
	DC.B	0,0,0,$08		;BACKSPACE
	DC.L	key42			;TAB
	DC.B	0,0,0,$0D		;ENTER
	DC.B	0,0,$0A,$0D		;RETURN
	DC.B	0,0,$9B,$1B		;ESC
	DC.B	0,0,0,$7F		;DEL
	DC.L	key47			;INSERT (101)

	DC.L	key48			;PAGE UP (101)			; $48
	DC.L	key49			;PAGE DOWN (101)
	DC.B	0,0,0,'-'		;NP subtract
	DC.L	key4B			;F11 (101)
	DC.L	key4C			;CRSR UP
	DC.L	key4D			;CRSR DOWN
	DC.L	key4E			;CRSR RIGHT
	DC.L	key4F			;CRSR LEFT

	DC.L	key50			;F1				; $50
	DC.L	key51			;F2
	DC.L	key52			;F3
	DC.L	key53			;F4
	DC.L	key54			;F5
	DC.L	key55			;F6
	DC.L	key56			;F7
	DC.L	key57			;F8

	DC.L	key58			;F9				; $58
	DC.L	key59			;F10
	DC.B	0,0,'{','['		;NP [, {
	DC.B	0,0,'}',']'		;NP ], }
	DC.B	0,0,0,'/'		;NP divide
	DC.B	0,0,0,'*'		;NP multiply
	DC.B	0,0,0,'+'		;NP add
	DC.L	key5F			;HELP

	DC.L	0			;LEFT SHIFT			; $60
	DC.L	0			;RIGHT SHIFT
	DC.L	0			;CAPS LOCK
	DC.L	0			;CTRL
	DC.L	0			;LEFT ALT
	DC.L	0			;RIGHT ALT
	DC.L	0			;LEFT AMIGA
	DC.L	0			;RIGHT AMIGA

	DC.L	0			;MOUSE LEFT			; $68
	DC.L	0			;MOUSE RIGHT
	DC.L	0			;MOUSE CENTER
	DC.L	0			;unused
	DC.L	0			;unused
	DC.L	0			;unused
	DC.L	key6E			;PAUSE/BREAK (101)
	DC.L	key6F			;F12 (101)

	DC.L	key70			;HOME (101)
	DC.L	key71			;END (101)
	DC.L	0			;CDTV 72-77 (78 reset warn)
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0
	DC.L	0


;------ possible dead keys

key1B:
		DC.B	DPF_DEAD,1,DPF_DEAD,2	; dead ', dead `
		DC.B	0,']',0,'}'		; ], }
		DC.B    0,$1D,0,$1D,0,$9D,0,$9D ; control translation
key2B:
		DC.B	DPF_DEAD,4,DPF_DEAD,3	; dead ~, dead ^
		DC.B	0,'h',0,'H'
key23:
		DC.B	0,'f',0,'F'	; f, F, dead '
		DC.B	DPF_DEAD,1,DPF_DEAD,1
		DC.B	0,$06,0,$06,0,$86,0,$86	; control translation
key24:
		DC.B	0,'g',0,'G'	; g, G, dead `
		DC.B	DPF_DEAD,2,DPF_DEAD,2
		DC.B	0,$07,0,$07,0,$87,0,$87	; control translation
key25:
		DC.B	0,'h',0,'H'	; h, H, dead ^
		DC.B	DPF_DEAD,3,DPF_DEAD,3
		DC.B	0,$08,0,$08,0,$88,0,$88	; control translation
key26:
		DC.B	0,'j',0,'J'	; j, J, dead ~
		DC.B	DPF_DEAD,4,DPF_DEAD,4
		DC.B	0,$0A,0,$0A,0,$8A,0,$8A	; control translation
key27:
		DC.B	0,'k',0,'K'	; k, K, dead "
		DC.B	DPF_DEAD,5,DPF_DEAD,5
		DC.B	0,$0B,0,$0B,0,$8B,0,$8B	; control translation


;------ deadable keys (modified by dead keys)

key20:					; a, A, ae, AE
		DC.B	DPF_MOD,key20u-key20,DPF_MOD,key20s-key20
		DC.B	0,$E6,0,$C6
		DC.B	0,$01,0,$01,0,$81,0,$81	; control translation
key20u:
		DC.B	'a',$E1,$E0,$E2,$E3,$E4
key20s:
		DC.B	'A',$C1,$C0,$C2,$C3,$C4

key12:					; e, E, (c), (c)
		DC.B	DPF_MOD,key12u-key12,DPF_MOD,key12s-key12
		DC.B	0,$A9,0,$A9
		DC.B	0,$05,0,$05,0,$85,0,$85	; control translation
key12u:
		DC.B	'e',$E9,$E8,$EA,'e',$EB
key12s:
		DC.B	'E',$C9,$C8,$CA,'E',$CB

key17:					; i, I, inverted !, broken |
		DC.B	DPF_MOD,key17u-key17,DPF_MOD,key17s-key17
		DC.B	0,$A1,0,$A6
		DC.B	0,$09,0,$09,0,$89,0,$89	; control translation
key17u:
		DC.B	'i',$ED,$EC,$EE,'i',$EF
key17s:
		DC.B	'I',$CD,$CC,$CE,'I',$CF

key36:					; n, N, SHY, overbar
		DC.B	DPF_MOD,key36u-key36,DPF_MOD,key36s-key36
		DC.B	0,$AD,0,$AF
		DC.B	0,$0E,0,$0E,0,$8E,0,$8E	; control translation
key36u:
		DC.B	'n','n','n','n',$F1,'n'
key36s:
		DC.B	'N','N','N','N',$D1,'N'

key18:					; o, O, bar o, bar O
		DC.B	DPF_MOD,key18u-key18,DPF_MOD,key18s-key18
		DC.B	0,$F8,0,$D8
		DC.B	0,$0F,0,$0F,0,$8F,0,$8F	; control translation
key18u:
		DC.B	'o',$F3,$F2,$F4,$F5,$F6
key18s:
		DC.B	'O',$D3,$D2,$D4,$D5,$D6

key16:					; u, U, micro, micro
		DC.B	DPF_MOD,key16u-key16,DPF_MOD,key16s-key16
		DC.B	0,$B5,0,$B5
		DC.B	0,$15,0,$15,0,$95,0,$95	; control translation
key16u:
		DC.B	'u',$FA,$F9,$FB,'u',$FC
key16s:
		DC.B	'U',$DA,$D9,$DB,'U',$DC

key15:					; y, Y, IMS, yen
		DC.B	DPF_MOD,key15u-key15,DPF_MOD,key15s-key15
		DC.B	0,$A4,0,$A5
		DC.B	0,$19,0,$19,0,$99,0,$99	; control translation
key15u:
		DC.B	'y',$FD,'y','y','y',$FF
key15s:
		DC.B	'Y',$DD,'Y','Y','Y','Y'


key40:					; space,,NBSP, and accents
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

;-- INSERT (101)
key47:
		DC.B	key47ue-key47us,key47us-key47
		DC.B	key47se-key47ss,key47ss-key47

key47us:
		DC.B	$9B,'40~'
key47ue:
key47ss:
		DC.B	$9B,'50~'
key47se:

;-- Page UP (101)

key48:
		DC.B	key48ue-key48us,key48us-key48
		DC.B	key48se-key48ss,key48ss-key48

key48us:
		DC.B	$9B,'41~'
key48ue:
key48ss:
		DC.B	$9B,'51~'
key48se:

;-- Page DOWN (101)

key49:
		DC.B	key49ue-key49us,key49us-key49
		DC.B	key49se-key49ss,key49ss-key49

key49us:
		DC.B	$9B,'42~'
key49ue:
key49ss:
		DC.B	$9B,'52~'
key49se:

;-- F11 (101)

key4B:
		DC.B	key4Bue-key4Bus,key4Bus-key4B
		DC.B	key4Bse-key4Bss,key4Bss-key4B

key4Bus:
		DC.B	$9B,'20~'
key4Bue:
key4Bss:
		DC.B	$9B,'30~'
key4Bse:


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

;-- PAUSE/BREAK (101)

key6E:
		DC.B	key6Eue-key6Eus,key6Eus-key6E
		DC.B	key6Ese-key6Ess,key6Ess-key6E

key6Eus:
		DC.B	$9B,'43~'
key6Eue:
key6Ess:
		DC.B	$9B,'53~'
key6Ese:


;-- F12 (101)

key6F:
		DC.B	key6Fue-key6Fus,key6Fus-key6F
		DC.B	key6Fse-key6Fss,key6Fss-key6F

key6Fus:
		DC.B	$9B,'21~'
key6Fue:
key6Fss:
		DC.B	$9B,'31~'
key6Fse:


;-- HOME (101)

key70:
		DC.B	key70ue-key70us,key70us-key70
		DC.B	key70se-key70ss,key70ss-key70

key70us:
		DC.B	$9B,'44~'
key70ue:
key70ss:
		DC.B	$9B,'54~'
key70se:


;-- END (101)

key71:
		DC.B	key71ue-key71us,key71us-key71
		DC.B	key71se-key71ss,key71ss-key71

key71us:
		DC.B	$9B,'45~'
key71ue:
key71ss:
		DC.B	$9B,'55~'
key71se:



POName:
		DC.B	'po',0
kmEnd

	END
