/**************************************************************************
*  Utility Function to Convert ASCII files to Binary Encoded Tokens       *
*                                                                         *
*  Started : 5/07/91  © Commodore-Amiga                                   *
*                                                                         *
*                                                                         *
**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <exec/types.h>
#include <libraries/dos.h>
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <m68881.h>
#include <math.h>

#include "pstypes.h"
#include "stream.h"
#include "ascbt.h"

//-------------------------------------------------------------------------

int Scan(stream *in,int *intval,float *realval,int *len,int *type,char *buffer);
int FindIndex(char *string,int len);
int StringComp(char *s1,char *s2,int len);

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

extern char FLAGS[256];
extern char *NameTable[256];

//-------------------------------------------------------------------------
//
// This routine takes normal ASCII encoded files and converts them to
// there equivalent Binary Encoded Token format. At present the routine
// does not handle Name Encodings so these are directly copied into the
// output file without any conversion.
//
// It uses a modified version of the Scanner which simply fills in the
// relevant variables depending on what the token is.
//
//-------------------------------------------------------------------------

void main(int argc,char **argv) {
char *buffer;
BPTR out;
stream *in;
UBYTE bt;
int err,len,intval,type,index;
float realval;
float size1,size2;

	printf("\nASCII -> Binary Tokens Utility, (c) 1991 Commodore-Amiga\n");
	printf("Processing");

//  We use a 4K buffer to hold things like names and strings.

	if((buffer = (char *)AllocMem(4096,MEMF_CLEAR|MEMF_PUBLIC))==NULL) {
		printf("\rCould not Allocate Memory\n");
		return ;
	}

	if(argc<3) {
		printf("\r\nUsage : ascbt ASCII-file BT-file\n");
		return;
	}

//  The stream package is used for the input file. This is because we
//  can push characters back onto the input stream if we have read too
//  much, a useful facility which isn't available through DOS.

	in = (stream *)AllocMem(sizeof(stream),MEMF_PUBLIC|MEMF_CLEAR);

	if((OpenFileStream(in,argv[1],S_READABLE|S_SEEKABLE))==NULL) {
		printf("\rCould not open %s\n",argv[1]);
		FreeMem((char *)in,sizeof(stream));
		return;
	}

	if((out=Open(argv[2],MODE_NEWFILE))==NULL) {
		printf("\rCould not open %s\n",argv[2]);
		sclose(in);
		FreeMem((char *)in,sizeof(stream));
		return;
	}

//  This is the main loop. It's similar to the interpreter, only this
//  writes the output in an encoded format to the BT-File depending
//  on what the token type is.

	while((err=Scan(in,&intval,&realval,&len,&type,buffer))==CONTINUE) {
		switch((type&TYPE_MASK)) {

		case TYPE_INT :
			if(intval>=-128&&intval<=127) {
				bt = BT_INT_8;
				Write(out,&bt,1);	
				Write(out,(char *)&intval+3,1);
			} else if(intval>=-32768&&intval<=32767) {
				bt = BT_INT_16_HILO;
				Write(out,&bt,1);
				Write(out,(char *)&intval+2,2);
			} else {
				bt = BT_INT_32_HILO;
				Write(out,&bt,1);
				Write(out,(char *)&intval,4);
			}
			break ; 

		case TYPE_REAL :
			bt = BT_NATIVE_REAL;
			Write(out,&bt,1);
			Write(out,(char *)&realval,4);
			break ;

		case TYPE_STRING :
			if(len<=255) {
				bt = BT_8_STRING;
				Write(out,&bt,1);
				Write(out,(char *)&len+3,1);
			} else {
				bt = BT_16_STRING_HILO;
				Write(out,&bt,1);
				Write(out,(char *)&len+2,2);
			}
			Write(out,buffer,len);
			break ;

		case TYPE_NAME :
			if(type&ATTR_EXECUTE) {
				index = FindIndex(buffer,len);
				if(index!=-1) {
					bt = BT_EXE_SYS;
					Write(out,&bt,1);
					Write(out,(char *)&index+3,1);
					break;
				}
			} 

			if(!(type&ATTR_EXECUTE)) {
				Write(out,"/",1);
			}
			Write(out,buffer,len);
			Write(out,"\n",1);
			break;

		default :
			printf("\rUnknown Object type from scanner %d\n",type);
			break;
		}
		len = 0;
	}
	

//  Ouput some stat's

	size1 = (float)Seek(in->file,0,OFFSET_END);
	size2 = (float)Seek(out,0,OFFSET_END);

	printf("\r                        \n");
	printf("-------------------------\n");
	printf("Input  File Size | %d\n",(int)size1);
	printf("Output File Size | %d\n",(int)size2);
	printf("-----------------+-------\n");
	printf("Compression      | %.1f\%\n",(100.0-(size2/size1)*100.0));
	printf("-------------------------\n");

	Close(out);
	sclose(in);
	FreeMem(buffer,4096);
	FreeMem((char *)in,sizeof(stream));
}
int FindIndex(char *string,int len) {
	int index,len2,max;

	for(index=0;index<256;index++) {
		if(string[0]==(NameTable[index])[0]) {
			len2 = strlen(NameTable[index]);
			if(len2==len) {
				max = max(len,len2);
				if(StringComp(string,NameTable[index],len2)) return index;
			}
		}
	}
	return -1;
}

int StringComp(char *s1,char *s2,int len) {
	int loop;
	for(loop=0;loop<len;loop++) if((s1[loop]-s2[loop])!=0) return NULL;
	return TRUE;
}
		


//---------------------------------------------------------------------------
//
// This version of the scanner plugs the results into the parameters.
//
//---------------------------------------------------------------------------

int Scan(stream *in,int *intval,float *realval,int *len,int *type,char *buffer) {

char c;
int i_number=0,quit=FALSE,state=S_START,format=0,sign=1,exp_sign=1,base,count=0;
double factor=10.0,exponent=0.0,d_number=0.0;
	
	*len=0;


	while(quit==FALSE) {
		c = sgetc(in);
		buffer[count++] = (char)c;
		switch(state) {
	
		case S_START:
			if(isspace(c)) {
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
				state = S_LITERAL_NAME;
				count--;
			} else if(c=='['||c==']'||
				  c=='{'||c=='}') {
				quit = TRUE; *type = TYPE_NAME|ATTR_EXECUTE;
				buffer[count] = 0;
				*len = 1;
			} else if(c=='(') {
				if((GetEntireString(in,len,buffer))!=CONTINUE) {
					return SYNTAX_ERROR;
				}
				*type = TYPE_STRING;
				quit = TRUE;
			} else if(c=='<') {
				if((GetEntireHexString(in,len,buffer))!=CONTINUE) {
					return SYNTAX_ERROR;
				}
				quit = TRUE;
				*type = TYPE_STRING;
			} else if(c=='%') {
				count--;
				while(c!='\n'&&c!=EOF) c = sgetc(in); 
				if(c==EOF) return COMPLETED ;
			} else if(c==EOF) {
				return COMPLETED;
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
				sungetc(in,c); *type = ATTR_EXECUTE|TYPE_NAME;
				count--; quit = TRUE;
			} else {
				state = S_EXEC_NAME;
			}
			break;

			// decimal point first character

			case S_INITIAL_DOT:
				if(isdigit(c)) {
					state = S_DOUBLE_TOKEN; format |= DIGIT_B;
					*type = ATTR_LITERAL|TYPE_REAL;
					d_number += ((c-'0')/10.0); factor = 100.0;
				} else if(is_delim(c)) {
					sungetc(in,c); *type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else if(is_space(c)) {
					*type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// Reading ASCII digits into an integer			
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
					*type = ATTR_LITERAL|TYPE_INT; count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(in,c); *type = ATTR_LITERAL|TYPE_INT;
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
						*type = ATTR_LITERAL|TYPE_INT; quit = TRUE;
					} else {
						*type = ATTR_EXECUTE|TYPE_NAME; count--; quit = TRUE;
					}
				} else if(is_delim(c)) {
					if(format&RADIX_NUM) {
						*type = ATTR_LITERAL|TYPE_INT; quit = TRUE;
					} else {
						*type = ATTR_EXECUTE|TYPE_NAME; count--; quit = TRUE;
					}
					sungetc(in,c);
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
					*type = ATTR_LITERAL|TYPE_REAL; count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(in,c); *type = ATTR_LITERAL|TYPE_REAL;
					count--; quit = TRUE;
				} else {
					state = S_EXEC_NAME;
				}
				break;
	
			// We have "seen" a decimal point so if the next ASCII digit represents
			// a number (0-9) then we have a double and proceed to state 40.
			case S_MIDWAY_DOT:
				if(isdigit(c)) {
					state = S_DOUBLE_TOKEN; format |= DIGIT_B;
					d_number +=(double)(c-'0')/factor; factor*=10;
				} else if(c=='E'||c=='e') {
					state = S_EXPONENT_E; factor = 10.0; format |=EXP;
				} else if(is_space(c)) {
					if(format&DIGIT_A) {
						*type = ATTR_LITERAL|TYPE_REAL;
					} else {
						*type = ATTR_EXECUTE|TYPE_NAME;
					}
					count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(in,c);
					if(format&DIGIT_A) {
						*type = ATTR_LITERAL|TYPE_REAL;
					} else {
						*type = ATTR_EXECUTE|TYPE_NAME;
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
					*type = ATTR_LITERAL|TYPE_REAL;
					count--; quit = TRUE;
				} else if(is_delim(c)) {
					sungetc(in,c); count--; quit = TRUE;
					*type = ATTR_LITERAL|TYPE_REAL;
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
					*type = ATTR_EXECUTE|TYPE_NAME;
					count--; quit = TRUE;
				} else if(is_delim(c)) {
					*type = ATTR_EXECUTE|TYPE_NAME;
					quit = TRUE; count--; sungetc(in,c);
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
					*type = ATTR_EXECUTE|TYPE_NAME;
					quit = TRUE; count--;
				} else if(is_delim(c)) {
					*type = ATTR_EXECUTE|TYPE_NAME;
					quit = TRUE; count--; sungetc(in,c);
				} else {
					state = S_EXEC_NAME;
				}
				break;

			// In this state we read in digits which make up the exponent
			case S_EXPONENT_VALUE:
				if(isdigit(c)) {
					exponent *= factor; exponent += (double)(c-'0');
				} else if(is_space(c)) {
					*type = ATTR_LITERAL|TYPE_REAL;
					quit = TRUE; count--;
				} else if(is_delim(c)) {
					*type = ATTR_LITERAL|TYPE_REAL;
					quit = TRUE; count--; sungetc(in,c);
				} else {
					state = S_EXEC_NAME;
				}
				break;
	
			// This is a literal name. The first character MUST have been a '/'.
			case S_LITERAL_NAME: 
				if(is_space(c)) {

					// Delete the counter by one to wipe out the white space character !
					count--; quit = TRUE; *type = ATTR_LITERAL|TYPE_NAME;
					*len = count;
					buffer[count] = 0;

				} else if(is_delim(c)) {
					count--; sungetc(in,c);
					quit = TRUE; *type = ATTR_LITERAL|TYPE_NAME;
					*len = count;
					buffer[count] = 0;
				}
				break;
			
			// Anything which the state machine can't recognise as being part of
			// a number ends up looping here as this is the domain of the....
			// EXECUTEABLE NAME.
			case S_EXEC_NAME:
				if(is_space(c)) {
					count--;
					quit = TRUE; *type = ATTR_EXECUTE|TYPE_NAME;
					*len = count;
					buffer[count] = 0;
				} else if(is_delim(c)) {
					sungetc(in,c); count--;
					quit = TRUE; *type = ATTR_EXECUTE|TYPE_NAME;
					*len = count;
					buffer[count] = 0;
				}
				break;
	
			default : break;
			}
		}

		switch(*type&TYPE_MASK) {
		case TYPE_REAL :
			if(!sign) d_number *= -1.0;
			if(exponent!=0.0) {
				if(exp_sign) {
					d_number *= pow(10.0,exponent);
				} else {
					d_number /= pow(10.0,exponent);
				}
			} 
			*realval = (float)d_number;
			break;

		case TYPE_INT :

			if(!sign) i_number *=-1.0;
			*intval = i_number;
			break;

		default :
			break;
		}
	return CONTINUE;
}
//-----------------------------------------------------------------------
//
// Support routines used by the scanner to read Hex/ASCII strings
//
//-----------------------------------------------------------------------
	
int GetEntireString(stream *in,int *len,char *buffer) {
	int length=0;
	int c;
	int balance=1;
	int quit=FALSE;

	while(!quit) {
		c = sgetc(in);
		switch(c) {
			case '\\' :
				c = Get_Esc_Char(in); /* Found an escape. Handle it */
				break;
			case EOF :
				return(SYNTAX_ERROR);
			case ')' :
				if(balance) {                   /* Strings can be nested */
					balance--;                      /* So do balance checking */
					if(balance == 0) {              /* here  */
						quit = TRUE;
					}
				} 
				break;
			case '(' :
				balance++;
				break;
			default : break;
		}
		if((c!=-1)&&!quit) buffer[length++] = (uchar)c;
	}
	*len = length;
	buffer[length] = 0;
	return(CONTINUE);
}

int Get_Esc_Char(stream *in) {
	int c,i;
	int octal_value;

	c = sgetc(in);

	if(is_octal(c)) {
		octal_value = c - '0';
		for(i=0;i<2;i++) {
			c = sgetc(in);
			if(is_octal(c)) octal_value =(octal_value << 3) | (c - '0');
			else {
				 sungetc(in,c);
				 break;
			}
		}
		return(octal_value&0xff);
	} else switch(c) {             /* It's not Octal so it must be one of */
		case '(' :                 /* these.   */
			return('(');
		case ')' :
			return(')');
		case '\\' :
			return('\\');
		case 'f' :
			return('\f');
		case 'r' :
			return('\r');
		case 'n' :
			return('\n');
		case 't' :
			return('\t');
		case 'b' :
			return('\b');
		case '\n' :                /* This is a real CR, and is not encoded */
			return(-1);            /* with the string */
		default :
		return(c);
	}
}

int GetEntireHexString(stream *in,int *len,char *buffer) {

	int length=0;
	int c,nibble;
	nibble = 0;

	while((c=sgetc(in))!=EOF) {
		if(!is_space(c)) {            /* Only allow non-white space through */
			if(c == '>') break;       /* The end ? */
			c = toupper(c)-'0';      
			if(c>9) { c -= 7; }       /* Do a bit of math to convert ASCII */
			if(c>=0 && c<=15) {       /* Digit to Hex Digit */
				if(nibble==0) {
					buffer[length] = (uchar)(c<<4); /* High nibble */
					nibble = 1;
				} else {
					buffer[length++] |= (uchar)c;   /* Low nibble */
					nibble = 0;	
				}
			} else {
				return(SYNTAX_ERROR);
			}
		}
	}
	if(nibble == 1) { length++; }
	*len = length;
	buffer[length] =0;
	return(CONTINUE);
}

