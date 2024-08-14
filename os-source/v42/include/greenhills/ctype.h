/*
    C Runtime library
    
    Copyright 1983,1984,1985,1986 by Green Hills Software Inc.

    This program is the property of Green Hills Software, Inc,
    its contents are proprietary information and no part of it
    is to be disclosed to anyone except employees of Green Hills
    Software, Inc., or as agreed in writing signed by the President
    of Green Hills Software, Inc.
*/
#define	_UPPER		0x01
#define _LOWER		0x02
#define _DIGIT		0x04
#define _SPACE		0x08
#define _PUNCT		0x10
#define _CONTROL	0x20
#define _HEXDIGIT	0x40
#define _UNDERSCORE	0x80

extern unsigned char _ctype_[];

#define isascii(ch) ((unsigned)(ch)<=0x7f)
#define toascii(ch) ((ch)&0x7f)
#define isupper(ch) (((_ctype_+1)[(ch)])&(_UPPER))
#define toupper(ch) ((ch)-('a'-'A'))
#define _toupper(ch) toupper(ch)
#define islower(ch) (((_ctype_+1)[(ch)])&(_LOWER))
#define tolower(ch) ((ch)+('a'-'A'))
#define _tolower(ch) tolower(ch)
#define isalpha(ch) (((_ctype_+1)[(ch)])&(_UPPER|_LOWER))
#define isdigit(ch) (((_ctype_+1)[(ch)])&(_DIGIT))
#define isxdigit(ch) (((_ctype_+1)[(ch)])&(_DIGIT|_HEXDIGIT))
#define isalnum(ch) (((_ctype_+1)[(ch)])&(_UPPER|_LOWER|_DIGIT))
#define iscsym(ch)  (((_ctype_+1)[(ch)])&(_UPPER|_LOWER|_DIGIT|_UNDERSCORE))
#define iscsymf(ch) (((_ctype_+1)[(ch)])&(_UPPER|_LOWER|_UNDERSCORE))
#define isspace(ch) (((_ctype_+1)[(ch)])&(_SPACE))
#define ispunct(ch) (((_ctype_+1)[(ch)])&(_PUNCT))
#define iscntl(ch)  (((_ctype_+1)[(ch)])&(_CONTROL))
#define isprint(ch) (!iscntl(ch))
#define isgraph(ch) (((_ctype_+1)[(ch)])&(_UPPER|_LOWER|_DIGIT|_PUNCT))
