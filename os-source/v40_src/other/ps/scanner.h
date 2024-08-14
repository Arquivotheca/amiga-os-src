// This file contains struct's, def's and macros for the scanner package
// Jan 1991 P.J. © CBM-Amiga
// Submitted to RCS on 05/APR/91

#ifndef SCANNER_H
#define SCANNER_H

/**************************************************************************
 *				struct's & defs										      *
 **************************************************************************/

/*          
 * Special Scanner Codes for token type. These are only used by the scanner 
 * functions and mean nothing to the outside world
 * 
 */

enum Token_Type {
		LITERAL_NAME,		/* The token is a literal name. ie starts with / */
		EXE_NAME,			/* Anything with valid characters */
		STRING,				/* delimeted with ( ) */
		PROCEDURE,			/* procedure start '{' */
		PROCEDURE_STOP,		/* procedure stop '}' */
		HEX,				/* delimeted with <> converted to hex pairs */
		NUMBER,				/* contains digits (NB could be a float??) */
		FLOATING } ;		/* contains digits and decimal point */


/* Binary Token types */

#define BT_INT_32_HILO		132
#define BT_INT_32_LOHI		133
#define BT_INT_16_HILO		134
#define BT_INT_16_LOHI		135
#define BT_INT_8			136
#define BT_INT_FIXED_POINT	137
#define BT_IEEE_32_HILO		138
#define BT_IEEE_32_LOHI		139
#define BT_NATIVE_REAL		140
#define BT_BOOLEAN			141
#define BT_8_STRING			142
#define BT_16_STRING_HILO	143
#define BT_16_STRING_LOHI	144
#define BT_LIT_SYS			145
#define BT_EXE_SYS			146
#define BT_LIT_USER			147
#define BT_EXE_USER			148
#define BT_HOMO_ARRAY		149
#define BT_UNUSED1			151
#define BT_UNUSED2			152
#define BT_UNUSED3			153
#define BT_UNUSED4			154
#define BT_UNUSED5			155
#define BT_UNUSED6			156
#define BT_UNUSED7			157
#define BT_UNUSED8			158
#define BT_RESERVED			159

#define DPS_HI_IEEE     128
#define DPS_LO_IEEE     129
#define DPS_HI_NATIVE   130
#define DPS_LO_NATIVE   131


/* Homogeneous Array Flags */

#define HA_FIXED_POINT		1L
#define HA_32_BIT_IEEE		2L
#define HA_32_BIT_NATIVE	3L
#define LOHI				0L
#define HILO				1L

/* Flags used for determining the type of a character by using its value
   as an index into an array of 255 chars */

#define ISSPACE 		0x01	
#define ISDELIM			0x02
#define ISDIGIT			0x04
#define ISTOKEN			0x08
#define ISESCAPE		0x10
#define ISBINTOKEN		0x20
#define ISBINOBJ		0x40

/* State machine values */

#define S_START			0x00
#define S_PLUS_MINUS	0x10
#define S_INITIAL_DOT	0x20
#define S_INTEGER_TOKEN	0x30
#define S_RADIX_TOKEN	0x40
#define S_INT_OVERFLOW	0x50
#define S_MIDWAY_DOT	0x60
#define S_DOUBLE_TOKEN	0x70
#define S_EXPONENT_E	0x80
#define S_EXPONENT_SIGN	0x90
#define S_EXPONENT_VALUE	0xa0
#define S_LOOKUP_NAME	0xb0
#define S_IMMED_LOOKUP	0xc0
#define S_LITERAL_NAME	0xd0
#define S_EXEC_NAME		0xe0

/* Macros used by the scanner to determine the type using the above flags */

#define is_space(c)  	((FLAGS+1)[c]&ISSPACE)	
#define is_delim(c)  	((FLAGS+1)[c]&ISDELIM)
#define is_token(c)  	((FLAGS+1)[c]&ISTOKEN)
#define is_digit(c)  	((FLAGS+1)[c]&ISDIGIT)
#define is_escape(c)	((FLAGS+1)[c]&ISESCAPE)
#define is_octal(c) 	((is_digit(c))&&(is_escape(c)))
#define is_bintkn(c)	((FLAGS+1)[c]&ISBINTOKEN)
#define is_binobj(c)	((FLAGS+1)[c]&ISBINOBJ)

/* Conversion Macros */

#define itoa(a,b)	sprintf(b,"%d ",a)
#define ftoa(a,b)	sprintf(b,"%f ",(double)a)
#define otoa(a,b)	sprintf(b,"\\%03o",a)

/* Stack Macros */


#define PushObject(oj,Type,Tag,Len,Union,Value) { \
							oj->type = Type;\
							oj->tag = Tag;\
							oj->len = Len;\
							oj->obj.Union = Value;\
							PUSHOPER(contxt,oj); }


/* Scanner return code */

#define COMPLETED       0x0100
#define CONTINUE		0x0200
#define ALLOC_FAILED	0x0400
#define BAD_STRING 		0x0600
#define SYNTAX_ERROR    0x0700
#define IMMED_ERROR		0x0800

/* These two flags put the scanner into either normal or deferred mode */
#define EX_NORMAL		0x00000
#define EX_DEFERRED		0x00001




/* These flags are used by NextToken to describe what it thinks the 
 * token is.
 */

/* flag to say all bytes read were numchar. However, may not fit in a long */
#define NT_NUMERIC			0x00001
/* flag to say a decimal point has been found in the token */
#define NT_DECIMAL_POINT	0x00002
/* flag to say an alpha character has been found in the token */
#define NT_ALPHA			0x00004



/* Misc stuff */

typedef struct Stack stack;
struct Stack {
	ps_obj object;
	struct Stack *prev; 
	} ;

#define APP ".obj"

#endif
