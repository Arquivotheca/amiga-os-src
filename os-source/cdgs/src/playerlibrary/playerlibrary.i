
        IFND    PLAYER_PLAYER_I
PLAYER_PLAYER_I      SET     1


PLAYERLIBRARYNAME      MACRO

        dc.b    'player.library',0
        ds.w    0
        ENDM


;
;***********************************************************************

        STRUCTURE PlayerOptions,0

                BYTE    plo_Loop        ; 0 = Disabled, 1 = Enabled

                BYTE    plo_Intro       ; 0 = Disabled, 1 = Enabled

                BYTE    plo_TimeMode    ; 0 =  Track Relative      
                                        ; 1 = -Track Relative      
                                        ; 2 =  Disk Absolute       
                                        ; 3 = -Disk Absolute       

                BYTE    plo_Subcode     ; 0 = Disabled, 1 = Enabled

                LABEL   plo_SIZE

;***********************************************************************

        STRUCTURE PlayerState,0

                UBYTE   pls_AudioDisk   ; An Audio disk is present            
                UBYTE   pls_Tracks      ; Number of tracks on audio disk
                UBYTE   pls_ListIndex   ; Current position of player in list  
                                        ;   (values = 1-99 & 0 (not selected))
                UBYTE   pls_LastModify  ; Last to modify PlayList
                                        ; (0 = 68000, 1 = internal player)
                UBYTE   pls_PlayMode    ; PLM_NORMAL, PLM_FFWD, PLM_FREV,     
                                        ;   PLM_SKIPFWD, PLM_SKIPREV          
                UBYTE   pls_PlayState   ; PLS_STOPPED, PLS_SELECTED,
                                        ;   PLS_PLAYING, PLS_PAUSED
    
                UBYTE   pls_Track       ; Current value in TRACK field of VFD 
                UBYTE   pls_Minute      ; Current value in MINUTE field of VFD
                UBYTE   pls_Second      ; Current value in HOUR field of VFD  
                                        ;   (values = 0-99 & 100 (blank))     
                UBYTE   pls_Minus       ; Current value in MINUS-SIGN field of
                                        ; VFD time display

                LABEL   pls_SIZE

PLM_NORMAL      equ     0
PLM_FFWD        equ     1
PLM_FREV        equ     2
PLM_SKIPFWD     equ     3
PLM_SKIPREV     equ     4

PLS_STOPPED     equ     0
PLS_SELECTED    equ     1
PLS_NUMENTRY    equ     2
PLS_PLAYING     equ     3
PLS_PAUSED      equ     4


;***********************************************************************

PKSF_STROKEDIR  equ     $80             ; Stroke direction (clear = down, set = up)
PKSB_STROKEDIR  equ     7
PKSF_PRESS      equ     $00
PKSF_RELEASE    equ     $80

PKS_STOP        equ     $72
PKS_PLAYPAUSE   equ     $73
PKS_REVERSE     equ     $74
PKS_FORWARD     equ     $75
PKS_EJECT       equ     $76

;
;***********************************************************************

        STRUCTURE PlayList,0

                UBYTE   pll_EntryCount
                STRUCT  pll_Entry,100
                UBYTE   pll_pad

                LABEL   pll_SIZE



PLEF_ENABLE     equ     $80
PLEF_TRACK      equ     $7F

PLEB_ENABLE     equ     7



RES_RESERVED    EQU     0
RES_USERDEF     EQU     LIB_BASE-(RES_RESERVED*LIB_VECTSIZE)
RES_NONSTD      EQU     RES_USERDEF

RESINIT         MACRO   ; [baseOffset ]
                IFC     '\1',''
COUNT_RES       SET     RES_USERDEF
                ENDC
                IFNC    '\1',''
COUNT_RES       SET     \1
                ENDC
                ENDM


RESDEF          MACRO   ; library Function Symbol
\1              EQU     COUNT_RES
COUNT_RES       SET     COUNT_RES-LIB_VECTSIZE
                ENDM

        RESINIT

        RESDEF  _LVOOwnPlayer
        RESDEF  _LVOReleasePlayer
        RESDEF  _LVOGetOptions
        RESDEF  _LVOSetOptions
        RESDEF  _LVOGetPlayerState
        RESDEF  _LVOObtainPlayList
        RESDEF  _LVOSubmitKeyStroke

        ENDC

