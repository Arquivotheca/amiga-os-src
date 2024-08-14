/************************************************************************

  Module :	Postscript "Type, Attribute & Conversion"   © Commodore-Amiga
			Written by L. Vanhelsuwe (started March 1991)

  Purpose:	This file contains C-entries for operators called directly
			by the interpreter using the Postscript execution model.

  Conventions: -The order in which functions appear is identical to the one
				in the Adobe Red Book (Chapter 6 Operator Summary).
		       -Variables called 'tos' and 'nos' point to the Top Of Stack
				and Next Of Stack elements resp. (on Operand stack).

*************************************************************************/

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

#include <stdio.h>
#include <ctype.h>
#include <m68881.h>
#include <math.h>
#include <limits.h>

//--------------------------------------------------------------------
char *typenames[] = {	"nulltype",				// **!! DONT MODIFY ORDER
						"integertype",			// **!! HAS TO MATCH objects.h
						"realtype",
						"nametype",
						"booleantype",
						"stringtype",
						"**!! type",
						"dicttype",
						"packedarraytype",
						"arraytype",
						"marktype",
						"filetype",
						"operatortype",
						"fonttype",
						"savetype",

						"locktype",
						"gstatetype",
						"conditiontype" };

char table[] = {'0','1','2','3','4','5','6','7','8','9',
				'A','B','C','D','E','F','G','H','I','J','K','L','M',
                 'N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
//--------------------------------------------------------------------
error ps_type		(DPSContext);	// extended for DPS
error ps_cvlit		(DPSContext);
error ps_cvx		(DPSContext);
error ps_xcheck		(DPSContext);
error ps_executeonly(DPSContext);
error ps_noaccess	(DPSContext);
error ps_readonly	(DPSContext);
error ps_rcheck		(DPSContext);
error ps_wcheck		(DPSContext);
error ps_cvi		(DPSContext);
error ps_cvn		(DPSContext);
error ps_cvr		(DPSContext);
error ps_cvrs		(DPSContext);
error ps_cvs		(DPSContext);
//--------------------------------------------------------------------

IMPORT char		   *AddName		(DPSContext ctxt, char *);
IMPORT ps_obj	   *FindOperator(DPSContext ctxt, ps_obj *);
IMPORT void		   set_round_mode(void);

//--------------------------------------------------------------------
#define ERROR		0x80
#define I_SIGN		0x40
#define DIGIT_A		0x20
#define DOT			0x10
#define DIGIT_B		0x08
#define EXP			0x04
#define EXP_SIGN	0x02
#define EXP_DIGIT	0x01

//--------------------------------------------------------------------
// Prototypes for other functions..

int GetNumber(char *,double *);

/************************************************************************/
/************** Type, Attribute & Conversion Operators ******************/
/************************************************************************/
error ps_type(DPSContext ctxt) {	// any "type" | typename

	ps_obj *tos;
	char * s;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	switch (OBJ_TYPE(tos)) {
	    case TYPE_NULL		: s=UNIQUE_STRPTR(typenames[ 0]);	break;
	    case TYPE_INT		: s=UNIQUE_STRPTR(typenames[ 1]);	break;
	    case TYPE_REAL		: s=UNIQUE_STRPTR(typenames[ 2]);	break;
	    case TYPE_NAME		: s=UNIQUE_STRPTR(typenames[ 3]);	break;
	    case TYPE_BOOL		: s=UNIQUE_STRPTR(typenames[ 4]);	break;
	    case TYPE_STRING	: s=UNIQUE_STRPTR(typenames[ 5]);	break;
	    case TYPE_IMMED		: s=UNIQUE_STRPTR(typenames[ 6]);	break;
	    case TYPE_DICT		: s=UNIQUE_STRPTR(typenames[ 7]);	break;
	    case TYPE_PACKED	: s=UNIQUE_STRPTR(typenames[ 8]);	break;
	    case TYPE_ARRAY		: s=UNIQUE_STRPTR(typenames[ 9]);	break;
	    case TYPE_MARK		: s=UNIQUE_STRPTR(typenames[10]);	break;
	    case TYPE_FILE		: s=UNIQUE_STRPTR(typenames[11]);	break;
		case TYPE_OPERATOR	: s=UNIQUE_STRPTR(typenames[12]);	break;
	    case TYPE_FONTID	: s=UNIQUE_STRPTR(typenames[13]);	break;
	    case TYPE_SAVE		: s=UNIQUE_STRPTR(typenames[14]);	break;

// DPS extended types
	    case TYPE_LOCK		: s=UNIQUE_STRPTR(typenames[15]);	break;
	    case TYPE_GSTAT		: s=UNIQUE_STRPTR(typenames[16]);	break;
	    case TYPE_COND		: s=UNIQUE_STRPTR(typenames[17]);	break;

	    default: //printf ("BUG! unknown type:%d\n",OBJ_TYPE(tos));
			return ERR_typecheck; //**!! NOT LEGAL PostScript !
	}

	tos->type = TYPE_NAME | ATTR_EXECUTE;	// return executable name
	tos->len = tos->tag = 0;
	tos->obj.nameval = s;

	return ERR_OK;
}
/************************************************************************/
error ps_cvlit(DPSContext ctxt) {	// any "cvlit" |  literal_any

	ps_obj *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	tos->type &= ~ATTR_EXECUTE;		// AND field with NOT the bit to clear
	return ERR_OK;
}
/************************************************************************/
error ps_cvx(DPSContext ctxt) {		// any "cvx" |  executable_any

	ps_obj *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	tos->type |= ATTR_EXECUTE;		// OR in bit to set
	return ERR_OK;
}
/************************************************************************/
// INTERNAL COMMON FUNCTION TO noaccess, readonly, executeonly

error chg_access(DPSContext ctxt, uchar new) {

	ps_obj *tos;

	NEED_ARGS(1);				// array,dict,file or string
	tos = OPERSP(ctxt);

	switch (OBJ_TYPE(tos)) {

	    case TYPE_ARRAY:
	    case TYPE_FILE:
	    case TYPE_STRING:

			if (OBJ_AXES(tos) == ACCESS_UNLIMITED) {
				tos->type = OBJ_TYPE(tos) | new;
				return ERR_OK;
			} else return ERR_invalidaccess;

//---------------------------------------------------
		case TYPE_DICT:

			if (OBJ_AXES(tos->obj.dictval) == ACCESS_UNLIMITED) {
				tos->obj.dictval->type = OBJ_TYPE(tos) | new;
				return ERR_OK;
			} else return ERR_invalidaccess;
//---------------------------------------------------
	    default:
			return ERR_typecheck;
	}
}
/************************************************************************/
error ps_noaccess(DPSContext ctxt) {	// arr.dict.file.str "noaccess" | ...
	return chg_access(ctxt, ACCESS_NONE); }
/************************************************************************/
error ps_readonly(DPSContext ctxt) {	// arr.dict.file.str "readonly" | ...
	return chg_access(ctxt, ACCESS_READ_ONLY); }
/************************************************************************/
error ps_executeonly(DPSContext ctxt) {	// arr.dict.file.str "executeonly" | ...

	pso *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);
	if (OBJ_TYPE(tos) == TYPE_DICT) return ERR_typecheck;

	return chg_access(ctxt, ACCESS_EXEC_ONLY);
}
/************************************************************************/
error ps_rcheck(DPSContext ctxt) {	// arr.dict.file.str "rcheck" | bool

	ps_obj *tos;

	NEED_ARGS(1);			// array,dict,file or string
	tos = OPERSP(ctxt);

	switch (OBJ_TYPE(tos)) {
		case TYPE_PACKED:
	    case TYPE_ARRAY:
	    case TYPE_FILE:
	    case TYPE_STRING:
			if (OBJ_AXES(tos)==ACCESS_UNLIMITED || OBJ_AXES(tos)==ACCESS_READ_ONLY) {
				*tos = t_obj;		// copy global R/O TRUE OBJ to TOS
			} else {
				*tos = f_obj;
			}
			break;
		//-------------------------------------------------------
	    case TYPE_DICT:		// Dictionaries are special case: check VALUE !!
			if (OBJ_AXES(tos->obj.dictval)==ACCESS_UNLIMITED ||
				OBJ_AXES(tos->obj.dictval)==ACCESS_READ_ONLY) {
				*tos = t_obj;		// copy global R/O TRUE OBJ to TOS
			} else {
				*tos = f_obj;
			}
			break;					
		//-------------------------------------------------------
	    default: return ERR_typecheck;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_wcheck(DPSContext ctxt) {	// arr.dict.file.str "wcheck" | bool

	ps_obj *tos;

	NEED_ARGS(1);			// array,dict,file or string

	tos = OPERSP(ctxt);

	switch (OBJ_TYPE(tos)) {
		case TYPE_PACKED:
	    case TYPE_ARRAY:
	    case TYPE_FILE:
	    case TYPE_STRING:
			if (OBJ_AXES(tos)==ACCESS_UNLIMITED) {
			    *tos = t_obj;
			} else {
			    *tos = f_obj;
			}
			break;
		//-------------------------------------------------------
		case TYPE_DICT:
			if (OBJ_AXES(tos->obj.dictval)==ACCESS_UNLIMITED) {
			    *tos = t_obj;
			} else {
			    *tos = f_obj;
			}
			break;
		//-------------------------------------------------------
	    default: return ERR_typecheck;
	}
	return ERR_OK;
}
/************************************************************************/
error ps_xcheck(DPSContext ctxt) {	// any "xcheck" | bool

	ps_obj *tos;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if (OBJ_ATTR(tos)==ATTR_EXECUTE) {	// Dictionaries can NEVER BE Xecutable
		*tos = t_obj;
	} else {
		*tos = f_obj;
	}

	return ERR_OK;
}
/************************************************************************/
error ps_cvi(DPSContext ctxt) {		//

ps_obj *tos,result;
char *buf;
int loop,format;
double value;

	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	switch(OBJ_TYPE(tos)) {

		case TYPE_REAL:		// truncate real value to an integer
			if(abs(tos->obj.realval)>INT_MAX) return ERR_rangecheck;
			tos->type = TYPE_INT|ATTR_LITERAL;
			tos->tag  = 0;
			tos->len  = 0;
			tos->obj.intval = (int)tos->obj.realval;

		case TYPE_INT:		// already an int, nothing to do
			break;

		case TYPE_STRING:
			buf = (char *)AllocVM(VM,tos->len+1);
			for(loop=0;loop<tos->len;loop++) buf[loop] = tos->obj.stringval[loop];
			buf[loop] = '\0';
			format = GetNumber(buf,&value);
			FreeVM(VM,buf,tos->len+1);
			if(format&ERROR) return ERR_syntaxerror;
			if(abs(value)>INT_MAX) return ERR_rangecheck;

			result.obj.intval = (int)value;
			result.type = TYPE_INT|ATTR_LITERAL;
			result.tag = 0;
			result.len = 0;
			POPOPER(ctxt);			// pop the string
			PUSHOPER(ctxt,&result);	// push the result

			break;

		default:
			return ERR_typecheck;
	}
	return ERR_OK;
}

/************************************************************************/
error ps_cvn(DPSContext ctxt) {		//
	ps_obj *tos,result;
	char *buf,*name_ptr;
	int loop;
	
	NEED_ARGS(1);

	tos = OPERSP(ctxt);
	if(OBJ_TYPE(tos)==TYPE_STRING){
		buf = (char *)AllocVM(VM,tos->len+1);
		for(loop=0;loop<tos->len;loop++) {
			buf[loop] = tos->obj.stringval[loop];
		}
		buf[loop] = '\0';
		name_ptr = AddName(ctxt,buf);
		FreeVM(VM,buf,tos->len+1);
		result.type = TYPE_NAME | (tos->type&ATTR_MASK);
		result.len = 0;
		result.obj.nameval = name_ptr;
		POPOPER(ctxt);
		PUSHOPER(ctxt,&result);
	} else return ERR_typecheck;
	return ERR_OK;
}
/************************************************************************/
error ps_cvr(DPSContext ctxt) {		//
	ps_obj *tos,result;
	char *buf;
	int format,loop;
	double value;
	
	NEED_ARGS(1);
	tos = OPERSP(ctxt);

	if(OBJ_TYPE(tos)==TYPE_INT) {
		tos->type = ATTR_LITERAL|TYPE_REAL;
		tos->obj.realval = (float)tos->obj.intval;
	} else if(OBJ_TYPE(tos)==TYPE_REAL) {
// Simply return with this object on the stack
	} else if(OBJ_TYPE(tos)==TYPE_STRING) {
		buf = (char *)AllocVM(VM,tos->len+1);
		for(loop=0;loop<tos->len;loop++) {
			buf[loop] = tos->obj.stringval[loop];
		}
		buf[loop] = '\0';
		format = GetNumber(buf,&value);
		FreeVM(VM,buf,tos->len+1);
		if(format&ERROR) return ERR_syntaxerror;
		result.obj.realval = (float)value;
		result.type = TYPE_REAL|ATTR_LITERAL;
		result.tag = 0;
		result.len = 0;
		POPOPER(ctxt); PUSHOPER(ctxt,&result);
	} else return ERR_typecheck;
	return ERR_OK;
}
/************************************************************************/
error ps_cvrs(DPSContext ctxt) {	//
	ps_obj *num,*radix,*string,result;
	char buf[32];
	int i_num,i_radix,count=0,c;

	NEED_ARGS(3);
	string = OPERSP(ctxt); radix = string + 1; num = string + 2;

	if(OBJ_TYPE(string)!=TYPE_STRING||
	   OBJ_TYPE(radix)!=TYPE_INT||
	   (OBJ_TYPE(num)!=TYPE_REAL&&OBJ_TYPE(num)!=TYPE_INT) ) {
		return ERR_typecheck;
	}
	
	if(OBJ_TYPE(num)==TYPE_REAL) {
		if(num->obj.realval>(float)INT_MAX) return ERR_rangecheck;
		i_num = (int)num->obj.realval;
	} else {
		i_num = (int)num->obj.intval;
	}
	if(i_num<0) return ERR_rangecheck;
	result.obj.stringval = string->obj.stringval;
	i_radix = radix->obj.intval;

	if(i_radix>=2&&i_radix<=36) {
		if(i_num==0) {
			result.len = 1;
			result.obj.stringval[0] = '0';
		} else {
			while(i_num!=0) {
				c = i_num % i_radix; i_num /= i_radix;
				if(c>35) return ERR_rangecheck;
				buf[count++] = table[c];
				if(count>string->len) return ERR_rangecheck;
			}
			result.len = count;
			count--;c = count; 
			for(;count>=0;count--) result.obj.stringval[c-count] = buf[count];
		}
	} else return ERR_rangecheck;
	result.type = TYPE_STRING|ATTR_LITERAL;
	SAVE_LEVEL(&result);
	POPNUMOPER(ctxt,3);
	PUSHOPER(ctxt,&result);
	return ERR_OK;
}

/************************************************************************/
error ps_cvs(DPSContext ctxt) {		//
	ps_obj *any,*string,result,*obj;
	int loop,len;
	char buf[256];

	NEED_ARGS(2);
	
	string = OPERSP(ctxt);
	any = string + 1;
	if(OBJ_TYPE(string)!=TYPE_STRING) return ERR_typecheck;

	result.obj.stringval = string->obj.stringval;

	switch(OBJ_TYPE(any)) {
	case TYPE_INT: 
		len = sprintf(buf,"%d",any->obj.intval);
		break;
	case TYPE_REAL:
		len = sprintf(buf,"%f",(double)any->obj.realval);
		break;
	case TYPE_NAME:
		len = sprintf(buf,"%s",any->obj.nameval);
		break;
	case TYPE_OPERATOR:
		if((obj = FindOperator(ctxt,any))==NULL) {
			len = sprintf(buf,"Internal");
		} else {
			loop = 0;
			while((buf[loop]=obj->obj.nameval[loop])!='\0') loop++;
			len = loop;
		}
		break;
	case TYPE_BOOL:
		if(any->obj.boolval == TRUE) {
			len = sprintf(buf,"true");
		} else {
			len = sprintf(buf,"false");
		}
		break;
	case TYPE_STRING:
		if(any->len > string->len) return ERR_rangecheck;
		for(loop=0;loop<any->len;loop++) {
			result.obj.stringval[loop] = any->obj.stringval[loop];
		}
		result.type = TYPE_STRING|ATTR_LITERAL;
		result.len = loop;
		POPNUMOPER(ctxt,2);
		PUSHOPER(ctxt,&result);
		return ERR_OK;
		break;
	default :
		len = sprintf(buf,"--nostringval--");
		break;
	}
	if(len > string->len) return ERR_rangecheck;

	result.type = TYPE_STRING|ATTR_LITERAL;
	result.len = len;
	for(loop=0;loop<len;loop++) {
		result.obj.stringval[loop] = buf[loop];
	}
	
	POPNUMOPER(ctxt,2);
	PUSHOPER(ctxt,&result);

	return ERR_OK;
}
/************************************************************************/
int GetNumber(char *c,double *number) {
	int state = 0,format=0,sign=1,exp_sign=1;
	double factor=10.0,exponent=0.0;
	

	while(*c!='\0') {
		switch(state) {

		case 0:
			if(isdigit(*c)) {
				state = 30; format |= DIGIT_A;
				*number +=(double)(*c-'0'); 
			} else if(*c=='+'||*c=='-') {
				state = 10; format |=I_SIGN;
				if(*c=='-') sign = 0;
			} else if(*c=='.') {
				state = 20; format |= DOT;
				factor = 10.0;
			} else return (format|ERROR);
			break;

		case 10:
			if(isdigit(*c)) {
				state = 30; format |= DIGIT_A;
				*number *=factor;
				*number += (double)(*c-'0');
			} else if(*c=='.') {
				state = 35; format |= DOT;
				factor = 10.0;
			} else return (format|ERROR);
			break;

		case 20:
			if(isdigit(*c)) {
				state = 40; format |= DIGIT_B;
			} else return (format|ERROR);
			break;
		
		case 30:
			if(isdigit(*c)) {
				state = 30;
				*number *=factor;
				*number +=(double)(*c-'0'); 
			} else if(*c=='E'||*c=='e') {
				state = 50;format |= EXP;
				factor = 10.0;
			} else if(*c=='.') {
				factor = 10.0;
				state = 35;format |= DOT;
			} else return (format|ERROR);
			break;

		case 35:
			if(isdigit(*c)) {
				state = 40; format |= DIGIT_B;
				*number +=(double)(*c-'0')/factor; factor*=10;
			} else if(*c=='E'||*c=='e') {
				state = 50; factor = 10.0;
				format |=EXP;
			} else return (format|ERROR);
			break;

		case 40:
			if(isdigit(*c)) {
				state = 40; 
				*number +=(double)(*c-'0')/factor; factor*=10;
			} else if(*c=='E'||*c=='e') {
				state = 50; format |= EXP;
				factor = 10.0;
			} else return (format|ERROR);
			break;

		case 50:
			if(isdigit(*c)) {
				state = 70; format |= EXP_DIGIT; 
				exponent = (double)(*c-'0');
			} else if(*c=='+'||*c=='-') {
				state = 60; format |= EXP_SIGN;
				if(*c=='-') exp_sign = 0;
			} else return (format|ERROR);
			break;

		case 60:
			if(isdigit(*c)) {
				state = 70;format |= EXP_DIGIT; 
				exponent = (double)(*c-'0');
			} else return (format|ERROR);
			break;

		case 70:
			if(isdigit(*c)) {
				state = 70; 
				exponent *= factor;
				exponent += (double)(*c-'0');
			} else return (format|ERROR);
			break;

		default : break;
		}
		c++;
	}
	if(!sign) *number *= -1.0;
	if(exponent!=0.0) {
		
		if(exp_sign) {
			*number *= pow(10.0,exponent);
		} else {
			*number /= pow(10.0,exponent);
		}
	}
	return (format);
}
