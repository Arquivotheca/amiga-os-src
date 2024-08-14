/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 5		* */
/* *	      (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.			* */
/* *										* */
/* *		    Last Modified :  11-APR-85	   (PJF)			* */
/* ****************************************************************************** */

/* M68KASM5 : Automatically translated from BCPL to C on 26-Feb-85 */


#include  "libhdr"
#include  "asmhdr"


/******************************************/
/*  External functions returning a value  */
/******************************************/

extern struct EXPRESSNODE *block1();	  /*  asm7  */
extern struct EXPRESSNODE *block2p();	  /*  asm7  */
extern struct EXPRESSNODE *block2l();	  /*  asm7  */
extern struct EXPRESSNODE *block3ii();	  /*  asm7  */
extern struct EXPRESSNODE *block3pp();	  /*  asm7  */

extern int absolute();			  /*  asm6  */
extern int bytesized(); 		  /*  asm6  */
extern int checkregister();		  /*  asm6  */
extern int relocatable();		  /*  asm6  */
extern int wordsized(); 		  /*  asm6  */


/******************************************/
/*  Local functions returning a value	  */
/******************************************/

struct EXPRESSNODE *effective_address();
struct EXPRESSNODE *expression();
struct EXPRESSNODE *factor();
struct EXPRESSNODE *term();
struct EXPRESSNODE *secondary();
struct EXPRESSNODE *primary();

long readnumber();

int digitval();
int abs16addr();
int evalnumexp();
int evalexp();
int sizevalue();
int finaltype();


/* OK */
readsymb ( )
{
   int base;

#ifdef DBUG
   mydebug("Readsymb","ch = %C",(long)ch);
#endif

   if ( setjmp(rdsymlevel) != 0 )
     goto rdsymlabel;
   locallabel = no;
   switch(ch)
   {
   case ';':
      skiprest (  );
   case ' ':
   case '\t':
   case '\n':
      symb = s_none;
      break;
   case '(':
      rch (  );
      symb = s_bra;
      break;
   case ')':
      rch (  );
      symb = s_ket;
      break;
   case '#':
      rch (  );
      symb = s_literal;
      break;
   case ',':
      rch (  );
      symb = s_comma;
      break;
   case ':':
      rch (  );
      symb = s_colon;
      break;
   case '0':   case '1':   case '2':   case '3':
   case '4':   case '5':   case '6':   case '7':
   case '8':   case '9':
      base = 10;
      goto readnumlab;
   case '$':
      base = 16;
      rch (  );
      goto readnumlab;
   case '@':
      base = 8;
      rch (  );
      goto readnumlab;
   case '%':
      base = 2;
      rch (  );
   readnumlab:
      number = readnumber(base);
      symb = s_number;
      break;
   case '+':
      symb = s_plus;
      rch (  );
      break;
   case '-':
      symb = s_minus;
      rch (  );
      break;
   case '*':
      symb = s_times;
      rch (  );
      break;
   case '/':
      symb = s_over;
      rch (  );
      break;
   case '&':
      symb = s_logand;
      rch (  );
      break;
   case '!':
      symb = s_logor;
      rch (  );
      break;
   case '~':
      symb = s_not;
      rch (  );
      break;
   case '<':
      rch (  );
      if ( ch == '<' )
      {
	 rch (	);
	 symb = s_lshift;
      }
      else
	 complain ( 56 );
      break;
   case '>':
      rch (  );
      if ( ch == '>' )
      {
	 rch (	);
	 symb = s_rshift;
      }
      else
	 complain ( 56 );
      break;
   case '\\':
      if ( ! inmacro )
	 complain ( 117 );
      while ( ! ( ( ( ch == ' ' ) || ( ch == '\t' ) ) || ( ch == '\n' )
	  ) )
	 rch (	);
      symb = s_none;
      break;
   case '.':   case '_':
   case 'A':   case 'B':   case 'C':   case 'D':
   case 'E':   case 'F':   case 'G':   case 'H':
   case 'I':   case 'J':   case 'K':   case 'L':
   case 'M':   case 'N':   case 'O':   case 'P':
   case 'Q':   case 'R':   case 'S':   case 'T':
   case 'U':   case 'V':   case 'W':   case 'X':
   case 'Y':   case 'Z':
   case 'a':   case 'b':   case 'c':   case 'd':
   case 'e':   case 'f':   case 'g':   case 'h':
   case 'i':   case 'j':   case 'k':   case 'l':
   case 'm':   case 'n':   case 'o':   case 'p':
   case 'q':   case 'r':   case 's':   case 't':
   case 'u':   case 'v':   case 'w':   case 'x':
   case 'y':   case 'z':
   rdsymlabel:
      readtag (  );
      break;
   case '\'':
      {
	 int count = 0;
	 rch (	);
	 number = 0;
	 while ( count <= 4 )
	 {
	    if ( ch == '\n' )
	       complain ( 57 );
	    if ( ch == '\'' )
	    {
	       rch();
	       if ( ch != '\'' )
	       {
		  symb = s_number;
		  return;
	       }
	    }
	    number = (number << 8) + ch;
	    count++;
	    rch();
	 }
	 complain ( 58 );
      }
      break;
   default:
      complain ( 59 );
   }
}

/* OK */
long readnumber ( base )
int base;
{
   int	val = digitval(ch);
   long num = 0;
   int	safepos = charpos - 1;

#ifdef DBUG
   mydebug("Readnumber","Base = %N", (long)base );
#endif

   if ( !(val < base) )
      complain (60);

   /* Problems are encountered on the IBM because
      all 32 bit quantities are signed. Therefore
      $7FFFFFFF * 2 will not become $FFFFFFFF.
      This problem is solved below for base 2,8,16
      but not for base 10. */

#ifdef IBM
   if (base != 10)
   {
     int shift = (base == 2) ? 1 :
                 (base == 8) ? 3 : 4;

     while (val < base)
     {
        num = val + (num << shift);
        rch();
        val = digitval(ch);
     }
   }
   else
#endif
   while ( val < base )
   {
      num = (num * base) + val;
      rch (  );
      val = digitval(ch);
   }

   if ( ( ch == '$' ) && ( base == 10 ) )
   {
      charpos = safepos;
      rch (  );
      getlocallabel = true;
      longjmp ( rdsymlevel );
   }
   else
      return num;
}

/* OK */
int digitval ( character )
int character;
{
    switch (character)
    {
       case '0' : case '1': case '2': case '3':
       case '4' : case '5': case '6': case '7':
       case '8' : case '9':
	      return  character - '0';
       case 'A' : case 'B': case 'C': case 'D':
       case 'E' : case 'F':
	      return  character + (10 - 'A');
       case 'a' : case 'b': case 'c': case 'd':
       case 'e' : case 'f':
	      return  character + (10 - 'a');
       default	:
	      return  100;
    }
}

/* OK */
struct EXPRESSNODE *effective_address ( )
{
   struct EXPRESSNODE *result = NULL;

#ifdef DBUG
   mydebug("Effective_address","symb = %N",(long)symb );
#endif

   bracount = 0;
   switch (symb)
   {
   case s_none:
      complain(61);
   case s_literal:
      readsymb();
      return block2p(ea_literal, expression());
   default:
      result = expression();
      if ( symb == s_bra )
      {
	 int rtype1 = 0;
	 int rnum1  = 0;
	 int rsize1 = 0;
	 int rtype2 = 0;
	 int rnum2  = 0;
	 int rsize2 = 0;
	 readsymb();
	 if ( ! ((symb == s_Ar) || (symb == s_Dr) || (symb == s_PC)) )
	    complain ( 62 );
	 rtype1    = symb;
	 rnum1	   = regnum;
	 rsize1    = tagsize_given;
	 readsymb();
	 if ( symb == s_ket )
	 {
	    readsymb();
	    return block3pp(ea_R_disp, result,
			  block3ii(rtype1, rnum1, rsize1));
	 }
	 else
	    if (symb == s_comma)
	    {
	       if ( ! ( ( rtype1 == s_Ar ) || ( rtype1 == s_PC ) ) )
		  complain ( 63 );
	       readsymb (  );
	       if ( ! ( ( symb == s_Ar ) || ( symb == s_Dr ) ) )
		  complain ( 64 );
	       rtype2 = symb;
	       rnum2 = regnum;
	       rsize2 = tagsize_given;
	       readsymb (  );
	       checkfor ( s_ket, 66 );

	       return block3pp(ea_R_index, result,
			   block3pp( -1, block3ii(rtype1, rnum1, rsize1),
					 block3ii(rtype2, rnum2, rsize2) ));
	    }
	    else
	       complain ( 65 );
      }
      else
	 return block2p(ea_exp, result);
   }
}


struct EXPRESSNODE *expression()
{
   struct EXPRESSNODE *f = factor();

#ifdef DBUG
   mydebug("Expression","symb = %N",(long)symb);
#endif

   while ( (symb == s_plus) || (symb == s_minus) )
   {
      int op = symb;
      readsymb();
      f = block2p(s_opapply, block3pp(op, f, factor()));
   }
   return f;
}


struct EXPRESSNODE *factor()
{
   struct EXPRESSNODE *t = term();

#ifdef DBUG
   mydebug("Factor","symb = %N",(long)symb);
#endif

   while ( ( symb == s_times ) || ( symb == s_over ) )
   {
      int op = symb;
      readsymb (  );
      t = block2p(s_opapply, block3pp(op, t, term()));
   }
   return t;
}

/* OK */
struct EXPRESSNODE *term ()
{
   struct EXPRESSNODE *s = secondary();

#ifdef DBUG
   mydebug("Term","symb = %N",(long)symb);
#endif

   while ( ( symb == s_logand ) || ( symb == s_logor ) )
   {
      int op = symb;
      readsymb (  );
      s = block2p(s_opapply, block3pp(op, s, secondary()));
   }
   return s;
}

/* OK */
struct EXPRESSNODE *secondary()
{
   struct EXPRESSNODE *p = primary();

#ifdef DBUG
   mydebug("Secondary","symb = %N",(long)symb);
#endif

   while ( (symb == s_lshift) || (symb == s_rshift) )
   {
      int op = symb;
      readsymb (  );
      p = block2p(s_opapply, block3pp(op, p, primary()));
   }
   return p;
}


struct EXPRESSNODE *primary ( )
{
   struct EXPRESSNODE *result = NULL;

#ifdef DBUG
   mydebug("Primary","symb = %N",(long)symb);
#endif

   switch ( symb & s_mask )
   {
   case s_Dr:
   case s_Ar:
      if ( ((symbtype->st_flags) & stb_setnow) == 0 )
	 complain(148);
      result = block3ii(symb, regnum, tagsize_given);
      if ( (ch == '-') || (ch == '/') )
	 if ( ! in_movem )
	    complain(67);
      checktagsize();
      readsymb();
      return result;
   case s_SR:
   case s_CCR:
   case s_USP:
   case s_PC:
      checktagsize();
   case s_star:
      result = block1(symb);
      readsymb();
      return result;
   case s_ext:
   case s_rel:
   case s_abs16:
   case s_abs32:
   case s_new:
      if ( pass2 && undefined )
	 complain(97);
      checktagsize();
      result = block2p(symb, symbtype);
      readsymb();
      return result;
   case s_number:
      result = block2p(s_number, NULL);
      (result->uvalleft).longvalue = number;
      readsymb();
      return result;

   case s_minus:
      readsymb();
      if ( symb == s_bra )
      {
	 readsymb (  );
	 if ( symb == s_Ar )
	 {
	    if ( bracount > 0 )
	       complain ( 68 );
	    result = block2p(s_Ar_predecr,
		       block3ii(symb, regnum, tagsize_given));
	    readsymb();
	    checkfor(s_ket, 65);
	    return result;
	 }
	 result = expression();
	 checkfor(s_ket, 69);
	 return block2p(s_monminus, result);
      }
      return block2p(s_monminus, primary());

   case s_not:
      readsymb();
      if ( symb == s_bra )
      {
	 readsymb();
	 result = expression();
	 checkfor(s_ket, 69);
	 return block2p(s_not, result);
      }
      return block2p(s_not, primary());

   case s_bra:
      readsymb();
      if (symb == s_Ar)
      {
	 if (bracount > 0)
	    complain(68);
	 result = block3ii(symb, regnum, tagsize_given);
	 readsymb();
	 checkfor(s_ket, 65);
	 if (symb == s_plus)
	 {
	    readsymb();
	    return block2p(s_Ar_postincr, result);
	 }
	 else
	    return block2p(s_Ar_indirect, result);
      }
      bracount++;
      result = expression();
      checkfor(s_ket, 69);
      bracount--;
      return result;

   case s_literal:
      complain(139);

   case s_plus:
   case s_over:
   case s_logand:
   case s_logor:
   case s_lshift:
   case s_rshift:
      complain ( 140 );
   case s_none:
      complain ( 141 );
   case s_ket:
      complain ( 142 );
   case s_comma:
      complain ( 143 );
   case s_colon:
      complain ( 144 );
   case s_instr:
      complain ( 135 );
   case s_dir:
      complain ( 136 );
   case s_macro:
      complain ( 137 );
   default:
      complain ( 70 );
   }
}


/* OK */
evaluate ( ptr )
struct EXPRESSNODE *ptr;
{
#ifdef DBUG
   mydebug("Evaluate","Ptr = %X8 Type = %N",(long)ptr, ((ptr == NULL)?0L:
		   (long)(ptr->type)));
#endif

   if ( ptr != NULL )
   {
      struct EXPRESSNODE *ptr1 = (ptr->uvalleft).eptr;

      fwdref = no;
      relocation_needed = no;
      op_ea = 0;

      switch ( ptr->type )
      {
      case ea_literal:
	 exptype = evalnumexp(ptr1);
	 exp	 = value;

	 if ( relocatable(exptype) )
	 {
	    relocation_needed = yes;
	    reloc_wrt = (exptype >> 8) & 0xFF;
	    exptype = s_abs32;
	 }

	 if ( ! relocation_needed )
	 {
	    if ( ( 1 <= exp ) && ( exp <= 8 ) )
	       op_ea = op_ea | am_imm3;
	    if ( wordsized(exp) )
	       op_ea = op_ea | am_imm16;
	 }

	 op_ea = op_ea | am_imm32;
	 switch ( instr_size )
	 {
	 case ts_byte:
	    if ( ! bytesized(exp) )
	       warning ( 176 );
	    break;
	 case ts_none:
	 case ts_word:
	    if ( ! wordsized(exp) )
	       warning ( 175 );
	    break;
	 case ts_long:
	    break;
	 default:
	    complain ( 1005 );
	 }
	 break;

      case ea_exp:
	 exptype = evalexp(ptr1);
	 exp	 = value;
	 {
	    switch ( exptype & s_mask )
	    {
	    case s_rel:
	       if ( fwdref || ( sectnumber != ( exptype >> 8 ) )  )
	       {
		  relocation_needed = yes;
		  reloc_wrt = (exptype >> 8) & 0xFF;
		  op_ea = am_abs32; break;
	       }
	       else
		  op_ea = am_PC_disp; break;
	    case s_abs16:
	       op_ea = abs16addr(exp); break;
	    case s_abs32:
	       op_ea = am_abs32; break;
	    case s_Ar:
	       op_ea = am_Ar; break;
	    case s_Dr:
	       op_ea = am_Dr; break;
	    case s_Ar_predecr:
	       op_ea = am_Ar_pd; break;
	    case s_Ar_postincr:
	       op_ea = am_Ar_pi; break;
	    case s_Ar_indirect:
	       op_ea = am_Ar_ind; break;
	    case s_SR:
	    case s_CCR:
	    case s_USP:
	       op_ea = am_special; break;
	    default:
	       complain ( 70 );
	    }
	 };
	 break;

      case ea_R_disp:
	 exptype = evalexp(ptr1);
	 if ( ( exptype & s_mask ) == s_rel )
	    if ( ! ( ((exptype >> 8) & 0xFF) == sectnumber ) )
	       complain(182);
	 registers = (ptr->uvalright).eptr;
	 if ( registers->type == s_PC )
	 {
	    if ( fwdref && pass1 )
	    {
	       exptype = locmode;
	       exp = location;
	    }
	    else
	       exp = value;
	    op_ea = am_PC_disp;
	 }
	 else
	 {
	    exp = value;
	    {
	       switch ( exptype & s_mask )
	       {
	       case s_rel:
		  op_ea = am_PC_index; break;
	       case s_abs16:
	       case s_abs32:
		  if ( ! ((-32768 <= exp) && (exp <= 32767)) )
		     complain(89);
		  op_ea = am_Ar_disp; break;
	       default:
		  complain(70);
	       }
	    }
	 }
	 break;

      case ea_R_index:	 /* X(Rx,Ry) */

	 /* First evaluate X */
	 exptype = evalexp(ptr1);
	 if ( (exptype & s_mask) == s_rel )
	    if ( ! (((exptype >> 8) & 0xFF) == sectnumber) )
	       complain(182);

	 /* Get ptr to (Rx,Ry) */
	 registers = (ptr->uvalright).eptr;
	 if ( ((registers->uvalleft).eptr)->type == s_PC )
	 {
	    registers = (registers->uvalright).eptr;
	    if ( fwdref && pass1 )
	    {
	       exptype = locmode;
	       exp     = location;
	    }
	    else
	       exp = value;

	    op_ea = am_PC_index;
	 }
	 else
	 {
	    if ( relocatable(exptype) )
	       complain(71);
	    exp   = value;
	    op_ea = am_Ar_index;
	 }
	 break;

      default:
	 complain(1006);
      }
   }
}


/* OK */
int abs16addr ( addr )
long addr;
{
   if ( fwdref || ( (addr & 0x7FFF) != addr ) )
      return am_abs32;
   else
      return am_abs16;
}

/* OK */
int evalnumexp ( ptr )
struct EXPRESSNODE *ptr;
{
#ifdef DBUG
   mydebug("Evalnumexp","ptr = %X8",(long)ptr);
#endif

    if (ptr == NULL)
      return 0;
    else
    {
       int type = evalexp(ptr);
       if (absolute(type) || relocatable(type))
	  return type;
       else
	  switch ( type )
	  {
	  case s_Ar:
	  case s_Dr:
	     complain ( 145 );
	  case s_Ar_indirect:
	  case s_Ar_postincr:
	  case s_Ar_predecr:
	     complain ( 146 );
	  case s_SR:
	  case s_CCR:
	  case s_USP:
	  case s_PC:
	     complain ( 147 );
	  default:
	     complain ( 70 );
	  }
    }
}

/* OK */
int evalexp ( ptr )
struct EXPRESSNODE *ptr;
{

#ifdef DBUG
   mydebug("Evalexp","ptr = %X8 type = %N",(long)ptr,
	      ((ptr == NULL)?0L:(long)(ptr->type)) );
#endif

   if (ptr == NULL)
     return 0;

   else
   {
      int ptr0		       = ptr->type;
      struct EXPRESSNODE *ptr1 = (ptr->uvalleft).eptr;
      int  fref 	       = no;

      switch ( ptr0 & s_mask )
      {
      case s_Dr:
      case s_Ar:
	 value = checkregister(ptr);
	 return ptr0;
      case s_Ar_indirect:
      case s_Ar_postincr:
      case s_Ar_predecr:
	 value = checkregister(ptr1);
	 return ptr0;
      case s_SR:
      case s_CCR:
      case s_USP:
      case s_PC:
	 value = ptr0;
	 return ptr0;
      case s_new:
	 fwdref = yes;
	 value = 1;
	 return fwdreftype;
      case s_rel:
      case s_abs16:
      case s_abs32:
	 {
	   struct SYMNODE *ptrs = (struct SYMNODE *)ptr1;
	   fref = ((ptrs->st_flags) & stb_setnow) == 0;
	   fwdref = fwdref | fref;
	   if ( (((ptrs->st_flags) & stb_set) != 0) && fref )
	      complain(106);
	   value = ptrs->st_value;
	   return ptr0;
	 }

      case s_star:
	 value = location;
	 return locmode == s_rel ? locmode | ( sectnumber << 8 ) :
			      wordsized(value) ? s_abs16 : s_abs32;
      case s_number:
	 value = (ptr->uvalleft).longvalue;
	 return wordsized(value) ? s_abs16 : s_abs32;

      case s_monminus:
	 exptype = evalexp( ptr1 );
	 if (extrnref)
	    complain(163);

	 switch(exptype & s_mask)
	 {
	    case s_rel:
	       complain(74);
	    case s_abs16:
	    case s_abs32:
	       value = -value ; break;
	    default:
	       complain(70);
	 }

	 return wordsized(value) ? s_abs16 : s_abs32;

      case s_not:
	 exptype = evalexp(ptr1);
	 if (extrnref)
	    complain(163);

	 switch ( exptype & s_mask )
	 {
	    case s_rel:
	       complain(74);
	    case s_abs16:
	    case s_abs32:
	       value = ~value ; break;
	    default:
	       complain(70);
	 }

	 return wordsized(value) ? s_abs16 : s_abs32;

      case s_opapply:
	 {
	    int type1 = 0;
	    int type2 = 0;
	    long value1 = 0;
	    long value2 = 0;
	    int ext1 = no;
	    int ext2 = no;
	    int ext = extrnref;
	    int result = 0;

	    extrnref = no;
	    type1    = evalexp( (ptr1->uvalleft).eptr );
	    value1   = value;
	    ext1     = extrnref;

	    extrnref = no;
	    type2    = evalexp( (ptr1->uvalright).eptr );
	    value2   = value;
	    ext2     = extrnref;

            /* Don't try to do error reporting on pass1 */
            /* It will probably be an incorrect diagnosis */
            /* resulting in a 'PHASING DIFFERENCE' error on pass 2 */

	    if ( pass2 && (ext1 || ext2) )
	    {
	       int op = ptr1->type;
	       if (ext || (ext1 && ext2))
		  complain(163);
	       if (ext1)
	       {
		  if ( ! (((op == s_plus) || (op == s_minus)) &&
			   absolute(type2)) )
		     complain(163);
	       }
	       else
		  if ( ! ((op == s_plus) && absolute(type1)) )
		     complain(163);
	    }
	    extrnref = (ext | ext1) | ext2;

	    switch ( ptr1->type )
	    {
	       case s_plus:
		  value = value1 + value2; break;
	       case s_minus:
		  value = value1 - value2; break;
	       case s_times:
		  value = value1 * value2; break;
	       case s_over:
		  value = value1 / value2; break;
	       case s_logand:
		  value = value1 & value2; break;
	       case s_logor:
		  value = value1 | value2; break;
	       case s_lshift:
		  value = value1 << value2; break;
	       case s_rshift:
               /* There are problems on the IBM here because a 32 bit
                  value is assumed to be signed, resulting in an ARITHMETIC
                  rather than a LOGICAL shift right */
#ifdef IBM 
		  value = (value1 >> value2) &
                          (~((0x80000000 & value1) >> (value2-1)));
#else       
		  value = (unsigned long)value1 >> value2; 
#endif
		  break;
	       default:
		  complain(1007);
	    }

	    result = finaltype(type1, type2, ptr1->type, value);
	    return extrnref ? s_abs32 : result;
	 }

      case s_ext:
	 extrnref   = yes;
	 extrnlsymb = (struct SYMNODE *)ptr1;
	 value	    = extrnlsymb->st_value;
	 return s_abs32;

      default:
	 complain(1008);
      }
   }
}

/* OK */
int sizevalue ( sizebit )
int sizebit;
{
   switch ( sizebit )
   {
   case size_b:
      return ts_byte;
   case size_w:
      return ts_word;
   case size_l:
      return ts_long;
   default:
      return ts_none;
   }
}

/* OK */
int finaltype ( type1, type2, op, value )
int type1, type2, op;
long value;
{
   int abs1 = absolute(type1);
   int abs2 = absolute(type2);
   int rel1 = relocatable(type1);
   int rel2 = relocatable(type2);
   int AA = abs1 & abs2;
   int RR = rel1 & rel2;
   int AR = abs1 & rel2;
   int RA = rel1 & abs2;
   int ws = wordsized(value);

   switch ( op )
   {
   case s_times:
   case s_over:
   case s_logand:
   case s_logor:
   case s_lshift:
   case s_rshift:
      if ( AA )
	 return ws ? s_abs16 : s_abs32;
      else
	 complain ( 75 );
   case s_plus:
      if ( AA )
	 return ws ? s_abs16 : s_abs32;
      else
	 if ( AR )
	    return type2;
	 else
	    if ( RA )
	       return type1;
	    else
	       complain ( 76 );
   case s_minus:
      if ( AA )
	 return ws ? s_abs16 : s_abs32;
      else
	 if ( RR )
	    if ( type1 == type2 )
	       return ws ? s_abs16 : s_abs32;
	    else
	       complain ( 183 );
	 else
	    if ( RA )
	       return type1;
	    else
	       if ( AR )
		  if ( pass1 )
		     return s_abs16;
		  else
		     complain ( 76 );
	       else
		  complain ( 76 );
   default:
      complain ( 1009 );
   }
}

