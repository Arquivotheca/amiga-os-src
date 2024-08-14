/*
 * Amiga Grand Wack
 *
 * $Id: decode_proto.h,v 39.1 93/07/16 18:24:03 peter Exp $
 *
 */

typedef char *caddr_t;

caddr_t do_decode( caddr_t addr );

ULONG quad_0( ULONG inst, caddr_t addr );

ULONG quad_1( ULONG inst, caddr_t addr );

ULONG move_common( ULONG inst, caddr_t addr, ULONG bwl );

ULONG quad_2( ULONG inst, caddr_t addr );

ULONG quad_3( ULONG inst, caddr_t addr );

ULONG quad_4( ULONG inst, caddr_t addr );

ULONG quad_5( ULONG inst, caddr_t addr );

ULONG quad_6( ULONG inst, caddr_t addr );

ULONG quad_7( ULONG inst, caddr_t addr );

ULONG quad_a( ULONG inst, caddr_t addr );

ULONG quad_e( ULONG inst, caddr_t addr );

ULONG quad_f( ULONG inst, caddr_t addr );

ULONG quad_89bcd( ULONG inst, caddr_t addr );

ULONG effective( ULONG inst, caddr_t addr );

ULONG eff_addr( ULONG mode, ULONG reg, caddr_t addr, ULONG len );

void nullroutine( void );

void index_mode( ULONG reg, ULONG format );

void put_signed_hex( LONG num );

ULONG bwl_6_7( ULONG inst );

ULONG illegal( ULONG inst );

void domovecreg( ULONG reg );

void printRegList( ULONG regword, ULONG mode );

void printRegRange( ULONG firstreg, ULONG lastreg );

void printReg( ULONG reg );

