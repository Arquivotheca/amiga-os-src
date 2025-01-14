**********************************************************************
*
*   usa2 (dvorak) key map
*
**********************************************************************

	INCLUDE		"exec/types.i"
	INCLUDE		"devices/keymap.i"

		DC.L	0,0		; ln_Succ, ln_Pred
		DC.B	0,0		; ln_Type, ln_Pri
		DC.L	DVName		; ln_Name
		DC.L	DVLowMapType
		DC.L	DVLowMap
		DC.L	DVLCapsable
		DC.L	DVLRepeatable
		DC.L	DVHighMapType
		DC.L	DVHighMap
		DC.L	DVHCapsable
		DC.L	DVHRepeatable


*------ Key Translation Table ----------------------------------------
*
*   00	01  02	03  04	05  06	07  08	09  0A	0B  0C	0D  3D	3E  3F
*	  10  11  12  13  14  15  16  17  18  19  1A  1B    2D	2E  2F
*	   20  21  22  23  24  25  26  27  28  29  2A  2B   1D	1E  1F
*	 30  31	 32  33	 34  35	 36  37	 38  39	 3A	    0F	    3C
*
*   `~	1!  2@	3#  4$	5%  6^	7&  8*	9( 0)  \|  =+  }{   7	8   9
*	  '"  ,<  .>  pP  yY  fF  gG  cC  rR  lL  /?  ][    4	5   6
*	   aA  oO  eE  uU  iI  dD  hH  tT  nN  sS  -_	    1   2   3
*	     ;:  qQ  jJ  kK  xX  bB  mM  wW  vV  zZ	    0	    .
*
*---------------------------------------------------------------------
*
*   for reference, here's USA
*
*   `~	1!  2@	3#  4$	5%  6^	7&  8*	9( 0)  -_  =+  \|   7	8   9
*	  qQ  wW  eE  rR  tT  yY  uU  iI  oO  pP  [{  ]}    4	5   6
*	   aA  sS  dD  fF  gG  hH  jJ  kK  lL  ;:  '"	    1   2   3
*	     zZ	 xX  cC	 vV  bB	 nN  mM	 ,<  .>	 /?	    0	    .
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
*	4A	Numeric Pad -
*	4C	Cursor Up
*	4D	Cursor Down
*	4E	Cursor Forward
*	4F	Cursor Backward
*	
*	50-59	Function keys F1-F10
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
DVLCapsable:
		DC.B	%00000000,%00000000,%11111000,%00000011
		DC.B	%11111111,%00000011,%11111100,%00000111

DVHCapsable:
		DC.B	%00000000,%00000000,%00000000,%00000000
		DC.B	%00000000,%00000000,%00000000

DVLRepeatable:
		DC.B	%11111111,%10111111,%11111111,%11101111
		DC.B	%11111111,%11111111,%11111111,%11110111

DVHRepeatable:
		DC.B	%01000111,%11110100,%11111111,%00000011
		DC.B	%00000000,%00000000,%00000000

DVLowMapType:
		DC.B	KC_VANILLA,KCF_SHIFT+KCF_ALT
		DC.B	    KC_VANILLA,KCF_SHIFT+KCF_ALT
		DC.B	KCF_SHIFT+KCF_ALT,KCF_SHIFT+KCF_ALT
		DC.B	    KC_VANILLA,KCF_SHIFT+KCF_ALT
		DC.B	KCF_SHIFT+KCF_ALT,KCF_SHIFT+KCF_ALT
		DC.B	    KCF_SHIFT+KCF_ALT,KC_VANILLA
		DC.B	KCF_SHIFT,KC_VANILLA,KCF_NOP,KC_NOQUAL

		DC.B	KCF_SHIFT+KCF_ALT,KCF_SHIFT+KCF_ALT
		DC.B	    KCF_SHIFT+KCF_ALT,KC_VANILLA
		DC.B	KCF_DEAD+KC_VANILLA,KC_VANILLA
		DC.B	    KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KCF_NOP,KC_NOQUAL,KC_NOQUAL,KC_NOQUAL

		DC.B	KCF_DEAD+KC_VANILLA
		DC.B	    KCF_DEAD+KC_VANILLA
		DC.B	    KCF_DEAD+KC_VANILLA
		DC.B	    KCF_DEAD+KC_VANILLA
		DC.B	KCF_DEAD+KC_VANILLA
		DC.B	    KCF_DEAD+KC_VANILLA
		DC.B	    KCF_DEAD+KC_VANILLA
		DC.B	    KCF_DEAD+KC_VANILLA
		DC.B	KCF_DEAD+KC_VANILLA,KC_VANILLA
		DC.B	    KC_VANILLA,KCF_NOP
		DC.B	KCF_NOP,KC_NOQUAL,KC_NOQUAL,KC_NOQUAL

		DC.B	KCF_SHIFT+KCF_ALT,KCF_SHIFT+KCF_ALT
		DC.B	    KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KCF_NOP
		DC.B	KC_NOQUAL,KC_NOQUAL,KC_NOQUAL,KC_NOQUAL

DVHighMapType:
		DC.B	KCF_DEAD+KCF_ALT,KC_NOQUAL
		DC.B	    KCF_STRING+KCF_SHIFT,KC_NOQUAL
		DC.B	KCF_CONTROL,KCF_ALT,KC_NOQUAL,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KC_NOQUAL,KCF_NOP
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	    KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT

		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	    KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	    KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_ALT+KCF_SHIFT,KCF_ALT+KCF_SHIFT
		DC.B	KCF_ALT+KCF_SHIFT,KCF_ALT+KCF_SHIFT
		DC.B	KCF_ALT+KCF_SHIFT,KCF_STRING

		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP

		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP

DVLowMap:
		DC.B	'~','`','~','`'	; `, ~ / Vanilla
		DC.B	'!',$B9,'!','1'	; 1, !, super 1, !
		DC.B	'@',$B2,'@','2'	; 2, @, super 2, @ / Vanilla
		DC.B	'#',$B3,'#','3'	; 3, #, super 3, #
		DC.B	'$',$A2,'$','4'	; 4, $, cents, $
		DC.B	'%',$BC,'%','5'	; 5, %, 1/4, %
		DC.B	'^',$BD,'^','6'	; 6, ^, 1/2, ^ / Vanilla
		DC.B	'&',$BE,'&','7'	; 7, &, 3/4, &
		DC.B	'*',$B7,'*','8'	; 8, *, bullet, *
		DC.B	'(',$AB,'(','9'	; 9, (, <<, (
		DC.B	')',$BB,')','0'	; 0, ), >>, )
		DC.B	'_','-','|','\'	; \, |, -, _ / Vanilla
		DC.B	'+','=','+','='	; =, +
		DC.B	'|','\','{','}'	; }, {, \, | / Vanilla
		DC.L	0

		DC.B	0,0,0,'0'	; 0

		DC.B	$C5,$E5,'"',$27	; ', ", dot a, dot A
		DC.B	$B0,$B0,'<',','	; ,, <, dot, dot
		DC.B	$A9,$A9,'>','.'	; ., >, (c), (c)
		DC.B	$AE,$AE,'P','p'	; p, P, (r), (r) / Vanilla
		DC.L	key14		; y, Y, thorn, THORN / Accents
		DC.B	$A5,$A4,'F','f'	; f, F, IMS, Yen / Vanilla
		DC.B	$B5,$B5,'G','g'	; g, G, micro, micro / Vanilla
		DC.B	$A6,$A1,'C','c'	; c, C, inverted !, broken | / Vanilla
		DC.B	$D8,$F8,'R','r'	; r, R, slash o, slash O / Vanilla
		DC.B	$B6,$B6,'L','l'	; l, L, paragraph, paragraph / Vanilla
		DC.B	'{','[','?','/'	; /, ?, [, { / Vanilla
		DC.B	'}',']','[',']'	; ], [, ], } / Vanilla
		DC.L	0

		DC.B	0,0,0,'1'	; 1
		DC.B	0,0,0,'2'	; 2
		DC.B	0,0,0,'3'	; 3

		DC.L	key20		; a, A, ae, AE / Accents
		DC.L	key21		; o, O, sharp s, section / Accents
		DC.L	key22		; e, E, bar d, bar D / Accents
		DC.L	key23		; u, U, dead ' / Accents
		DC.L	key24		; i, I, dead ` / Accents
		DC.L	key25		; d, D, dead ^
		DC.L	key26		; h, H, dead ~
		DC.L	key27		; t, T, dead "
		DC.L	key28		; n, N, pound, pound / Accents
		DC.B	':',';','S','s'	; s, S, ;, : / Vanilla
		DC.B	'"',$27,'_','-'	; -, _, ', " / Vanilla
		DC.L	0
		DC.L	0

		DC.B	0,0,0,'4'	; 4
		DC.B	0,0,0,'5'	; 5
		DC.B	0,0,0,'6'	; 6

		DC.B	$BB,$AB,'>','<'	; <, >, <<, >>
		DC.B	$AC,$B1,':',';'	; ;, :, +/-, not
		DC.B	$F7,$D7,'Q','q'	; q, Q, times, divide / Vanilla
		DC.B	$C7,$E7,'J','j'	; j, J, c cedilla, C cedilla / Vanilla
		DC.B	$AA,$AA,'K','k'	; k, K, female ordinal, / Vanilla
		DC.B	$BA,$BA,'X','x'	; x, X, male ordinal / Vanilla
		DC.B	$AF,$AD,'B','b'	; b, B, SHY, overbar / Vanilla
		DC.B	$BF,$B8,'M','m'	; m, M, cedilla, inverted ? / Vanilla
		DC.B	'<',',','W','w'	; w, W, ,, < / Vanilla
		DC.B	'>','.','V','v'	; v, V, ., > / Vanilla
		DC.B	'?','/','Z','z'	; z, Z, /, ? / Vanilla
		DC.L	0

		DC.B	0,0,0,'.'	; .

		DC.B	0,0,0,'7'	; 7
		DC.B	0,0,0,'8'	; 8
		DC.B	0,0,0,'9'	; 9



DVHighMap:
		DC.L	key40
		DC.B	0,0,0,$08
		DC.L	key42
		DC.B	0,0,0,$0D
		DC.B	0,0,$0A,$0D
		DC.B	0,0,$9B,$1B
		DC.B	0,0,0,$7F
		DC.L	0
		DC.L	0
		DC.L	0
		DC.B	0,0,0,'-'
		DC.L	0
		DC.L	key4C
		DC.L	key4D
		DC.L	key4E
		DC.L	key4F

		DC.L	key50
		DC.L	key51
		DC.L	key52
		DC.L	key53
		DC.L	key54
		DC.L	key55
		DC.L	key56
		DC.L	key57
		DC.L	key58
		DC.L	key59
		DC.B    '(','(','(','('
		DC.B	')',')',')',')'
		DC.B    '/','/','/','/'
		DC.B    '*','*','*','*'
		DC.B    '+','+','+','+'
		DC.L	key5F

		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0

		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0


;------ dead keys

key14:					; y, Y, thorn, THORN
		DC.B	DPF_MOD,key14u-key14,DPF_MOD,key14s-key14
		DC.B	0,$FE,0,$DE
		DC.B	0,$19,0,$19,0,$99,0,$99	; control translation
key14u:
		DC.B	'y',$FD,'y','y','y',$FF
key14s:
		DC.B	'Y',$DD,'Y','Y','Y','Y'

key20:					; a, A, ae, AE
		DC.B	DPF_MOD,key20u-key20,DPF_MOD,key20s-key20
		DC.B	0,$E6,0,$C6
		DC.B	0,$01,0,$01,0,$81,0,$81	; control translation
key20u:
		DC.B	'a',$E1,$E0,$E2,$E3,$E4
key20s:
		DC.B	'A',$C1,$C0,$C2,$C3,$C4

key21:					; o, O, sharp s, section
		DC.B	DPF_MOD,key21u-key21,DPF_MOD,key21s-key21
		DC.B	0,$DF,0,$A7
		DC.B	0,$0F,0,$0F,0,$8F,0,$8F	; control translation
key21u:
		DC.B	'o',$F3,$F2,$F4,$F5,$F6
key21s:
		DC.B	'O',$D3,$D2,$D4,$D5,$D6

key22:					; e, E, bar d, bar D
		DC.B	DPF_MOD,key22u-key22,DPF_MOD,key22s-key22
		DC.B	0,$F0,0,$D0
		DC.B	0,$05,0,$05,0,$85,0,$85	; control translation
key22u:
		DC.B	'e',$E9,$E8,$EA,'e',$EB
key22s:
		DC.B	'E',$C9,$C8,$CA,'E',$CB

key23:					; u, U, dead '
		DC.B	DPF_MOD,key23u-key23,DPF_MOD,key23s-key23
		DC.B	DPF_DEAD,1,DPF_DEAD,1
		DC.B	0,$15,0,$15,0,$95,0,$95	; control translation
key23u:
		DC.B	'u',$FA,$F9,$FB,'u',$FC
key23s:
		DC.B	'U',$DA,$D9,$DB,'U',$DC

key24:					; i, I, dead `
		DC.B	DPF_MOD,key24u-key24,DPF_MOD,key24s-key24
		DC.B	DPF_DEAD,2,DPF_DEAD,2
		DC.B	0,$09,0,$09,0,$89,0,$89	; control translation
key24u:
		DC.B	'i',$ED,$EC,$EE,'i',$EF
key24s:
		DC.B	'I',$CD,$CC,$CE,'I',$CF

key25:
		DC.B	0,'d',0,'D'	; d, D, dead ^
		DC.B	DPF_DEAD,3,DPF_DEAD,3
		DC.B	0,$04,0,$04,0,$84,0,$84	; control translation

key26:
		DC.B	0,'h',0,'H'	; h, H, dead ~
		DC.B	DPF_DEAD,4,DPF_DEAD,4
		DC.B	0,$08,0,$08,0,$88,0,$88	; control translation

key27:
		DC.B	0,'t',0,'T'	; t, T, dead "
		DC.B	DPF_DEAD,5,DPF_DEAD,5
		DC.B	0,$14,0,$14,0,$94,0,$94	; control translation

key28:					; n, N, pound, pound
		DC.B	DPF_MOD,key28u-key28,DPF_MOD,key28s-key28
		DC.B	0,$A3,0,$A3
		DC.B	0,$0E,0,$0E,0,$8E,0,$8E	; control translation
key28u:
		DC.B	'n','n','n','n',$F1,'n'
key28s:
		DC.B	'N','N','N','N',$D1,'N'

key40:					; space,, NBSP, and accents
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


DVName:
		DC.B	'usa2',0
kmEnd

		END
