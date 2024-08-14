	TTL	'$Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/digraphs.i,v 1.2 90/04/03 12:12:11 andy Exp $'
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
* $Header: /usr/machines/heartofgold/amiga/V37/src/workbench/devs/narrator/RCS/digraphs.i,v 1.2 90/04/03 12:12:11 andy Exp $
*
* $Locker:  $
*
* $Log:	digraphs.i,v $
Revision 1.2  90/04/03  12:12:11  andy
*** empty log message ***

Revision 1.1  90/04/02  16:40:37  andy
Initial revision

* Revision 32.1  86/01/22  00:25:08  sam
* placed under source control
* 
*
**********************************************************************



            NOLIST
*
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*                            ;
*  PHONEME INPUT DIGRAPHS    ;
*                            ;
*  DC.W   0  =  THIS PHON    ;
*  IS NOT ACCESSED THROUGH   ;
*  ASCII INPUT.              ;
*                            ;
*;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
*
DIGRAPHS    DC.B        ' ',0
            DC.B        '.',0
            DC.B        '?',0
            DC.B        ',',0
            DC.B        '-',0
            DC.B        '(',0  ;P-UNIT INITIATOR
            DC.B        ')',0  ;P-UNIT TERMINATOR
            DC.W        0
            DC.W        0
            DC.W         'IY'    ;9
            DC.W         'IH'    ;10
            DC.W         'EH'    ;11
            DC.W         'AE'    ;12
            DC.W         'AA'    ;13
            DC.W         'AH'    ;14
            DC.W         'AO'    ;15
            DC.W         'UH'    ;16
            DC.W         'AX'    ;17
            DC.W         'IX'    ;18
            DC.W         'ER'    ;19
            DC.W         'UX'    ;20
            DC.W         'QX'    ;21   SILENT VOWEL
            DC.W         'OH'    ;22
            DC.W         'RX'    ;23
            DC.W         'LX'    ;24
            DC.W         'EY'    ;25
            DC.W        0
            DC.W         'AY'    ;27
            DC.W        0
            DC.W         'OY'    ;29
            DC.W        0
            DC.W         'AW'    ;31
            DC.W        0
            DC.W         'OW'    ;33
            DC.W        0
            DC.W         'UW'    ;35
            DC.W        0        ;36
            DC.W         'WH'    ;37
            DC.B        'R',0    ;38
            DC.B        'L',0    ;39
            DC.B        'W',0    ;40
            DC.B        'Y',0    ;41
            DC.B        'M',0    ;42
            DC.B        'N',0    ;43
            DC.W         'NX'    ;44
            DC.W         'NH'    ;45
            DC.W         'DX'    ;46
            DC.B        'Q',0    ;47
            DC.B        'S',0    ;48
            DC.W         'SH'    ;49
            DC.B        'F',0    ;50
            DC.W         'TH'
            DC.B        'Z',0
            DC.W         'ZH'
            DC.B        'V',0
            DC.W         'DH'
            DC.W         'CH'
            DC.W        0
            DC.W        0
            DC.B        'J',0     ;59
            DC.W        0         ;60
            DC.W         '/H'
            DC.W         '/M'
            DC.W         '/B'
            DC.W         '/R'
            DC.W         '/C'     ;65
            DC.B        'B',0
            DC.W        0
            DC.W        0
            DC.B        'D',0
            DC.W        0         ;70
            DC.W        0
            DC.B        'G',0
            DC.W        0
            DC.W        0
            DC.W         'GX'     ;75
            DC.W        0
            DC.W        0
            DC.W         'GH'
            DC.W        0
            DC.W        0         ;80
            DC.B        'P',0     ;81
            DC.W        0
            DC.W        0
            DC.B        'T',0     ;84
            DC.W        0         ;85
            DC.W        0
            DC.B        'K',0     ;87
            DC.W        0
            DC.W        0
            DC.W         'KX'     ;90
            DC.W        0
            DC.W        0
            DC.W         'KH'     ;93
            DC.W        0
            DC.W        0
            DC.W         'UL'     ;96
            DC.W         'UM'     ;97
            DC.W         'UN'     ;98
            DC.W         'IL'     ;99
            DC.W         'IM'     ;100
            DC.W         'IN'     ;101

*     THE NUMERALS ARE INCLUDED IN THE DIGRAPHS TABLE AS TARGETS FOR
*     PARSE, BUT DO NOT GENERATE ENTRIES IN THE PHON ARRAY

            DC.B        '0',0
            DC.B        '1',0
            DC.B        '2',0
            DC.B        '3',0
            DC.B        '4',0
            DC.B        '5',0
            DC.B        '6',0
            DC.B        '7',0
            DC.B        '8',0
            DC.B        '9',0
DIGRAFCT    EQU         (*-DIGRAPHS)/2

            LIST


