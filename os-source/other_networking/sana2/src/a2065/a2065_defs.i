**
** A2065 SANA-II Device Driver source
** Copyright (c) 1992 by Commodore-Amiga, Inc.
**
** All rights Reserved.
**


MANUFACTURER    EQU     $202
PRODUCT         EQU     $70
MAN_AMERISTAR   EQU     1053
PROD_AMERISTAR  EQU     1
PROD_AMERISTAR_500 EQU  10
CHIP_OFFSET     EQU     $4000
CHIP_OFFSET_500 EQU     $24000
MEMORY_OFFSET   EQU     $8000
MEMORY_OFFSET_500 EQU   $60000
COMMODORE       EQU     $00801000
AMERISTAR       equ     $00009f00
INTERRUPT_LIST  EQU     INTB_PORTS
INTERRUPT_MASK  EQU     INTF_PORTS

LANCE_RAP       EQU     2
LANCE_RDP       EQU     0

CSR0            EQU     0
CSR1            EQU     1
CSR2            EQU     2
CSR3            EQU     3

RLEN_1          EQU     0
RLEN_2          EQU     1
RLEN_4          EQU     2
RLEN_8          EQU     3
RLEN_16         EQU     4
RLEN_32         EQU     5
RLEN_64         EQU     6
RLEN_128        EQU     7

        BITDEF  CSR0,INIT,0
        BITDEF  CSR0,STRT,1
        BITDEF  CSR0,STOP,2
        BITDEF  CSR0,TDMD,3
        BITDEF  CSR0,TXON,4
        BITDEF  CSR0,RXON,5
        BITDEF  CSR0,INEA,6
        BITDEF  CSR0,INTR,7
        BITDEF  CSR0,IDON,8
        BITDEF  CSR0,TINT,9
        BITDEF  CSR0,RINT,10
        BITDEF  CSR0,MERR,11
        BITDEF  CSR0,MISS,12
        BITDEF  CSR0,CERR,13
        BITDEF  CSR0,BABL,14
        BITDEF  CSR0,ERR,15

        BITDEF  CSR3,BCON,0
        BITDEF  CSR3,ACON,1
        BITDEF  CSR3,BSWP,2

        BITDEF  MODE,DRX,0
        BITDEF  MODE,DTX,1
        BITDEF  MODE,LOOP,2
        BITDEF  MODE,DTCR,3
        BITDEF  MODE,COLL,4
        BITDEF  MODE,DRTY,5
        BITDEF  MODE,INTL,6
        BITDEF  MODE,PROM,15

        BITDEF  RMD1,ENP,8
        BITDEF  RMD1,STP,9
        BITDEF  RMD1,BUFF,10
        BITDEF  RMD1,CRC,11
        BITDEF  RMD1,OFLO,12
        BITDEF  RMD1,FRAM,13
        BITDEF  RMD1,ERR,14
        BITDEF  RMD1,OWN,15

        BITDEF  TMD1,ENP,8
        BITDEF  TMD1,STP,9
        BITDEF  TMD1,DEF,10
        BITDEF  TMD1,ONE,11
        BITDEF  TMD1,MORE,12
        BITDEF  TMD1,ERR,14
        BITDEF  TMD1,OWN,15

        BITDEF  TMD3,RTRY,2
        BITDEF  TMD3,LCAR,3
        BITDEF  TMD3,LCOL,4
        BITDEF  TMD3,UFLO,6
        BITDEF  TMD3,BUFF,7


        STRUCTURE       RMD,0
        UWORD           RMD0
        UWORD           RMD1
        UWORD           RMD2
        UWORD           RMD3
        LABEL           RMD_SIZE

        STRUCTURE       TMD,0
        UWORD           TMD0
        UWORD           TMD1
        UWORD           TMD2
        UWORD           TMD3
        LABEL           TMD_SIZE

;----------------------------------------------------------------------------
; Memory Struct For The Commodore Ethernet Board As Supported By This Driver
;
; ADDR  0-1             Mode
;       2-7             Physical Address
;       8-15            Logical Address Filter
;       16-19           Address Of Receive Buffer Ring
;       20-23           Address Of Transmit Buffer Ring
;
;       24-31           The Single Transmit Descriptor
;       32-159          The Receive Descriptors (16 of them)
;
;       160-1695        The One Transmit Buffer (1536 Bytes Long)
;       1696-26271      The Receive Buffers (16 @ 1536 Each)
;       26272-32767     UNUSED
;----------------------------------------------------------------------------

TXBUF_COUNT     EQU             4
TX_RING_LENGTH  EQU             RLEN_4
W_BUFF_MASK     EQU             TXBUF_COUNT-1

RXBUF_COUNT     EQU             16
RX_RING_LENGTH  EQU             RLEN_16

RXBUFLEN        EQU             1536
TXBUFLEN        EQU             1536

        STRUCTURE       LANCE_MEMORY,0
        USHORT          LANCE_MODE
        STRUCT          LANCE_PA,6
        STRUCT          LANCE_LA,8
        APTR            LANCE_RX_RING_POINTER
        APTR            LANCE_TX_RING_POINTER
        STRUCT          LANCE_TX_RING,8*TXBUF_COUNT
        STRUCT          LANCE_RX_RING,8*RXBUF_COUNT
        STRUCT          LANCE_TX_BUFFER,TXBUFLEN*TXBUF_COUNT
        STRUCT          LANCE_RX_BUFFER,RXBUFLEN*RXBUF_COUNT
        LABEL           LANCE_USED_MEMORY
        STRUCT          LANCE_UNUSED,32768-LANCE_USED_MEMORY
        LABEL           LANCE_MEM_SIZE

TXRING_0        EQU             (MEMORY_OFFSET+LANCE_TX_RING)
TXRING_2        EQU             (TX_RING_LENGTH<<13)

RXRING_0        EQU             (MEMORY_OFFSET+LANCE_RX_RING)
RXRING_2        EQU             (RX_RING_LENGTH<<13)

        STRUCTURE       EPACKET,0
        STRUCT          EP_DSTADDR,6
        STRUCT          EP_SRCADDR,6
        STRUCT          EP_PTYPE,2
        LABEL           EP_SIZE

