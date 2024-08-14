
/***********************************************************************
*
*			   G R A N D W A C K
*
************************************************************************
*
*	Copyright (C) 1985, Commodore-Amiga. All rights reserved.
*
************************************************************************
*
*   Source Control:
*
*	$Id: decode.c,v 1.3 91/04/24 20:51:45 peter Exp $
*
*	$Locker:  $
*
*	$Log:	decode.c,v $
 * Revision 1.3  91/04/24  20:51:45  peter
 * Changed $Header to $Id.
 * 
 * Revision 1.2  88/01/21  13:35:54  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/

#include <exec/types.h>

typedef char *caddr_t;

/*
 * decode.c -- disassemble 68000 code
 */



ULONG quad_0(), quad_1(), quad_2(), quad_3(), quad_4(), quad_5(), quad_6(),
    quad_7(), quad_a(),
    quad_e(), quad_f();


ULONG move_common();
ULONG index_mode(), put_signed_hex(), bwl_6_7(), illegal(), domovecreg();
ULONG eff_addr(), effective();


short GetMem();
long GetMemL();

#define get_word( addr )    ( (LONG)GetMem(addr))
#define get_uword( addr )   ( (ULONG)GetMem(addr))
#define get_long( addr )    ( (LONG)GetMemL(addr))
#define get_ulong( addr )   ( (ULONG)GetMemL(addr))


#define LEN_BYTE    0L
#define LEN_WORD    1L
#define LEN_LONG    2L

char *condition_codes[] = {
    "t",
    "f",
    "hi",
    "ls",
    "hs",
    "lo",
    "ne",
    "eq",
    "vc",
    "vs",
    "pl",
    "mi",
    "ge",
    "lt",
    "gt",
    "le"
};

char bwl_names[] = "bwl";       /* used to print out op code types */

int printf(), strcat(), strcpy(), mnprintf();


caddr_t
do_decode( addr )
    caddr_t addr;
{
    ULONG inst;

    inst = (ULONG)GetMem( addr ) & 0xffff;

    printf( "%06lx  ", addr );        /* address label */

    switch( inst >> 12 ) {
    case 0:
        return( (caddr_t) quad_0( inst, addr + 2 ) + 2 );
    case 1:
        return( (caddr_t) quad_1( inst, addr + 2 ) + 2 );
    case 2:
        return( (caddr_t) quad_2( inst, addr + 2 ) + 2 );
    case 3:
        return( (caddr_t) quad_3( inst, addr + 2 ) + 2 );
    case 4:
        return( (caddr_t) quad_4( inst, addr + 2 ) + 2 );
    case 5:
        return( (caddr_t) quad_5( inst, addr + 2 ) + 2 );
    case 6:
        return( (caddr_t) quad_6( inst, addr + 2 ) + 2 );
    case 7:
        return( (caddr_t) quad_7( inst, addr + 2 ) + 2 );
    case 10:
        return( (caddr_t) quad_a( inst, addr + 2 ) + 2 );
    case 8:
    case 9:
    case 11:
    case 12:
    case 13:
        return( (caddr_t) quad_89bcd( inst, addr + 2 ) + 2 );
    case 14:
        return( (caddr_t) quad_e( inst, addr + 2 ) + 2 );
    case 15:
        return( (caddr_t) quad_f( inst, addr + 2 ) + 2 );
    }
}



char *bit_names[4] = {
    "tst", "chg", "clr", "set"
};

char *imm_reg_names[2] = {
    "ccr", "sr"
};


ULONG
quad_0( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* bit manipulation/MOVEP/immediate */
    ULONG bwl = bwl_6_7( inst );
    ULONG reg;
    char *name;
    char ad;
    ULONG ext;
    ULONG len;

    reg = (inst >> 9) & 0x7;
    if( inst & 0x0100 ) {
        if( (inst & 070) == 010 ) {
            mnprintf( "movep.%lc", inst & 0x40 ? 'l' : 'w' );    /* bit 6 */
            if( inst & 0x80 ) {     /* bit 7 */
                printf( "d%ld,", reg );
            }
            put_signed_hex( get_word( addr ) );
            printf( "(a%ld)", inst & 0x7 );
            if( !(inst & 0x80) ) {  /* still bit 7 */
                printf( ",d%ld", reg );
            }
            len = 2;
        } else {
            /* dynamic bit */
            len = (inst & 070) ? LEN_BYTE : LEN_LONG;
            mnprintf( "b%s.%lc", bit_names[bwl], bwl_names[len] );
            printf( "d%ld,", reg );
            len = eff_addr( inst >> 3, inst, addr, len );
        }
    } else {
        /* bit 8 is zero */
        switch( reg ) {
        case 0:
            name = "or";

common:     mnprintf( "%si.%lc", name, bwl_names[bwl] );
            printf( "#" );

            if( bwl == LEN_LONG ) {
                put_signed_hex( get_long( addr ) );
                len = 4;
            } else {
                put_signed_hex( get_word( addr ) );
                len = 2;
            }
            addr += len;

            printf( "," );
            if( (inst & 077) == 074 ) {
                printf( "%s", imm_reg_names[bwl] );
            } else {
                len += effective( inst, addr );
            }
            break;

        case 1:
            name = "and";
            goto common;

        case 2:
            name = "sub";
            goto common;

        case 3:
            name = "add";
            goto common;

        case 4:     /* static bit */
            reg = get_uword( addr );
            len = (inst & 070) ? LEN_BYTE : LEN_LONG;
            mnprintf( "b%s.%lc", bit_names[bwl], bwl_names[len] );
            printf( "#%ld,", reg );
            addr += 2;
            len = eff_addr( inst >> 3, inst, addr, len ) + 2;
            break;

        case 5:
            name = "eor";
            goto common;

        case 6:
            name = "cmp";
            goto common;

        case 7:
            mnprintf( "moves.%lc", bwl_names[bwl] );
            ext = get_uword( addr );
            reg = (ext >> 12) & 0x7;
            ad = (ext & 0x8000) ? 'a' : 'd';
            if( ext & 0x800 ) {
                printf( "%lc%ld,", ad, reg );
            }
            addr += 2;
            len = effective( inst, addr ) + 2;
            if( !(ext & 0x800) ) {
                printf( ",%lc%ld", ad, reg );
            }
            break;
        }
    }

    return( len );
}



ULONG
quad_1( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* move byte */

    return( move_common( inst, addr, LEN_BYTE ) );
}

ULONG
move_common( inst, addr, bwl )
ULONG inst;
caddr_t addr;
ULONG bwl;
{
    ULONG len;
    ULONG mode = (inst >> 6) & 0x7;

    mnprintf( "move%s.%lc", mode == 0x1 ? "a" : "", bwl_names[bwl] );
    len = eff_addr( inst >> 3, inst, addr, bwl );
    printf( "," );
    if( mode == 0x1 ) {
        /* movea */
        printf( "a%ld", (inst >> 9) & 0x7 );
    } else {
        len += eff_addr( mode, inst >> 9, addr + len, bwl );
    }
    return( len );
}


ULONG
quad_2( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* move long */
    ULONG len;

    return( move_common( inst, addr, LEN_LONG ) );
}


ULONG
quad_3( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* move word */
    ULONG len;

    return( move_common( inst, addr, LEN_WORD ) );
}

char *quad_4_0x4e7x[] = {
    "reset",
    "nop",
    "stop",
    "rte",
    "rtd",
    "rts",
    "trapv",
    "rtr"
};

char *q4_05_norm[] = {
    "negx", "clr", "neg", "not", 0, "tst"
};
char *q4_05_head[] = {
    "sr,", "ccr,", "", "", 0, ""
};
char *q4_05_tail[] = {
    "", "", ",ccr", ",sr", 0, ""
};

ULONG
quad_4( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* misc */
    ULONG reg;
    char c;
    ULONG len;
    ULONG bwl;

    reg = (inst >> 9) & 0x7;

    if( inst & 0x0100 ) {   /* bit 8 */
        if( inst & 0x0040 ) {   /* bit 4 */
            mnprintf( "lea.l" );
            bwl = LEN_LONG;
            c = 'a';
        } else {
            mnprintf( "chk.w" );
            bwl = LEN_WORD;
            c = 'd';
        }
        len = eff_addr( inst >> 3, inst, addr, bwl );
        printf( ",%lc%ld", c, reg );
        return( len );
    }

    bwl = bwl_6_7( inst );

    switch( reg ) {
    case 0: case 1: case 2: case 3: case 5:
        if( bwl != 3 ) {
            mnprintf( "%s.%lc", q4_05_norm[reg], bwl_names[bwl] );
            len = effective( inst, addr );
            break;
        }
        if( reg == 5 ) {
            if( (inst & 0x3f) == 0x3c ) {
                mnprintf( "illegal" );
                return( 0 );
            }
            bwl = LEN_BYTE;
            mnprintf( "tas.b" );
        } else {
            bwl = LEN_WORD;
            mnprintf( "move.w" );
        }
        printf( q4_05_head[reg] );
        len = effective( inst, addr );
        printf( q4_05_tail[reg] );
        break;

    case 4:
        reg = inst & 0x7;
        switch( bwl ) {
        case 0:
            mnprintf( "nbcd.b" );
            len = effective( inst, addr );
            break;
        case 1:
            if( !( inst & 070 ) ) {
                mnprintf( "swap.w" );
                printf( "d%ld", reg );
                len = 0;
            } else {
                mnprintf( "pea.l" );
                len = eff_addr( inst >> 3, inst, addr, LEN_LONG );
            }
            break;
        case 2:
        case 3:
            bwl--;
            if( !( inst & 070 ) ) {
                mnprintf( "ext.%lc", bwl_names[bwl] );
                printf( "d%ld", reg );
                len = 0;
            } else {
                bwl = (inst & 0x0040 ? LEN_LONG : LEN_WORD );
                mnprintf( "movem.%lc", bwl_names[bwl] );
                printf( "<$%lx>,", get_uword( addr ) );
                len = eff_addr( inst >> 3, inst, addr + 2, bwl ) + 2;
            }
            break;
        }
        break;

    case 6:
        bwl = (inst & 0x0040 ? LEN_LONG : LEN_WORD );
        mnprintf( "movem.%lc", bwl_names[bwl] );
        len = eff_addr( inst >> 3, inst, addr + 2, bwl ) + 2;
        printf( ",<$%lx>", get_word( addr ) & 0xffff );
        break;

    case 7:
        switch( bwl ) {
        case 0:
            return( illegal( inst ) );

        case 1:
            reg = inst & 0x7;
            switch( inst & 070 ) {
            case 000:
            case 010:
                mnprintf( "trap" );
                printf( "#%ld", inst & 0xf );
                len = 0;
                break;
            case 020:
                mnprintf( "link" );
                printf( "a%ld,#", reg );
                put_signed_hex( get_word( addr ) );
                len = 2;
                break;
            case 030:
                mnprintf( "unlk" );
                printf( "a%ld", reg );
                len = 0;
                break;
            case 040:
                mnprintf( "move.l" );
                printf( "a%ld,usp", reg );
                len = 0;
                break;
            case 050:
                mnprintf( "move.l" );
                printf( "usp,a%ld", reg );
                len = 0;
                break;
            case 060:
                mnprintf( quad_4_0x4e7x[reg] );
                switch( reg ) {
                case 4:
                    /* rtd needs some help */
                    printf( "#" );
                    put_signed_hex( get_word( addr ) );
                    len = 2;
                    break;
                case 2:
                    /* stop */
                    printf( "#$%lx", get_uword( addr ) );
                    len = 2;
                    break;
                default:
                    len = 0;
                }
                break;
            case 070:
                reg = get_uword( addr );
                mnprintf( "movec.l" );
                if( inst & 0x1 ) {
                    domovecreg( reg );
                }
                printf( "%lc%ld", reg & 0x8000 ? 'a' : 'd', (reg >> 12) & 7 );
                if( !( inst & 0x1 ) ) {
                    domovecreg( reg );
                }
                len = 2;
                break;
            }
            break;

        case 2:
            mnprintf( "jsr" );
            len = eff_addr( inst >> 3, inst, addr, LEN_LONG );
            break;

        case 3:
            mnprintf( "jmp" );
            len = eff_addr( inst >> 3, inst, addr, LEN_LONG );
            break;
        }
        break;
    }
    return( len );
}



ULONG
quad_5( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* ADDQ/SUBQ/Scc/DBcc */
    ULONG bwl, quick;
    char *string;

    bwl = bwl_6_7( inst );

    if( bwl == 3 ) {
        if( (inst & 0x0038) == 0x0008 ) {   /* mode 001 */
            /* DBcc */
            mnprintf( "db%s", condition_codes[(inst>>8)&0xf] );
            printf( "d%lx,$%lx",
                inst & 0x7, addr + get_word( addr ) );
            return( 2 );
        } else {
            mnprintf( "s%s", condition_codes[(inst>>8)&0xf] );
            return( eff_addr( inst >> 3, inst, addr, LEN_BYTE ) );
        }
    }

    if( inst & 0x0100 ) {
        string = "sub";
    } else {
        string = "add";
    }

    quick = (inst>>9) & 0x7;    /* bits 9-11 */
    if( ! quick ) quick = 8;
    mnprintf( "%sq.%lc", string, bwl_names[bwl] );
    printf( "#%lx,", quick );
    return( effective( inst, addr ) );
}


ULONG
quad_6( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* Bcc/BSR/BRA */
    ULONG cond, len;
    BYTE byte;
    char *string;

    cond = (inst >> 8) & 0xf;

    if( (byte = inst) & 0xff ) {
        string = ".s";
        addr += byte;
        len = 0;
    } else {
        string = "";
        addr += get_word( addr );
        len = 2;
    }

    if( cond == 0 ) {
        mnprintf( "bra%s", string );
    } else if( cond == 1 ) {
        mnprintf( "bsr%s", string );
    } else {
        mnprintf( "b%s%s", condition_codes[cond], string );
    }

    printf( "$%lx", addr );
    return( len );

}


ULONG
quad_7( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* MOVEQ */
    BYTE data;

    if( inst & 0x0100 ) {
        return( illegal( inst ) );
    } else {
        mnprintf( "moveq.l" );
        printf( "#" );
        data = inst & 0xff;
        put_signed_hex( (long) data );
        printf( ",d%lx", (inst >> 9) & 0x7 );
    }
    return( 0 );
}




ULONG
quad_a( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* Unassigned */

    return( illegal( inst ) );
}



char *qe_names[] = {
    "as", "ls", "rox", "ro"
};

char qe_direction[] = { 'r', 'l' };

ULONG
quad_e( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* SHIFT/ROTATE */
    ULONG bwl = bwl_6_7( inst );
    ULONG len;
    ULONG bit8 = (inst & 0x0100) >> 8;  /* direction field */
    char *string;

    if( bwl == 0x3 ) {
        /* shift/rotate to memory */
        /* type is in bits 9 & 10 */
        mnprintf( "%s%lc.w", qe_names[(inst >> 9) & 0x3], qe_direction[bit8] );
        len = eff_addr( inst >> 3, inst, addr, LEN_WORD );
    } else {
        /* shift/rotate to register */
        /* type is in bits 3 & 4 */
        mnprintf( "%s%lc.%lc", qe_names[(inst >> 3) & 0x3], qe_direction[bit8],
            bwl_names[bwl] );

        if( inst & 0x20 ) {     /* bit 5 -- i/r field */
            string = "d%ld,d%ld";
        } else {
            string = "#%ld,d%ld";
        }
        printf( string, (inst >> 9) & 0x7, inst & 0x7  );
        len = 0;
    }

    return( len );
}


ULONG
quad_f( inst, addr )
ULONG inst;
caddr_t addr;
{
    /* unassigned */

    return( illegal( inst ) );
}


#define SBCD    0
#define SUBX    1
#define ABCD    2
#define EXG     3
#define ADDX    4
#define CMPM    5
#define DREG    6
#define ADEC    7
#define AINC    8
#define AREG    9
#define ADREG   10




BYTE quadmap[14] = {
    -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, -1, 2, 3, 4
};

BYTE mode0inst[3][6] = {
    {   SBCD,   SUBX,   -2,     ABCD,   ADDX  },
    {   -1,     SUBX,   -2,     EXG,    ADDX  },
    {   -1,     SUBX,   -2,     -1,     ADDX  },
};

BYTE mode1inst[3][6] = {
    {   SBCD,   SUBX,   CMPM,   ABCD,   ADDX  },
    {   -1,     SUBX,   CMPM,   EXG,    ADDX  },
    {   -1,     SUBX,   CMPM,   EXG,    ADDX  },
};

BYTE mode1arg[3][6] = {
    {   ADEC,   ADEC,   AINC,   ADEC,   ADEC  },
    {   -1,     ADEC,   AINC,   AREG,   ADEC  },
    {   -1,     ADEC,   AINC,  ADREG,   ADEC  },
};

char *modeops[] = {
    "sbcd.%lc",
    "subx.%lc",
    "abcd.%lc",
    "exg",
    "addx.%lc",
    "cmpm.%lc",
    "d%ld,d%ld",
    "-(a%ld),-(a%ld)",
    "(a%ld)+,(a%ld)+",
    "a%ld,a%ld",
    "d%ld,a%ld"
};


#define Q_MAIN012   0
#define Q_MAIN3     1
#define Q_MAIN456   2
#define Q_MAIN7     3

#define OR          0
#define DIVU        1
#define DIVS        2
#define SUB         3
#define SUBA        4
#define CMP         5
#define CMPA        6
#define EOR         7
#define AND         8
#define MULU        9
#define MULS        10
#define ADD         11
#define ADDA        12

/* minor dimention MUST be divisable by two -- Compiler bug */
BYTE nametable[4][6] = {
    {   OR,     SUB,    CMP,    AND,    ADD },
    {   DIVU,   SUBA,   CMPA,   MULU,   ADDA },
    {   OR,     SUB,    EOR,    AND,    ADD },
    {   DIVS,   SUBA,   CMPA,   MULS,   ADDA }
};

char *stringtable[] = {
    "or",
    "divu",
    "divs",
    "sub",
    "suba",
    "cmp",
    "cmpa",
    "eor",
    "and",
    "mulu",
    "muls",
    "add",
    "adda"
};


quad_89bcd( inst, addr )
ULONG inst;
caddr_t addr;
{

    ULONG bwl  = bwl_6_7( inst );
    ULONG op   = (inst >> 6) & 0x7;
    ULONG mode = (inst >> 3) & 0x7;
    ULONG reg  = (inst >> 9) & 0x7;
    ULONG quad = (inst >> 12) & 0xf;
    ULONG q    = quadmap[quad];
    ULONG len;
    ULONG nameindex;
    LONG  index0, index1;
    char let;
    char *string;

    switch( op ) {
    case 0:
        if( mode == 1 ) goto notlegal;
        nameindex = Q_MAIN012;
        let = 'd';
        goto doit0;

    case 1:
    case 2:
        if( quad == 8 || quad == 0xc ) {
            if( mode == 1 ) goto notlegal;
        }
        nameindex = Q_MAIN012;
        let = 'd';
        goto doit0;

    case 3:
        if( quad == 8 || quad == 0xc ) {
            if( mode == 1 ) goto notlegal;
            let = 'd';
        } else {
            let = 'a';
        }
        bwl = LEN_WORD;
        nameindex = Q_MAIN3;
        goto doit0;

    case 7:
        if( quad == 8 || quad == 0xc ) {
            if( mode == 1 ) goto notlegal;
            let = 'd';
            bwl = LEN_WORD;
        } else {
            let = 'a';
            bwl = LEN_LONG;
        }
        nameindex = Q_MAIN7;
        goto doit0;

    case 4:
    case 5:
    case 6:
        if( mode == 0 ) {
            index0 = mode0inst[op - 4][q];
            index1 = DREG;
            goto domode;
        } else if( mode == 1 ) {
            index0 = mode1inst[op - 4][q];
            index1 = mode1arg[op - 4][q];
domode:
            if( index0 == -1 ) goto notlegal;
            if( index0 == -2 ) goto nomode;
            mnprintf( modeops[index0], bwl_names[bwl] );
            printf( modeops[index1], reg, inst & 0x7 );
            return( 0L );
        } else {
nomode:
            index0 = nametable[Q_MAIN456][q];
            mnprintf( "%s.%lc", stringtable[index0], bwl_names[bwl] );
            printf( "d%ld,", reg );
            return( effective( inst, addr ) );
        }

    }

notlegal:
    return( illegal( inst ) );

doit0:
    index0 = nametable[nameindex][q];
    mnprintf( "%s.%lc", stringtable[index0], bwl_names[bwl] );
    len = eff_addr( mode, inst, addr, bwl );
    printf( ",%lc%ld", let, reg );
    return( len );

}

ULONG
effective( inst, addr )
ULONG inst;
caddr_t addr;
{
    return( eff_addr( inst >> 3, inst, addr, bwl_6_7(inst) ) );
}

ULONG
eff_addr( mode, reg, addr, len )
ULONG mode, reg;
caddr_t addr;
ULONG len;
{

    mode &= 0x7;
    reg &= 0x7;

    switch( mode ) {
    case 0:     /* data register direct */
        printf( "d%lx", reg );
        return( 0 );
    case 1:     /* address register direct */
        printf( "a%lx", reg );
        return( 0 );
    case 2:     /* address register indirect */
        printf( "(a%lx)", reg );
        return( 0 );
    case 3:     /* address register indirect with postincrement */
        printf( "(a%lx)+", reg );
        return( 0 );
    case 4:     /* address register indirect with predecrement */
        printf( "-(a%lx)", reg );
        return(0);
    case 5:     /* address register indirect with displacement */

        put_signed_hex( get_word( addr ) );
        printf( "(a%lx)", reg );
        return( 2 );
    case 6:     /* address register indirect with index */
        index_mode( reg, get_word( addr ) );
        return( 2 );
    case 7:     /* misc */
        switch( reg ) {
        case 0:     /* absolute short address */
            put_signed_hex( get_word( addr ) );
            printf( ".w" );
            return( 2 );
        case 1:     /* absolute long address */
            put_signed_hex( get_long( addr ) );
            printf( ".l" );
            return( 4 );
        case 2:     /* program counter with displacement */
            put_signed_hex( get_word( addr ) );
            printf( "(pc)" );
            return( 2 );
        case 3:     /* program counter with index */
            index_mode( -1, get_word( addr ) );
            return( 2 );
        case 4:     /* immediate */
            printf( "#" );
            if( len == LEN_LONG ) {
                put_signed_hex( get_long( addr ) );
                return( 4 );
            } else {
                put_signed_hex( get_word( addr ) );
                return( 2 );
            }
        default:
            printf( "<mode 7-%ld: illegal>", reg );
            return( 0 );
        }
    }
}

int
nullroutine(){}

ULONG
index_mode( reg, format )
ULONG reg, format;
{
    BYTE offset;
    LONG offset1;
    offset = format & 0xff;
    if( offset ) {
        nullroutine();      /* kill registers */
        offset1 = offset;   /* sign extend offset to a long */
        put_signed_hex( offset1 );
    }
    if( reg == -1 ) {
        printf( "(pc" );
    } else {
        printf( "(a%ld", reg );
    }
    printf( ",%lc%ld.%lc)", format & 0x8000 ? 'a' : 'd',
        (format >> 12) & 0x7, format & 0x0800 ? 'l' : 'w' );
}


/* output hex in a signed fashion */
ULONG
put_signed_hex( num )
LONG num;
{
    if( num < 0 ) {
        printf( "-" );
        num = -num;
    }
    printf( "$%lx", num );
}


/* this routine decodes size fields that are in bits six and seven */
ULONG
bwl_6_7( inst )
ULONG inst;
{
    return( (inst >> 6) & 0x3 );
}


ULONG
illegal( inst )
ULONG inst;
{
    printf( "<$%04lx: illegal instruction>", inst & 0xffff );
    return( 0 );
}

ULONG
domovecreg( reg )
ULONG reg;
{
    reg &= 0xfff;
    switch( reg ) {
    case 0x000:
        printf( "sfc" );
        break;

    case 0x001:
        printf( "dfc" );
        break;

    case 0x800:
        printf( "usp" );
        break;

    case 0x801:
        printf( "vbr" );
        break;
    }
}

