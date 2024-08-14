/*	fncall.c
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <clib/exec_protos.h>

#include <ctype.h>
#include <scdir.h>

#include "make.h"
#include "depend.h"

#if FNCALLS

/*	function calls (mac, string)
 *
 *	The string parameter is the rest of the arguments passed to the
 *	function call.  string is modifiable in fncalls because it is
 *	actually contained within the macroname array[] which isn't
 *	really used again after the fncall returns.
 *
 *	returns 0 if successful, 1 if error
 *	with side effect:  mac->expansion = strdup( result_string );
 *
 */

static char argument_missing[] = "argument missing: %s %s\n";
static char error_no_memory[] = "error:  no memory\n";

static int
do_basename( struct macro *mac, char *string, how_basename )
{
	char *filelist = NULL;
	char *out, *text, *cptr;
	int len = 0;
	char word[ MAXPATHNAME ];

	filelist = (char *)calloc( Param.MaxLine, 1 );
	if( !filelist ) {
		logfile( error_no_memory );
		return( 1 );
	}
	
	out = filelist;
	text = string;

	while( len < Param.MaxLine ) {
		text = parse_str( word, text, sizeof(word));
		if( !*word )
			break; /* no more words in text */
		cptr = strrchr( word, '.' );
		if( how_basename ) { /* take the basename */
			if( cptr && basename(word) <= cptr ) {
				*cptr = (char)0; /* truncate the word at the `.' */
			}
		}
		else { /* take the suffix */
			if( cptr && basename(word) <= cptr ) {
				shift_string_left( word, (int)(cptr - word));
			}
		}
		len += strlen( word );
		if( len + 1 < Param.MaxLine ) {
			if( out != filelist ) {
				*out++ = ' ';
				len++;
			}
			strcpy( out, word );
			out = filelist + len;
		}
		else break;
	} /* while */
	*out = (char)0;
	if( len > 0 )
		mac->expansion = strdup( filelist );
	free( filelist );
	return( 0 );
}

static int
fn_basename( struct macro *mac, char *string )
{
	return( do_basename( mac, string, 1 ));
}

static int
fn_suffix( struct macro *mac, char *string )
{
	return( do_basename( mac, string, 0 ));
}

static int
do_addfix( struct macro *mac, char *string, int how_fix )
{
	char *filelist = NULL;
	char *suf, *str = string;

	filelist = (char *)calloc( Param.MaxLine, 1 );
	if( !filelist ) {
		logfile( error_no_memory );
		return( 1 );
	}

	while( isspace( *str )) str++;
	suf = str;
	while( *str && *str != ',' && str[-1] != '\\' ) str++;
	if( *suf != ',' && *str == ',' && str[-1] != '\\' ) {
		char *out, *text, *cptr;
		int suflen, len = 0;
		char word[ MAXPATHNAME ];

		*str++ = (char)0; /* null terminate the suffix */
		suflen = strlen( suf );
		out = filelist;
		text = str;

		while( len < Param.MaxLine ) {
			text = parse_str( word, text, sizeof(word));
			if( !*word )
				break; /* no more words in text */
			if( (strlen( word )+suflen) < sizeof(word)) {
				if( how_fix ) /* append the suffix */
					strcat( word, suf );
				else { /* prepend the prefix */
					len += strlen( suf );
					if( len + 1 < Param.MaxLine ) {
						if( out != filelist ) {
							*out++ = ' ';
							len++;
						}
						strcpy( out, suf );
						out = filelist + len;
					}
				}
			}
			len += strlen( word );
			if( len + 1 < Param.MaxLine ) {
				if( out != filelist & how_fix ) {
					*out++ = ' ';
					len++;
				}
				strcpy( out, word );
				out = filelist + len;
			}
			else break;
		} /* while */
		*out = (char)0;
		if( len > 0 )
			mac->expansion = strdup( filelist );
	}
	free( filelist );
	return( 0 );
}

static int
fn_addprefix( struct macro *mac, char *string )
{
	return( do_addfix( mac, string, 0 ));
}

static int
fn_addsuffix( struct macro *mac, char *string )
{
	return( do_addfix( mac, string, 1 ));
}

static int
do_dir( struct macro *mac, char *string, int how_dir )
{
	char *filelist = NULL;
	char *str = string;
	char *out, *text, *cptr;
	int len = 0;
	char word[ MAXPATHNAME ];

	filelist = (char *)calloc( Param.MaxLine, 1 );
	if( !filelist ) {
		logfile( error_no_memory );
		return( 1 );
	}

	while( isspace( *str )) str++;
	out = filelist;
	text = str;

	while( len < Param.MaxLine ) {
		text = parse_str( word, text, sizeof(word));
		if( !*word )
			break; /* no more words in text */
		cptr = basename( word );
		if( how_dir ) { /* keep only the directory part */
			*cptr = (char)0;
		}
		else { /* prepend the prefix */
			shift_string_left( word, (int)(cptr - word));
		}
		len += strlen( word );
		if( len + 1 < Param.MaxLine ) {
			if( out != filelist ) {
				*out++ = ' ';
				len++;
			}
			strcpy( out, word );
			out = filelist + len;
		}
		else break;
	} /* while */
	*out = (char)0;
	if( len > 0 )
		mac->expansion = strdup( filelist );
	free( filelist );
	return( 0 );
}

static int
fn_dir( struct macro *mac, char *string )
{
	return( do_dir( mac, string, 1 ));
}

static int
fn_notdir( struct macro *mac, char *string )
{
	return( do_dir( mac, string, 0 ));
}

static int
do_filter( struct macro *mac, char *string, int how_filter )
{
	char *str = string;
	char *pat;
	int len;

	while( isspace( *str )) str++;
	pat = str;
	while( *str && *str != ',' && str[-1] != '\\' ) str++;
	if( *pat != ',' && *str == ',' && str[-1] != '\\' ) {
		char word[ 80 ];
		char *text, *out;

		*str++ = (char)0;

		if( !( mac->expansion = strdup( str )))
			return( 0 ); /* no mem */
		out = mac->expansion;

		for( text = out;; ) {
			text = parse_str( word, text, sizeof(word));
			if( !*word )
				break; /* no more words in text */

			if( how_filter ) {
				if( pattern_match( pat, word )) {
					out = text + 1;
				}
				else { /* no match remove it */
					while( isspace( *text )) text++;
					shift_string_left( out, (int)(text - out));
					text = out;
				}
			}
			else {
				if( pattern_match( pat, word )) {
					while( isspace( *text )) text++;
					shift_string_left( out, (int)(text - out));
					text = out;
				}
				else { /* no match remove it */
					out = text + 1;
				}
			}
		} /* for */
		*out = (char)0;
		return( 0 );
	}
	logprintf( argument_missing, "filter", string );
	return( 1 );
}

static int
fn_filter( struct macro *mac, char *string )
{
	return( do_filter( mac, string, 1 ));
}

static int
fn_filter_out( struct macro *mac, char *string )
{
	return( do_filter( mac, string, 0 ));
}

static int
fn_findstring( struct macro *mac, char *string )
{
	char *find, *in, *str = string;
	int len;
	
	while( isspace( *str )) str++;
	find = str;
	while( *str && *str != ',' && str[-1] != '\\' ) str++;
	if( *find != ',' && *str == ',' && str[-1] != '\\' ) {
		*str++ = (char)0;
		in = str;
		while( *str ) str++;	/* find end string */
		len = strlen( find );
		str -= len;	/* str marks the end of searching */

		while( in <= str ) {
			if( !strnicmp( find, in, len )) { /* found */
				mac->expansion = strdup( find );
				break;
			}
			in++; 
		}
	}
	else {
		logprintf( argument_missing, "findstring", string );
		return( 1 );
	}
	return( 0 );
}

static int
fn_getenv( struct macro *mac, char *string )
{
	char *evalue;
	while( isspace( *string )) string++;

	evalue = getenv( string );
	if( evalue && *evalue )
		mac->expansion = strdup( evalue );
	return( 0 );
}

static int
fn_join( struct macro *mac, char *string )
{
	char *filelist = NULL;
	char *str = string;
	char *text1, *text2;
	
	filelist = (char *)calloc( Param.MaxLine, 1 );
	if( !filelist ) {
		logfile( error_no_memory );
		return( 1 );
	}

	while( isspace( *str )) str++;
	text1 = str;
	while( *str && *str != ',' && str[-1] != '\\' ) str++;
	if( *text1 != ',' && *str == ',' && str[-1] != '\\' ) {
		char *out;
		int len = 0;
		char word1[ MAXPATHNAME ], word2[ MAXPATHNAME ];

		*str++ = (char)0; /* null terminate the suffix */
		out = filelist;
		text2 = str;

		*word1 = *word2 = ' '; /* dummy for first iteration */
		while( len < Param.MaxLine ) {
			if( *word1 )
				text1 = parse_str( word1, text1, sizeof(word1));
			if( *word2 )
				text2 = parse_str( word2, text2, sizeof(word2));
			if( !*word1 && !*word2 )
				break; /* no more words */

			len += strlen( word1 ) + strlen( word2 );
			if( len + 1 < Param.MaxLine ) {
				if( out != filelist ) {
					*out++ = ' ';
					len++;
				}
				strcpy( out, word1 );
				strcat( out, word2 );
				out = filelist + len;
			}
			else break;
		} /* while */
		*out = (char)0;
		if( len > 0 )
			mac->expansion = strdup( filelist );
	}
	if( filelist )
		free( filelist );
	return( 0 );
}

static int
fn_patsubst( struct macro *mac, char *string )
{
	char *str = string;
	char *from, *to, *text;
	
	while( isspace( *str )) str++;
	from = str;
	while( *str && *str != ',' && str[-1] != '\\' ) str++;
	if( *from != ',' && *str == ',' && str[-1] != '\\' ) {
		*str++ = (char)0;
		to = str;
		while( *str && *str != ',' && str[-1] != '\\' ) str++;
		if( *to != ',' && *str == ',' && str[-1] != '\\' && str[ 1 ] ) {
			char *expansion = NULL, *out;
			int len = 0;

			*str++ = (char)0;
			while( *str && isspace( *str )) str++;
			text = str;

			out = expansion = (char *)malloc( Param.MaxLine );
			if( !expansion ) {
				logfile( error_no_memory );
				return( 1 );
			}
			while( *str ) {
				while( *str && !isspace( *str )) str++;
				if( !*str && str == text )
					break;
				if( *str )
					*str++ = (char)0;
				if( len + (int)(str - text) < Param.MaxLine ) {
					if( out > expansion ) {
						*out++ = ' ';
						len++;
					}
					if( pattern_match( from, text )) {
						/* needs to do bounds checking to prevent */
						/* overflowing of the expansion string! */
						/* neglected for now - user discretion advised */
						if( map_to_pattern( out, from, to, text ))
							break;
					}
					else strcpy( out, text );
				}
				len += strlen( out );
				out = expansion + len;
				while( *str && isspace( *str )) str++;
				text = str;
			}
			*out = (char)0;
			if( *expansion )
				mac->expansion = strdup( expansion );
			free( expansion );
			return( 0 );
		}
	}
	logprintf( argument_missing, "patsubst", string );
	return( 1 );
}

static int
sortcomp( char **s1, char **s2 )
{
    return( stricmp( *s1, *s2 ));
}

static int
fn_sort( struct macro *mac, char *string )
{
	int i, nel;
	char **array = NULL;
	char *expansion = NULL;
	char *out, *str = string;

	while( isspace( *str )) str++;
	nel = count_args( str );
	array = (char **)calloc( sizeof(char *), nel );
	expansion = (char *)malloc( Param.MaxLine );
	if( !array || !expansion ) {
		logfile( error_no_memory );
		if( array )
			free( array );
		if( expansion )
			free( expansion );
		return( 1 );
	}

	for( i = 0; i < nel && *str; i++ ) {
		while( isspace( *str ))
			str++;
		if( *str )
			array[ i ] = str++;
		while( !isspace( *str ) && *str )
			str++;
		if( isspace( *str ))
			*str++ = (char)0;
	}

    qsort( array, nel, sizeof(char *), sortcomp );
	out = expansion;
	for( i = 0; i < nel; i++ ) {
		if( i ) {
			if( stricmp( array[i], array[i-1] ) == 0 )
				continue; /* skip duplicates */
			*out++ = ' ';
		}
		strcpy( out, array[ i ] );
		out += strlen( out );
	}

	free( array );
	mac->expansion = strdup( expansion );
	free( expansion );
	return( 0 );
}

static int
fn_strip( struct macro *mac, char *str )
{
	while( isspace( *str )) str++;
	if( *str && (mac->expansion = strdup( str ))) {
		register char *d = mac->expansion;
		register char *s = mac->expansion;
		while( *s ) {
			*d++ = *s;
			if( isspace(*s))
				while( isspace( *s)) s++;
			else
				s++;
		}
		if( isspace( *d )) {
			while( isspace( *d ) && d > mac->expansion )
				d--;
			d++;
		}
		*d = (char)0;
	}
	return( 0 );
}

static int
fn_subst( struct macro *mac, char *string )
{
	char *str = string;
	char *from, *to, *text, *out;
	char *expansion = NULL;
	int fromlen, tolen;
	
	while( isspace( *str )) str++;
	from = str;
	while( *str && *str != ',' && str[-1] != '\\' ) str++;
	if( *from != ',' && *str == ',' && str[-1] != '\\' ) {
		*str++ = (char)0;
		to = str;
		while( *str && *str != ',' && str[-1] != '\\' )
			str++;
		if( *to != ',' && *str == ',' && str[-1] != '\\' && str[ 1 ] ) {
			*str++ = (char)0;
			text = str;
			fromlen = strlen( from );
			tolen = strlen( to );
			while( *str ) str++;	/* find end string */
			str -= fromlen;

			out = expansion = (char *)malloc( Param.MaxLine );
			if( !expansion ) {
				logfile( error_no_memory );
				return( 1 );
			}
			while( text <= str ) {
				if( !strnicmp( from, text, fromlen )) { /* found */
					if( (int)(out - expansion) + tolen < Param.MaxLine ) {
						strcpy( out, to );
						out += tolen;
					}
					else break;
					text += fromlen;
				}
				else if((int)(out - expansion) + tolen < Param.MaxLine ) {
					*out++ = *text++;
				}
				else break;
			}
			*out = (char)0;
			if( *expansion )
				mac->expansion = strdup( expansion );
			free( expansion );
			return( 0 );
		}
	}
	logprintf( argument_missing, "subst", string );
	return( 1 );
}

static int
fn_wildcard( struct macro *mac, char *string )
{
	char *filelist = NULL;
	char *pat = string;

	filelist = (char *)calloc( Param.MaxLine, 1 );
	if( !filelist ) {
		logfile( error_no_memory );
		return( 1 );
	}
	while( isspace( *pat )) pat++;
	if( *pat ) {
		char *out = filelist;
		char *fn;
		int len = 0;

		while( len < Param.MaxLine ) {
			if( !(fn = scdir( pat )))
				break; /* no more */
			if( out != filelist ) {
				*out++ = ' ';
				len++;
			}
			len += strlen( fn );
			if( len < Param.MaxLine ) {
				strcpy( out, fn );
				out = filelist + len;
			}
		}
		if( len > 0 )
			mac->expansion = strdup( filelist );
		scdir_abort();
	}
	free( filelist );
	return( 0 );
}


static int
do_word( struct macro *mac, char *string, int word )
{
	char *begin, *end;

	if( begin = find_word( string, word )) {
		end = begin;
		while( *end && !isspace( *end )) end++;
		*end = (char)0; /* null terminate past the end of word */
		mac->expansion = strdup( begin );
	}
	return( 0 );
}

static int
fn_firstword( struct macro *mac, char *string )
{
	while( isspace( *string )) string++;
	return( do_word( mac, string, 1 ));
}

static int
fn_word( struct macro *mac, char *string )
{
	char *comma;
	int word;

	while( isspace( *string )) string++;
	if( *string && (comma = find_token( string, ',' ))) {
		*comma = (char)0;
		word = atoi( string );
		/* else word not found */
		return( do_word( mac, comma + 1, word ));
	}
	logprintf( argument_missing, "word", string );
	return( 1 );
}

static int
fn_words( struct macro *mac, char *string )
{
	char buf[ 20 ];
	sprintf( buf, "%d", count_args( string ));
	mac->expansion = strdup( buf );
	return( 0 );
}

static int
fn_notimp( struct macro *mac, char *string )
{
	logfile( "Function call not implemented\n" );
	return( 1 );
}

static int fn_head(struct macro *mac, char *string)
{
	char *end;
	end=string+strlen(string)/2;
/* now, scan for a space or null */
	while ((*end != ' ') && (*end != '\t') && (*end)) end++;
	*end=0;
	mac->expansion=strdup(string);
}

static int fn_tail(struct macro *mac, char *string)
{
	char *end;
	end=string+strlen(string)/2;
/* now, scan for a space or null */
	while ((*end != ' ') && (*end != '\t') && (*end)) end++;
	mac->expansion=strdup(end);
}


/*************************************************************************/

/* sorted array for binary search */
#define MAX_FNCALL 22
static struct fncall fncarray[ MAX_FNCALL ] = {
	"addprefix",	fn_addprefix,
	"addsuffix",	fn_addsuffix,
	"basename",		fn_basename,
	"dir",			fn_dir,
	"filter",		fn_filter,
	"filter-out",	fn_filter_out,
	"findstring",	fn_findstring,
	"firstword",	fn_firstword,
	"foreach",		fn_notimp,
	"getenv",		fn_getenv,
	"join",			fn_join,
	"notdir",		fn_notdir,
	"origin",		fn_notimp,
	"patsubst",		fn_patsubst,
	"shell",		fn_notimp,
	"sort",			fn_sort,
	"strip",		fn_strip,
	"subst",		fn_subst,
	"suffix",		fn_suffix,
	"wildcard",		fn_wildcard,
	"word",			fn_word,
	"words",		fn_words,
	"head",			fn_head,
	"tail",			fn_tail
};

/*	binary search to find the function */
struct fncall *
find_fncall( char *name )
{
	register struct fncall *array = fncarray;
	int first = 0;
	int last = MAX_FNCALL - 1;
	int mid;
	int diff;

	/* binary search */
	while( first <= last ) {
		mid = (first+last) / 2;
		diff = stricmp( name, array[ mid ].name );
		if( !diff )
			return( &array[ mid ] ); /* found */
		if( first == last )
			break; /* not found */
		if( diff < 0 )
			last = mid - 1;
		else
			first = mid + 1;
	}
	return( NULL ); /* not found */
}

#else

/*	No FNCALLS */
struct fncall *
find_fncall( char *name )
{
	return( NULL ); /* not found */
}

#endif

