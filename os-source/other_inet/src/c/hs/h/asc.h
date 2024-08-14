/* -----------------------------------------------------------------------
 * asc.h        handshake_src
 *
 * $Locker:  $
 *
 * $Id: asc.h,v 1.1 91/05/09 14:30:50 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: HOG:Other/inet/src/c/hs/h/RCS/asc.h,v 1.1 91/05/09 14:30:50 bj Exp $
 *
 * $Log:	asc.h,v $
 * Revision 1.1  91/05/09  14:30:50  bj
 * Initial revision
 * 
 *
 *------------------------------------------------------------------------
 */

/***
*
* CSET definitions
*
***/

/* C0 Set */
#define CRC_CHAR 'C'
#define SOH     0x01
#define STX     0x02
#define EOT     0x04
#define ENQ     0x05
#define ACK     0x06
#define BELL    0x07
#define BKSPACE 0x08
#define SO      0x0e
#define SI      0x0f
#define XON     0x11
#define XOFF    0x13
#define NAK     0x15
#define CAN     0x18
#define SUB     0x1a
#define ESC     0x1b
#define BLANK   0x20
#define DEL     0x7f

/* C1 Set */
#define IND     0x84
#define NEL     0x85
#define HTS     0x88
#define RI      0x8d
#define SS2     0x8e
#define SS3     0x8f
#define DCS     0x90
#define CSI     0x9b
#define ST      0x9c
