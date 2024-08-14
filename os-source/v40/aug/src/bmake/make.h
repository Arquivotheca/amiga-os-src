/*	make.h
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <exec/nodes.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifndef DEBUG
#define DEBUG 1
#endif

#ifndef MAXPATHNAME
#define MAXPATHNAME	108
#endif

struct parameters {
	struct List filelist;
	int MaxLine;	/* maximum line length */
	int verbosity;	/* 0 = silent */
	int log;
	int debug;
	int touch_mode;
	int all_mode;
	int no_builtins;
	int pretend_mode;
	int print_database;
	char *makefile;
};

struct globals {
	struct Process *me;
	FILE *logfile;
	struct Screen *screen;
	struct Window *window;
	struct DrawInfo *drinfo;

	struct List targetlist;	/* target rules */
	struct List speciallist; /* special rules */
	struct List patternlist; /* pattern rules */
	struct List macrolist;	/* macros */

	int builtin_flag;
	int recursion_level;
	BPTR oldcwd;
};

struct string_node {
	struct Node node;
	char data[2];
};

#define min(a,b)	(((a) < (b)) ? (a) : (b))
#define max(a,b)	(((a) > (b)) ? (a) : (b))


extern struct parameters Param;
extern struct globals Global;
extern char version_string[];
extern char verdate_string[];
extern char copyright_string[];

extern void NewList( struct List * );
extern char *basename( const char * );

#if DEBUG
#define debugprintf(l,x) \
	if( Param.debug && Param.verbosity >= (l)) logprintf x ;
#else
#define debugprintf(l,x)
#endif

int parse_parameters( int argc, char *argv[] );
void delete_params( void );

struct string_node *new_snode( const char *str );
struct string_node *renew_snode( const struct string_node *old, const char *str );
struct string_node *delete_snode( struct string_node *old );
void delete_slist( struct List *list );

void *for_list( struct List *list, long (*node_fn)(void *));
void attach_list( struct List *newlist, struct List *oldlist );

int help( void );

/*
int prompt( BPTR tty, const char *str );
void tty_gotoxy( BPTR tty, int x, int y );
void tty_put( BPTR tty, char *str, long outsize );
void tty_puts( BPTR tty, char *str );
void tty_printf( BPTR tty, const char *fmt, ... );
void tty_clear( BPTR tty );
void tty_clear_eol( BPTR tty );
void tty_open_line( BPTR tty );
void tty_normal( BPTR tty );		
void tty_bold( BPTR tty );		
void tty_underline( BPTR tty );
void tty_backspace( BPTR tty );
void tty_scroll_up( BPTR tty, int lines );
void tty_scroll_down( BPTR tty, int lines );

int tty_getchar( void );
int hit_a_key( void );
void statusline( const char *s );
int get_string( char *buf, int mlen );
int get_number( char *buf, int mlen );
int get_valid_string( char *buf, int mlen, int (*validate)());
*/

int open_logfile( void );
void close_logfile( void );
void logfile( char *string );
void logprintf( const char *fmt, ... );

long doCommand( char *command, BPTR other );
long xsystem( char *cmd );

int input_makefile( const char *makefile );
int make_filename( const char *goalname, int *made );

char *find_token( char *line, int tok );
int isemptyline( char *line );
char *parse_strtok( char *dest, char *source, int len, int (*isdelim)( int ));
char *parse_str( char *dest, char *source, int len);
int count_args(unsigned char *string);
char *find_word( char *string, int word );
void strip_trailspace( char *line );
int isnotsuf( int ch );

int init_builtins( void );
