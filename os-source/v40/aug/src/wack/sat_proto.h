/*
 * Amiga Grand Wack
 *
 * $Id: sat_proto.h,v 39.1 93/05/21 17:34:31 peter Exp $
 *
 */

void clearBPT( void );

unsigned long getBrkCnt( unsigned long addr );

void freeCPU( void );

void setBrkPt( unsigned long addr );

void evalBrkPts( void );

STRPTR ToglTrace( void );
