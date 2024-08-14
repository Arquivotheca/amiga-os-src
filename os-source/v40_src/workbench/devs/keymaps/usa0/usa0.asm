	TTL    '$Header: usa0.asm,v 36.0 89/05/25 17:05:04 eric Exp $'
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
*   USA key map
*
*   Source Control
*   --------------
*   $Header: usa0.asm,v 36.0 89/05/25 17:05:04 eric Exp $
*
*   $Locker:  $
*
*   $Log:	usa0.asm,v $
*   Revision 36.0  89/05/25  17:05:04  eric
*   Initial Submission
*   
*   Revision 33.1  86/06/11  17:07:13  kodiak
*   fix dead accents so acute is first
*   
*   Revision 33.0  86/06/11  15:50:30  kodiak
*   added to rcs for updating
*   
*   Revision 33.0  86/02/15  12:32:54  kodiak
*   added to rcs for updating
*   
*
**********************************************************************

	SECTION		rawinput

*------ Included Files -----------------------------------------------

	INCLUDE		"exec/types.i"
	INCLUDE		"devices/keymap.i"

*------ Exported Names -----------------------------------------------

	XDEF		USANode

USANode:
		DC.L	0,0		; ln_Succ, ln_Pred
		DC.B	0,0		; ln_Type, ln_Pri
		DC.L	USAName		; ln_Name
		DC.L	USALowMapType
		DC.L	USALowMap
		DC.L	USALCapsable
		DC.L	USALRepeatable
		DC.L	USAHighMapType
		DC.L	USAHighMap
		DC.L	USAHCapsable
		DC.L	USAHRepeatable


*------ Key Translation Table ----------------------------------------
*
*
*   00	01  02	03  04	05  06	07  08	09  0A	0B  0C	0D  3D	3E  3F
*	  10  11  12  13  14  15  16  17  18  19  1A  1B    2D	2E  2F
*	   20  21  22  23  24  25  26  27  28  29  2A  2B   1D	1E  1F
*	 30  31	 32  33	 34  35	 36  37	 38  39	 3A	    0F	    3C
*
*   `~	1!  2@	3#  4$	5%  6^	7&  8*	9( 0)  -_  =+  \|   7	8   9
*	  qQ  wW  eE  rR  tT  yY  uU  iI  oO  pP  [{  ]}    4	5   6
*	   aA  sS  dD  fF  gG  hH  jJ  kK  lL  ;:  '"	    1   2   3
*	     zZ	 xX  cC	 vV  bB	 nN  mM	 ,<  .>	 /?	    0	    .
*
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

VANILLA		MACRO   * &U,&S
		DC.B	(\2)+$80
		DC.B	(\1)+$80
		DC.B	\2
		DC.B	\1
		ENDM

USALCapsable:
		DC.B	%00000000,%00000000,%11111111,%00000011
		DC.B	%11111111,%00000001,%11111110,%00000000

USAHCapsable:
		DC.B	%00000000,%00000000,%00000000,%00000000
		DC.B	%00000000,%00000000,%00000000

USALRepeatable:
		DC.B	%11111111,%10111111,%11111111,%11101111
		DC.B	%11111111,%11101111,%11111111,%11110111

USAHRepeatable:
		DC.B	%01000111,%11110100,%11111111,%00000011
		DC.B	%00000000,%00000000,%00000000

USALowMapType:
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KCF_NOP,KC_VANILLA

		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KCF_NOP,KC_VANILLA,KC_VANILLA,KC_VANILLA

		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KCF_NOP
		DC.B	KCF_NOP,KC_VANILLA,KC_VANILLA,KC_VANILLA

		DC.B	KCF_NOP,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KCF_NOP
		DC.B	KC_VANILLA,KC_VANILLA,KC_VANILLA,KC_VANILLA

USAHighMapType:
		DC.B	KCF_ALT,KC_NOQUAL,KCF_STRING+KCF_SHIFT,KC_NOQUAL
		DC.B	KCF_CONTROL,KCF_ALT,KC_NOQUAL,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_ALT,KCF_NOP
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT

		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_STRING+KCF_SHIFT,KCF_STRING+KCF_SHIFT
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_STRING

		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP

		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP
		DC.B	KCF_NOP,KCF_NOP,KCF_NOP,KCF_NOP

USALowMap:
		VANILLA	<'`'>,<'~'>
		VANILLA	<'1'>,<'!'>
		VANILLA	<'2'>,<'@'>
		VANILLA	<'3'>,<'#'>
		VANILLA	<'4'>,<'$'>
		VANILLA	<'5'>,<'%'>
		VANILLA	<'6'>,<'^'>
		VANILLA	<'7'>,<'&'>
		VANILLA	<'8'>,<'*'>
		VANILLA	<'9'>,<'('>
		VANILLA	<'0'>,<')'>
		VANILLA	<'-'>,<'_'>
		VANILLA	<'='>,<'+'>
		VANILLA	<'\'>,<'|'>
		DC.L	0

		VANILLA	<'0'>,<'0'>

		VANILLA	<'q'>,<'Q'>
		VANILLA	<'w'>,<'W'>
		VANILLA	<'e'>,<'E'>
		VANILLA	<'r'>,<'R'>
		VANILLA	<'t'>,<'T'>
		VANILLA	<'y'>,<'Y'>
		VANILLA	<'u'>,<'U'>
		VANILLA	<'i'>,<'I'>
		VANILLA	<'o'>,<'O'>
		VANILLA	<'p'>,<'P'>
		VANILLA	<'['>,<'{'>
		VANILLA	<']'>,<'}'>
		DC.L	0

		VANILLA	<'1'>,<'1'>
		VANILLA	<'2'>,<'2'>
		VANILLA	<'3'>,<'3'>

		VANILLA	<'a'>,<'A'>
		VANILLA	<'s'>,<'S'>
		VANILLA	<'d'>,<'D'>
		VANILLA	<'f'>,<'F'>
		VANILLA	<'g'>,<'G'>
		VANILLA	<'h'>,<'H'>
		VANILLA	<'j'>,<'J'>
		VANILLA	<'k'>,<'K'>
		VANILLA	<'l'>,<'L'>
		VANILLA	<';'>,<':'>
		VANILLA	<''''>,<'"'>
		DC.L	0
		DC.L	0

		VANILLA	<'4'>,<'4'>
		VANILLA	<'5'>,<'5'>
		VANILLA	<'6'>,<'6'>

		DC.L	0
		VANILLA	<'z'>,<'Z'>
		VANILLA	<'x'>,<'X'>
		VANILLA	<'c'>,<'C'>
		VANILLA	<'v'>,<'V'>
		VANILLA	<'b'>,<'B'>
		VANILLA	<'n'>,<'N'>
		VANILLA	<'m'>,<'M'>
		VANILLA	<','>,$3C ;<'<'>
		VANILLA	<'.'>,$3E ;<'>'>
		VANILLA	<'/'>,<'?'>
		DC.L	0

		VANILLA	<'.'>,<'.'>

		VANILLA	<'7'>,<'7'>
		VANILLA	<'8'>,<'8'>
		VANILLA	<'9'>,<'9'>

USAHighMap:
		DC.B	0,0,$A0,$20
		DC.B	0,0,0,$08
		DC.L	key42
		DC.B	0,0,0,$0D
		DC.B	0,0,$0A,$0D
		DC.B	0,0,$9B,$1B
		DC.B	0,0,0,$7F
		DC.L	0
		DC.L	0
		DC.L	0
		DC.B	0,0,$FF,'-'
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
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
		DC.L	0
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

USAName:
		DC.B	'usa0',0
kmEnd

		END
