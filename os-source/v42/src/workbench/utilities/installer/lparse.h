/*
 *		lparse.h
 *
 *
 * ======================================================================= 
 * LParse.h - include file for generic token analyser                      
 * By Talin and Joe Pearce.                                                
 * ======================================================================= 
 *
 *
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */



#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/* used by application to obtain a source line address for error reporting */

struct TokenLine 
{
	long				source_line;		/* source line number */
	char				*line_text;			/* beginning of line */
	short				line_length,
						token_start,
						token_stop;
};

/* used by parser to track previous tokens */

struct TokenPos 
{
	long				source_line;		/* source line number */
	char				*line_start,		/* beginning of line */
						*token_start,		/* start of token */
						*token_end;			/* end of token */
	struct InputFrame	*old_frame;			/* a deleted frame */
	char				frame_delete;		/* frame delete flag */
};

/* used by parser to push parsing states */

struct InputFrame 
{
	struct InputFrame	*prev_frame;		/* pointer to previous frame */
	char				frame_type;			/* type of frame - file or buffer */
	char				*filename;			/* name of file */
	char				*buffer,			/* memory buffer */
						*scan,				/* scan pointer */
						*end;				/* end of buffer */
	void				(*term_func)(void *),	/* function to call when macro finished */
						*term_data;			/* data to call function with */
	long				source_line;		/* current source line */
	char				*line_start;		/* current line start */
};

#define IF_FILE 	0						/* a file frame */
#define IF_MEMORY	1						/* an input frame */

/* the main parsing structure */

struct TokenState 
{
	struct InputFrame	*current_frame;		/* pointer to current read frame */
	char				flags;				/* flags used in parsing */
	char				token_type;			/* last token read */
	char				escape_character;	/* escape character */
	short				max_symbol,			/* maximum symbol length */
						max_string;			/* maximum string length */
	int					(*valid_first)(char),	/* returns TRUE if first char symbol */
						(*valid_rest)(char);	/* returns TRUE if non-first char symbol */
	char				*text_val;			/* symbol / string pointer */
	char				*begin_comment,		/* comment begin string */
						*end_comment;		/* comment end string */
	short				bc_length;			/* length of begin comment string */
	long				numeric_val;		/* numeric value */
	char				*begin_hex,			/* char string which begins hex # */
						*begin_bin;			/* char string which begins bin # */
	short				hex_length,			/* length of hex intro */
						bin_length;			/* length of bin intro */
	struct TokenPos		token_history[4];	/* token history */
};

/* parsing flags */

#define SKIP_RETURN		(1<<0)				/* treat end of line as whitespace */
#define SKIP_BLANKS		(1<<1)				/* don't return token for whitespace */
#define LEADING_BLANKS	(1<<2)				/* return token for leading blanks */
#define FRESH_LINE		(1<<3)				/* set by parser to indicate new line */
#define EOL_BAD_STRING	(1<<4)				/* string cannot span lines */

/* error codes */

enum {
	LPARSE_NOERR=0,
	LPARSE_FILEERR,
	LPARSE_MEMERR
};

/* token types -- can be extended by application */

enum token_types 
{
	TOKEN_EOF=0,							/* end of file */
	TOKEN_BLANKS,							/* white space */
	TOKEN_EOL,								/* end of line */
	TOKEN_SYMBOL,							/* symbol */
	TOKEN_SYMERR,							/* symbol that was too long */
	TOKEN_INTEGER,							/* a numeric value */
	TOKEN_STRING,							/* a doubly-quoted string */
	TOKEN_TEXT,								/* a singly-quoted string */
	TOKEN_STRINGERR,						/* a string that was too long */
	TOKEN_TERMERR,							/* string terminated by EOF error */
	TOKEN_COMMERR,							/* comment terminated by EOF error */
	TOKEN_PUNC,								/* a special character */
	TOKEN_LAST								/* last token in list */
};


	/*	Some function prototypes for the parser.  */
	
short begin_parse (
	struct TokenState *pstate, 
	int max_symbol, 
	int max_string,
	int flags, 
	int (*valid_first)(char), 
	int (*valid_rest)(char),
	char *beginc, 
	char *endc, 
	char *hexc, 
	char *binc);
	
short push_file(
	struct TokenState *pstate, 
	char *filename);
	
void end_parse(
	struct TokenState *pstate);
	
short next_char(
	struct TokenState *pstate);
	
void next_line(
	struct InputFrame *frame, 
	char *scan);
	
short parse_escape(
	int c);
	
short next_token(
	struct TokenState *pstate);
	
short format_token_line(
	struct TokenState *pstate, 
	struct TokenLine *tl, 
	int token);
