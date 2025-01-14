	TTL	'$Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/mdbyte.i,v 1.2 90/04/03 12:12:53 andy Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
* $Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/mdbyte.i,v 1.2 90/04/03 12:12:53 andy Exp $
*
* $Locker:  $
*
* $Log:	mdbyte.i,v $
Revision 1.2  90/04/03  12:12:53  andy
*** empty log message ***

Revision 1.1  90/04/02  16:49:47  andy
Initial revision

* Revision 32.1  86/01/22  00:25:23  sam
* placed under source control
* 
*
**********************************************************************

***************************************************************
*							      *
*                       Mouth Shape Data		      *
*							      *
* Data for height and width of mouth shape is packed into a   *
* single byte.  Height in upper nibble, width in lower nibble.*
* Height is actual vertical mouth opening in millimeters.     *
* Width is horizontal opening in millimeters / 3.667 .        *
* Values in macro invocations are actual millimeters.         *           
***************************************************************
*
	XDEF	MOUTHS

MD	MACRO
	DC.B	(16*\1)+((\2*1000)/3666)
	ENDM

MOUTHS  DC.B	0	;blank
	MD      0,40	;period
	MD      0,40	;question
        MD      0,40	;comma
	MD      0,40	;dash
	DC.B    0	;(
	DC.B    0	;)
        DC.B    0	;Invalid phoneme
	DC.B    0	;reserved
	MD	9,55	;IY
	MD	9,50	;IH
	MD	10,50	;EH
	MD	15,50	;AE
	MD	15,47	;AA
	MD	8,45	;AH
	MD	13,40	;AO
	MD	7,30	;UH
	MD	8,45	;AX
	MD	9,50	;IX
	MD	7,30	;ER
	MD	5,25	;UX
	MD	0,45	;QX
	MD	7,25	;OH
	MD	10,35	;RX
	MD	8,35	;LX
	MD	10,50	;EY
	MD	9,50	;EY'
	MD	15,47	;AY
	MD	9,50	;AY'
	MD	8,30	;OY
	MD	9,50	;OY'
	MD	13,45	;AW
	MD	6,29	;AW'
	MD	5,26	;OW
	MD	2,18	;OW'
	MD	3,18	;UW
	MD	2,14	;UW'
	MD	2,14	;WH
	MD	4,20	;R
	MD	7,35	;L
	MD	2,7	;W
	MD	8,40	;Y
	MD	0,40	;M
	MD	6,40	;N
	MD	6,42	;NX
	MD	6,35	;NH
	MD	7,40	;DX
	MD	7,40	;Q
	MD	5,35	;S
	MD	6,25	;SH
	MD	2,20	;F
	MD	8,34	;TH
	MD	5,35	;Z
	MD	6,25	;ZH
	MD	2,20	;V
	MD	8,34	;DH
	MD	6,25	;CH
	MD	6,25	;CH'
	MD	6,25	;CH"
	MD	6,25	;J
	MD	6,25	;J'
	MD	9,44	;/H
	MD	6,37	;/M
	MD	7,25	;/B
	MD	4,20	;/R
	MD	9,45	;/C
	MD	0,40	;B
	MD	0,40	;B'
	MD	0,40	;B"
	MD	6,40	;D
	MD	6,40	;D'
	MD	6,40	;D"
	MD	9,44	;G
	MD	9,44	;G'
	MD	9,44	;G"
	MD	6,37	;GX
	MD	6,37	;GX'
	MD	6,37	;GX"
	MD	4,20	;GH
	MD	4,20	;GH'
	MD	4,20	;GH"
	MD	0,40	;P
	MD	0,40	;P'
	MD	0,40	;P"
	MD	6,40	;T
	MD	6,40	;T'
	MD	6,40	;T"
	MD	9,44	;K
	MD	9,44	;K'
	MD	9,44	;K"
	MD	6,37	;KX
	MD	6,37	;KX'
	MD	6,37	;KX"
	MD	4,20	;KH
	MD	4,20	;KH'
	MD	4,20	;KH"
	DC.B	0	;UL
	DC.B	0	;UM
	DC.B	0	;UN
	DC.B	0	;IL
	DC.B	0	;IM
	DC.B	0	;IN


