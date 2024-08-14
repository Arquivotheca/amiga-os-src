
; Structure defs for buffer mem

AmigaPCX       EQU   0
AmigaPCY       EQU   2
AmigaPCLeftP   EQU  4
AmigaPCRightP  EQU 6
AmigaPCLeftR   EQU  8
AmigaPCRightR  EQU 10
AmigaPCStatus  EQU 12
PCAmigaX       EQU 14
PCAmigaY       EQU 16
PCVideoMode    EQU  18

; structure defs for param mem


PCParam     EQU   2
WriteLock   EQU   5

; Access masks

WordParam   EQU  MEM_WORDACCESS+MEMF_PARAMETER
WordBuff    EQU  MEM_WORDACCESS+MEMF_BUFFER

; Constant defs

DataSize    EQU  64
ParamSize   EQU  8
JIntNum     EQU   17
