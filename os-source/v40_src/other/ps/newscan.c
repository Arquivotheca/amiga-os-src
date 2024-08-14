/************************************************************************

  Module :  Postscript "New Scanner"		© Commodore-Amiga 
            (started May 1991)

  Purpose:  This file contains the C-entry for the Scan routine. This is
            called by the interpreter every time it encounters a File-Object
            on the execution stack

  Conventions: 

  NOTES: 

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <m68881.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "errors.h"
#include "exec/types.h"
#include "pstypes.h"
#include "stream.h"
#include "memory.h"
#include "objects.h"

#include "misc.h"
#include "stack.h"
#include "gstate.h"
#include "context.h"

#include "scanner.h"
//----------------------------------------------------------------------
//------- Protos -------------------------------------------------------
//----------------------------------------------------------------------

extern error 	ps_closesquare(DPSContext );
extern error	ps_counttomark(DPSContext );
extern error	ps_packedarray(DPSContext );
extern int 		scan_add(int *,int);
extern int 		scan_addbase(int *,int,int );
extern error	GetEntireString(DPSContext,stream *,int,ps_obj *);
extern error	GetEntireHexString(DPSContext,stream *,ps_obj *);
extern int		HandleBinaryTokens(DPSContext,stream *,int);
extern char 	*AddName(DPSContext,char *);
extern error 	ps_exch(DPSContext );
extern error 	ps_pop(DPSContext );
extern ps_obj 	*NameLookUp(DPSContext,ps_obj *);	
extern ps_obj	*FindDictEntry(DPSContext,ps_obj *,ps_obj *);

extern char *errnames[];
int  Scan(DPSContext ,stream *,int);
error 	ImmedError(DPSContext,int);
error	SyntaxError(DPSContext,ps_obj *);

//----------------------------------------------------------------------

#define RADIX_NUM	0x80
#define MAN_SIGN	0x40
#define DIGIT_A		0x20
#define DOT			0x10
#define DIGIT_B		0x08
#define EXP			0x04
#define EXP_SIGN	0x02
#define EXP_DIGIT	0x01

//----------------------------------------------------------------------
ps_obj paren_obj = { (TYPE_STRING | ATTR_LITERAL | ACCESS_UNLIMITED),0,1,(int)")" };
ps_obj angle_obj = { (TYPE_STRING | ATTR_LITERAL | ACCESS_UNLIMITED),0,1,(int)">" };
ps_obj squig_obj = { (TYPE_STRING | ATTR_LITERAL | ACCESS_UNLIMITED),0,1,(int)"}" };
ps_obj eof_obj = { (TYPE_STRING | ATTR_LITERAL | ACCESS_UNLIMITED),0,1,(int)"@EOF@" };



extern char FLAGS[256];

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// The scanner has now been polymorphed into a state machine. 
// It initially checks to see if the first character is  : '(','{','}','<','/',
// in which case it can determine what to do with the rest of the token.
//
// The basic strategy of the scanner is to try and convert the incoming 
// characters into an integer. If the integer overflows or a decimal point
// or 'e|E' appears it will then promote the token into a double and 
// continue to build the number in that format.
//
// There is a 'catch all' state which converts the above to an executeable name
// if the current ASCII character is not part of any number format.
//
// Literal Name      : '/'+ANY
// Number Format     : [+/-]DIGITS[.]DIGITS[e/E][+/-]DIGITS :
// Executeable Name  : Anything which can't be described by the above formats
//----------------------------------------------------------------------

int Scan(DPSContext ctxt, stream *ip, int flag) {

short int c;
int curly=0,err,i_number,quit,state,format,sign,exp_sign,base;
int immed_lookup=FALSE;
double factor,exponent,d_number;
ps_obj result;
ps_obj *dummy;
	
char token[256];
int count=0;

	result.len = 0;
	result.tag = 0;

	// Do this block at least once and then check for to see if we
	// are inside a procedure def'.

	do {

		// Reset all variables every time we enter the State-Machine....

		count = i_number = format = 0; state = S_START;
		quit = FALSE;
		sign = exp_sign =1;
		factor = 10.0;
		exponent = d_number = 0.0;
		result.type = -1;
		immed_lookup = FALSE;

		while(quit==FALSE) {
			c = sgetc(ip);
			token[count++] = (char)c;
			switch(state) {
	
			case S_START:
				if(is_bintkn(c)) {
					err = HandleBinaryTokens(ctxt,ip,c);
					dummy = OPERSP(ctxt);
					i_number = OBJ_TYPE(dummy);
					if(err!=CONTINUE) return err;
					quit=TRUE; result.type = NULL;
				} else if(isspace(c)) {
					count--;
				} else if(isdigit(c)) {
					state = S_INTEGER_TOKEN; format |= DIGIT_A;
					i_number +=(c-'0'); 
				} else if(c=='+'||c=='-') {
					state = S_PLUS_MINUS; format |=MAN_SIGN;
					if(c=='-') sign = 0;
				} else if(c=='.') {
					state = S_INITIAL_DOT; format |= DOT;
					factor = 10.0;
				} else if(c=='/') {
					state = S_LOOKUP_NAME;
					count--;
				} else if(c=='['||c==']') {
					quit = TRUE; result.type = TYPE_NAME|ATTR_EXECUTE;
				} else if(c=='(') {
					if((GetEntireString(ctxt,ip,flag,&result))!=CONTINUE) {
						SyntaxError(ctxt,&paren_obj);
						return SYNTAX_ERROR;
					}
					quit = TRUE;
				} else if(c=='<') {
					if((GetEntireHexString(ctxt,ip,&result))!=CONTINUE) {
						SyntaxError(ctxt,&angle_obj);
						return SYNTAX_ERROR;
					}
					quit = TRUE;
				} else if(c=='%') {
					count--;
					while(c!='\n'&&c!=EOF) c = sgetc(ip); 
					if(c==EOF) return COMPLETED;
				} else if(c=='{') {
					curly++; result.type = TYPE_MARK|ATTR_LITERAL;
					quit = TRUE;
				} else if(c=='}') {
					if(curly) {
						curly--;
						if (ctxt->space.PackingOn) {
							ps_counttomark(ctxt);
							if((err = ps_packedarray(ctxt))!=ERR_OK) {
								return err;
							}
							// Delete mark left by packedarray()
							ps_exch(ctxt);
							ps_pop(ctxt);
						} else {
							if((err = ps_closesquare(ctxt))!=ERR_OK) {
								return err;
							}
						}
						(OPERSP(ctxt))->type |= ATTR_EXECUTE;
					} else {
						SyntaxError(ctxt,&squig_obj);
						return SYNTAX_ERROR;
					}
					quit = TRUE;
				} else if(c==EOF) {
					if(curly) {
						SyntaxError(ctxt,&eof_obj);
						return SYNTAX_ERROR;
					} else {
						return COMPLETED;
					}
				} else if(c==')') {
					SyntaxError(ctxt,&paren_obj);
					return SYNTAX_ERROR;
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// +/- first character
	
			case S_PLUS_MINUS:
				if(isdigit(c)) {
					state = S_INTEGER_TOKEN; format |= DIGIT_A;
					i_number *=10;
					i_number += (c-'0');
				} else if(c=='.') {
					state = S_MIDWAY_DOT; format |= DOT;
					factor = 10.0;
				} else if(is_delim(c)) {
					sungetc(ip,c); result.type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// decimal point first character

			case S_INITIAL_DOT:
				if(isdigit(c)) {
					state = S_DOUBLE_TOKEN; format |= DIGIT_B;
					result.type = ATTR_LITERAL|TYPE_REAL;
					d_number += ((c-'0')/10.0); factor = 100.0;
				} else if(is_delim(c)) {
					sungetc(ip,c); result.type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else if(is_space(c)) {
					result.type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else {
					state = S_EXEC_NAME;
				}
				break;

			//  Reading ASCII digits into an integer			
			case S_INTEGER_TOKEN:
				if(isdigit(c)) {
					if(scan_add(&i_number,(int)(c-'0'))) {
						state = S_INT_OVERFLOW; d_number =(double)(i_number)*10.0+(double)(c-'0');
					}
				} else if(c=='E'||c=='e') {
					d_number = (double)i_number; state = S_EXPONENT_E;format |= EXP;
					factor = 10.0;
				} else if(c=='.') {
					d_number = (double)i_number; factor = 10.0;
					state = S_MIDWAY_DOT;format |= DOT;
				} else if(is_space(c)) {
					result.type = ATTR_LITERAL|TYPE_INT; count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(ip,c); result.type = ATTR_LITERAL|TYPE_INT;
					count--; quit = TRUE;
				} else if(c=='#'&&!(format&MAN_SIGN)&&
						(i_number>=2&&i_number<=36)) {
					state = S_RADIX_TOKEN;base = i_number;i_number=0;
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// We may have a number in radix notation
			case S_RADIX_TOKEN :
				if(isalnum(c)) {
					format |= RADIX_NUM;
					c = toupper(c);
					c -='0';
					if(c>9) {
						c -= 7;
					}
					if(c>base-1) {
						state = S_EXEC_NAME; break;
					}
					if(scan_addbase(&i_number,c,base)) {
						state = S_EXEC_NAME;
					}
				} else if(is_space(c)) {
					if(format&RADIX_NUM) {
						result.type = ATTR_LITERAL|TYPE_INT; quit = TRUE;
					} else {
						result.type = ATTR_EXECUTE|TYPE_NAME; count--; quit = TRUE;
					}
				} else if(is_delim(c)) {
					if(format&RADIX_NUM) {
						result.type = ATTR_LITERAL|TYPE_INT; quit = TRUE;
					} else {
						result.type = ATTR_EXECUTE|TYPE_NAME; count--; quit = TRUE;
					}
					sungetc(ip,c);
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// The above integer has overflowed so we have converted it into a double
			case S_INT_OVERFLOW:
				if(isdigit(c)) {
					d_number *=10; d_number +=(double)(c-'0'); 
				} else if(c=='E'||c=='e') {
					state = S_EXPONENT_E;format |= EXP;
					factor = 10.0;
				} else if(c=='.') {
					factor = 10.0; state = S_MIDWAY_DOT;format |= DOT;
				} else if(is_space(c)) {
					result.type = ATTR_LITERAL|TYPE_REAL; count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(ip,c); result.type = ATTR_LITERAL|TYPE_REAL;
					count--; quit = TRUE;
				} else {
					state = S_EXEC_NAME;
				}
				break;
	
			//  We have "seen" a decimal point so if the next ASCII digit represents
			//  a number (0-9) then we have a double and proceed to state 40.
			case S_MIDWAY_DOT:
				if(isdigit(c)) {
					state = S_DOUBLE_TOKEN; format |= DIGIT_B;
					d_number +=(double)(c-'0')/factor; factor*=10;
				} else if(c=='E'||c=='e') {
					state = S_EXPONENT_E; factor = 10.0; format |=EXP;
				} else if(is_space(c)) {
					if(format&DIGIT_A) {
						result.type = ATTR_LITERAL|TYPE_REAL;
					} else {
						result.type = ATTR_EXECUTE|TYPE_NAME;
					}
					count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(ip,c);
					if(format&DIGIT_A) {
						result.type = ATTR_LITERAL|TYPE_REAL;
					} else {
						result.type = ATTR_EXECUTE|TYPE_NAME;
					}
					quit = TRUE;
				} else { 
					state = S_EXEC_NAME;
				}
				break;
	
			// In this state we are reading in the decimal fraction of a number
			case S_DOUBLE_TOKEN:
				if(isdigit(c)) {
					d_number +=(double)(c-'0')/factor; factor*=10;
				} else if(c=='E'||c=='e') {
					state = S_EXPONENT_E; format |= EXP; factor = 10.0;
				} else if(is_space(c)) {
					result.type = ATTR_LITERAL|TYPE_REAL;
					count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(ip,c); count--; quit = TRUE;
					result.type = ATTR_LITERAL|TYPE_REAL;
				} else { 
					state = S_EXEC_NAME;
				}
				break;

			// In this state we have just had an "e|E" indicating a possible exponent
			// format eg 2134.324e10

			case S_EXPONENT_E:
				if(isdigit(c)) {
					state = S_EXPONENT_VALUE; format |= EXP_DIGIT; 
					exponent = (double)(c-'0');
				} else if(c=='+'||c=='-') {
					state = S_EXPONENT_SIGN; format |= EXP_SIGN;
					if(c=='-') exp_sign = 0;
				} else if(is_space(c)) {
					result.type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else if(is_delim(c)) {
					result.type = ATTR_EXECUTE|TYPE_NAME;
					quit = TRUE; count--; sungetc(ip,c);
				} else {
					state = S_EXEC_NAME;
				}
				break;
	
			// The exponent contains a sign character
			case S_EXPONENT_SIGN:
				if(isdigit(c)) {
					state = S_EXPONENT_VALUE;format |= EXP_DIGIT; 
					exponent = (double)(c-'0');
				} else if(is_space(c)) {
					result.type = ATTR_EXECUTE|TYPE_NAME;
					quit = TRUE; count--;
				} else if(is_delim(c)) {
					result.type = ATTR_EXECUTE|TYPE_NAME;
					quit = TRUE; count--; sungetc(ip,c);
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// In this state we read in digits which make up the exponent
			case S_EXPONENT_VALUE:
				if(isdigit(c)) {
					exponent *= factor; exponent += (double)(c-'0');
				} else if(is_space(c)) {
					result.type = ATTR_LITERAL|TYPE_REAL;
					quit = TRUE; count--;
				} else if(is_delim(c)) {
					result.type = ATTR_LITERAL|TYPE_REAL;
					quit = TRUE; count--; sungetc(ip,c);
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// The first character was a '/', if the second character is a '/'
			// '/' then we have to do Immediate name lookup.
			case S_LOOKUP_NAME:
				if(c=='/') {
					count--; state = S_IMMED_LOOKUP;
				} else if(is_delim(c)) {
					count--; sungetc(ip,c);
					quit = TRUE; result.type = ATTR_LITERAL|TYPE_NAME;
				} else if(is_space(c)) {
					count--;
					quit = TRUE; result.type = ATTR_LITERAL|TYPE_NAME;
				} else {
					state = S_LITERAL_NAME;
				}
				break;

			case S_IMMED_LOOKUP :
				if(is_space(c)) {
					// Delete the counter by one to wipe out the white space character !
					count--; quit = TRUE; result.type = ATTR_EXECUTE|TYPE_NAME;
					immed_lookup = TRUE;
				} else if(is_delim(c)) {
					count--; sungetc(ip,c); quit = TRUE; 
					result.type = ATTR_EXECUTE|TYPE_NAME;
					immed_lookup = TRUE;
				}
				break;
	
			// This is a literal name. The first character MUST have been a '/'.
			case S_LITERAL_NAME: 
				if(is_space(c)) {
					// Delete the counter by one to wipe out the white space character !
					count--; quit = TRUE; result.type = ATTR_LITERAL|TYPE_NAME;
				} else if(is_delim(c)) {
					count--; sungetc(ip,c);
					quit = TRUE; result.type = ATTR_LITERAL|TYPE_NAME;
				}
				break;
			
			// Anything which the state machine can't recognise as being part of
			// a number ends up looping here as this is the domain of the....
			// EXECUTEABLE NAME.
			case S_EXEC_NAME:
				if(is_space(c)) {
					count--;
					quit = TRUE; result.type = ATTR_EXECUTE|TYPE_NAME;
				} else if(is_delim(c)) {
					sungetc(ip,c); count--;
					quit = TRUE; result.type = ATTR_EXECUTE|TYPE_NAME;
				}
				break;
	
			default : break;
			}
		}

		// Depending on what type the object (if any), do something with it then
		// stick it on the stack.

		switch(OBJ_TYPE(&result)) {
		case TYPE_REAL :

			// If there was a sign character then adjust the double
			if(!sign) d_number *= -1.0;
			if(exponent!=0.0) {
	
			// Doubles have to be adjusted according to the exponent if it's been used
				if(exp_sign) {
					d_number *= pow(10.0,exponent);
				} else {
					d_number /= pow(10.0,exponent);
				}
			} 
			result.obj.realval = (float)d_number;
			PUSHOPER(ctxt,&result);
			break;

		case TYPE_INT :

			// If there was a sign character then adjust the integer
			if(!sign) i_number *=-1.0;
			result.obj.intval = i_number;
			PUSHOPER(ctxt,&result);
			break;

		case TYPE_NAME :

			if(count) { 
				//if((result.obj.nameval = (char *)AllocVM(VM,sizeof(char)*count+1))==NULL) {
				//	return ALLOC_FAILED;
				//}
				token[count] = '\0';
				result.obj.nameval = AddName(ctxt,token);
			} else {
				result.obj.nameval = NULL;
			}
			if(!immed_lookup) {
				PUSHOPER(ctxt,&result);
			} else {
				if((dummy = NameLookUp(ctxt,&result))==NULL) {
					PUSHOPER(ctxt,&result);
					ImmedError(ctxt,ERR_undefined);
					return IMMED_ERROR;
				} else {
					PUSHOPER(ctxt,dummy);
				}
			}
			break;

			case TYPE_MARK : case TYPE_STRING :
				PUSHOPER(ctxt,&result);
				break;

			default :
				break;
			}
	} while (curly!=0) ;
	return CONTINUE;
}
error ImmedError(DPSContext ctxt,int number) {
	ps_obj error_object,*value;

	error_object.obj.nameval = AddName(ctxt, errnames[number-1]);
	error_object.type = TYPE_NAME;
	error_object.len = 0;
	error_object.tag = 0;
	value = FindDictEntry(ctxt,&ctxt->space.errdict,&error_object);
	if (value) {
		PUSHOPER(ctxt,value);
	}
	return ERR_OK;
}
error	SyntaxError(DPSContext ctxt,ps_obj *obj){

	PUSHOPER(ctxt,obj);
	return ERR_OK;
}
