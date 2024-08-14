/* ****************************************************************************** */
/* *   Assembler for the Motorola MC68000 Microprocessor:  Section 7		* */
/* *	      (c) Copyright 1985, Tenchstar Ltd., Bristol, UK.			* */
/* *										* */
/* *		    Last Modified :  11-APR-85	   (PJF)			* */
/* ****************************************************************************** */

/* M68KASM7 : Automatically translated from BCPL to C on 26-Feb-85 */


#include  "libhdr"
#include  "asmhdr"


/******************************************/
/*  External functions returning a value  */
/******************************************/

extern int digits();			  /*  asm2  */

/******************************************/
/*  Local functions returning a value	  */
/******************************************/

struct RELOCNODE    *heap4();
struct MACRONODE    *heap3ip();
struct CROSSREFNODE *heap3pp();
struct REFNODE	    *heap2i();
struct XDEFNODE     *heap2p();
struct EXPRESSNODE  *block1();
struct EXPRESSNODE  *block2i();
struct EXPRESSNODE  *block2p();
struct EXPRESSNODE  *block3ii();
struct EXPRESSNODE  *block3pp();
struct EXPRESSNODE  *expspace();
struct NEWVECNODE   *newvec1();

char *msg();
int *newvec();
int *gvec();
int readline();

/* OK */
struct RELOCNODE *heap4 ( a, b, c, d )
struct RELOCNODE *a;
long b;
int c, d;
{
   struct RELOCNODE *s =
    (struct RELOCNODE *)newvec( (sizeof(struct RELOCNODE)-1)/BYTESPERWORD );
   s->link    = a;
   s->address = b;
   s->sectno  = c;
   s->wrt     = d;
   return s;
}

/* OK */
struct MACRONODE *heap3ip ( a, b, c )
string a;
int b;
struct MACRONODE *c;
{
   struct MACRONODE *s =
    (struct MACRONODE *)newvec( (sizeof(struct MACRONODE)-1)/BYTESPERWORD );
   s->lineptr = a;
   s->length  = b;
   s->link    = c;
   return s;
}

/* OK */
struct CROSSREFNODE *heap3pp ( a, b, c )
struct SYMNODE *a;
struct CROSSREFNODE *b;
struct CROSSREFNODE *c;
{
   struct CROSSREFNODE *s =
    (struct CROSSREFNODE *)newvec( (sizeof(struct CROSSREFNODE)-1)/BYTESPERWORD );
   s->symbol = a;
   s->left   = b;
   s->right  = c;
   return s;
}

/* OK */
struct REFNODE *heap2i ( a, b )
struct REFNODE *a;
int b;
{
   struct REFNODE *s =
       (struct REFNODE *)newvec( (sizeof(struct REFNODE)-1)/BYTESPERWORD );
   s->link  = a;
   s->value = b;
   return s;
}

/* OK */
struct XDEFNODE *heap2p ( a, b )
struct XDEFNODE *a;
struct SYMNODE	*b;
{
   struct XDEFNODE *s =
      (struct XDEFNODE *)newvec( (sizeof(struct XDEFNODE)-1)/BYTESPERWORD );
   s->link   = a;
   s->symbol = b;
   return s;
}


/* OK */
complain ( code )
int code;
{
   asmerror ( code, yes );
}

/* OK */
warning ( code )
int code;
{
   warnings++;
   asmerror ( code, no );
}

/* OK */
asmerror ( code, fatal )
int code, fatal;
{
   if ( pass2 )
   {
      int i;
      int offset = errors*2;

      if ( errors == errorsize )
      {
	 selectoutput ( sysout );
	 writef ( "\071\n***  More than %N errors detected  -  Assembly aborted.\n"
	    , (long)errorsize );
	 selectoutput ( liststream );
	 aborted = yes;
	 ended = yes;
      }
      else
      {
         errorvec[ offset ] = sourcelinenumber;
         errorvec[ offset + 1 ] = code;

         linepos = 0;
         clearbuffer();

         /* Put  'file: lineno: message'  in buffer  */
         writechar( '"' );
         writestring ( current_filename->fname );
         writechar( '"' );
         writestring ( "\007, line " );
         writenumber ( sourcelinenumber, digits(sourcelinenumber) );
         writestring ( "\002: " );
         writestring ( fatal ? "\006Error " : "\010Warning " );
         write0 ( code, digits(code) );
         writestring ( "\002: " );
         writestring ( msg(code) );

         /* Now output the message to the screen */
         selectoutput ( verstream );

         /* First the line causing the error */
         /* (unless it's warning of a missing 'END' statement */
         if ( code != 4 )
         {
            writes ( "\007***    " );
            for ( i = 0 ; i <= ( length - 1 ) ; i++ )
              wrch ( (int)(inputbuff[ i ]) );
            newline();
         }

         /* Now the reason for the error */
         for ( i = 0 ; i <= ( linepos - 1 ) ; i++ )
           wrch ( (int)(outbuff[ i ]) );
         newline();
         newline();
         selectoutput ( liststream );
      }
   }

   errors++;             /* total errors/warnings in file */
   error_found = yes;    /* error found on line */

   if ( fatal )
   {
      commentline = yes;
      longjmp ( recoverlevel );
   }
}

/* OK */
error ( code )
int code;
{
   string curr_file = current_filename->fname;
   if (curr_file == NULL) curr_file = (string)"\000";
   selectoutput ( sysout );
   writef ( "\060\n\"%S\", Pass %N, line %N.  Fatal error %N  -  %S\n",
         (long)(curr_file),
         pass1 ? 1L : pass2 ? 2L : 0L,
         (long)sourcelinenumber, (long)code, (long)msg(code) );
   aborted = yes;
   ended = yes;
   longjmp ( fatalerrorp );
}

/* OK */
checkfor ( symbol, msgnumber )
int symbol, msgnumber;
{
   if ( symb == symbol )
      readsymb (  );
   else
      complain ( msgnumber );
}

/* OK */
char *msg ( code )
int code;
{
   switch ( code )
   {
   case 1:
      return "\042Size specifier is illegal on Label";
   case 2:
      return "\035Garbage found in Opcode field";
   case 3:
      return "\024Illegal Opcode field";
   case 4:
      return "\027\'END\' statement missing";
   case 5:
      return "\034Garbage found in Label field";
   case 6:
      return "\046Illegal size specifier for this Opcode";
   case 7:
      return "\045Illegal address mode for this Operand";
   case 8:
      return "\046Illegal address mode for First Operand";
   case 9:
      return "\047Illegal address mode for Second Operand";
   case 10:
      return "\040\',\' expected after First Operand";
   case 11:
      return "\041Too many operands for this Opcode";
   case 12:
      return "\041Garbage found after Operand field";
   case 13:
      return "\044First Operand must be of \'DATA\' mode";
   case 14:
      return "\055First Operand must be of \'DATA REGISTER\' mode";
   case 15:
      return "\061Second Operand must be of \'MEMORY ALTERABLE\' mode";
   case 16:
      return "\050Illegal address mode for Relative Branch";
   case 17:
      return "\043Label must be of \'RELOCATABLE\' type";
   case 18:
      return "\040Label must be of \'ABSOLUTE\' type";
   case 19:
      return "\054Short Branch displacement of ZERO is illegal";
   case 20:
      return "\046Location out of range for Short Branch";
   case 21:
      return "\045Location out of range for Long Branch";
   case 22:
      return "\056Second Operand must be of \'DATA REGISTER\' mode";
   case 23:
      return "\057Second Operand must be of \'DATA ALTERABLE\' mode";
   case 24:
      return "\055Illegal register combination for \'EXG\' Opcode";
   case 25:
      return "\043Operand must be of \'IMMEDIATE\' mode";
   case 26:
      return "\065Immediate value out of range of 4-Bit Unsigned Number";
   case 27:
      return "\051First Operand must be of \'IMMEDIATE\' mode";
   case 28:
      return "\060Inconsistent Size Specifier for \'SR/CCR\' Operand";
   case 29:
      return "\067\'.B\' size specifier illegal for \'ADDRESS REGISTER\' mode";
   case 30:
      return "\061Second Operand must be of \'ADDRESS REGISTER\' mode";
   case 31:
      return "\077First Operand must be of \'ADDRESS REGISTER POST INCREMENT\' mode";
   case 32:
      return "\100Second Operand must be of \'ADDRESS REGISTER POST INCREMENT\' mode";
   case 33:
      return "\045Second Operand must be of \'DATA\' mode";
   case 34:
      return "\064Illegal Size Specifier for \'SR\' Operand (W expected)";
   case 35:
      return "\065Illegal Size Specifier for \'USP\' Operand (L expected)";
   case 36:
      return "\060First Operand must be of \'ADDRESS REGISTER\' mode";
   case 37:
      return "\060\'SHORT\' Size Specifier illegal for \'MOVE\' Opcode";
   case 38:
      return "\073Illegal Size Specifier for \'MOVEM\' Opcode (W or L expected)";
   case 39:
      return "\061Register Range must be for Registers of same type";
   case 40:
      return "\065Size Specifier on Register is illegal in \'MOVEM\' mask";
   case 41:
      return "\055Register missing or malformed in \'MOVEM\' mask";
   case 42:
      return "\072Second Operand must be of \'ADDRESS REGISTER + OFFSET\' mode";
   case 43:
      return "\071First Operand must be of \'ADDRESS REGISTER + OFFSET\' mode";
   case 44:
      return "\073Illegal Size Specifier for \'MOVEP\' Opcode (W or L expected)";
   case 45:
      return "\071\'QUICK\' Operand value out of range of 8-Bit Signed Number";
   case 46:
      return "\074Illegal Size Specifier for \'ORG\' directive (W or L expected)";
   case 47:
      return "\035Garbage found after Directive";
   case 48:
      return "\051Directive requires \'NUMERIC\' type Operand";
   case 49:
      return "\047\'TTL\' string longer than 132 characters";
   case 51:
      return "\065INTERNAL ERROR:  Phasing Difference  -  PLEASE REPORT";
   case 52:
      return "\044\'REGISTER\' is illegal in Label field";
   case 53:
      return "\047\'INSTRUCTION\' is illegal in Label field";
   case 54:
      return "\045\'DIRECTIVE\' is illegal in Label field";
   case 55:
      return "\023Illegal Label field";
   case 56:
      return "\032Malformed \'SHIFT\' Operator";
   case 57:
      return "\054Closing \'QUOTE\' missing from \'ASCII LITERAL\'";
   case 58:
      return "\050\'ASCII LITERAL\' Longer than 4 characters";
   case 59:
      return "\040Illegal character in Source File";
   case 60:
      return "\020Malformed Number";
   case 61:
      return "\040Too few Operands for this Opcode";
   case 62:
      return "\042Register or \'PC\' missing after \'(\'";
   case 63:
      return "\073First Register after \'(\' must be \'ADDRESS REGISTER\' or \'PC\'";
   case 64:
      return "\031Register missing afer \',\'";
   case 65:
      return "\032\')\' missing after Register";
   case 66:
      return "\033\')\' missing after Registers";
   case 67:
      return "\070\'-\' or \'/\' after Register only valid with \'MOVEM\' Opcode";
   case 68:
      return "\041Illegal use of \'ADDRESS REGISTER\'";
   case 69:
      return "\034Overall Parenthesis Mismatch";
   case 70:
      return "\032Syntax Error in Expression";
   case 71:
      return "\054Symbol/Expression must be of \'ABSOLUTE\' type";
   case 72:
      return "\060Index value out of range for 8-Bit Signed Number";
   case 73:
      return "\062Forward Reference must not be \'LONG ABSOLUTE\' mode";
   case 74:
      return "\047Illegal negation of \'RELOCATABLE\' value";
   case 75:
      return "\055Dyadic Operator must have \'ABSOLUTE\' Operands";
   case 76:
      return "\044Illegal Operands for Diadic Operator";
   case 77:
      return "\047Illegal termination of \'CONSTANTS LIST\'";
   case 78:
      return "\054Value out of range for 8-Bit Unsigned Number";
   case 79:
      return "\031Illegal Forward Reference";
   case 80:
      return "\042Size Specifier in illegal position";
   case 81:
      return "\051Size Specifier on \'REGISTER\' illegal here";
   case 82:
      return "\037Statement must have Label field";
   case 83:
      return "\043Statement must not have Label Field";
   case 84:
      return "\032Operand must be a Register";
   case 85:
      return "\047Invalid Operand type for this Directive";
   case 86:
      return "\062\'.S\' Size Specifier only valid on \'BRANCH\' Opcodes";
   case 87:
      return "\072Illegal Size Specifier on Index Register (W or L expected)";
   case 88:
      return "\032Displacement type mismatch";
   case 89:
      return "\061Index value out of range for 16-Bit Signed Number";
   case 90:
      return "\044Tag Symbol longer than 30 characters";
   case 91:
      return "\054Illegal Size Specifier (B W L or S expected)";
   case 92:
      return "\027Multiply Defined Symbol";
   case 93:
      return "\057Workspace Exhausted  -  Increase Workspace Size";
   case 94:
      return "\067INTERNAL ERROR:  Parse Stack Overflow  -  PLEASE REPORT";
   case 95:
      return "\037Undefined Symbol in Label Field";
   case 96:
      return "\040Undefined Symbol in Opcode Field";
   case 97:
      return "\041Undefined Symbol in Operand Field";
   case 99:
      return "\052Second Operand must be of \'ALTERABLE\' mode";
   case 100:
      return "\046Invalid parameter for \'PLEN\' directive";
   case 101:
      return "\046Invalid parameter for \'LLEN\' directive";
   case 102:
      return "\062Instruction alignment error (Must be WORD aligned)";
   case 103:
      return "\030\'ENDC\' statement missing";
   case 104:
      return "\061Illegal use of \'SET\' on a symbol defined by \'EQU\'";
   case 105:
      return "\061Illegal use of \'EQU\' on a symbol defined by \'SET\'";
   case 106:
      return "\070Forward reference must not be to symbol defined by \'SET\'";
   case 107:
      return "\033Mismatched \'ENDC\' statement";
   case 108:
      return "\026Macro nesting too deep";
   case 109:
      return "\036Bad reference to Macro Operand";
   case 110:
      return "\042Illegally nested Macro Definitions";
   case 111:
      return "\033Mismatched \'ENDM\' statement";
   case 112:
      return "\034Mismatched \'MEXIT\' statement";
   case 113:
      return "\030\'ENDM\' statement missing";
   case 114:
      return "\031Mismatched Macro Brackets";
   case 115:
      return "\054Incorrect termination of Macro Operand Field";
   case 116:
      return "\043Too many Assembler Generated Labels";
   case 117:
      return "\062Illegal use of Macro Operands outside a Macro Body";
   case 118:
      return "\033Too many operands for Macro";
   case 119:
      return "\056Illegal generation of \'END\' in Macro Expansion";
   case 120:
      return "\057Illegal generation of \'ENDM\' in Macro Expansion";
   case 121:
      return "\060Illegal generation of \'MACRO\' in Macro Expansion";
   case 122:
      return "\025User \'FAIL\' Statement";
   case 123:
      return "\054Byte value must not be of \'RELOCATABLE\' type";
   case 124:
      return "\051Illegal re-definition of \'PLEN\' parameter";
   case 125:
      return "\051Illegal re-definition of \'LLEN\' parameter";
   case 126:
      return "\052Illegal use of \'END\' within \'INCLUDE\' file";
   case 127:
      return "\063Terminating \'QUOTE\' missing from \'INCLUDE\' argument";
   case 128:
      return "\040Input not provided for \'INCLUDE\'";
   case 129:
      return "\040Malformed argument for \'INCLUDE\'";
   case 130:
      return "\032\'INCLUDE\' nesting too deep";
   case 131:
      return "\040\'ENDC\' missing in \'INCLUDE\' file";
   case 132:
      return "\040\'ENDM\' missing in \'INCLUDE\' file";
   case 133:
      return "\062Illegal generation of \'INCLUDE\' in Macro Expansion";
   case 134:
      return "\065Register after \'(\' must be \'ADDRESS REGISTER\' or \'PC\'";
   case 135:
      return "\055Illegal use of \'INSTRUCTION\' in Operand Field";
   case 136:
      return "\053Illegal use of \'DIRECTIVE\' in Operand Field";
   case 137:
      return "\054Illegal use of \'MACRO NAME\' in Operand Field";
   case 138:
      return "\044Address out of range of 24-bit Value";
   case 139:
      return "\037Illegal \'#\' found in expression";
   case 140:
      return "\046Illegal \'OPERATOR\' found in expression";
   case 141:
      return "\034Unexpected end of expression";
   case 142:
      return "\037Illegal \')\' found in expression";
   case 143:
      return "\037Illegal \',\' found in expression";
   case 144:
      return "\037Illegal \':\' found in expression";
   case 145:
      return "\046Illegal \'REGISTER\' found in expression";
   case 146:
      return "\056Illegal \'(An)/-(An)/(An)+\' found in expression";
   case 147:
      return "\053Illegal \'SR/CCR/USP/PC\' found in expression";
   case 148:
      return "\067Illegal forward reference to register defined by \'EQUR\'";
   case 149:
      return "\006BREAK.";
   case 150:
      return "\046Illegal \'CNOP\' alignment value of ZERO";
   case 151:
      return "\047Illegal forward reference to MACRO name";
   case 152:
      return "\062Illegal use of \'XREF\' symbol in BRANCH instruction";
   case 153:
      return "\037Undefined symbol in \'XDEF\' list";
   case 154:
      return "\044Illegal \'XREF\' symbol in \'XDEF\' list";
   case 155:
      return "\043Illegal symbol found in \'XDEF\' list";
   case 156:
      return "\042Illegal termination of \'XDEF\' list";
   case 157:
      return "\040\'XREF\' symbol is already defined";
   case 158:
      return "\043Illegal symbol found in \'XREF\' list";
   case 159:
      return "\042Illegal termination of \'XREF\' list";
   case 160:
      return "\047\'XREF\' symbol is illegal in Label Field";
   case 161:
      return "\036Illegal size for \'XREF\' symbol";
   case 162:
      return "\054Illegal use of \'XREF\' symbol as displacement";
   case 163:
      return "\043Illegal arithmetic on \'XREF\' symbol";
   case 164:
      return "\067Illegal use of \'XREF\' symbol as argument to \'DIRECTIVE\'";
   case 165:
      return "\031\'XDEF\' Symbol is too long";
   case 166:
      return "\031\'XREF\' Symbol is too long";
#ifndef AMI
   case 167:
      return "\053TRIPOS Object Module Error [HUNK too large]";
   case 168:
      return "\056TRIPOS Object Module Error [RELOC16 too large]";
   case 169:
      return "\056TRIPOS Object Module Error [RELOC32 too large]";
   case 170:
      return "\056TRIPOS Object Module Error [ABSHUNK too large]";
   case 171:
      return "\057TRIPOS Object Module Error [ABSREL16 too large]";
   case 172:
      return "\057TRIPOS Object Module Error [ABSREL32 too large]";
#endif
   case 173:
      return "\043Overlong input record  -  truncated";
   case 174:
      return "\052Illegal forward reference to \'XREF\' symbol";
   case 175:
      return "\044Illegal value for WORD sized operand";
   case 176:
      return "\044Illegal value for BYTE sized operand";
   case 177:
      return "\041\'RELOCATABLE\' symbol out of range";
   case 178:
      return "\047Illegal negative argument for Directive";
   case 179:
      return "\053Illegal Forward Reference in First Argument";
   case 180:
      return "\063First argument for \'DCB\' must be of \'ABSOLUTE\' type";
   case 181:
      return "\051First argument for \'DCB\' must be positive";
   case 182:
      return "\074Illegal use of symbol from different section as displacement";
   case 183:
      return "\067Illegal arithmetic with symbols from different sections";
   case 184:
      return "\044Illegal \'XREF\' symbol in \'BSS\' block";
   case 185:
      return "\047Limit of 255 sections has been exceeded";
   case 186:
      return "\057Relocation within this section is not permitted";
   case 187:
      return "\041A local label cannot be used here";
   case 188:
      return "\042Matching \" missing in SECTION name";
   case 189:
      return "\042Illegal type for SECTION directive";
   case 190:
      return "\024Local label too long";
   case 191:
      return "\040Format is IFxx \'string\',\'string\'";
   case 192:
      return "\051Only 32 bit and 16 bit relocation allowed";
   case 193:
      return "\063Illegal use of symbol from another section with Bxx";
   case 194:
      return "\072Code producing instructions not allowed in current section";
   case 195:
      return "\037Line requires 16 bit relocation";
   case 196:
      return "\050Symbol name required with this directive";
   case 197:
      return "\045Too many arguments for this directive";

   case 1000:	case 1001:   case 1002:   case 1003:
   case 1004:	case 1005:   case 1006:   case 1007:
   case 1008:	case 1009:   case 1010:   case 1011:
   case 1012:	case 1013:   case 1014:   case 1015:
   case 1016:	case 1017:
      return "\055INTERNAL ERROR IN ASSEMBLER  -  PLEASE REPORT";

   default:
      return "\067INTERNAL ERROR:  Undefined Error Code  -  PLEASE REPORT";
   }
}

/* OK */
write0 ( number, field )
int number, field;
{
   if ( field > 1 )
      write0 ( number / 10, field - 1 );
   writechar ( (char)((number % 10) + '0') );
}

/* OK */
readtag ( )
{
   int length = 0;
   do
      if ( length == tagchars )
      {
	 warning( 90 );

	 while (  ( ('A' <= ch) && (ch <= 'Z') ) ||
		  ( ('a' <= ch) && (ch <= 'z') ) ||
		  ( ('0' <= ch) && (ch <= '9') ) ||
		  ( ch == '_' )                  ||
		  ( ch == '.' )                      )
	    rch ();
	 break;
      }

      else
      {
	 tagv[ ++length ] = ch;
	 rch (	);
      }
	while (  ( ('A' <= ch) && (ch <= 'Z') ) ||
		 ( ('a' <= ch) && (ch <= 'z') ) ||
		 ( ('0' <= ch) && (ch <= '9') ) ||
		 ( ch == '_' )                  ||
		 ( ch == '.' )                         );

   if ( getlocallabel )
   {
      int i; /* FOR loop variables */
      int labnum = symbcount;
      int size = digits(labnum);
      int temp = tagchars - ( size + 1 );

      getlocallabel = no;
      if ( length > temp )
	 complain ( 190 );
      tagv[ ++length ] = '$';
      temp = length + size;

      for ( i = temp ; i >= ( length + 1 ) ; i-- )
      {
	 tagv[ i ] = ( labnum % 10 ) + '0';
	 labnum /= 10;
      }

      locallabel = yes;
      length = temp;
      rch();
   }

   tagv[ 0 ] = length;

   if ( ch == '\\' )
   {
      if ( inmacro )
	 while ( ! ( ( ch == ' ' )   ||
		     ( ch == '\t' )  ||
		     ( ch == '\n' ) )   )
	    rch ();
      else
	 complain ( 117 );

      tagsize_given = ts_none;
      return;
   }

   checksuffix ( tagv );
   lookup ( tagv );
}

/* OK */
checksuffix ( s )
string s;
{
   int strl = s[ 0 ];
   int sufl = 2;

   if ( strl > sufl )
      if ( s[ strl - 1 ] == '.' )
      {
	 int c = s[strl];

	 if ( ('a' <= c) && (c <= 'z') )
	    c -= ('a' - 'A');

	 switch ( c )
	 {
	 case 'L':
	    tagsize_given = ts_long;
	    break;
	 case 'W':
	    tagsize_given = ts_word;
	    break;
	 case 'B':
	    tagsize_given = ts_byte;
	    break;
	 case 'S':
	    tagsize_given = ts_short;
	    break;
	 default:
	    tagsize_given = ts_none;
	    return;
	 }

	 s[ 0 ] = strl - sufl;
	 return;
      }

   tagsize_given = ts_none;
}


/* OK*/
lookup ( tagvector )
string tagvector;
{
   int i;
   int hashval = 0;
   int offset = 0;
   int length = tagvector[ 0 ];

#ifdef DBUG
   if (!systemwords)
     mydebug("Lookup","tag = %S", (long)tagvector );
#endif
/*
   {
      SCBPTR safe = output();
      selectoutput(verstream);
      writef( "\012Lookup %S\n", (long)tagvector );
      selectoutput(safe);
   }
*/

   for ( i = 0 ; i <= ( length < 10 ? length : 10 ) ; i++ )
   {
      int c = tagvector[i];
      hashval = (hashval << 1) +
		    (('a' <= c) && (c <= 'z') ? (c - ('a'-'A')) : c);
   }

   hashval =  abs( hashval % tagtablesize );
   symbtype = tagtable[ hashval ];

#ifdef DBUG
   if (!systemwords)
   {
     mydebug("Lookup","Hashval = %X8", (long)hashval );
     mydebug("Lookup","Symbtype = %X8", (long)symbtype );
   }
#endif

   while ( symbtype != NULL )
   {
      string name = symbtype->st_name;

#ifdef DBUG
      if (!systemwords)
	mydebug("Lookup","Comparing with : %S", (long)name );
#endif

      if ( length == name[0] )
	 switch ( (symbtype->st_type) & st_type_mask )
	 {
	   case s_instr:
	   case s_dir:
	   case s_Dr:
	   case s_Ar:
	   case s_PC:
	   case s_SR:
	   case s_CCR:
	   case s_USP:
	    {
	       /* Uppercase just one side of the '=' */
	       /* since all system names are held in uppercase */

	       for ( i = 1 ; i <= length ; i++ )
	       {
		  register int c1 = tagvector[i];

		  if ( ('a' <= c1) && (c1 <= 'z') )
		     c1 -= ('a' - 'A');

		  if ( c1 != name[i] )
		     goto getnextsymb;
	       }
	       goto endsearchlabel;
	    }

	   default:
	    {
	       if ( UCflag )
	       {
		 /* Uppercase both sides of the '='  */
		 /* It's slow, but he asked for it ! */

		 for ( i = 1 ; i <= length ; i++ )
		 {
		   register int c1 = tagvector[i];
		   register int c2 = name[i];

		   if ( ('a' <= c1) && (c1 <= 'z') )
		      c1 -= ('a' - 'A');

		   if ( ('a' <= c2) && (c2 <= 'z') )
		      c2 -= ('a' - 'A');

		   if ( c1 != c2 )
		      goto getnextsymb;
		 }
		 goto endsearchlabel;
	       }
	       else
	       {
		 /* Simply compare the names as they appear */

		 for ( i = 1 ; i <= length ; i++ )
		    if ( tagvector[ i ] != name[ i ] )
		       goto getnextsymb;
		 goto endsearchlabel;
	       }
	    }
	 }

getnextsymb:
      symbtype = symbtype->st_link;
   }

endsearchlabel:

   if ( symbtype == NULL )
   {
      /* Create a new symbol node */
#ifdef DBUG
      if (!systemwords)
	 mydebug("Lookup","Creating new node");
#endif

      symbtype = (struct SYMNODE *)newvec( (sizeof(struct SYMNODE)-1)/BYTESPERWORD );
      symbtype->st_name   = (string)newvec( length/BYTESPERWORD );
      symbtype->st_link   = tagtable[ hashval ];
      tagtable[ hashval ] = symbtype;

      for ( i = 0 ; i <= length ; i++ )
	(symbtype->st_name)[i] = tagvector[i];

      symbtype->st_type       = s_new;
      symbtype->st_flags      = 0;
      symbtype->st_number     = 0;
      symbtype->st_value      = 0;
      symbtype->st_definition	= systemwords ? 0 : cr_undefined;
      symbtype->st_references	= NULL;
   }

   symb = (symbtype->st_type) & st_type_mask;
   regnum = symbtype->st_value_low;

   if ( symb == s_rel )
      symb = symbtype->st_type;

   if ( pass2 )
   {
      if ( xref )
	 if ( symbtype->st_definition != 0 )
	    addref ( symbtype, sourcelinenumber );
      if ( symb == s_new )
	 undefined = yes;
      if ( ((symbtype->st_flags) & stb_muldef) != 0 )
	 complain ( 92 );
   }
}

/* OK */
addref ( p, ln )
struct SYMNODE *p;
int ln;
{
   struct REFNODE *t = (struct REFNODE *)(&(p->st_references));

   if ( (symbtype->st_definition == ln) ) 
      return;

   /* Go down the list of references to find the end */

   while ( t->link != NULL )
   {
      t = t->link;   
      if ( t->value == ln )
        return;
   }

   /* Add new reference at end of the list */

   t->link = heap2i(NULL, ln);
}

/* old scheme removed 
int *newvec ( n )
int n;
{
   int *nstvecp = stvecp - (n+1);

   if ( n >= (stvecp-stvec) )
   {
      selectoutput ( sysout );
      writef ( "\040*** Failed to allocate %N words\n", (long)n );
      writef ( "\007*** %S\n", n < 10000 ? "\027Increase workspace size"
	  : "\030Bad ORG or DS directive?" );
      selectoutput ( liststream );
      error ( 93 );
   }

   stvecp = nstvecp;

   return stvecp;
}
*/

int *gvec ( n )
int n;
{
    return newvec(n);
}

/* OK */
skiprest ( )
{
   charpos = MAXINT;
}

/* OK */
skiplayout ( )
{
   while ( ( ch == ' ' ) || ( ch == '\t' ) )
      rch (  );
}

/* OK */
rch ( )
{
   if ( charpos > length )
      nextline (  );
   ch = ended ? ENDSTREAMCH : inputbuff[ charpos ];
   charpos++;
}


nextline ( )
{
   charpos = 0;
   length = readline();
   ended = length == ENDSTREAMCH;
}


int readline ( )
{
#ifdef SUN
   register int i;
   register int c; 
   register int linelength = 0;
   register int length = 0;
   register char *my_inputbuff = inputbuff;
   register char tempbuff[ maxllen+1 ];
#else
   int i;
   int c;
   int linelength = 0;
   int length = 0;
   char *my_inputbuff = inputbuff;
   char tempbuff[ maxllen+1 ];
#endif

   int line_too_long = false;

   sourcelinenumber++;

   c = getc(sourcestream);

   if ( c == EOF ) return ENDSTREAMCH;

   while ( ! (( c == '\n') || (c == EOF)) )
   {
      if ( length == maxllen )
      {
	 warning ( 173 );
         line_too_long = true;
	 while ( ! ((c == '\n') || (c == EOF)) )
	    c = getc(sourcestream);
	 break;
      }

      my_inputbuff[ length++ ] = c;
      c = getc(sourcestream);
   }

   for ( i = 0 ; i <= ( maxllen - 1 ) ; i++ )
      tempbuff[ i ] = ' ';

   for ( i = 0 ; i <= ( length - 1 ) ; i++ )
   {
      int ch = my_inputbuff[ i ];
      if ( ch == '\t' )
	 linelength = ((linelength + tabspace)/tabspace)*tabspace;
      else
	 tempbuff[ linelength++ ] = ch;

      if (linelength >= maxllen)
      {
	if (!line_too_long) warning( 173 );
        linelength = maxllen;
	break;
      }
   }

   for ( i = 0 ; i <= ( linelength - 1 ) ; i++ )
      my_inputbuff[ i ] = tempbuff[ i ];

   for ( i = linelength - 1 ; i >= 0 ; i-- )
      if ( my_inputbuff[ i ] == ' ' )
	 linelength--;
      else
	 break;

   my_inputbuff[ linelength ] = '\n';

   return linelength;
}

/* OK */
declare ( w )
string w;
{
   register int i = 1;
   register int length = 0;
   register VAR16BIT *dv = datavector;
   struct SYMNODE *st;

   do
   {
      int ch = w[ i++ ];

      if ( ch == '/' )
      {
	 register temp = dataptr;

	 if ( length == 0 )
	    return;

	 tagv[ 0 ] = length;
	 lookup ( tagv );

	 st = symbtype;
	 (st->uval).st_template   = dv[ temp++ ];
	 st->st_type		  = dv[ temp++ ];
	 st->st_type		 += dv[ temp++ ] << 4;
	 st->st_flags		  = stb_setnow;
	 st->st_value_high	  = dv[ temp++ ];
	 st->st_value_low	  = dv[ temp++ ];

	 dataptr = temp;

	 length = 0;
      }

      else
      {
	 tagv[ ++length ] = ch;
      }

   } while ( true );
}

/* OK */
struct EXPRESSNODE *expspace()
{
   expvecp -= (sizeof(struct EXPRESSNODE)-1)/BYTESPERWORD + 1;
   if ( expvecp < expvec )
      error ( 94 );
   return (struct EXPRESSNODE *)expvecp;
}

/* OK */
struct EXPRESSNODE *block1 ( a )
int a;
{
   struct EXPRESSNODE *space = expspace();
   space->type = a;
   return space;
}

struct EXPRESSNODE *block2i ( a, b )
int a, b;
{
   struct EXPRESSNODE *space = expspace();
   space->type = a;
   (space->uvalleft).intvalue = b;
   return space;
}

struct EXPRESSNODE *block2p ( a, b )
int a;
struct EXPRESSNODE *b;
{
   struct EXPRESSNODE *space = expspace();
   space->type = a;
   (space->uvalleft).eptr = b;
   return space;
}


struct EXPRESSNODE *block3ii ( a, b, c )
int a, b, c;
{
   struct EXPRESSNODE *space = expspace();
   space->type = a;
   (space->uvalleft).intvalue = b;
   (space->uvalright).value   = c;
   return space;
}


struct EXPRESSNODE *block3pp ( a, b, c )
int a;
struct EXPRESSNODE *b, *c;
{
   struct EXPRESSNODE *space = expspace();
   space->type = a;
   (space->uvalleft).eptr  = b;
   (space->uvalright).eptr = c;
   return space;
}


/* Memory management scheme for the assembler */

int *newvec ( n )
int n;
{
   struct NEWVECNODE *list;

   n += 1;                  /* we have to give n+1 words back */

   if  ( n > newvecmax )       /* if a single request exceeds size of a block */
      return (int*)newvec1(n);     /* handle it separately. */

   list = stvec;                    /* Point to start of list */

   while ( list != NULL )           /* For each block in the list .. */
   {
      int len    = list->length;    /* find amount used so far */
      int newlen = len + n;          

      if ( newlen > newveclimit )   /* can we accommodate the request? */
      {
         list = list->link;         /* no, so go to next block */
         continue;
      }

      list->length = newlen;        /* yes, so update amount used */

/*      printf("return1 %ld\n", (int*)((long)list + len*BYTESPERWORD) );
  */
      return (int*)((long)list + len*BYTESPERWORD);
   }


   /* If we get as far as here it means we have to create a new block */

   list = (struct NEWVECNODE *)getvec(newvecsize);
   if ( list == NULL )
      newvecerr ( n );

   /* If the request that failed was for less than 10 words, say */
   /* it means stvec points to a lot of 'dead wood'. Make stvecp */
   /* point to this and start 'stvec' off afresh pointing to a new */
   /* one element list. */

   if ( n < 20 )
   {
      struct NEWVECNODE *liste = (struct NEWVECNODE *)stvec;

      /* Get last block in 'stvec' list */

      while ( liste->link != NULL )
         liste = liste->link;

      /* tag a block pointed to by 'stvecp' on to the end */
      /* of those pointed to by 'stvec' */

      liste->link = stvecp;
      stvecp = stvec;

      /* Set up a new list for 'stvec' starting with this new block */
      stvec  = list;
      list->link = NULL;
   }
   else   /* put the new block at the top of the list */
   {
      list->link = stvec;
      stvec = list;
   }

   list->length = n + NEWNODEPTRSIZE;

/*   printf("return2 %ld %ld\n", list,
      (int*)((long)list + NEWNODEPTRSIZE*BYTESPERWORD) );
*/  
   return (int*)((long)list + (NEWNODEPTRSIZE*BYTESPERWORD));
}


/* Get the space via a call to 'getvec' and link it in to */
/* the list we have so far. This routine is only called when */
/* a request for space larger than the standard block size arises. */
/* 'stvecp' is a pointer to all the extra blocks */

struct NEWVECNODE *newvec1 ( n )
int n;
{
   struct NEWVECNODE *list = (struct NEWVECNODE *)getvec(n+NEWNODEPTRSIZE);
   list->link = stvecp;
   list->length = n + NEWNODEPTRSIZE;
   stvecp = list;

/* printf("newvec1(%d): return %ld\n", n,
       (struct NEWVECNODE *)((long)list + (NEWNODEPTRSIZE*BYTESPERWORD)));
*/  
   return (struct NEWVECNODE *)((long)list + (NEWNODEPTRSIZE*BYTESPERWORD));
}


/* Go through chain of 'NEWVECed items' and free them all */
/* 'stvec' points to one linked list, 'stvecp' to the other. */

long freenewvec ( )
{
   int i; 
   long count = 0;

   for ( i = 1 ; i <= 2 ; i++ )
   {
      while ( stvec != NULL )
      {
         struct NEWVECNODE *temp = stvec;
         count += stvec->length;
         stvec = stvec->link;

         freevec ( temp );
      }

      stvec = stvecp;
   }

   return count*BYTESPERWORD;
}


/* Give error and wind up id getvec fails */

newvecerr ( n )
int n;
{
   selectoutput ( sysout );
   writef ( "\052\nRun out of space. %n longwords requested\n", (long)n );
   if ( (n > 10000) || (n < 0) )
      writes ( "\032Bad RORG or DS directive?\n" );
   selectoutput ( liststream );
   error ( 93 );
}

