
/***************************************************************************
 * 
 * Ascii Translation Tables  --  Zaphod Project
 *
 * Copyright (C) 1988, Commodore-Amiga, Inc.
 * 
 * CONFIDENTIAL and PROPRIETARY
 *
 * HISTORY    Name         Description
 * ---------  -----------  -------------------------------------------------
 * 18 jul 89   -BK         Added function entry debug kprintfs to EVERY
 *                         function. The Exorcism of RJ begins!
 * 2 Nov 88   =RJ Mical=   Created this file
 *
 **************************************************************************/

#include <exec/types.h>

/* Conversion table translates between Amiga and IBM character set 
 * PC char is index, entry is char to send to Amiga
 */
UBYTE   PCToAmigaTable[256] = 
   {
   0x00,   /* Entry 00 */
   0x01,   /* Entry 01 */
   0x02,   /* Entry 02 */
   0x03,   /* Entry 03 */
   0x04,   /* Entry 04 */
   0x05,   /* Entry 05 */
   0x06,   /* Entry 06 */
   0x07,   /* Entry 07 */
   0x08,   /* Entry 08 */
   0x09,   /* Entry 09 */
   0x0a,   /* Entry 0a */
   0x0b,   /* Entry 0b */
   0x0c,   /* Entry 0c */
   0x0d,   /* Entry 0d */
   0x0e,   /* Entry 0e */
   0x0f,   /* Entry 0f */
   0x10,   /* Entry 10 */
   0x11,   /* Entry 11 */
   0x12,   /* Entry 12 */
   0x13,   /* Entry 13 */
   0x14,   /* Entry 14 */
   0x15,   /* Entry 15 */
   0x16,   /* Entry 16 */
   0x17,   /* Entry 17 */
   0x18,   /* Entry 18 */
   0x19,   /* Entry 19 */
   0x1a,   /* Entry 1a */
   0x1b,   /* Entry 1b */
   0x1c,   /* Entry 1c */
   0x1d,   /* Entry 1d */
   0x1e,   /* Entry 1e */
   0x1f,   /* Entry 1f */
   0x20,   /* Entry 20 */
   0x21,   /* Entry 21 */
   0x22,   /* Entry 22 */
   0x23,   /* Entry 23 */
   0x24,   /* Entry 24 */
   0x25,   /* Entry 25 */
   0x26,   /* Entry 26 */
   0x27,   /* Entry 27 */
   0x28,   /* Entry 28 */
   0x29,   /* Entry 29 */
   0x2a,   /* Entry 2a */
   0x2b,   /* Entry 2b */
   0x2c,   /* Entry 2c */
   0x2d,   /* Entry 2d */
   0x2e,   /* Entry 2e */
   0x2f,   /* Entry 2f */
   0x30,   /* Entry 30 */
   0x31,   /* Entry 31 */
   0x32,   /* Entry 32 */
   0x33,   /* Entry 33 */
   0x34,   /* Entry 34 */
   0x35,   /* Entry 35 */
   0x36,   /* Entry 36 */
   0x37,   /* Entry 37 */
   0x38,   /* Entry 38 */
   0x39,   /* Entry 39 */
   0x3a,   /* Entry 3a */
   0x3b,   /* Entry 3b */
   0x3c,   /* Entry 3c */
   0x3d,   /* Entry 3d */
   0x3e,   /* Entry 3e */
   0x3f,   /* Entry 3f */
   0x40,   /* Entry 40 */
   0x41,   /* Entry 41 */
   0x42,   /* Entry 42 */
   0x43,   /* Entry 43 */
   0x44,   /* Entry 44 */
   0x45,   /* Entry 45 */
   0x46,   /* Entry 46 */
   0x47,   /* Entry 47 */
   0x48,   /* Entry 48 */
   0x49,   /* Entry 49 */
   0x4a,   /* Entry 4a */
   0x4b,   /* Entry 4b */
   0x4c,   /* Entry 4c */
   0x4d,   /* Entry 4d */
   0x4e,   /* Entry 4e */
   0x4f,   /* Entry 4f */
   0x50,   /* Entry 50 */
   0x51,   /* Entry 51 */
   0x52,   /* Entry 52 */
   0x53,   /* Entry 53 */
   0x54,   /* Entry 54 */
   0x55,   /* Entry 55 */
   0x56,   /* Entry 56 */
   0x57,   /* Entry 57 */
   0x58,   /* Entry 58 */
   0x59,   /* Entry 59 */
   0x5a,   /* Entry 5a */
   0x5b,   /* Entry 5b */
   0x5c,   /* Entry 5c */
   0x5d,   /* Entry 5d */
   0x5e,   /* Entry 5e */
   0x5f,   /* Entry 5f */
   0x60,   /* Entry 60 */
   0x61,   /* Entry 61 */
   0x62,   /* Entry 62 */
   0x63,   /* Entry 63 */
   0x64,   /* Entry 64 */
   0x65,   /* Entry 65 */
   0x66,   /* Entry 66 */
   0x67,   /* Entry 67 */
   0x68,   /* Entry 68 */
   0x69,   /* Entry 69 */
   0x6a,   /* Entry 6a */
   0x6b,   /* Entry 6b */
   0x6c,   /* Entry 6c */
   0x6d,   /* Entry 6d */
   0x6e,   /* Entry 6e */
   0x6f,   /* Entry 6f */
   0x70,   /* Entry 70 */
   0x71,   /* Entry 71 */
   0x72,   /* Entry 72 */
   0x73,   /* Entry 73 */
   0x74,   /* Entry 74 */
   0x75,   /* Entry 75 */
   0x76,   /* Entry 76 */
   0x77,   /* Entry 77 */
   0x78,   /* Entry 78 */
   0x79,   /* Entry 79 */
   0x7a,   /* Entry 7a */
   0x7b,   /* Entry 7b */
   0x7c,   /* Entry 7c */
   0x7d,   /* Entry 7d */
   0x7e,   /* Entry 7e */
   0x7f,   /* Entry 7f */
   0xc7,   /* Entry 80 */
   0xfc,   /* Entry 81 */
   0xe9,   /* Entry 82 */
   0xe2,   /* Entry 83 */
   0xe4,   /* Entry 84 */
   0xe0,   /* Entry 85 */
   0xe5,   /* Entry 86 */
   0xe7,   /* Entry 87 */
   0xea,   /* Entry 88 */
   0xeb,   /* Entry 89 */
   0xe8,   /* Entry 8a */
   0xef,   /* Entry 8b */
   0xee,   /* Entry 8c */
   0xec,   /* Entry 8d */
   0xc4,   /* Entry 8e */
   0xc5,   /* Entry 8f */
   0xc9,   /* Entry 90 */
   0xe6,   /* Entry 91 */
   0xc6,   /* Entry 92 */
   0xf4,   /* Entry 93 */
   0xf6,   /* Entry 94 */
   0xf2,   /* Entry 95 */
   0xfb,   /* Entry 96 */
   0xf9,   /* Entry 97 */
   0xff,   /* Entry 98 */
   0xd6,   /* Entry 99 */
   0xdc,   /* Entry 9a */
   0xa2,   /* Entry 9b */
   0xa3,   /* Entry 9c */
   0xa5,   /* Entry 9d */
   0x7f,   /* Entry 9e */
   0x7f,   /* Entry 9f */
   0xe1,   /* Entry a0 */
   0xed,   /* Entry a1 */
   0xf3,   /* Entry a2 */
   0xfa,   /* Entry a3 */
   0xf1,   /* Entry a4 */
   0xd1,   /* Entry a5 */
   0xaa,   /* Entry a6 */
   0xba,   /* Entry a7 */
   0xbf,   /* Entry a8 */
   0x7f,   /* Entry a9 */
   0xac,   /* Entry aa */
   0xbd,   /* Entry ab */
   0xbc,   /* Entry ac */
   0xa1,   /* Entry ad */
   0xab,   /* Entry ae */
   0xbb,   /* Entry af */
   0x7f,   /* Entry b0 */
   0x7f,   /* Entry b1 */
   0x7f,   /* Entry b2 */
   0x7f,   /* Entry b3 */
   0x7f,   /* Entry b4 */
   0x7f,   /* Entry b5 */
   0x7f,   /* Entry b6 */
   0x7f,   /* Entry b7 */
   0x7f,   /* Entry b8 */
   0x7f,   /* Entry b9 */
   0x7f,   /* Entry ba */
   0x7f,   /* Entry bb */
   0x7f,   /* Entry bc */
   0x7f,   /* Entry bd */
   0x7f,   /* Entry be */
   0x7f,   /* Entry bf */
   0x7f,   /* Entry c0 */
   0x7f,   /* Entry c1 */
   0x7f,   /* Entry c2 */
   0x7f,   /* Entry c3 */
   0x7f,   /* Entry c4 */
   0x7f,   /* Entry c5 */
   0x7f,   /* Entry c6 */
   0x7f,   /* Entry c7 */
   0x7f,   /* Entry c8 */
   0x7f,   /* Entry c9 */
   0x7f,   /* Entry ca */
   0x7f,   /* Entry cb */
   0x7f,   /* Entry cc */
   0x7f,   /* Entry cd */
   0x7f,   /* Entry ce */
   0x7f,   /* Entry cf */
   0x7f,   /* Entry d0 */
   0x7f,   /* Entry d1 */
   0x7f,   /* Entry d2 */
   0x7f,   /* Entry d3 */
   0x7f,   /* Entry d4 */
   0x7f,   /* Entry d5 */
   0x7f,   /* Entry d6 */
   0x7f,   /* Entry d7 */
   0x7f,   /* Entry d8 */
   0x7f,   /* Entry d9 */
   0x7f,   /* Entry da */
   0x7f,   /* Entry db */
   0x7f,   /* Entry dc */
   0x7f,   /* Entry dd */
   0x7f,   /* Entry de */
   0x7f,   /* Entry df */
   0x7f,   /* Entry e0 */
   0xdf,   /* Entry e1 */
   0x7f,   /* Entry e2 */
   0x7f,   /* Entry e3 */
   0x7f,   /* Entry e4 */
   0x7f,   /* Entry e5 */
   0xb5,   /* Entry e6 */
   0x7f,   /* Entry e7 */
   0x7f,   /* Entry e8 */
   0x7f,   /* Entry e9 */
   0x7f,   /* Entry ea */
   0xf0,   /* Entry eb */
   0x7f,   /* Entry ec */
   0xf8,   /* Entry ed */
   0x7f,   /* Entry ee */
   0x7f,   /* Entry ef */
   0x7f,   /* Entry f0 */
   0xb1,   /* Entry f1 */
   0x7f,   /* Entry f2 */
   0x7f,   /* Entry f3 */
   0x7f,   /* Entry f4 */
   0x7f,   /* Entry f5 */
   0xf7,   /* Entry f6 */
   0x7f,   /* Entry f7 */
   0xb0,   /* Entry f8 */
   0xb7,   /* Entry f9 */
   0x7f,   /* Entry fa */
   0x7f,   /* Entry fb */
   0x7f,   /* Entry fc */
   0xb2,   /* Entry fd */
   0xaf,   /* Entry fe */
   0x7f,   /* Entry ff */
};

/* Conversion table translates between Amiga and IBM character set
 * Amiga char is index, entry is char to send to PC
 */
UBYTE   AmigaToPCTable[256] = 
   {
   0x00,   /* Entry 00 */
   0x01,   /* Entry 01 */
   0x02,   /* Entry 02 */
   0x03,   /* Entry 03 */
   0x04,   /* Entry 04 */
   0x05,   /* Entry 05 */
   0x06,   /* Entry 06 */
   0x07,   /* Entry 07 */
   0x08,   /* Entry 08 */
   0x09,   /* Entry 09 */
   0x0a,   /* Entry 0a */
   0x0b,   /* Entry 0b */
   0x0c,   /* Entry 0c */
   0x0d,   /* Entry 0d */
   0x0e,   /* Entry 0e */
   0x0f,   /* Entry 0f */
   0x10,   /* Entry 10 */
   0x11,   /* Entry 11 */
   0x12,   /* Entry 12 */
   0x13,   /* Entry 13 */
   0x14,   /* Entry 14 */
   0x15,   /* Entry 15 */
   0x16,   /* Entry 16 */
   0x17,   /* Entry 17 */
   0x18,   /* Entry 18 */
   0x19,   /* Entry 19 */
   0x1a,   /* Entry 1a */
   0x1b,   /* Entry 1b */
   0x1c,   /* Entry 1c */
   0x1d,   /* Entry 1d */
   0x1e,   /* Entry 1e */
   0x1f,   /* Entry 1f */
   0x20,   /* Entry 20 */
   0x21,   /* Entry 21 */
   0x22,   /* Entry 22 */
   0x23,   /* Entry 23 */
   0x24,   /* Entry 24 */
   0x25,   /* Entry 25 */
   0x26,   /* Entry 26 */
   0x27,   /* Entry 27 */
   0x28,   /* Entry 28 */
   0x29,   /* Entry 29 */
   0x2a,   /* Entry 2a */
   0x2b,   /* Entry 2b */
   0x2c,   /* Entry 2c */
   0x2d,   /* Entry 2d */
   0x2e,   /* Entry 2e */
   0x2f,   /* Entry 2f */
   0x30,   /* Entry 30 */
   0x31,   /* Entry 31 */
   0x32,   /* Entry 32 */
   0x33,   /* Entry 33 */
   0x34,   /* Entry 34 */
   0x35,   /* Entry 35 */
   0x36,   /* Entry 36 */
   0x37,   /* Entry 37 */
   0x38,   /* Entry 38 */
   0x39,   /* Entry 39 */
   0x3a,   /* Entry 3a */
   0x3b,   /* Entry 3b */
   0x3c,   /* Entry 3c */
   0x3d,   /* Entry 3d */
   0x3e,   /* Entry 3e */
   0x3f,   /* Entry 3f */
   0x40,   /* Entry 40 */
   0x41,   /* Entry 41 */
   0x42,   /* Entry 42 */
   0x43,   /* Entry 43 */
   0x44,   /* Entry 44 */
   0x45,   /* Entry 45 */
   0x46,   /* Entry 46 */
   0x47,   /* Entry 47 */
   0x48,   /* Entry 48 */
   0x49,   /* Entry 49 */
   0x4a,   /* Entry 4a */
   0x4b,   /* Entry 4b */
   0x4c,   /* Entry 4c */
   0x4d,   /* Entry 4d */
   0x4e,   /* Entry 4e */
   0x4f,   /* Entry 4f */
   0x50,   /* Entry 50 */
   0x51,   /* Entry 51 */
   0x52,   /* Entry 52 */
   0x53,   /* Entry 53 */
   0x54,   /* Entry 54 */
   0x55,   /* Entry 55 */
   0x56,   /* Entry 56 */
   0x57,   /* Entry 57 */
   0x58,   /* Entry 58 */
   0x59,   /* Entry 59 */
   0x5a,   /* Entry 5a */
   0x5b,   /* Entry 5b */
   0x5c,   /* Entry 5c */
   0x5d,   /* Entry 5d */
   0x5e,   /* Entry 5e */
   0x5f,   /* Entry 5f */
   0x60,   /* Entry 60 */
   0x61,   /* Entry 61 */
   0x62,   /* Entry 62 */
   0x63,   /* Entry 63 */
   0x64,   /* Entry 64 */
   0x65,   /* Entry 65 */
   0x66,   /* Entry 66 */
   0x67,   /* Entry 67 */
   0x68,   /* Entry 68 */
   0x69,   /* Entry 69 */
   0x6a,   /* Entry 6a */
   0x6b,   /* Entry 6b */
   0x6c,   /* Entry 6c */
   0x6d,   /* Entry 6d */
   0x6e,   /* Entry 6e */
   0x6f,   /* Entry 6f */
   0x70,   /* Entry 70 */
   0x71,   /* Entry 71 */
   0x72,   /* Entry 72 */
   0x73,   /* Entry 73 */
   0x74,   /* Entry 74 */
   0x75,   /* Entry 75 */
   0x76,   /* Entry 76 */
   0x77,   /* Entry 77 */
   0x78,   /* Entry 78 */
   0x79,   /* Entry 79 */
   0x7a,   /* Entry 7a */
   0x7b,   /* Entry 7b */
   0x7c,   /* Entry 7c */
   0x7d,   /* Entry 7d */
   0x7e,   /* Entry 7e */
/*!*/   0x20,   /* Entry 7f */
/*!*/   0x20,   /* Entry 80 */
/*!*/   0x20,   /* Entry 81 */
/*!*/   0x20,   /* Entry 82 */
/*!*/   0x20,   /* Entry 83 */
/*!*/   0x20,   /* Entry 84 */
/*!*/   0x20,   /* Entry 85 */
/*!*/   0x20,   /* Entry 86 */
/*!*/   0x20,   /* Entry 87 */
/*!*/   0x20,   /* Entry 88 */
/*!*/   0x20,   /* Entry 89 */
/*!*/   0x20,   /* Entry 8a */
/*!*/   0x20,   /* Entry 8b */
/*!*/   0x20,   /* Entry 8c */
/*!*/   0x20,   /* Entry 8d */
/*!*/   0x20,   /* Entry 8e */
/*!*/   0x20,   /* Entry 8f */
/*!*/   0x20,   /* Entry 90 */
/*!*/   0x20,   /* Entry 91 */
/*!*/   0x20,   /* Entry 92 */
/*!*/   0x20,   /* Entry 93 */
/*!*/   0x20,   /* Entry 94 */
/*!*/   0x20,   /* Entry 95 */
/*!*/   0x20,   /* Entry 96 */
/*!*/   0x20,   /* Entry 97 */
/*!*/   0x20,   /* Entry 98 */
/*!*/   0x20,   /* Entry 99 */
/*!*/   0x20,   /* Entry 9a */
/*!*/   0x20,   /* Entry 9b */
/*!*/   0x20,   /* Entry 9c */
/*!*/   0x20,   /* Entry 9d */
/*!*/   0x20,   /* Entry 9e */
/*!*/   0x20,   /* Entry 9f */
/*!*/   0x20,   /* Entry a0 */
   0xad,   /* Entry a1 */
   0x9b,   /* Entry a2 */
   0x9c,   /* Entry a3 */
/*!*/   0x20,   /* Entry a4 */
   0x9d,   /* Entry a5 */
/*!*/   0x20,   /* Entry a6 */
/*!*/   0x20,   /* Entry a7 */
/*!*/   0x20,   /* Entry a8 */
/*!*/   0x20,   /* Entry a9 */
   0xa6,   /* Entry aa */
   0xae,   /* Entry ab */
   0xaa,   /* Entry ac */
/*!*/   0x20,   /* Entry ad */
/*!*/   0x20,   /* Entry ae */
   0xfe,   /* Entry af */
   0xf8,   /* Entry b0 */
   0xf1,   /* Entry b1 */
   0xfd,   /* Entry b2 */
/*!*/   0x20,   /* Entry b3 */
/*!*/   0x20,   /* Entry b4 */
   0xe6,   /* Entry b5 */
/*!*/   0x20,   /* Entry b6 */
   0xf9,   /* Entry b7 */
/*!*/   0x20,   /* Entry b8 */
/*!*/   0x20,   /* Entry b9 */
   0xa7,   /* Entry ba */
   0xaf,   /* Entry bb */
   0xac,   /* Entry bc */
   0xab,   /* Entry bd */
/*!*/   0x20,   /* Entry be */
   0xa8,   /* Entry bf */
/*!*/   0x20,   /* Entry c0 */
/*!*/   0x20,   /* Entry c1 */
/*!*/   0x20,   /* Entry c2 */
/*!*/   0x20,   /* Entry c3 */
   0x8e,   /* Entry c4 */
   0x8f,   /* Entry c5 */
   0x92,   /* Entry c6 */
   0x80,   /* Entry c7 */
/*!*/   0x20,   /* Entry c8 */
   0x90,   /* Entry c9 */
/*!*/   0x20,   /* Entry ca */
/*!*/   0x20,   /* Entry cb */
/*!*/   0x20,   /* Entry cc */
/*!*/   0x20,   /* Entry cd */
/*!*/   0x20,   /* Entry ce */
/*!*/   0x20,   /* Entry cf */
/*!*/   0x20,   /* Entry d0 */
   0xa5,   /* Entry d1 */
/*!*/   0x20,   /* Entry d2 */
/*!*/   0x20,   /* Entry d3 */
/*!*/   0x20,   /* Entry d4 */
/*!*/   0x20,   /* Entry d5 */
   0x99,   /* Entry d6 */
/*!*/   0x20,   /* Entry d7 */
/*!*/   0x20,   /* Entry d8 */
/*!*/   0x20,   /* Entry d9 */
/*!*/   0x20,   /* Entry da */
/*!*/   0x20,   /* Entry db */
   0x9a,   /* Entry dc */
/*!*/   0x20,   /* Entry dd */
/*!*/   0x20,   /* Entry de */
   0xe1,   /* Entry df */
   0x85,   /* Entry e0 */
   0xa0,   /* Entry e1 */
   0x83,   /* Entry e2 */
/*!*/   0x20,   /* Entry e3 */
   0x84,   /* Entry e4 */
   0x86,   /* Entry e5 */
   0x91,   /* Entry e6 */
   0x87,   /* Entry e7 */
   0x8a,   /* Entry e8 */
   0x82,   /* Entry e9 */
   0x88,   /* Entry ea */
   0x89,   /* Entry eb */
   0x8d,   /* Entry ec */
   0xa1,   /* Entry ed */
   0x8c,   /* Entry ee */
   0x8b,   /* Entry ef */
   0xeb,   /* Entry f0 */
   0xa4,   /* Entry f1 */
   0x95,   /* Entry f2 */
   0xa2,   /* Entry f3 */
   0x93,   /* Entry f4 */
/*!*/   0x20,   /* Entry f5 */
   0x94,   /* Entry f6 */
   0xf6,   /* Entry f7 */
   0xed,   /* Entry f8 */
   0x97,   /* Entry f9 */
   0xa3,   /* Entry fa */
   0x96,   /* Entry fb */
   0x81,   /* Entry fc */
/*!*/   0x20,   /* Entry fd */
/*!*/   0x20,   /* Entry fe */
   0x98,   /* Entry ff */
   };