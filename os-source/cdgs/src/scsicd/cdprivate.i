
        INCLUDE "include:exec/types.i"
        INCLUDE "include:exec/nodes.i"
        INCLUDE "include:exec/lists.i"
        INCLUDE "include:exec/ports.i"
        INCLUDE "include:exec/interrupts.i"
        INCLUDE "include:exec/libraries.i"
        INCLUDE "include:exec/tasks.i"
        INCLUDE "include:exec/io.i"
        INCLUDE "include:devices/scsidisk.i"

************************************************************************
***
***  Miscellaneous Definitions
***
************************************************************************

STACK_SIZE      equ     8192            ; task stack size
SKIP_BYTES      equ     2048            ; size of skip buffer

BLOCK_SIZE      equ     2048            ; sector size
BLOCK_SHIFT     equ     11              ; bits used in bytes per sector

PERFORMIO       equ     -42             ; for calling ourself
UNIT_PATTERN    equ     'UNIT'          ; used for unit number

ior             equr    a4              ; saved I/O request pointer
hb              equr    a5              ; hardware base pointer
db              equr    a6              ; device node pointer

CDROMPAGESIZE   equ     4096            ; just 1 buffer needed
CDCOMPAGESIZE   equ     4*256           ; 1 page of subcode, 1 page of drive packet receive, 1 page for transmit

PBXSIZE         equ     16              ; Number of buffers in PBX

ROM_STATUS      equ     $000            ; Status byte (EDC, Sync inserted, etc...)
ROM_LSNPOS      equ     $004            ; Header converted to LSN format
ROM_HEADER      equ     $00C            ; Actual MSF BCD header
ROM_DATA        equ     $010            ; Start of CD-ROM data
ROM_C2P0        equ     $C04            ; Start of C2P0 data

SECSTSB_C2P0        equ 5               ; Set if any C2 bits are set in the sector
SECSTSB_EDC1        equ 6               ; EDC error detected in mode 2 format
SECSTSB_EDC2        equ 7               ; EDC error detected in mode 1 format
SECSTSB_SHORTSECTOR equ 8               ; Short sector (unexpected sync)
SECSTSB_SYNCINSERT  equ 9               ; Sync inserted
SECSTSB_CORRECTED   equ 14              ; ECC has been performed on this sector
SECSTSB_INVALID     equ 15              ; New sector flag
SECSTSF_CNT         equ $0000001F       ; - Count mask
SECSTSF_C2P0        equ $00000020
SECSTSF_EDC1        equ $00000040
SECSTSF_EDC2        equ $00000080
SECSTSF_SHORTSECTOR equ $00000100
SECSTSF_SYNCINSERT  equ $00000200
SECSTSF_CORRECTED   equ $00004000
SECSTSF_INVALID     equ $00008000

SECSTS_ERROR        equ (SECSTSF_C2P0|SECSTSF_EDC1|SECSTSF_EDC2|SECSTSF_SHORTSECTOR)



************************************************************************
***
***  Command Options
***
************************************************************************

 BITDEF O,QUEUE,0               ; queue the command
 BITDEF O,DISKACCESS,1          ; This command accesses disk or modifies head position
 BITDEF O,MULTITASK,2           ; Other commands may concurrently execute
 BITDEF O,DISK,3                ; requires a disk in drive
 BITDEF O,NOREPLY,4             ; send no reply message
 BITDEF O,ROM,5                 ; CD-ROM disk required
 BITDEF O,INVALID,6             ; an invalid command

OF_ABTD EQU     $0100           ; abort processing function number
OF_ABTQ EQU     $0200


************************************************************************
***
***  CDTV Device Driver Base Data Segment
***
************************************************************************

 STRUCTURE CDDB,LIB_SIZE
        ALIGNLONG
        APTR        db_CDROMPage                    ; Hardware buffer pointers
        APTR        db_CDCOMRXPage
        APTR        db_CDCOMTXPage
        APTR        db_CDSUBPage

        STRUCT      db_Info,CDINFO_SIZE             ; Info structure (must remain here for CDTV compatibility)
        ALIGNLONG
        STRUCT      db_Task,TC_SIZE                 ; This task
        STRUCT      db_StatIntr,IS_SIZE             ; Device Interrupt server
        STRUCT      db_XLIntr,IS_SIZE               ; CDXL interrupt server

        ULONG       db_UNIT                         ; Sole open device number

        STRUCT      db_ClassACmdPort,MP_SIZE        ; Port for non-disk access type commands
        STRUCT      db_ClassDCmdPort,MP_SIZE        ; Port for disk access type commands
        ALIGNLONG
        APTR        db_ClassDReq                    ; active class-2 command

        APTR        db_InitTask                     ; Initialization task pointer
        UBYTE       db_InitSignal                   ; Initialization task signal

        UBYTE       db_ComRXInx                     ; Drive communications receive page index
        UBYTE       db_ComTXInx                     ; Drive communications transmit page index
        UBYTE       db_SubInx                       ; Subcode page index
        UBYTE       db_CDCOMRXCMP                   ; Drive communications receive page shadow
        UBYTE       db_ReceivingCmd                 ; Receiving a packet from the drive
        UBYTE       db_ComRXPacket                  ; Index to packet currently being received
        UBYTE       db_ReadingTOC                   ; TOC is being read flag
        UBYTE       db_Disabled                     ; Interrupts disabled flag
        UBYTE       db_NoHardware                   ; Flag is set if drive is not present
        UBYTE       db_PhotoCD                      ; Flag is set if this is a PhotoCD disk

        STRUCT      db_Packet,16                    ; Drive transmit packet

        ALIGNLONG
        ULONG       db_TOCNext                      ; Pointer to next session
        ULONG       db_MultiSession                 ; Multi-session disk possibly present?
        UBYTE       db_MSLastTrackTemp              ; Temp variables when doing multi-session
        ALIGNLONG
        ULONG       db_MSLeadOutTemp
        UBYTE       db_Remap                        ; Should sectors be remapped?
        ALIGNLONG
        ULONG       db_RemapStart                   ; Starting sector to remap
        ULONG       db_RemapStop                    ; Stopping sector to remap
        UWORD       db_VolDscType                   ; Volume descriptor type read in when attemping sector relocation
        STRUCT      db_IOR,IOSTD_SIZE               ; Fake IO Request for TOC

        ULONG       db_TimerPort                    ; Test Unit Ready timer stuff
        ULONG       db_TimerIOR
        ULONG       db_SCSIPort                     ; SCSI device port and request
        ULONG       db_SCSIIOR

        STRUCT      db_cmdblk,scsi_SIZEOF           ; SCSI command block
        STRUCT      db_command,20
        STRUCT      db_SenseData,22
        STRUCT      db_SCSIBuffer,1000
        ALIGNLONG

        UBYTE       db_PacketIndex                  ; Index ORed with command byte
        UBYTE       db_FlickerLight                 ; Light flicker flag

        UBYTE       db_IgnoreResponse               ; If non-zero, ignore responses with this command byte
        UBYTE       db_PackSize                     ; Copy of CDCOMCMP
        UBYTE       db_PacketAddress                ; Index of last drive command receive packet
        UBYTE       db_PlayStatus

        UWORD       db_CMD                          ; Primary drive communications packet
        ALIGNLONG
        ULONG       db_ARG1
        ULONG       db_ARG2
        ULONG       db_ARG3
        ULONG       db_ARG4
        ULONG       db_ARG5

        ULONG       db_PlayStart                    ; Start and stop position of a play in progress
        ULONG       db_PlayStop
        ULONG       db_LastQPos                     ; Last Q-Code position reported (BCD MSF)
        UWORD       db_LastQState                   ; State drive was in when last Q-Code packet was reported
        UWORD       db_AbortTimer
        ULONG       db_Speed

        ALIGNLONG
        STRUCT      db_XferNode,CDXL_SIZE           ; Transfer node for CD_READ command
        UWORD       db_RetryCount                   ; Read retry counter
        APTR        db_XferEntry                    ; Current transfer entry

        UBYTE       db_BufferCount                  ; Expected buffer count from status long
        UBYTE       db_ReadError
        UBYTE       db_ECC                          ; ECC enabled
        UBYTE       db_Reading                      ; Are we trying to read data
        ULONG       db_SeekAdjustment               ; Play sector adjustment
        ULONG       db_SectorIndex                  ; Index to data within a sector
        ULONG       db_ReadStart                    ; Place where read began
        ULONG       db_ListEnd                      ; Dummy end-of-list pointer (NULL)

        UBYTE       db_OpenState                    ; Drive door open state
        ALIGNLONG
        ULONG       db_ChgCount                     ; Disk change counter
        UWORD       db_CurrentSpeed                 ; Current frame rate of drive

        UWORD       db_Attenuation                  ; Attenuation data             
        UWORD       db_LastAttenuation
        UWORD       db_TargetAttenuation
        UWORD       db_FadeStepSize

        UBYTE       db_AutoQ                        ; Automatic Q-Code command
        UBYTE       db_AutoFrame                    ; Automatic Frame interrupt
        UBYTE       db_AutoFade                     ; Automatic attenuation fading

        ALIGNLONG
        STRUCT      db_ChgList,LH_SIZE              ; Interrupt lists
        STRUCT      db_FrameList,LH_SIZE

        ULONG       db_PortFrm
        ULONG       db_IORFrm
        STRUCT      db_FrameIntStruct,IS_SIZE

        STRUCT      db_TOC,TOCEntry_SIZE*101        ; Table of contents
        ALIGNLONG
        STRUCT      db_QCode,16                     ; Q-Code packet
        STRUCT      db_ModeSelect,20                ; Mode select packet

        ULONG       db_NULL                         ; Just a NULL
        LABEL       db_SizeOf


************************************************************************
***
***  Miscellaneous Macro Definitions
***
************************************************************************

FIRSTNODE       MACRO
                tst.l   4(\1)           ; is it a list header?
                bne.s   0$              ; no
                move.l  (\1),\1         ; fetch first entry
0$
                ENDM


************************************************************************
***
***  Task Internal Signal Related Definitions
***
************************************************************************

*** Task Signals

 BITDEF SIG,CMDPORT,16
 BITDEF SIG,DISKCMDPORT,17
 BITDEF SIG,PLAYDONE,18
 BITDEF SIG,DISKCHANGE,19
 BITDEF SIG,TESTUNITREADY,20

TASK_SIGS equ (SIGF_CMDPORT|SIGF_DISKCMDPORT|SIGF_CMDDONE|SIGF_PLAYDONE|SIGF_DISKCHANGE|SIGF_ABORTDRIVE|SIGF_TESTUNITREADY)

*** Class A and D Signals

 BITDEF SIG,CMDDONE,21

*** Class A Signals

 BITDEF SIG,QCODE,22
 BITDEF SIG,ABORTQCODE,23

CLASSA_SIGS     equ     (SIGF_CMDDONE|SIGF_QCODE|SIGF_ABORTQCODE)

*** Class D Signals

 BITDEF SIG,PBX,24
 BITDEF SIG,ABORTDRIVE,25

CLASSD_SIGS     equ     (SIGF_CMDDONE|SIGF_PBX|SIGF_ABORTDRIVE)

*** All Signals

SIGNALS         equ     (TASK_SIGS|CLASSA_SIGS|CLASSD_SIGS)

*************** SIGNAL our device task

SIGNAL          MACRO
                move.l  #\1,d0
                lea     db_Task(db),a1
                exec    Signal
                ENDM



************************************************************************
***
***  CD Commands
***
************************************************************************


CDCMD_SPIN              equ $0          ;     Spin motor


CDCMD_STOP              equ $1          ;     Stop motor


CDCMD_READTOC           equ $2          ;     Read Table of Contents


CDCMD_SEEK              equ $3          ;     Seek to position on disk
                                        ;     <POSH><POSM><POSL>

CDCMD_PLAY              equ $4          ; (M) Play from start, stop before end
                                        ;     <SMIN><SSEC><SFRM> <EMIN><ESEC><EFRM> <MODE>

CDCMD_READ              equ $5          ;     Read bytes starting at POS and read LEN bytes. Transfer to ADR, handshake after XLCNT bytes
                                        ;     <POSH><POSM><POSL> <LENH><LENM><LENL> <ADRH><ADRMH><ADRML><ADRL> <XLCNTH><XLCNTM><XLCNTL>

CDCMD_PAUSE             equ $6          ; (m) Pause Play                           


CDCMD_RESUME            equ $7          ; (m) Resume Play                          


CDCMD_SETPLAYMODE       equ $8          ; (m) Set Speed/Direction of Play
                                        ;     <MODE>

                                        ;     76543210
                                        ;     000000--      Must be zero
                                        ;     ------00      Standard play
                                        ;     ------01      Fast Forward
                                        ;     ------10      Fast Reverse
                                        ;     ------11      Double speed play

CDCMD_QCODE             equ $9          ;     Retrieve Q-Code packet from drive

CDCMD_ID                equ $A          ;     Get ID packet from drive


S_TEST_UNIT_READY       equ $00
S_REZERO_UNIT           equ $01
S_REQUEST_SENSE         equ $03
S_FORMAT_UNIT           equ $04
S_REASSIGN_BLOCKS       equ $07
S_READ                  equ $08
S_WRITE                 equ $0a
S_SEEK                  equ $0b
S_INQUIRY               equ $12
S_MODE_SELECT           equ $15
S_RESERVE               equ $16
S_RELEASE               equ $17
S_COPY                  equ $18
S_MODE_SENSE            equ $1a
S_START_STOP_UNIT       equ $1b
S_PREVENT_ALLOW_REMOVAL equ $1e
S_READ_CAPACITY         equ $25
S_READ10                equ $28
S_WRITE10               equ $2a
S_SEEK10                equ $2b
S_WRITE_VERIFY          equ $2e
S_VERIFY                equ $2F
S_PREFETCH              equ $34
S_SYNCHRONIZE_CACHE     equ $35
S_LOCK_UNLOCK_CACHE     equ $36
S_READ_DEFECT_DATA      equ $37
S_WRITE_BUFFER          equ $3b
S_READ_BUFFER           equ $3c
S_READ_SUB_CHANNEL      equ $42
S_READTOC               equ $43
S_READ_HEADER           equ $44
S_PLAY_AUDIO_MSF        equ $47
S_PAUSE_RESUME          equ $4b
S_MODE_SELECT_10        equ $55
S_MODE_SENSE_10         equ $5a

;/* sense codes */
CHECK_CONDITION         equ $02

;/* sense keys */
RECOVERED_ERROR         equ $01
MEDIUM_ERROR            equ $03
HARDWARE_ERROR          equ $04
ILLEGAL_REQUEST         equ $05

SCSI_READ               equ 0
SCSI_WRITE              equ 1
SCSI_AUTOSENSE          equ 2


AUDIOSTAT_INVALID       equ $00
AUDIOSTAT_PLAYING       equ $11
AUDIOSTAT_PAUSED        equ $12
AUDIOSTAT_DONE          equ $13
AUDIOSTAT_ERROR         equ $14
AUDIOSTAT_NOSTAT        equ $15



;       *********************** Chinon Command values ********************

CHCMD_INIT              equ     $00
CHCMD_STOP              equ     $01
CHCMD_PAUSE             equ     $02
CHCMD_PLAY              equ     $03
CHCMD_SETPLAY           equ     $04
CHCMD_QCODE             equ     $06
CHCMD_SENDID            equ     $07
CHCMD_SEEK              equ     $08
CHCMD_ABORT             equ     $09
CHCMD_DISKCHANGE        equ     $0A

************************************************************************
***
***  Error definitions
***
************************************************************************


CD_ERR_INVALIDCOMMAND   equ     1       ; invalid command                                                               
CD_ERR_CMDINCOMPLETE    equ     2       ; you tried to send a command before previous command was complete              
                                        ;   ... and you probably just royally screwed things up.                        
                                        ;   I'd recommend doing a RESET at this point.                                  
CD_ERR_CHINON           equ     3       ; chinon chip reported an unexpected error                                      
CD_ERR_CXD1186Q         equ     4       ; CXD1186Q reported an unexpected error                                         
CD_ERR_DECTIMEOUT       equ     5       ; 1186 decoder timeout                                                          
CD_ERR_NOTFOUND         equ     6       ; Could not read information                                                    
CD_ERR_NOTOC            equ     7       ; Table of contents not read, can't do a PLAYTRACK                              
CD_ERR_INVALIDTRACK     equ     8       ; The track(s) you specified are invalid                                        
CD_ERR_SEEKERROR        equ     9       ; Seek past end of disk                                                         
CD_ERR_REVERSEINDEX     equ     10      ; Play track with a subindex greater than 1 cannot be initiated in reverse mode 
CD_ERR_EJECTABORT       equ     11      ; Eject button was pressed while command was in progress                        
CD_ERR_DRIVELOST        equ     12      ; Drive seems confused as to where it should be                                 
                                                                                                                        
CD_ERR_INVALIDPHASE     equ     13      ; phase error (should be impossible as well)                                    
CD_ERR_INVALIDSTATE     equ     14      ; Drive is not in proper state to execute this command                          
CD_ERR_INTERRUPTPHASE   equ     15      ; interrupt occured at an unexpected time                                       
CD_ERR_BADPACKET        equ     16      ; Bad CheckSum in packet from chinon                                            


***  Errors returned by Chinon drive

CH_ERR_BADCOMMAND       equ     %10000000
CH_ERR_CHECKSUM         equ     %10001000
CH_ERR_DRAWERSTUCK      equ     %10010000
CH_ERR_DISKUNREADABLE   equ     %10011000
CH_ERR_INVALIDADDRESS   equ     %10100000
CH_ERR_WRONGDATA        equ     %10101000
CH_ERR_FOCUSERROR       equ     %11001000
CH_ERR_SPINDLEERROR     equ     %11010000
CH_ERR_TRACKINGERROR    equ     %11011000
CH_ERR_SLEDERROR        equ     %11100000
CH_ERR_TRACKJUMP        equ     %11101000
CH_ERR_ABNORMALSEEK     equ     %11110000
CH_ERR_NODISK           equ     %11111000


PRINT           MACRO
                XREF    Print

                save    a0
                lea     90\1(pc),a0
                jsr     Print
                bra     95\1
90\1              dc.b    \2,10,0
                ds.w    0
95\1             
                restore a0
                ENDM

