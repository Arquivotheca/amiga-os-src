/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 3            * */
/* *          (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.                  * */
/* *                                                                            * */
/* *                Last Modified :  11-APR-85     (PJF)                        * */
/* ****************************************************************************** */

/* M68KASM3 : Automatically translated from BCPL to C on 26-Feb-85 */


#include  "libhdr"
#include  "asmhdr"



/******************************************/
/*  External functions returning a value  */
/******************************************/

extern struct EXPRESSNODE *block2i();             /*  asm7  */
extern struct EXPRESSNODE *block3ii();            /*  asm7  */
extern struct EXPRESSNODE *block3pp();            /*  asm7  */
extern struct EXPRESSNODE *effective_address();   /*  asm5  */

extern int *newvec();                      /*  asm7  */
extern string msg();                       /*  asm7  */
extern int  sizefield();                   /*  asm6  */
extern int  eafield();                     /*  asm6  */

/******************************************/
/*  Local functions returning a value     */
/******************************************/

int checkAform();
int checkcmpm();
int movesize();
int reverse();
int evalm();

long checkIform();
long checkIargs();

struct EXPRESSNODE *readmult();
struct EXPRESSNODE *readreg();

/* OK */
specialinstruction ( inum )
int inum;
{
   int      r_m = 0;
   long     rc;

   switch (inum)
   {
      case 1:   /* ABCD  ADDX  SBCD  SUBX */
         get_operands();

         switch (op1_ea)
         {
            case am_Dr:
                r_m = 0;
                break;
            case am_Ar_pd:
                r_m = 1;
                break;
            default:
                complain(8);
                break;
         }

         if ( op_ea != op1_ea )
            complain(9);

         codeword (  instr_mask                      |
                     ( (int)(exp) << 9 )             |
                     ( sizefield(instr_size ) << 6 ) |
                     ( r_m << 3 )                    |
                     (int)(op1_exp)        );
         break;

      case 2:   /* AND  OR */
         get_operands();

         rc = checkIform();
         if ( rc >= 0 )
         {
            genIform( (int)(rc) );
            break;
         }

         if ( op_ea == am_Dr )
         {
            r_m = 0;
            if ( (op1_ea & am_data) == 0 )
               complain(13);
            else
               swapoperands();
         }

         else
            if ( op1_ea != am_Dr )
               complain(14);
            else
               if ( (op_ea & am_mem_alt) == 0 )
                  complain(15);
               else
                  r_m = 1;

         codeword (  instr_mask                      |
                     ( (int)(op1_exp) << 9 )         |
                     ( r_m << 8 )                    |
                     ( sizefield(instr_size) << 6 )  |
                     eafield()     );
         genea();

         break;

      case 3:   /* ADD  SUB */
         get_operands();

         switch (source_ea)
         {
            case ins_A:
            {
               if ( (op1_ea & am_all) == 0 )
                  complain(8);
               rc = checkAform();
               if ( rc < 0 )
                  complain( (int)(-rc) );
               else
                  genNform( (int)(rc) );
               break;
            }
            case ins_I:
            {
               rc = checkIform();
               if ( rc >= 0 )
               {
                  source_ea = am_imm;
                  generate(2);
               }
               else
                  complain( (int)(-rc) );
               break;
            }
            case ins_N:
            {
               rc = checkAform();
               if ( rc >= 0 )
               {
                  genNform( (int)(rc) );
                  break;
               }

               rc = checkIform();
               if ( rc >= 0 )
               {
                  instr_mask = (int)(rc);
                  source_ea  = am_imm;
                  generate(2);
                  break;
               }

               if ( op_ea == am_Dr )
               {
                  r_m = 0;
                  if ( (op1_ea & am_all) == 0 )
                     complain(8);
                  swapoperands();
               }
               else
               {
                  r_m = 1;
                  if ( op1_ea != am_Dr )
                     complain(14);
                  else
                     if ( (op_ea & am_mem_alt) == 0 )
                        complain(15);
               }

               codeword(  instr_mask                    |
                          ((int)(op1_exp) << 9)         |
                          (r_m << 8)                    |
                          (sizefield(instr_size) << 6)  |
                          eafield()  );
               genea();
               break;
            }
         }

         break;

      case 4:
         nextsymb();

         if (nargs == 2)
         {
            checkshort();
            evaluate( effective_address() );
            checkfor( s_comma, 10 );
            if ( op_ea != am_Dr )
               complain(14);
            swapoperands();
         }

         evaluate( effective_address() );
         checkfor( s_none, 12 );

         if (relocation_needed)
           if (reloc_wrt==sectnumber)
              op_ea = am_PC_disp;
           else complain(193);

         if (extrnref)
            op_ea = am_PC_disp;
         else
            if ( (exptype & s_mask) == s_rel )
               if ( ((exptype >> 8) & 0xFF) != sectnumber )
                  complain(182);

         if ( op_ea == am_PC_disp )
         {
            if ( locmode != s_rel )
               complain (18);
         }

         else
            if ( (op_ea == am_abs16) || (op_ea == am_abs32) )
            {
               if ( ! pass1 )
                  if ( locmode != s_abs )
                     complain(17);
            }
            else
               complain(16);

         if ( (nargs == 1) && (instr_size == ts_none) )
            if ( ! extrnref )
            {
               long offset = exp - (location + 2);
               int inrange = (-128  <= offset) & (offset <= 127);
               if ( ( inrange && ( offset != 0 ) ) &&  ! fwdref  )
                  instr_size = ts_short;
            }

         if ( instr_size == ts_short )
         {
            long offset;

            if ( extrnref )
            {
               offset = exp;
               addextrnref( extrnlsymb, location + 1, 1 );
            }
            else
            {
               offset = exp - ( location + 2 );
               if ( (offset == 0) && pass2 )
                  complain(19);
            }

            codeword (  instr_mask             |
                        (source_ea << 8)       |
                        ((int)(offset) & 0xFF)  );

            if ( !( ((-128 <= offset) && (offset <= 127)) || pass1) )
               complain(20);
         }
         else
         {
            long offset = exp - (location+2);

            if (extrnref)
            {
               offset = exp;
               addextrnref (extrnlsymb, location+2, 2);
            }

            if ( pass1 || ((-32768 < offset) && (offset < 32767)) )
            {
               codeword ( instr_mask         |
                          (source_ea << 8)   |
                          (nargs == 2 ? (int)(op1_exp) : 0x0)     );
               codeword ( (int)(offset) );
            }

            else
               complain(21);
         }
         break;

      case 5:
         checkshort();
         nextsymb();
         evaluate( effective_address() );

         if ( (op_ea == am_Dr) || ( (op_ea & am_imm3) != 0 ) )
         {
            int r   = exp & 0x7;
            int i_r = op_ea == am_Dr ? 1 : 0;
            int dr  = source_ea & 1;

            if ( extrnref )
               complain(161);

            checkfor(s_comma, 10);
            swapoperands();
            evaluate( effective_address() );
            checkfor( s_none, 12 );

            if ( op_ea == am_Dr )
               codeword (  instr_mask                    |
                           (r << 9)                      |
                           (dr << 8)                     |
                           (sizefield(instr_size) << 6)  |
                           (i_r << 5)                    |
                           (int)(exp)                   );
            else
               complain(22);
         }
         else
            if ( (op_ea & am_mem_alt) != 0 )
            {
               static VAR16BIT tempmask[] = { 0xE0C0, 0xE0C0,
                                              0xE2C0, 0xE2C0,
                                              0xE6C0, 0xE6C0,
                                              0xE4C0, 0xE4C0 };
               int mask      = tempmask[ source_ea ];
               int dr        = source_ea & 1;
   
               checkfor( s_none, 12 );
               codeword( (mask | (dr << 8) | eafield()) );
               genea();
            }
            else
               complain(8);
         break;

      case 6:
         get_operands();

#ifndef BTST_FIX
         if (op1_ea == am_Dr)
#endif BTST_FIX
         {
            int btst = source_ea == 0x0;

#ifndef BTST_FIX
            if ( op_ea && ( (btst ? am_data : am_data_alt) == 0 ) )
#else BTST_FIX
            if (op1_ea == am_Dr)
            {
               if ( ( op_ea & (btst ? am_data : am_data_alt) ) == 0 )
#endif BTST_FIX
               complain ( btst ? 33 : 23 );
            else
            {
               codeword(  0x100                    |
                          ((int)(op1_exp) << 9)    |
                          (source_ea << 6)         |
                          eafield()            );
               genea();
            }
         }
         else
            if ( (op1_ea & am_imm16) != 0 )
#ifndef BTST_FIX
               if ( (op_ea & am_data_alt) == 0 )
                  complain (23);
#else BTST_FIX
                  if ((op_ea & (btst ? (am_data-am_imm) : am_data_alt) ) == 0 )
                     complain ( btst ? 9 : 23 );
#endif BTST_FIX
               else
               {
                  codeword ( (0x800 | (source_ea << 6) | eafield()) );
                  if ( op1_extrnref )
                     addextrnref( op1extrnlsymb, location+codewords*2, 2 );
                  codeword ( (int)(op1_exp & 0xFF) );
                  genea();
               }
            else
               complain (8);
#ifdef BTST_FIX
	 }
#endif BTST_FIX
         break;

      case 7:
         {
            int regs = am_Dr | am_Ar;
            int opmode = 0;

            get_operands();

            if ( (op1_ea & regs) == 0 )
               complain (24);
            if ( (op_ea & regs) == 0 )
               complain (24);

            opmode = (op_ea == op1_ea) ? ((op_ea == am_Dr) ? 0x8 : 0x9) :
                                         0x11;

            if ( (op_ea != op1_ea) )
               if (op1_ea == am_Ar)
                  swapoperands ();

            codeword (  instr_mask             |
                        ((int)(op1_exp) << 9)  |
                        (opmode << 3)          |
                        (int)(exp) );
         }
         break;

      case 8:
         switch ( source_ea )
         {
            case 0:
               genmove();
               break;
            case 1:
               genmovea();
               break;
            case 2:
               in_movem = yes;
               genmovem();
               in_movem = no;
               break;
            case 3:
               genmovep();
               break;
            case 4:
               genmoveq();
               break;
            default:
               complain(1001);
         }
         break;

      case 11:
         checkshort();
         nextsymb();
         evaluate( effective_address() );

         if (extrnref)
            complain(161);

         checkfor(s_none, 12);

         if ( (op_ea & am_imm) == 0 )
            complain ( 25 );
         else
            if (  !((0 <= exp) && (exp <= 15)) )
               complain(26);
            else
               codeword( instr_mask | (int)(exp) );
         break;

      case 12:
         get_operands();
         rc = checkIform();

         if (rc<0)
            complain ( (int)(-rc) );
         else
            genIform ( (int)(rc) );
         break;

      case 15:
         get_operands();
         rc = checkIform();
         if (rc >= 0)
            genIform( (int)(rc) );
         else
         {
            if (op1_ea != am_Dr)
               complain (14);

            if ( (op_ea & am_data_alt) == 0 )
               complain (23);

            codeword ( instr_mask                     |
                       ((int)(op1_exp) << 9)          |
                       (sizefield( instr_size ) << 6) |
                       eafield()     );

            genea();
         }
         break;

      case 16:
      {
         int sizebits = 0;
         int cw;

         get_operands();

         if ( (op1_ea & am_all) == 0 )
            complain(8);

         switch (source_ea)
         {
            case ins_A:
            {
               rc = checkAform();
               if (rc < 0)
                  complain( (int)(-rc) );
               else
                  genNform( (int)(rc) );
               break;
            }
            case ins_I:
            {
               rc = checkIform();
               if (rc < 0)
                  complain ( (int)(-rc) );
               else
               {
                  source_ea = am_imm;
                  generate(2);
               }
               break;
            }
            case ins_M:
            {
               rc = checkcmpm();
               if ( rc < 0 )
                  complain ( (int)(-rc) );
               else
                  gencmpm();
               break;
            }
            case ins_N:
            {
               rc = checkAform();
               if ( rc >= 0 )
               {
                  genNform( (int)(rc) );
                  break;
               }

               rc = checkIform();
               if ( rc >= 0 )
               {
                  instr_mask = m_CMPI;
                  source_ea  = am_imm;
                  generate(2);
                  break;
               }

               rc = checkcmpm();
               if (rc >= 0)
               {
                  instr_mask = m_CMPM;
                  gencmpm();
                  break;
               }

               if (op_ea != am_Dr)
                  complain(9);

               genNform(sizefield(instr_size));
               break;
            }
         }
         break;
      }

      default:
         complain ( 1002 );
         break;

   }

   skiprest ();
}

/* OK */
get_operands ( )
{
   checkshort();
   nextsymb();
   evaluate(effective_address());
   checkfor(s_comma, 10);
   swapoperands();
   evaluate(effective_address());
   checkfor(s_none, 12);
}

/* OK */
int checkAform()
{
   if (op_ea != am_Ar)
      return  -30 ;

   if ( (instr_size == ts_word) || (instr_size == ts_none) )
      return 0x3;

   if ( instr_size == ts_long )
      return 0x7;

   return (instr_size == ts_byte) ?  -29 : -6;
}

/* OK */
long checkIform()
{
   switch (instr_mask)
   {
   case m_ADD:
   case m_ADDI:
      return checkIargs(no, (long)(m_ADDI) );
   case m_SUB:
   case m_SUBI:
      return checkIargs(no, (long)(m_SUBI) );
   case m_CMP:
   case m_CMPI:
      return checkIargs(no, (long)(m_CMPI) );
   case m_EOR:
   case m_EORI:
      return checkIargs(yes, (long)(m_EORI) );
   case m_OR:
   case m_ORI:
      return checkIargs(yes, (long)(m_ORI) );
   case m_AND:
   case m_ANDI:
      return checkIargs(yes, (long)(m_ANDI) );
   default:
      complain(1016);
   }
}

/* OK */
long checkIargs(special, newmask)
int special;
long newmask;
{
   if (special)
   {
      if ( (op1_ea & am_imm) == 0 )
         return  -27;

      if (op_ea == am_special)
      {
         int correct_size = (exp == s_SR)  ? ts_word :
                            (exp == s_CCR) ? ts_byte : -9;
         if (correct_size < 0)
            return -9;
         if (instr_size != ts_none)
            if (instr_size != correct_size)
               return -28;

         return newmask;
      }
      else
         return ((op_ea & am_data_alt) == 0) ? (long)(-23) : newmask;
   }
   else
   {
      if ((op_ea & am_data_alt) == 0)
         return  (long)(-9);
      if ((op1_ea & am_imm) == 0)
         return  (long)(-8);

      return newmask;
   }
}


/* OK */
int checkcmpm()
{
   if (op1_ea != am_Ar_pi)
      return -31;
   if (op_ea != am_Ar_pi)
      return -32;
   return 0;
}

/* OK */
genNform ( sz )
int sz;
{
   swapoperands();
   codeword ( (instr_mask            |
               ((int)(op1_exp) << 9) |
               (sz << 6)             |
               eafield())         );
   genea();
}

/* OK */
gencmpm()
{
   codeword ( instr_mask                   |
              ((int)(exp) << 9)            |
              (sizefield(instr_size) << 6) |
              (int)(op1_exp) );
}

/* OK */
genIform ( imask )
int imask;
{
   instr_mask = imask;

   if (op_ea == am_special)
   {
      int size = (exp == s_SR)  ? ts_word :
                 (exp == s_CCR) ? ts_byte : complain(1017);

      codeword ( (instr_mask | (sizefield(size) << 6) ) | 0x3C );
      codeword ( (int)op1_exp );
   }
   else
   {
      source_ea = am_imm;
      generate(2);
   }
}

/* OK */
checkshort()
{
   if (instr_size == ts_short)
      complain(86);
}


/* OK */
genmove ( )
{
   get_operands();

   if (op_ea == am_special)
   {
      int size = ((int)exp == s_SR)  ? ts_word :
                 ((int)exp == s_CCR) ? ts_byte : -1;

      if ( size != -1 )
         if ( (op1_ea & am_data) == 0 )
            complain (13);
         else
         {
            if (instr_size != ts_none)
               if (instr_size != ts_word)
                  complain(28);

            swapoperands();
            codeword ( 0x44C0                  |
                       (sizefield(size) << 9)  |
                       eafield()             );
            genea();
            return;
         }
   }

   if (op1_ea == am_special)
      if (op1_exp == s_SR)
      {
         if ( ! ((instr_size == ts_word) || (instr_size == ts_none)) )
            complain(34);
         if ( (op_ea & am_data_alt) == 0 )
            complain(23);
         else
         {
            codeword( 0x40C0 | eafield() );
            genea();
            return;
         }
      }

   if ( ((op1_ea == am_special) && (op1_exp == s_USP)) ||
        ((op_ea  == am_special) && (exp == s_USP))  )
   {
      int dr = (op1_ea == am_special) ? 1 : 0;

      if ( !((instr_size == ts_long) || (instr_size == ts_none)) )
         complain(35);
      if (op_ea == am_special)
         swapoperands();
      if (op_ea == am_Ar)
         codeword ( 0x4E60 | (dr << 3) | (int)(exp) );
      else
         complain( (dr == 1) ? 30 : 36 );
      return;
   }

   if ( (op1_ea & am_all) == 0 )
      complain(8);
   else
      if ( (op_ea & am_alt) == 0 )
         complain (99);
      else
         generalmove();
}

/* OK */
genmovea ( )
{
   get_operands();

   if ( (op1_ea & am_all) == 0 )
      complain(8);
   else
      if (op_ea != am_Ar)
         complain(30);
      else
         generalmove();
}

/* OK */
generalmove()
{
   swapoperands();
   {
      int operand1 = 0;
      int operand2 = eafield();
      swapoperands();
      operand1 = eafield();
      operand1 = ((operand1 << 3) | (operand1 >> 3)) & 0x3F;
      codeword ( 0x0                          |
                 (movesize(instr_size) << 12) |
                 (operand1 << 6)              |
                 operand2 );
      swapoperands();
      genea();
      swapoperands();
      genea();
   }
}

/* OK */
int movesize ( size )
int  size;
{
   switch ( size )
   {
     case ts_byte:
        return 0x1;
     case ts_word:
        return 0x3;
     case ts_long:
        return 0x2;
     case ts_none:
        return movesize(ts_default);
     default:
        complain(37);
   }
}

/* OK */
genmovem ( )
{
   int dr = 0;
   int sz = 0;
   int rbits = 0;

   checkshort();
   nextsymb();

   if ( (symb == s_Ar) || (symb == s_Dr) || (symb == s_reg) )
   {
      int bits = evalm(readmult());

      checkfor(s_comma, 10);
      evaluate(effective_address());
      checkfor(s_none, 12);

      if ( (op_ea & am_contr_alt) != 0 )
         rbits = bits;
      else
         if ( (op_ea & am_Ar_pd) != 0 )
            rbits = reverse(bits);
         else
            complain(9);
      dr = 0;
   }
   else
   {
      evaluate(effective_address());
      checkfor(s_comma, 10);
      {
         int bits = evalm(readmult());
         checkfor(s_none, 12);

         if ( (op_ea & am_contr) != 0 )
            rbits = bits;
         else
            if ( (op_ea & am_Ar_pi) != 0 )
               rbits = bits;
            else
               complain(8);
      }
      dr = 1;
   }

   sz = (instr_size == ts_long) ? 1 :
        (instr_size == ts_word) ? 0 :
        (instr_size == ts_none) ? 0 :
        complain(38);

   codeword ( 0x4880 | (dr << 10) | (sz << 6) | eafield() );
   codeword ( rbits & 0xFFFF );
   genea();
}

/* OK */
int reverse ( bits )
int bits;
{
   int i;
   int newbits = 0;

   for ( i = 1 ; i <= 16 ; i++ )
   {
      newbits = (newbits << 1) | (bits & 1);
      bits >>= 1;
   }

   return newbits;
}

/* OK */
int evalm ( ptr )
struct EXPRESSNODE *ptr;
{
   int   ptr0 = ptr->type;

   switch ( ptr0 )
   {
   case s_Ar:
      return (0x100 << (ptr->uvalleft).intvalue);
   case s_Dr:
      return (0x1 << (ptr->uvalleft).intvalue);
   case s_reg:
      return (ptr->uvalleft).intvalue;
   case s_slash:
      return (evalm( (ptr->uvalleft).eptr ) |
              evalm( (ptr->uvalright).eptr) );
   case s_hyphen:
      {
         int  r1     = evalm( (ptr->uvalleft).eptr );
         int  r2     = evalm( (ptr->uvalright).eptr );
         int  rtype1 = ((ptr->uvalleft).eptr)->type;
         int  rnum1  = (((ptr->uvalleft).eptr)->uvalleft).intvalue;
         int  rtype2 = ((ptr->uvalright).eptr)->type;
         int  rnum2  = (((ptr->uvalright).eptr)->uvalleft).intvalue;

         if ( rtype1 == rtype2 )
         {
            int result = 0;

            if (rnum2 < rnum1)
            {
               int t = r1;
               r1 = r2;
               r2 = t;
            }

            result = r1;

            while ( r1 != r2 )
            {
               r1 <<= 1;
               result |= r1;
            }

            return result;
         }
         else
            complain(39);
      }
      return 0;

   default:
      complain(1003);
   }
}

/* OK */
struct EXPRESSNODE *readmult()
{
   if (symb == s_reg)
   {
      readsymb ();
      return (block3ii(s_reg, (int)(symbtype->st_value), 0));
   }
   else
   {
      struct EXPRESSNODE *result = readreg();
      do
      {
         if (symb == s_over)
         {
            readsymb();
            return (block3pp(s_slash, result, readmult()));
         }
         else
            if (symb == s_minus)
            {
               readsymb();
               result = (block3pp(s_hyphen, result, readreg()));
               if (symb == s_over)
                  continue;
               return result;
            }
            else
               return result;
      }
         while (true);
   }
}

/* OK */
struct EXPRESSNODE *readreg()
{
   if ( (symb == s_Ar) || (symb == s_Dr) )
      if ( tagsize_given != ts_none )
         complain(40);
      else
      {
         struct EXPRESSNODE *result = block2i(symb, regnum);
         readsymb();
         return result;
      }
   else
      complain(41);

   return NULL;
}

/* OK */
doreg ( )
{
   checklabel(yes);
   nextsymb();
   value = evalm( readmult() );
   setlabel( s_reg, value, no );
   listEQU(s_abs16);
}

/* OK */
listEQU ( restype )
int restype;
{
   if ( pass2 && (listing || error_found) )
   {
      preparedefaultbuffer();
      linepos = 0;
      writestring("\006      ");
      linepos = 10;
      writechar(directive == d_set ? '>' : '=');
      linepos = 11;

      if ( (restype == s_abs16) && ((value & (long)(0xFFFF)) != 0) )
        restype = s_abs32;
      
      writehexvalue ( value, (restype == s_abs32) ? 8 :
                      ((restype == s_Ar) || ( restype == s_Dr )) ? 1 : 4 );

      if (restype == s_rel)
         writechar('\'');
      if (restype == s_Dr)
         writestring ("\003-Dr");
      if (restype == s_Ar)
         writestring ("\003-Ar");

      error_found = no;
      printbuffer();
      listed = yes;
   }
}

/* OK */
genmovep ( )
{
   int  dr = 0;
   int  sz = 0;

   get_operands();

   if (op1_ea == am_Dr)
      if (op_ea == am_Ar_disp)
      {
         dr = 1;
         swapoperands();
      }
      else
         complain(42);
   else
      if (op1_ea != am_Ar_disp)
         complain(43);
      else
         if (op_ea != am_Dr)
            complain(22);
         else
            dr = 0;

   sz = (instr_size == ts_long) ? 1 :
        (instr_size == ts_word) ? 0 :
        (instr_size == ts_none) ? 0 : complain(44);

   codeword ( 0x108             |
              ((int)(exp) << 9) |
              (dr << 7)         |
              (sz << 6)         |
              (op1_registers->uvalleft).intvalue    );

   codeword ( (int)(op1_exp & 0xFFFF) );
}

/* OK */
genmoveq()
{
   get_operands();

   if ((op1_ea & am_imm) == 0 )
      complain(27);

   if (op_ea != am_Dr)
      complain(22);

   if ( ! ((-128 <= op1_exp) && (op1_exp <= 127)) )
      complain(45);

   if (op1_extrnref)
      addextrnref(op1extrnlsymb, (location + (codewords*2)) + 1, 1 );

   codeword ( 0x7000                | 
              ((int)(exp) << 9)     |
              ((int)(op1_exp) & 0xFF) );
}

