/*
 *		lparse.c
 *
 * ======================================================================= 
 * LParse.c - generic token analyser                                       
 * By Talin and Joe Pearce.                                                
 * ======================================================================= 
 *
 *
 * Prototypes for functions defined in lparse.c
 *
 *	BOOL 	begin_parse			(struct TokenState * , int , int , int , int (* )(unsigned char ), int (* )(unsigned char ), unsigned char * , unsigned char * , unsigned char * , unsigned char * );
 *	WORD 	push_file			(struct TokenState * , unsigned char * );
 *	void 	end_parse			(struct TokenState * );
 *	WORD 	next_char			(struct TokenState * );
 *	void 	next_line			(struct InputFrame * , unsigned char * );
 *	WORD 	parse_escape		(int );
 *	BOOL 	next_token			(struct TokenState * );
 *	BOOL 	format_token_line	(struct TokenState * , struct TokenLine * , int );
 *	
 *	
 *	Revision History:
 *
 *	lwilton	07/11/93:
 *		General cleanup and reformatting to work with SAS 6.x and the
 *		standard header files.
 */



#define INSTALLER				/* used to cut out unneeded code for installer */

/* Other optimizations that could be done:
	1. Hex / bin / coment identifiers could be hard-coded instead of strcmp
	2. identifier function could be hard-coded
	3. Don't keep so many history tokens
	4. Use the "powerpacker" utility to decrease size of executable.
*/

/* general purpose parsing routines. Put in a fancier header. */

/* stuff to add:
	conventions for token-merging.
	conventions for escape sequences and escape character.
	"ignore the rest of the line" type thing.
	A generic symbol table (seperate module).
	Assembly-language to speed up next_token.
*/

#include <libraries/dosextens.h>
#include <exec/memory.h>
#include "functions.h"

#include <string.h>
#include <ctype.h>

#include "xfunctions.h"
#include "macros.h"

#include "lparse.h"

static default_first(char c) { return (isalpha(c) || c == '_'); }
static default_rest(char c) { return (isalnum(c) || c == '_'); }


BOOL begin_parse(

	struct TokenState *pstate,
	int max_symbol,
	int max_string,
	int flags,
	int (*valid_first)(char),
	int (*valid_rest)(char),
	char *beginc,
	char *endc,
	char *hexc,
	char *binc)
{
	pstate->max_symbol = max_symbol;		/* setup variables */
	pstate->max_string = max_string;
	pstate->flags = flags | FRESH_LINE;
											/* set up symbol test function (1) */
	if (valid_first) 
		pstate->valid_first = valid_first;
	else 
		pstate->valid_first = default_first;
											/* set up symbol test function (2) */
	if (valid_rest) 
		pstate->valid_rest = valid_rest;
	else 
		pstate->valid_rest = default_rest;

	pstate->current_frame = NULL;
	pstate->escape_character = '\\';		/* set default escape character */

	pstate->begin_comment = beginc;			/* set comment string beginning */
	pstate->end_comment = endc;				/* set comment string end */
	if (beginc)
		pstate->bc_length = strlen(beginc);	/* comment string length */

	if (pstate->begin_hex = hexc)			/* if hex string prefix */
		pstate->hex_length = strlen(hexc);	/* set prefix length */
	if (pstate->begin_bin = binc)			/* if bin string prefix */
		pstate->bin_length = strlen(binc);	/* set prefix length */

	pstate->token_history[0].old_frame =	/* reset frame pointers */
		pstate->token_history[1].old_frame =
		pstate->token_history[2].old_frame =
		pstate->token_history[3].old_frame = NULL;
	pstate->token_history[0].frame_delete =	/* mark frames as undeleted */
		pstate->token_history[1].frame_delete =
		pstate->token_history[2].frame_delete =
		pstate->token_history[3].frame_delete = FALSE;

											/* allocate text buffer */
	if (!(pstate->text_val = MemAlloc(MAX(max_symbol,max_string)+1,0L)))
		return FALSE;
	return TRUE;
}



static struct InputFrame *make_frame(char *start, long size)
{
	struct InputFrame *frame;
	
	/* allocate frame struct */
	
	if (!(frame = MemAlloc(sizeof (struct InputFrame),MEMF_CLEAR))) 
		return FALSE;
	frame->buffer = frame->scan = start;	/* setup scan pointers */
	frame->end = start+size;				/* setup buffer end */
	frame->prev_frame = NULL;
	frame->term_func = NULL;
	frame->term_data = NULL;
	frame->source_line = 1;
	frame->line_start = start;
	return frame;
}



static void delete_frame(struct InputFrame *frame)
{
	/* if file, then we made the buffer */
	
	if (frame->frame_type == IF_FILE)
	{
		MemFree(frame->buffer);
		MemFree(frame->filename);
	}
	MemFree(frame);							/* delete the frame */
}



WORD push_file (struct TokenState *pstate, char *filename)
{
	long 				size;				/* size of file */
	struct InputFrame	*frame;				/* new frame */
	BPTR				file;				/* file handle */
	char 				*buffer;			/* buffer */

	size = GetFileSize(filename);			/* get size of file */
	if (size < 0) 
		return LPARSE_FILEERR;				/* if error, quit */

											/* allocate buffer */
	if (!(buffer = MemAlloc(size,0L))) 
		return LPARSE_MEMERR;

	if (frame = make_frame(buffer,size))	/* make frame */
	{
		frame->filename = MemAlloc(strlen(filename)+1,0L);
		strcpy(frame->filename,filename);	/* copy filename to temp area */
		frame->frame_type = IF_FILE;		/* set type of frame */
	}
	else 
		return LPARSE_MEMERR;

	if (file = Open(filename,MODE_OLDFILE))	/* open file */
	{
		long status = Read(file,buffer,size);	/* read in file to buffer */
		Close(file);						/* close it */
		if (status != size) 
			return LPARSE_FILEERR;			/* test error and return */
	}
	else return LPARSE_FILEERR;

	frame->prev_frame = pstate->current_frame;
	pstate->current_frame = frame;
	return LPARSE_NOERR;
}



#if 0

BOOL push_macro(

	struct TokenState *pstate,
	char *buffer,
	long size,
	void (*efunc)(...),
	void *edata)
{
	struct InputFrame *frame;

	if (frame = make_frame(buffer,size))	/* make frame */
	{
		frame->frame_type = IF_MEMORY;		/* set type */
		frame->prev_frame = pstate->current_frame;
		frame->term_func = efunc;
		frame->term_data = edata;
		pstate->current_frame = frame;
	}
	return (frame ? TRUE : FALSE);			/* return success / failure */
}

#endif



void end_parse(struct TokenState *pstate)
{
	struct InputFrame	*frame = pstate->current_frame,
						*next_frame;
	short				i;

	while (frame)							/* for each frame */
	{
		next_frame = frame->prev_frame;		/* save next pointer */
		delete_frame(frame);				/* delete frame */
		frame = next_frame;
	}
	
	for (i=0; i<4; i++)
	{
		if (pstate->token_history[i].frame_delete)
			delete_frame(pstate->token_history[i].old_frame);
	}
	pstate->current_frame = NULL;			/* nullify frame ptr */
	MemFree(pstate->text_val);				/* dealloc text buffer */
}



static popframe(struct TokenState *pstate)
{
	struct InputFrame *frame;

	while (TRUE)
	{
		frame = pstate->current_frame;		/* current frame */
		if (!frame) 
			return FALSE;					/* if no frames left, return */
		if (frame->scan < frame->end) 
			return TRUE;					/* if frame not done, continue */
		pstate->current_frame = frame->prev_frame;	/* pop one level */
		if (frame->term_func)				/* call end-macro function */
			(frame->term_func)(frame->term_data);
		if (frame->frame_type == IF_MEMORY)	/* if macro */
			delete_frame(frame);			/* get rid of macro */
		else								/* else if include file */
		{
			short i;
			for (i=0; i<4; i++)
			{
				if (pstate->token_history[i].old_frame == frame)
				{
					pstate->token_history[i].frame_delete = TRUE;
					break;
				}
			}
			if (i==4) 
				delete_frame(frame);
		}
/*		delete_frame(frame);		*/		/* and delete frame */
	}
}



#ifndef INSTALLER

BOOL match_char (struct TokenState *pstate, int c)
{
	struct InputFrame *frame = pstate->current_frame;

	if (frame->scan >= frame->end)			/* if end of frame */
	{
		if (!popframe(pstate)) 
			return FALSE;					/* pop it and any others */
		frame = pstate->current_frame;
	}
	if (*frame->scan == c)					/* if chars match */
	{
		*frame->scan++;						/* bump scan pointer */
		return TRUE;						/* return MATCH */
	}
	return FALSE;							/* return NO MATCH */
}



BOOL match_ichar(struct TokenState *pstate, int c)
{
	struct InputFrame *frame = pstate->current_frame;

	if (frame->scan >= frame->end)			/* if end of frame */
	{
		if (!popframe(pstate)) 
			return FALSE;					/* pop it and any others */
		frame = pstate->current_frame;
	}
	if (toupper(*frame->scan) == toupper(c)) /* if chars match */
	{
		*frame->scan++;						/* bump scan pointer */
		return TRUE;						/* return MATCH */
	}
	return FALSE;							/* return NO MATCH */
}

#endif



WORD next_char(struct TokenState *pstate)
{
	struct InputFrame *frame = pstate->current_frame;

	if (frame->scan >= frame->end)			/* if end of frame */
	{
		if (!popframe(pstate)) 
			return FALSE; 					/* pop it and any others */
		frame = pstate->current_frame;
	}
	return (WORD)(*frame->scan++);			/* bump scan pointer */
}



void next_line (struct InputFrame *frame, char *scan)
{
	frame->source_line++;
	frame->line_start = scan;
}



WORD parse_escape(int c)
{
	switch (c) 
	{
	case 'n': c = '\n'; break;
	case 'r': c = '\r'; break;
	case 't': c = '\t'; break;
	case '0': c = '\0'; break;
	case '\'': c = '\''; break;
	case '"': c = '\"'; break;
	}
	return (WORD)c;
}



/* rules of tokens:
 *
 *	1. The information for a token will be destroyed when you
 *		call next_token, so you need to peek at the token, and look at the
 *		info before calling it. This is due to pipelining.
 *
 *	2. Similarly, a match_ichar will match the character AFTER the
 *		current token.
 *
 *	3. Currently, a token cannot span across frame boundaries.
 *		(Aztec C seems to have the same behavior).
 */

BOOL next_token (struct TokenState *pstate)
{
	/* look for white space */
	
	struct InputFrame *frame = pstate->current_frame;
	char	*scan, *end;
	struct TokenPos newtoken;
	LONG	sign;

	if (!frame)								/* if no frame */
	{
		pstate->token_type = TOKEN_EOF;		/* token type == EOF */
		return FALSE;						/* return */
	}

end_test:
	if (frame->scan >= frame->end)			/* if end of frame */
	{
		if (!popframe(pstate))				/* pop it and any others */
		{
			pstate->token_type = TOKEN_EOF;	/* if no frames left, token type == EOF */
			return FALSE;					/* return */
		}
		frame = pstate->current_frame;		/* adjust frame pointer */
	}
											/* setup token history for this token */
	newtoken.source_line 	= frame->source_line;
	newtoken.line_start 	= frame->line_start;
	newtoken.old_frame 		= frame;
	newtoken.frame_delete 	= FALSE;
	scan 					= frame->scan;	/* setup scan and end */
	end 					= frame->end;

	while (scan+1 < end && scan[0] == pstate->escape_character 
	&&	  (scan[1] == '\r' || scan[1] == '\n'))
	{
		/* do standard eol processing. */
		/* don't freshen line */
		
		scan += 2;
		next_line(frame,scan);
	}

dospace:
	/* REM: add test for line continuation... */
	/* skip white space and maybe returns */
	
#ifndef INSTALLER
	if (pstate->flags & SKIP_RETURN)		/* if returns don't count */
#endif
	{
		char c;								/* save character */
											/* if space or return */
		/* ====BUG==== This is a whitespace test -- isspace() */
		
		if (*scan == ' ' || *scan == '\t' || *scan == '\r' || *scan == '\n')
		{
			if (*scan == '\r' || *scan == '\n')	/* if return, do eol processing */
			{
#ifndef INSTALLER
				pstate->flags |= FRESH_LINE; /* mark next line as fresh */
#endif
				next_line(frame,scan+1);
				
				/* add to linecount, set new beginning of line, etc. */
			}
			c = *scan++;					/* skip char and save */

	dospace1:
			while (scan < end &&			/* scan for whitespace */
				*scan == ' ' || *scan == '\t' ||	/* whitespace */
				*scan == '\r' || *scan == '\n')		/* and returns */
			{
				if (*scan == '\r' || *scan == '\n')	/* if was return */
				{
#ifndef INSTALLER
					pstate->flags |= FRESH_LINE;	/* do eol processing */
#endif
					next_line(frame,scan+1);
					
					/* add to linecount, set new beginning of line, etc. */
				}
				c = *scan++;				/* skip character and save */
			}
											/* check for line continuation */
			if (scan+1 < end && scan[0] == pstate->escape_character &&
				(scan[1] == '\r' || scan[1] == '\n'))
			{
				/* do standard eol processing. */
				/* don't freshen line */
				
				scan += 2;
				next_line(frame,scan);
				goto dospace1;
			}

			if (!(pstate->flags & SKIP_BLANKS) ||	/* if blanks don't count */
				((c == ' ' || c == '\t') &&			/* or, if it's a leading blank */
					pstate->flags & LEADING_BLANKS && pstate->flags & FRESH_LINE))
			{
				pstate->token_type = TOKEN_BLANKS;	/* return a blanks token */
				goto cleanup;				/* fix up pstate */
			}
		}
	}
#ifndef INSTALLER
	else
	{
		if (*scan == ' ' || *scan == '\t')	/* if blank space */
		{
			scan++;							/* skip it */
											/* skip space until done */
	dospace2:
			while (scan < end && *scan == ' ' || *scan == '\t') 
				scan++;

			if (scan+1 < end && scan[0] == pstate->escape_character &&
				(scan[1] == '\r' || scan[1] == '\n'))
			{
				/* do standard eol processing. */
				/* don't freshen line */
				
				scan += 2;
				next_line(frame,scan);
				goto dospace2;
			}

			if (!(pstate->flags & SKIP_BLANKS) ||	/* set token if needed */
				(pstate->flags & LEADING_BLANKS && pstate->flags & FRESH_LINE))
			{
				pstate->token_type = TOKEN_BLANKS;	/* return a blanks token */
				goto cleanup;				/* fix up pstate */
			}
		}
		if (*scan == '\n' || *scan == '\r')	/* look for eol */
		{									/* do eol processing */
			/* add to linecount and all that */
			
			scan++;
			next_line(frame,scan);
			pstate->flags |= FRESH_LINE;	/* mark line as fresh */
			pstate->token_type = TOKEN_EOL; /* return an EOL token */
			goto cleanup;					/* fix up pstate */
		}
	}
#endif

#ifndef INSTALLER
	pstate->flags &= ~FRESH_LINE;			/* mark line as not fresh */
#endif
	frame->scan = scan;						/* mark beginning of token here */
	if (scan >= end) 
		goto end_test;						/* if space was last thing, test eof */

	/* look for comment */
	/* compare scan with comment string */
	
	if (pstate->begin_comment && scan + pstate->bc_length <= end)
	{
		if (strncmp(scan,pstate->begin_comment,pstate->bc_length) == 0)
		{
			scan += pstate->bc_length;		/* skip comment string begin */
			
			/* REM - if no comment end string, the comments ends at line end. */
#ifndef INSTALLER

			/* ====BUG==== There is no 'end comment' in the documentation! */
			
			if (pstate->end_comment)
			{
				short ec_length = strlen(pstate->end_comment);

				while (TRUE)
				{
					if (scan + ec_length >= end)
					{
						frame->scan = end;
						if (!popframe(pstate)) /* pop it and any others */
						{
							pstate->token_type = TOKEN_COMMERR;	/* comment error */
							goto cleanup;
						}
						frame = pstate->current_frame;		/* adjust frame pointer */
											/* and adjust token histor record */
						newtoken.source_line  = frame->source_line;
						newtoken.line_start   = frame->line_start;
						newtoken.old_frame    = frame;
						newtoken.frame_delete = FALSE;
						scan = frame->scan;	/* new scan */
						end  = frame->end;	/* new end */
					}						/* if found end */
					
					if (strncmp(scan,pstate->end_comment,ec_length)==0)
					{
						scan += ec_length;	/* skip end string */
						frame->scan = scan;	/* fix frame */
						goto end_test;		/* test for EOF */
					}
					if (*scan == '\r' || *scan == '\n')
					{
						/* do standard eol processing, don't freshen line. */
						next_line(frame,scan+1);
					}
					scan++;
				}
			}
			else
#endif
			{	while (!(*scan == '\r' || *scan == '\n')) /* if eol */
				{
					if (scan >= end)		/* if end of frame */
					{
						frame->scan = scan; /* set scan pointer in frame */
						goto end_test;		/* and go to beginning */
					}
					scan++;					/* else skip char */
				}
				goto dospace;				/* process return normally */
			}
		}
	}

		/* look for hex string */

	if (pstate->begin_hex && scan + pstate->hex_length <= end)
	{
		if (strncmp(scan,pstate->begin_hex,pstate->hex_length)==0)
		{
			long nval = 0;
			scan += pstate->hex_length;

			while (scan < end) /* scan for more digits */
			{
				if (isdigit(*scan))		/* accumulate numeric value */
					nval = (nval * 16) + (*scan++ - '0');
				else 
				if (*scan >= 'A' && *scan <= 'F')
					nval = (nval * 16) + (*scan++ - 'A' + 10);
				else 
				if (*scan >= 'a' && *scan <= 'f')
					nval = (nval * 16) + (*scan++ - 'a' + 10);
				else 
				if (isalpha(*scan))
				{	pstate->token_type = TOKEN_SYMERR;
					goto cleanup;
				}
				else 
					break;
			}
			pstate->numeric_val = nval;			/* return numeric value */
			pstate->token_type = TOKEN_INTEGER;	/* token type is integer */
			goto cleanup;						/* fix up pstate */
		}
	}

		/* look for binary string */

	if (pstate->begin_bin && scan + pstate->bin_length <= end)
	{
		if (strncmp(scan,pstate->begin_bin,pstate->bin_length)==0)
		{
			long nval = 0;
			scan += pstate->bin_length;

			while (scan < end && isdigit(*scan)) /* scan for more digits */
			{
				if (*scan >= '0' && *scan <= '1')
					nval = (nval * 2) + (*scan++ - '0');
				else
				{
					pstate->token_type = TOKEN_SYMERR;
					goto cleanup;
				}
			}
			pstate->numeric_val = nval;			/* return numeric value */
			pstate->token_type = TOKEN_INTEGER;	/* token type is integer */
			goto cleanup;						/* fix up pstate */
		}
	}

	sign = 1;

		/* look for digit string */
		
	if (*scan == '-' && isdigit(scan[1]))
	{
		sign = -1;
		scan++;
	}

	if (isdigit(*scan))						/* if next char is a digit */
	{
		long nval = *scan++ - '0';			/* compute numeric value */
		while (scan < end && isdigit(*scan)) /* scan for more digits */
		{
			nval = (nval * 10) + (*scan++ - '0');	/* accumulate numeric value */
		}
		pstate->numeric_val = sign * nval;	/* return numeric value */
		pstate->token_type = TOKEN_INTEGER;	/* token type is integer */
		goto cleanup;						/* fix up pstate */
	}

		/* look for quoted string */

	if (*scan == '"' || *scan == '\'')		/* if it's a quote */
	{
		short	strlength = 0;				/* initially zero length */
		char	quote = *scan;
		WORD	type = (quote == '"' ? TOKEN_STRING : TOKEN_TEXT);

		scan++;								/* skip over quote */
		while (scan < end && *scan != quote)	/* scan for rest of string */
		{
			if (strlength >= pstate->max_string)	/* if string too long */
			{
				pstate->text_val[strlength] = '\0';	/* terminate it */
				pstate->token_type = TOKEN_STRINGERR; /* return an error */
				goto cleanup;
			}
			if (*scan == pstate->escape_character)	/* if escape sequence */
			{
				/* char c;	*/					/* temp var */

					/* REM: this should actually be a table provided by
						the caller...or a function...
					*/

				scan++;						/* skip escape character */
				if (*scan == pstate->escape_character)	/* if two escapes */
				{
					pstate->text_val[strlength++] = *scan++;	/* then just use it */
				}
				else 
				if (*scan == '\n' || *scan == '\r')		/* eol */
				{
					/* this is where line continuation would be done... */
					/* need to do eol processing, but don't freshen line... */
					scan++;
					next_line(frame,scan);
				}							/* escape character */
				else 
					pstate->text_val[strlength++] = parse_escape(*scan++);
			}
			else 
			if (*scan == '\n' && (pstate->flags & EOL_BAD_STRING))
			{
				pstate->text_val[strlength] = '\0';	/* terminate it */
				pstate->token_type = TOKEN_TERMERR; /* return an error */
				goto cleanup;				
			}
			else 
				pstate->text_val[strlength++] = *scan++; /* next string character */
		}
		
		pstate->token_type = type;			/* token type is string */
		if (scan >= end)
		{
			pstate->token_type = TOKEN_TERMERR;	/* token type is string error */
		}
		else 
			scan++;							/* skip quote */
		pstate->text_val[strlength] = '\0';	/* null-terminate string */
		pstate->numeric_val = strlength;	/* return length of string in pstate */
		goto cleanup;						/* fix up pstate */
	}

		/* look for alpha symbol */

	if ((pstate->valid_first)(*scan))		/* if it's a symbol */
	{
		short symlength = 0;				/* initially zero length */

		pstate->text_val[symlength++] = *scan++;	/* get first character */
											/* scan for rest of sym */
		while (scan < end && (pstate->valid_rest)(*scan))
		{
			if (symlength >= pstate->max_symbol)	/* if symbol too long */
			{
				pstate->text_val[symlength] = '\0';	/* terminate it */
				pstate->token_type = TOKEN_SYMERR;	/* return an error */
				goto cleanup;
			}
			pstate->text_val[symlength++] = *scan++; /* next symbol character */
		}
		pstate->text_val[symlength] = '\0';	/* null-terminate symbol */
		pstate->numeric_val = symlength;	/* return length of symbol in pstate */
		pstate->token_type = TOKEN_SYMBOL;	/* token type is symbol */
		goto cleanup;						/* fix up pstate */
	}

	pstate->numeric_val = *scan++;			/* otherwise, it's a special character */
	pstate->token_type = TOKEN_PUNC;

cleanup:
	/* note:
		Beginning of current token is frame->scan.
		End of current token is scan.

		You can use this info for the highlighting calculation.

	*/

	newtoken.token_start = frame->scan;
	newtoken.token_end = scan;

	if (pstate->token_history[3].frame_delete)	/* flush out old frames */
		delete_frame(pstate->token_history[3].old_frame);
		
	pstate->token_history[3] = pstate->token_history[2];
	pstate->token_history[2] = pstate->token_history[1];
	pstate->token_history[1] = pstate->token_history[0];
	pstate->token_history[0] = newtoken;

	frame->scan = scan;
	return TRUE;
}



BOOL format_token_line(struct TokenState *pstate, struct TokenLine *tl, int token)
{
	struct InputFrame *frame;
	struct TokenPos *t;
	char *search;

#if 0
	for (frame = pstate->current_frame;		/* don't print out macros as errors */
		frame->frame_type != IF_FILE;
		frame = frame->prev_frame)
	{
		if (!frame) 
			return NULL;
		token=0;							/* set token # to zero */
	}
#endif

	t = &pstate->token_history[token];		/* get the token requested */
	if (!t->line_start) 
		return NULL;						/* if no source line, fail */

	frame = t->old_frame;
											/* look for line end */
	for (search = t->line_start; search < frame->end; search++)
	{
		if (*search == '\r' || *search == '\n') 
			break;
	}

	tl->source_line = t->source_line;		/* set source line # */
	tl->line_text   = t->line_start;		/* set source line start ptr */
	tl->line_length = search - t->line_start;
	tl->token_start = t->token_start - t->line_start;
	tl->token_stop  = t->token_end - t->line_start;

	if (tl->token_start < 0) 				tl->token_start = 0;
	if (tl->token_start > tl->line_length)  tl->token_start = tl->line_length;
	if (tl->token_stop  < 0) 				tl->token_stop  = 0;
	if (tl->token_stop  > tl->line_length) 	tl->token_stop  = tl->line_length;

	return TRUE;
}


#if 0

/* test routines */

is_valid_first(c) 
	char c; 
{ 
	return (isalpha(c) || c == '_'); 
}


is_valid_rest(c) 
	char c; 
{ 
	return (isalnum(c) || c == '_'); 
}


main()
{
	struct TokenState ptest;
	char c;

	begin_parse(&ptest,32,256,
		SKIP_RETURN|SKIP_BLANKS|LEADING_BLANKS,
		is_valid_first,is_valid_rest,
		";",NULL,
		"$","%");

	push_file(&ptest,"install-script");

	next_token(&ptest);
	while (ptest.token_type != TOKEN_EOF)
	{
		switch (ptest.token_type) 
		{
		case TOKEN_EOL: 	Puts("<return>"); break;
		case TOKEN_BLANKS: 	Puts("<blanks>"); break;
		case TOKEN_INTEGER: Printf("%ld\n",ptest.numeric_val); break;
		case TOKEN_SYMBOL: 	Printf("%ls\n",ptest.text_val); break;
		case TOKEN_STRING: 	Printf("\"%ls\"\n",ptest.text_val); break;
		case TOKEN_TERMERR: Printf("Unterminated string.\n"); break;
		case TOKEN_COMMERR: Printf("Unterminated comment.\n"); break;
		case TOKEN_SYMERR: 	Printf("Symbol too long: %ls\n",ptest.text_val); break;
		case TOKEN_PUNC: 	Printf("%lc\n",ptest.numeric_val); break;
		}
		next_token(&ptest);
	}
#if 0
	while (c = next_char(&ptest))
	{
		Write(Output(),&c,1L);
	}
#endif
	end_parse(&ptest);
}

#endif
