************************************************************************
*                                                                      *
*   CD-ROM Hardware Include File                                       *
*                                                                      *
*       CDGS Hardware register definitions                             *
*                                                                      *
************************************************************************


CDHARDWARE      equ     $B80000         ; Hardware base offset

CDCHIPVERSION   equ     $0000           ; LONG  Revision register
CDSTATUS        equ     $0004           ; LONG  Interrupt status register
CDINT2ENABLE    equ     $0008           ; LONG  Int 2 interrupt enable register
INTB_SUBCODE        equ 31              ;       Frame interrupt
INTB_DRIVEXMIT      equ 30              ;       Drive command transmit
INTB_DRIVERECV      equ 29              ;       Drive command receive
INTB_RXDMADONE      equ 28              ;       Receive DMA complete
INTB_TXDMADONE      equ 27              ;       Transmit DMA complete
INTB_PBX            equ 26              ;       CD-ROM data available
INTB_OVERFLOW       equ 25              ;       No free buffers available
INTF_SUBCODE        equ $80000000
INTF_DRIVEXMIT      equ $40000000
INTF_DRIVERECV      equ $20000000
INTF_RXDMADONE      equ $10000000
INTF_TXDMADONE      equ $08000000
INTF_PBX            equ $04000000
INTF_OVERFLOW       equ $02000000


CDROMHIGH       equ     $0010           ; LONG  CD-ROM data high DMA address bits 
CDCOMHIGH       equ     $0014           ; LONG  Drive communications data high address bits
CDSUBINX        equ     $0018           ; BYTE  Drive subcode page index
CDCOMTXINX      equ     $0019           ; BYTE  Drive communications receive page index
CDCOMRXINX      equ     $001A           ; BYTE  Drive communications transmit page index
CDCOMTXCMP      equ     $001D           ; BYTE  Drive communications transmit compare register
CDCOMRXCMP      equ     $001F           ; BYTE  Drive communications receive compare register

CDPBX           equ     $0020           ; LONG  bits identifying used/unused buffers
CDCONFIG        equ     $0024           ; LONG  Drive configuration
CB_SUBCODE          equ 31              ;       Subcode DMA enable
CB_CDCOMTX          equ 30
CB_CDCOMRX          equ 29
CB_DBLCAS           equ 28
CB_PBX              equ 27              ;       PBX enable
CB_CDROM            equ 26              ;       CDROM data path enable
CB_DATA             equ 25              ;       RAW data mode enable
CB_2500Q            equ 24              ;       2500Q mode
CB_PALNTSC          equ 23              ;       PAL/NTSC config
CF_SUBCODE          equ $80000000
CF_CDCOMTX          equ $40000000
CF_CDCOMRX          equ $20000000
CF_DBLCAS           equ $10000000
CF_PBX              equ $08000000
CF_CDROM            equ $04000000
CF_DATA             equ $02000000
CF_2500Q            equ $01000000
CF_PALNTSC          equ $00800000
CDCOMPORT       equ     $0028           ; BYTE  Write a command byte to the Drive
CDPORTDAT       equ     $002C           ; BYTE  Twiddle the bits, or read the bits
CDPORTDIR       equ     $002E           ; BYTE  Port direction

CIAAPRA         equ     $BFE001         ; Address to find mute signal
MUTEB           equ     0               ; Mute bit
MUTEF           equ     $01
