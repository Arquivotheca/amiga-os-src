/************************************************************************

  Module :  Postscript "Binary Encoding Functions"    © Commodore-Amiga
            (Jan 1991)

  Purpose:  This file contains the C-entries for handling the two Binary Encoded
			format's. These functions are called by the scanner when it recieves
            a binary encoding code.

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

/**************************************************************************
 *				Proto's												      *
 **************************************************************************/

int HandleBinaryTokens(DPSContext,stream *,int);
int HandleBinaryObjects(stream *,int);
static int ReadHILO(stream *ip,char *result,char num);
static int ReadLOHI(stream *ip,char *result,char num);
static int ReadString(DPSContext ctxt,stream *ip,int bin_value,ps_obj *object);

/**************************************************************************/

extern UBYTE FLAGS[256];
DPSContext contxt;


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

int HandleBinaryTokens(DPSContext ctxt,stream *ip,int c) {

	int bin_integer=0,bin_value,loop,format;	
	UBYTE scale;
	float flt_value=0.1;
	SHORT short_int=0;
	ps_obj object,*arrayval;

	object.tag = 0;
	object.len = 0;

	contxt = ctxt;
		switch(c) {

			case BT_INT_32_HILO :
				if(ReadHILO(ip,(char *)&bin_integer,4)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_INT;
				object.obj.intval = bin_integer;
				break;

			case BT_INT_16_HILO :
				if(ReadHILO(ip,(char *)&short_int,2)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_INT;
				object.obj.intval = (int)short_int;
				break;

			case BT_INT_8 :
				if((bin_integer = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				bin_integer =(signed char)bin_integer;
				object.type = TYPE_INT;
				object.obj.intval = bin_integer;
				break;

			case BT_INT_32_LOHI :

				if(ReadLOHI(ip,(char *)&bin_integer+3,4)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_INT;
				object.obj.intval = (int)bin_integer;
				break;

			case BT_INT_16_LOHI :
				
				if(ReadLOHI(ip,(char *)&short_int+1,2)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_INT;
				object.obj.intval = (int)short_int;
				break;

			case BT_INT_FIXED_POINT :
				if((bin_integer = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				scale = (UBYTE)bin_integer;
				if(scale<127) {
					format = TRUE;
				} else {
					format = FALSE;
					scale-=128;
				}
				
				if(scale<32) {
					if(format) {
						if(ReadHILO(ip,(char *)&bin_integer,4)==EOF) return SYNTAX_ERROR;
					} else {
						if(ReadLOHI(ip,(char *)&bin_integer+3,4)==EOF) return SYNTAX_ERROR;
					}
					if(scale == 0) {
						object.type = TYPE_INT;
						object.obj.intval = bin_integer;
					} else {
						flt_value = (float)bin_integer/((float)(1<<(scale)));
						object.type = TYPE_REAL;
						object.obj.realval = flt_value;
					}
				} else if(scale<48) {
					scale -= 32;
					if(format) {
						if(ReadHILO(ip,(char *)&short_int,2)==EOF) return SYNTAX_ERROR;
					} else {
						if(ReadLOHI(ip,(char *)&short_int+1,2)==EOF) return SYNTAX_ERROR;
					}
					if(scale == 0) {
						object.type = TYPE_INT;
						object.obj.intval = (int)short_int;
					} else {
						flt_value = (float)short_int/((float)(1<<(scale)));
						object.type = TYPE_REAL;
						object.obj.realval = flt_value;
					}
				} else return SYNTAX_ERROR;

				return(CONTINUE);

			case BT_IEEE_32_HILO :
				if(ReadHILO(ip,(char *)&flt_value,4)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_REAL;
				object.obj.realval = flt_value;
				break;

			case BT_IEEE_32_LOHI :
				if(ReadLOHI(ip,(char *)&flt_value+3,4)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_REAL;
				object.obj.realval = flt_value;
				break;

			case BT_NATIVE_REAL :
				if(ReadHILO(ip,(char *)&flt_value,4)==EOF) return SYNTAX_ERROR;
				object.type = TYPE_REAL;
				object.obj.realval = flt_value;
				break;

			case BT_BOOLEAN :
				if((bin_value=sgetc(ip))==EOF) return(SYNTAX_ERROR);
				object.type = TYPE_BOOL;
				if(bin_value) {
					object.obj.boolval = TRUE;
				} else {
					object.obj.boolval = FALSE;
				}
				break;

			case BT_8_STRING :
				if((bin_value=sgetc(ip))==EOF) return(SYNTAX_ERROR);
				if(ReadString(ctxt,ip,bin_value,&object)) {
					return SYNTAX_ERROR;
				}
				break;

			case BT_16_STRING_HILO :
				if(ReadHILO(ip,(char *)&short_int,2)==EOF) return SYNTAX_ERROR;
				if(ReadString(ctxt,ip,(unsigned int)short_int,&object)) {
					return SYNTAX_ERROR;
				}
				break;

			case BT_16_STRING_LOHI :
				if(ReadLOHI(ip,(char *)&short_int+1,2)==EOF) return SYNTAX_ERROR;
				if(ReadString(ctxt,ip,(unsigned char)short_int,&object)) {
					return SYNTAX_ERROR;
				}
				break;

			case BT_LIT_SYS :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				return(CONTINUE);

			case BT_EXE_SYS :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				object.type = ATTR_EXECUTE|TYPE_NAME;
				object.obj.nameval = ctxt->space.SysNameTab[bin_value];
				break;

			case BT_LIT_USER :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				return(CONTINUE);

			case BT_EXE_USER :
				if((bin_value = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				return(CONTINUE);

			case BT_HOMO_ARRAY :
				if((bin_integer = sgetc(ip))==EOF) return(SYNTAX_ERROR);
				scale = (UBYTE) bin_integer;
				if(scale<128) {
					format = TRUE;
					if(ReadHILO(ip,(char *)&short_int,2)==EOF) return SYNTAX_ERROR;
				} else {
					scale-=128;
					format = FALSE;
					if(ReadLOHI(ip,(char *)&short_int+1,2)==EOF) return SYNTAX_ERROR;
				}
				
				if((arrayval = (ps_obj *)AllocVM(VM,sizeof(ps_obj)*short_int))==NULL) 
						return SYNTAX_ERROR;

				object.type = TYPE_ARRAY;
				object.len = short_int;
				object.obj.arrayval = (g_obj *)arrayval;

				if(scale<=31) {
					for(loop=0;loop<object.len;loop++) {
						if(format) {
							if(ReadHILO(ip,(char *)&bin_integer,4)==EOF) {
								FreeVM(VM,(void *)arrayval,object.len);
								return SYNTAX_ERROR;
							}
						} else {
							if(ReadLOHI(ip,(char *)&bin_integer+3,4)==EOF) {
								FreeVM(VM,(void *)arrayval,object.len);
								return SYNTAX_ERROR;
							}
						}
						if(scale) {
							flt_value = (float)bin_integer/(float)(1<<scale);
							arrayval->type = TYPE_REAL;
							arrayval->obj.realval = flt_value;
						} else {
							arrayval->type = TYPE_INT;
							arrayval->obj.intval = bin_integer;
						}
						arrayval++;
					}
				} else if(scale<=47) {
					scale -=32;
					for(loop=0;loop<object.len;loop++) {
						if(format) {
							if(ReadHILO(ip,(char *)&short_int,2)==EOF) {
								FreeVM(VM,(void *)arrayval,object.len);
								return SYNTAX_ERROR;
							}
						} else {
							if(ReadLOHI(ip,(char *)&bin_integer+3,4)==EOF) {
								FreeVM(VM,(void *)arrayval,object.len);
								return SYNTAX_ERROR;
							}
						}
						if(scale) {
							flt_value = (float)short_int/(float)(1<<scale);
							arrayval->type = TYPE_REAL;
							arrayval->obj.realval = flt_value;
						} else {
							arrayval->type = TYPE_INT;
							arrayval->obj.intval = short_int;
						}
						arrayval++;
					}
				} else if(scale==48||scale==49) {
					for(loop=0;loop<object.len;loop++) {
						if(format) {
							if(ReadHILO(ip,(char *)&flt_value,4)==EOF) {
								FreeVM(VM,(void *)arrayval,object.len);
								return SYNTAX_ERROR;
							}
						} else {
							if(ReadLOHI(ip,(char *)&flt_value+3,4)==EOF) {
								FreeVM(VM,(void *)arrayval,object.len);
								return SYNTAX_ERROR;
							}
						}
						flt_value = (float)bin_integer/(float)(1<<scale);
						arrayval->type = TYPE_REAL;
						arrayval->obj.realval = flt_value;
						arrayval++;
					}
				} else return(SYNTAX_ERROR);
				break;

			case BT_UNUSED1 : case BT_UNUSED2 : case BT_UNUSED3 : 
			case BT_UNUSED4 : case BT_UNUSED5 : case BT_UNUSED6 :
			case BT_UNUSED7 : case BT_UNUSED8 : case BT_RESERVED :
				return(SYNTAX_ERROR);

			default : 
				return(SYNTAX_ERROR);
		}
		PUSHOPER(ctxt,&object);
		return CONTINUE;
}

int ReadHILO(stream *ip,char *result,char num) {
	int val;
	while(num--) {
		if((val = sgetc(ip))==EOF) return EOF;
		*result++=(UBYTE)val;
	}
	return TRUE;
}

int ReadLOHI(stream *ip,char *result,char num) {
	int val;
	while(num--) {
		if((val = sgetc(ip))==EOF) return EOF;
		*result--=(UBYTE)val;
	}
	return TRUE;
}

int ReadString(DPSContext ctxt,stream *ip,int bin_value,ps_obj *object) {
	char *string_ptr;
	if(bin_value) {
		if((string_ptr = AllocVM(VM,bin_value))==NULL) {
			return TRUE;
		}
		if(sgets(ip,string_ptr,bin_value)!=bin_value) {
			FreeVM(VM,(void *)string_ptr,bin_value);
			return TRUE;
		}
		object->obj.stringval = string_ptr;
	} else {
		object->obj.stringval = NULL;
	} 
	object->type = TYPE_STRING|ACCESS_UNLIMITED|ATTR_LITERAL;
	object->len = bin_value;
	return FALSE;
}


int HandleBinaryObjects(stream *ip,int encoding) {
	return(CONTINUE);
}
