/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 8            * */
/* *          (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.                  * */
/* *                                                                            * */
/* *                Last Modified :  11-APR-85     (PJF)                        * */
/* ****************************************************************************** */

/* M68KASM8 : Automatically translated from BCPL to C on 26-Feb-85 */


#include "libhdr"
#include "asmhdr"


declsyswords ( )
{
   static VAR16BIT dv[] = {

/* Instr      Template     Type       Mask     Source          Dest  */
/* =====      ========     ====       ====     ======          ====  */
                                               
/* ABCD  */    0xC100,   ins_2op_b,     0,          0,           1,
/* ADD   */    m_ADD,    ins_2op_bwl,   0,      ins_N,           3,
/* ADDA  */    m_ADD,    ins_2op_wl,    0,      ins_A,           3,
/* ADDI  */    m_ADDI,   ins_2op_bwl,   0,      ins_I,           3,
/* ADDQ  */    0x5000,   ins_2op_bwl,   5,    am_imm3,      am_alt,
/* ADDX  */    0xD100,   ins_2op_bwl,   0,          0,           1,
/* AND   */    m_AND,    ins_2op_bwl,   0,          0,           2,
/* ANDI  */    m_ANDI,   ins_2op_bwl,   0,          0,          12,
/* ASL   */    0xE000,   ins_2op_bwl,   0,        0x1,           5,
/* ASR   */    0xE000,   ins_2op_bwl,   0,        0x0,           5,
/* BCC   */    0x6000,   ins_1op_bl,    0,        0x4,           4,
/* BCS   */    0x6000,   ins_1op_bl,    0,        0x5,           4,
/* BEQ   */    0x6000,   ins_1op_bl,    0,        0x7,           4,
/* BGE   */    0x6000,   ins_1op_bl,    0,        0xC,           4,
/* BGT   */    0x6000,   ins_1op_bl,    0,        0xE,           4,
/* BHI   */    0x6000,   ins_1op_bl,    0,        0x2,           4,
/* BLE   */    0x6000,   ins_1op_bl,    0,        0xF,           4,
/* BLS   */    0x6000,   ins_1op_bl,    0,        0x3,           4,
/* BLT   */    0x6000,   ins_1op_bl,    0,        0xD,           4,
/* BMI   */    0x6000,   ins_1op_bl,    0,        0xB,           4,
/* BNE   */    0x6000,   ins_1op_bl,    0,        0x6,           4,
/* BPL   */    0x6000,   ins_1op_bl,    0,        0xA,           4,
/* BVC   */    0x6000,   ins_1op_bl,    0,        0x8,           4,
/* BVS   */    0x6000,   ins_1op_bl,    0,        0x9,           4,
/* BCHG  */    0x0,      ins_2op_bl,    0,        0x1,           6,
/* BCLR  */    0x0,      ins_2op_bl,    0,        0x2,           6,
/* BRA   */    0x6000,   ins_1op_bl,    0,        0x0,           4,
/* BSET  */    0x0,      ins_2op_bl,    0,        0x3,           6,
/* BSR   */    0x6000,   ins_1op_bl,    0,        0x1,           4,
/* BTST  */    0x0,      ins_2op_bl,    0,        0x0,           6,
/* CHK   */    0x4180,   ins_2op_w,     7,    am_data,       am_Dr,
/* CLR   */    0x4200,   ins_1op_bwl,   2,          0, am_data_alt,
/* CMP   */    m_CMP,    ins_2op_bwl,   0,      ins_N,          16,
/* CMPA  */    m_CMP,    ins_2op_bwl,   0,      ins_A,          16,
/* CMPI  */    m_CMPI,   ins_2op_bwl,   0,      ins_I,          16,
/* CMPM  */    m_CMPM,   ins_2op_bwl,   0,      ins_M,          16,
/* DBCC  */    0x50C8,   ins_2op,       0,        0x4,           4,
/* DBCS  */    0x50C8,   ins_2op,       0,        0x5,           4,
/* DBEQ  */    0x50C8,   ins_2op,       0,        0x7,           4,
/* DBF   */    0x50C8,   ins_2op,       0,        0x1,           4,
/* DBGE  */    0x50C8,   ins_2op,       0,        0xC,           4,
/* DBGT  */    0x50C8,   ins_2op,       0,        0xE,           4,
/* DBHI  */    0x50C8,   ins_2op,       0,        0x2,           4,
/* DBLE  */    0x50C8,   ins_2op,       0,        0xF,           4,
/* DBLS  */    0x50C8,   ins_2op,       0,        0x3,           4,
/* DBLT  */    0x50C8,   ins_2op,       0,        0xD,           4,
/* DBMI  */    0x50C8,   ins_2op,       0,        0xB,           4,
/* DBNE  */    0x50C8,   ins_2op,       0,        0x6,           4,
/* DBPL  */    0x50C8,   ins_2op,       0,        0xA,           4,
/* DBT   */    0x50C8,   ins_2op,       0,        0x0,           4,
/* DBVC  */    0x50C8,   ins_2op,       0,        0x8,           4,
/* DBVS  */    0x50C8,   ins_2op,       0,        0x9,           4,
/* DBRA  */    0x50C8,   ins_2op,       0,        0x1,           4,
/* DIVS  */    0x81C0,   ins_2op_w,     7,    am_data,       am_Dr,
/* DIVU  */    0x80C0,   ins_2op_w,     7,    am_data,       am_Dr,
/* EOR   */    m_EOR,    ins_2op_bwl,   0,          0,          15,
/* EORI  */    m_EORI,   ins_2op_bwl,   0,          0,          12,
/* EXG   */    0xC100,   ins_2op_l,     0,          0,           7,
/* EXT   */    0x4880,   ins_1op_wl,   10,          0,       am_Dr,
/* JMP   */    0x4EC0,   ins_1op,       9,          0,    am_contr,
/* JSR   */    0x4E80,   ins_1op,       9,          0,    am_contr,
/* LEA   */    0x41C0,   ins_2op_l,     7,   am_contr,       am_Ar,
/* LINK  */    0x4E50,   ins_2op,       4,      am_Ar,    am_imm16,
/* LSL   */    0xE008,   ins_2op_bwl,   0,        0x3,           5,
/* LSR   */    0xE008,   ins_2op_bwl,   0,        0x2,           5,
/* MOVE  */    0x0,      ins_2op_bwl,   0,          0,           8,
/* MOVEA */    0x40,     ins_2op_bwl,   0,          1,           8,
/* MOVEM */    0x4880,   ins_2op_wl,    0,          2,           8,
/* MOVEP */    0x8,      ins_2op_wl,    0,          3,           8,
/* MOVEQ */    0x7000,   ins_2op_l,     0,          4,           8,
/* MULS  */    0xC1C0,   ins_2op_w,     7,    am_data,       am_Dr,
/* MULU  */    0xC0C0,   ins_2op_w,     7,    am_data,       am_Dr,
/* NBCD  */    0x4800,   ins_1op_b,     9,          0, am_data_alt,
/* NEG   */    0x4400,   ins_1op_bwl,   2,          0, am_data_alt,
/* NEGX  */    0x4000,   ins_1op_bwl,   2,          0, am_data_alt,
/* NOP   */    0x4E71,   ins_zop,      15,          0,           0,
/* NOT   */    0x4600,   ins_1op_bwl,   2,          0, am_data_alt,
/* OR    */    m_OR,     ins_2op_bwl,   0,          0,           2,
/* ORI   */    m_ORI,    ins_2op_bwl,   0,          0,          12,
/* PEA   */    0x4840,   ins_1op_l,     9,          0,    am_contr,
/* RESET */    0x4E70,   ins_zop,      15,          0,           0,
/* ROL   */    0xE018,   ins_2op_bwl,   0,        0x5,           5,
/* ROR   */    0xE018,   ins_2op_bwl,   0,        0x4,           5,
/* ROXL  */    0xE010,   ins_2op_bwl,   0,        0x7,           5,
/* ROXR  */    0xE010,   ins_2op_bwl,   0,        0x6,           5,
/* RTE   */    0x4E73,   ins_zop,      15,          0,           0,
/* RTR   */    0x4E77,   ins_zop,      15,          0,           0,
/* RTS   */    0x4E75,   ins_zop,      15,          0,           0,
/* SBCD  */    0x8100,   ins_2op_b,     0,          0,           1,
/* SCC   */    0x50C0,   ins_1op_b,     6,        0x4, am_data_alt,
/* SCS   */    0x50C0,   ins_1op_b,     6,        0x5, am_data_alt,
/* SEQ   */    0x50C0,   ins_1op_b,     6,        0x7, am_data_alt,
/* SF    */    0x50C0,   ins_1op_b,     6,        0x1, am_data_alt,
/* SGE   */    0x50C0,   ins_1op_b,     6,        0xC, am_data_alt,
/* SGT   */    0x50C0,   ins_1op_b,     6,        0xE, am_data_alt,
/* SHI   */    0x50C0,   ins_1op_b,     6,        0x2, am_data_alt,
/* SLE   */    0x50C0,   ins_1op_b,     6,        0xF, am_data_alt,
/* SLS   */    0x50C0,   ins_1op_b,     6,        0x3, am_data_alt,
/* SLT   */    0x50C0,   ins_1op_b,     6,        0xD, am_data_alt,
/* SMI   */    0x50C0,   ins_1op_b,     6,        0xB, am_data_alt,
/* SNE   */    0x50C0,   ins_1op_b,     6,        0x6, am_data_alt,
/* SPL   */    0x50C0,   ins_1op_b,     6,        0xA, am_data_alt,
/* ST    */    0x50C0,   ins_1op_b,     6,        0x0, am_data_alt,
/* SVC   */    0x50C0,   ins_1op_b,     6,        0x8, am_data_alt,
/* SVS   */    0x50C0,   ins_1op_b,     6,        0x9, am_data_alt,
/* STOP  */    0x4E72,   ins_1op,      15,          0,    am_imm16,
/* SUB   */    m_SUB,    ins_2op_bwl,   0,      ins_N,           3,
/* SUBA  */    m_SUB,    ins_2op_wl,    0,      ins_A,           3,
/* SUBI  */    m_SUBI,   ins_2op_bwl,   0,      ins_I,           3,
/* SUBQ  */    0x5100,   ins_2op_bwl,   5,    am_imm3,      am_alt,
/* SUBX  */    0x9100,   ins_2op_bwl,   0,          0,           1,
/* SWAP  */    0x4840,   ins_1op_w,     4,          0,       am_Dr,
/* TAS   */    0x4AC0,   ins_1op_b,     9,          0, am_data_alt,
/* TRAP  */    0x4E40,   ins_1op,       0,          0,          11,
/* TRAPV */    0x4E76,   ins_zop,      15,          0,           0,
/* TST   */    0x4A00,   ins_1op_bwl,   2,          0, am_data_alt,
/* UNLK  */    0x4E58,   ins_1op,       4,          0,       am_Ar,

/* Now for the directives */
/* ====================== */

/* EQU      */      0,   s_dir,    0,    0,        d_equ,
/* EQUR     */      0,   s_dir,    0,    0,       d_equr,
/* SET      */      0,   s_dir,    0,    0,        d_set,
/* DC       */      0,   s_dir,    0,    0,         d_dc,
/* DS       */      0,   s_dir,    0,    0,         d_ds,
/* PAGE     */      0,   s_dir,    0,    0,       d_page,
/* LIST     */      0,   s_dir,    0,    0,       d_list,
/* NOLIST   */      0,   s_dir,    0,    0,     d_nolist,
/* NOL      */      0,   s_dir,    0,    0,     d_nolist,
/* SPC      */      0,   s_dir,    0,    0,        d_spc,
/* NOPAGE   */      0,   s_dir,    0,    0,     d_nopage,
/* LLEN     */      0,   s_dir,    0,    0,       d_llen,
/* PLEN     */      0,   s_dir,    0,    0,       d_plen,
/* TTL      */      0,   s_dir,    0,    0,        d_ttl,
/* NOOBJ    */      0,   s_dir,    0,    0,      d_noobj,
/* IFEQ     */      0,   s_dir,    0,    0,       d_ifeq,
/* IFNE     */      0,   s_dir,    0,    0,       d_ifne,
/* ENDC     */      0,   s_dir,    0,    0,       d_endc,
/* MACRO    */      0,   s_dir,    0,    0,      d_macro,
/* ENDM     */      0,   s_dir,    0,    0,       d_endm,
/* MEXIT    */      0,   s_dir,    0,    0,      d_mexit,
/* GET      */      0,   s_dir,    0,    0,        d_get,
/* END      */      0,   s_dir,    0,    0,        d_end,
/* FAIL     */      0,   s_dir,    0,    0,       d_fail,
/* CNOP     */      0,   s_dir,    0,    0,       d_cnop,
/* EXTRN    */      0,   s_dir,    0,    0,      d_extrn,
/* ENTRY    */      0,   s_dir,    0,    0,      d_entry,
/* EXTRN    */      0,   s_dir,    0,    0,      d_extrn,
/* ENTRY    */      0,   s_dir,    0,    0,      d_entry,
/* GET      */      0,   s_dir,    0,    0,        d_get,
/* IFLT     */      0,   s_dir,    0,    0,       d_iflt,
/* IFLE     */      0,   s_dir,    0,    0,       d_ifle,
/* IFGT     */      0,   s_dir,    0,    0,       d_ifgt,
/* IFGE     */      0,   s_dir,    0,    0,       d_ifge,
/* DCB      */      0,   s_dir,    0,    0,        d_dcb,
/* MASK2    */      0,   s_dir,    0,    0,      d_mask2,
/* IDNT     */      0,   s_dir,    0,    0,       d_idnt,
/* FORMAT   */      0,   s_dir,    0,    0,     d_format,
/* NOFORMAT */      0,   s_dir,    0,    0,   d_noformat,
/* SECTION  */      0,   s_dir,    0,    0,    d_section,
/* IFC      */      0,   s_dir,    0,    0,        d_ifc,
/* IFNC     */      0,   s_dir,    0,    0,       d_ifnc,
/* ABSOLUTE */      0,   s_dir,    0,    0,        d_abs,
/* NARG     */      0, s_abs16,    0,    0,            0,
/* OFFSET   */      0,   s_dir,    0,    0,     d_offset,
/* REG      */      0,   s_dir,    0,    0,        d_reg,
/* RORG     */      0,   s_dir,    0,    0,       d_rorg,
/* TEXT     */      0,   s_dir,    0,    0,       d_text,
/* DATA     */      0,   s_dir,    0,    0,       d_data,
/* BSS      */      0,   s_dir,    0,    0,        d_bss,
/* ORG      */      0,   s_dir,    0,    0,        d_org,
/* IFD      */      0,   s_dir,    0,    0,        d_ifd,
/* IFND     */      0,   s_dir,    0,    0,       d_ifnd,

/* Now for the registers */
/* ===================== */

/* D0  */           0, s_Dr,       0,    0,           0,
/* D1  */           0, s_Dr,       0,    0,           1,
/* D2  */           0, s_Dr,       0,    0,           2,
/* D3  */           0, s_Dr,       0,    0,           3,
/* D4  */           0, s_Dr,       0,    0,           4,
/* D5  */           0, s_Dr,       0,    0,           5,
/* D6  */           0, s_Dr,       0,    0,           6,
/* D7  */           0, s_Dr,       0,    0,           7,

/* A0  */           0, s_Ar,       0,    0,           0,
/* A1  */           0, s_Ar,       0,    0,           1,
/* A2  */           0, s_Ar,       0,    0,           2,
/* A3  */           0, s_Ar,       0,    0,           3,
/* A4  */           0, s_Ar,       0,    0,           4,
/* A5  */           0, s_Ar,       0,    0,           5,
/* A6  */           0, s_Ar,       0,    0,           6,
/* A7  */           0, s_Ar,       0,    0,           7,
/* SR  */           0, s_SR,       0,    0,           0,
/* CCR */           0, s_CCR,      0,    0,           0,
/* SP  */           0, s_Ar,       0,    0,           7,
/* USP */           0, s_USP,      0,    0,           0,
/* PC  */           0, s_PC,       0,    0,           0

};

   datavector = dv;
   dataptr    = 0;

   declare ( "\057ABCD/ADD/ADDA/ADDI/ADDQ/ADDX/AND/ANDI/ASL/ASR//" );
   declare ( "\061BCC/BCS/BEQ/BGE/BGT/BHI/BLE/BLS/BLT/BMI/BNE/BPL//" );
   declare ( "\061BVC/BVS/BCHG/BCLR/BRA/BSET/BSR/BTST/CHK/CLR/CMP//" );
   declare ( "\062CMPA/CMPI/CMPM/DBCC/DBCS/DBEQ/DBF/DBGE/DBGT/DBHI//" );
   declare ( "\062DBLE/DBLS/DBLT/DBMI/DBNE/DBPL/DBT/DBVC/DBVS/DBRA//" );
   declare ( "\061DIVS/DIVU/EOR/EORI/EXG/EXT/JMP/JSR/LEA/LINK/LSL//" );
   declare ( "\061LSR/MOVE/MOVEA/MOVEM/MOVEP/MOVEQ/MULS/MULU/NBCD//" );
   declare ( "\060NEG/NEGX/NOP/NOT/OR/ORI/PEA/RESET/ROL/ROR/ROXL//" );
   declare ( "\062ROXR/RTE/RTR/RTS/SBCD/SCC/SCS/SEQ/SF/SGE/SGT/SHI//" );
   declare ( "\062SLE/SLS/SLT/SMI/SNE/SPL/ST/SVC/SVS/STOP/SUB/SUBA//" );
   declare ( "\055SUBI/SUBQ/SUBX/SWAP/TAS/TRAP/TRAPV/TST/UNLK//" );

   declare ( "\071EQU/EQUR/SET/DC/DS/PAGE/LIST/NOLIST/NOL/SPC/NOPAGE/LLEN//" );
   declare ( "\070PLEN/TTL/NOOBJ/IFEQ/IFNE/ENDC/MACRO/ENDM/MEXIT/GET/END//" );
   declare ( "\070FAIL/CNOP/EXTRN/ENTRY/XREF/XDEF/INCLUDE/IFLT/IFLE/IFGT//" );
   declare ( "\066IFGE/DCB/MASK2/IDNT/FORMAT/NOFORMAT/SECTION/IFC/IFNC//" );
   declare ( "\061ABSOLUTE/NARG/OFFSET/REG/RORG/CODE/DATA/BSS/ORG/IFD/IFND//" );

   declare ( "\102D0/D1/D2/D3/D4/D5/D6/D7/A0/A1/A2/A3/A4/A5/A6/A7/SR/CCR/SP/USP/PC//" );
}

