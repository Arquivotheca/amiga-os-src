/************************************************************************
**    filltab.h                                                        **
**    -------------------------------------------------------------    **
**    Copyright 1986 by Compugraphic, Inc. All rights reserved.        **
*************************************************************************
    Product:   Intellifont Sub-System	
    Component: Tables used by the fill module	
    Author:    Al Ristow	
*************************************************************************/
/* History                                                          */
/*   filltab.h    29-Nov-85                                         */
/*                 4-Oct-89  removed last two complimentary tables  */
/*                                                                  */
/*  These tables are used by the "fill" module to generate bit maps */
/*  from transition arrays.                                         */
/*                                                                  */
/*  These tables were generated (in crude form) by the program      */
/*  "b.c" and then edited to their current exquisite form.          */
/*                                                                  */
/********************************************************************/


static
char const far off_tab[256] =

            { 0x00, 0x01, 0x03, 0x02, 0x07, 0x06, 0x04, 0x05, 
              0x0F, 0x0E, 0x0C, 0x0D, 0x08, 0x09, 0x0B, 0x0A, 
              0x1F, 0x1E, 0x1C, 0x1D, 0x18, 0x19, 0x1B, 0x1A, 
              0x10, 0x11, 0x13, 0x12, 0x17, 0x16, 0x14, 0x15, 
              0x3F, 0x3E, 0x3C, 0x3D, 0x38, 0x39, 0x3B, 0x3A, 
              0x30, 0x31, 0x33, 0x32, 0x37, 0x36, 0x34, 0x35, 
              0x20, 0x21, 0x23, 0x22, 0x27, 0x26, 0x24, 0x25, 
              0x2F, 0x2E, 0x2C, 0x2D, 0x28, 0x29, 0x2B, 0x2A, 
              0x7F, 0x7E, 0x7C, 0x7D, 0x78, 0x79, 0x7B, 0x7A, 
              0x70, 0x71, 0x73, 0x72, 0x77, 0x76, 0x74, 0x75, 
              0x60, 0x61, 0x63, 0x62, 0x67, 0x66, 0x64, 0x65, 
              0x6F, 0x6E, 0x6C, 0x6D, 0x68, 0x69, 0x6B, 0x6A, 
              0x40, 0x41, 0x43, 0x42, 0x47, 0x46, 0x44, 0x45, 
              0x4F, 0x4E, 0x4C, 0x4D, 0x48, 0x49, 0x4B, 0x4A, 
              0x5F, 0x5E, 0x5C, 0x5D, 0x58, 0x59, 0x5B, 0x5A, 
              0x50, 0x51, 0x53, 0x52, 0x57, 0x56, 0x54, 0x55, 
              0xFF, 0xFE, 0xFC, 0xFD, 0xF8, 0xF9, 0xFB, 0xFA, 
              0xF0, 0xF1, 0xF3, 0xF2, 0xF7, 0xF6, 0xF4, 0xF5, 
              0xE0, 0xE1, 0xE3, 0xE2, 0xE7, 0xE6, 0xE4, 0xE5, 
              0xEF, 0xEE, 0xEC, 0xED, 0xE8, 0xE9, 0xEB, 0xEA, 
              0xC0, 0xC1, 0xC3, 0xC2, 0xC7, 0xC6, 0xC4, 0xC5, 
              0xCF, 0xCE, 0xCC, 0xCD, 0xC8, 0xC9, 0xCB, 0xCA, 
              0xDF, 0xDE, 0xDC, 0xDD, 0xD8, 0xD9, 0xDB, 0xDA, 
              0xD0, 0xD1, 0xD3, 0xD2, 0xD7, 0xD6, 0xD4, 0xD5, 
              0x80, 0x81, 0x83, 0x82, 0x87, 0x86, 0x84, 0x85, 
              0x8F, 0x8E, 0x8C, 0x8D, 0x88, 0x89, 0x8B, 0x8A, 
              0x9F, 0x9E, 0x9C, 0x9D, 0x98, 0x99, 0x9B, 0x9A, 
              0x90, 0x91, 0x93, 0x92, 0x97, 0x96, 0x94, 0x95, 
              0xBF, 0xBE, 0xBC, 0xBD, 0xB8, 0xB9, 0xBB, 0xBA, 
              0xB0, 0xB1, 0xB3, 0xB2, 0xB7, 0xB6, 0xB4, 0xB5, 
              0xA0, 0xA1, 0xA3, 0xA2, 0xA7, 0xA6, 0xA4, 0xA5, 
              0xAF, 0xAE, 0xAC, 0xAD, 0xA8, 0xA9, 0xAB, 0xAA };


static
char const far off_nxt[256] = 

            { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

#if 0

    The next two tables are compliments of the first two. Fill.c
    has been changed to use only the first two. Leave these around
    for awhile just in case.

static
char   on_tab[256] = 

            { 0xFF, 0xFE, 0xFC, 0xFD, 0xF8, 0xF9, 0xFB, 0xFA, 
              0xF0, 0xF1, 0xF3, 0xF2, 0xF7, 0xF6, 0xF4, 0xF5, 
              0xE0, 0xE1, 0xE3, 0xE2, 0xE7, 0xE6, 0xE4, 0xE5, 
              0xEF, 0xEE, 0xEC, 0xED, 0xE8, 0xE9, 0xEB, 0xEA, 
              0xC0, 0xC1, 0xC3, 0xC2, 0xC7, 0xC6, 0xC4, 0xC5, 
              0xCF, 0xCE, 0xCC, 0xCD, 0xC8, 0xC9, 0xCB, 0xCA, 
              0xDF, 0xDE, 0xDC, 0xDD, 0xD8, 0xD9, 0xDB, 0xDA, 
              0xD0, 0xD1, 0xD3, 0xD2, 0xD7, 0xD6, 0xD4, 0xD5, 
              0x80, 0x81, 0x83, 0x82, 0x87, 0x86, 0x84, 0x85, 
              0x8F, 0x8E, 0x8C, 0x8D, 0x88, 0x89, 0x8B, 0x8A, 
              0x9F, 0x9E, 0x9C, 0x9D, 0x98, 0x99, 0x9B, 0x9A, 
              0x90, 0x91, 0x93, 0x92, 0x97, 0x96, 0x94, 0x95, 
              0xBF, 0xBE, 0xBC, 0xBD, 0xB8, 0xB9, 0xBB, 0xBA, 
              0xB0, 0xB1, 0xB3, 0xB2, 0xB7, 0xB6, 0xB4, 0xB5, 
              0xA0, 0xA1, 0xA3, 0xA2, 0xA7, 0xA6, 0xA4, 0xA5, 
              0xAF, 0xAE, 0xAC, 0xAD, 0xA8, 0xA9, 0xAB, 0xAA, 
              0x00, 0x01, 0x03, 0x02, 0x07, 0x06, 0x04, 0x05, 
              0x0F, 0x0E, 0x0C, 0x0D, 0x08, 0x09, 0x0B, 0x0A, 
              0x1F, 0x1E, 0x1C, 0x1D, 0x18, 0x19, 0x1B, 0x1A, 
              0x10, 0x11, 0x13, 0x12, 0x17, 0x16, 0x14, 0x15, 
              0x3F, 0x3E, 0x3C, 0x3D, 0x38, 0x39, 0x3B, 0x3A, 
              0x30, 0x31, 0x33, 0x32, 0x37, 0x36, 0x34, 0x35, 
              0x20, 0x21, 0x23, 0x22, 0x27, 0x26, 0x24, 0x25, 
              0x2F, 0x2E, 0x2C, 0x2D, 0x28, 0x29, 0x2B, 0x2A, 
              0x7F, 0x7E, 0x7C, 0x7D, 0x78, 0x79, 0x7B, 0x7A, 
              0x70, 0x71, 0x73, 0x72, 0x77, 0x76, 0x74, 0x75, 
              0x60, 0x61, 0x63, 0x62, 0x67, 0x66, 0x64, 0x65, 
              0x6F, 0x6E, 0x6C, 0x6D, 0x68, 0x69, 0x6B, 0x6A, 
              0x40, 0x41, 0x43, 0x42, 0x47, 0x46, 0x44, 0x45, 
              0x4F, 0x4E, 0x4C, 0x4D, 0x48, 0x49, 0x4B, 0x4A, 
              0x5F, 0x5E, 0x5C, 0x5D, 0x58, 0x59, 0x5B, 0x5A, 
              0x50, 0x51, 0x53, 0x52, 0x57, 0x56, 0x54, 0x55 };

static
char   on_nxt[256] = 

            { 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 
              1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1 };
 
#endif
