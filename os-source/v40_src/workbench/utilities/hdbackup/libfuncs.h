/************************************************************************
 *									*
 *	Copyright (c) 1989 Enhanced Software Technologies, Inc.		*
 *			   All Rights Reserved				*
 *									*
 *	This software and/or documentation is protected by U.S.		*
 *	Copyright Law (Title 17 United States Code).  Unauthorized	*
 *	reproduction and/or sales may result in imprisonment of up	*
 *	to 1 year and fines of up to $10,000 (17 USC 506).		*
 *	Copyright infringers may also be subject to civil liability.	*
 *									*
 ************************************************************************
 */



extern void SetWaitPointer ( struct Window *window );
extern void ClearWaitPointer ( struct Window *window );
extern void SetRangePointer ( struct Window *window );
extern void ClearRangePointer ( struct Window *window );
extern LONG amigados_to_secs ( LONG days, LONG secs, LONG ticks );
extern void secs_to_amigados ( LONG fullsecs, LONG *days, LONG *secs,
	LONG *ticks );
extern void days_to_string ( LONG days, char *string );
extern void days_to_string2 ( LONG days, char *string );
extern void days_to_date ( LONG days, int *day_ptr, int *month_ptr,
	int *year_ptr );
extern LONG days_since ( int day, int month, int year );
extern LONG ascwhen_to_secs ( char *string );
extern LONG ascdate_to_days ( char *string );
extern char *MyFindToolType ( char **ttype, char *tname );
extern BOOL MyMatchToolValue ( char *string, char *substring );
extern struct TextFont *GetFont ( struct TextAttr *ta );
extern struct DiskObject *ReadInfoFile ( char *name );
extern void *RemAlloc ( ULONG size, ULONG flags );
extern void *RemFree ( void *p );
extern void strlcpy ( char *dest, char *src, unsigned int width );
extern void add_path ( char *path, char *dir );
extern void trim_leading ( char *p, char c );
extern void subtract_path ( char *path );
extern BOOL strequal ( char *s1, char *s2 );
extern char *stringright ( char *string, unsigned int width );
extern void padstring ( char string[], int width );
extern int parse_inplace ( char *p, char **argptr, int maxargs );
extern BOOL kmstring_to_long( LONG *lp, char *string );
extern LONG stacksize ( void );
char *when_pattern( char *pattern );
int is_a_dir( char *fname );
LONG EasyRequest( struct Window *win, struct EasyStruct *es, ULONG *idcmp,
	APTR arg1, ... );

