/**
*
* This header file defines the function forms of the various "is" and
* "to" operations.
*
**/
#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif

extern int isupper __ARGS((int));
extern int islower __ARGS((int));
extern int isdigit __ARGS((int));
extern int isxdigit __ARGS((int));
extern int isspace __ARGS((int));
extern int ispunct __ARGS((int));
extern int isalpha __ARGS((int));
extern int isalnum __ARGS((int));
extern int isprint __ARGS((int));
extern int isgraph __ARGS((int));
extern int iscntrl __ARGS((int));
extern int isascii __ARGS((int));
extern int iscsym __ARGS((int));
extern int iscsymf __ARGS((int));
extern int toupper __ARGS((int));
extern int tolower __ARGS((int));
extern int toascii __ARGS((int));
/**
*
* Define NULL if it's not already defined
*
*/
#ifndef NULL
#define NULL 0L
#endif
