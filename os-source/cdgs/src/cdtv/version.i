VERSION         EQU     37
REVISION        EQU     12

VSTRING MACRO
                dc.b    'cdtv 37.12 (3.9.92)',13,10,0,0
        ENDM
VERSTAG MACRO
                dc.b    0,'$VER: cdtv 37.12 (3.9.92)',0,0
        ENDM
