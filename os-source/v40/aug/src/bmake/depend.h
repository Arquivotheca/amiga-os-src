/*	depend.h
 *	(c) Copyright 1991 by Ben Eng, All Rights Reserved
 *
 */

#include <time.h>

#ifndef MAXSUFFIX
#define MAXSUFFIX 16
#endif

#ifndef MAX_MACRONAME
#define MAX_MACRONAME 256
#endif

#ifndef FNCALLS
#define FNCALLS 1
#endif

#define PATMATCH_CHAR '%'
#define DEFAULT_TARGET ".DEFAULT"

#define ONCE_TARGET "ONCE"
#define ALWAYS_TARGET "ALWAYS"
#define NEVER_TARGET "NEVER"
#define INVIS_TARGET "INVISIBLE"

struct target {
	struct Node node;
	struct List dependlist;
	struct List commandlist;
	struct List *alternate; /* if not OWNER of the commandlist */
	time_t mtime;
	long flags;
	int line_number;
	char name[ 2 ];
};

/* target flags */
#define TF_ADDED	0x00000001
#define TF_OWNER	0x00000002
#define TF_PATTERN	0x00000004
#define TF_DBLCOLON 0x00000008
#define TF_PHONY	0x00000010
#define TF_ONCE		0x00000020
#define TF_ALWAYS	0x00000040
#define TF_NEVER	0x00000080
#define TF_INVIS	0x00000100
#define TF_BUILTIN	0x40000000
#define TF_MADE		0x80000000

struct depend {
	struct Node node;
	char name[ 2 ];
};

struct command {
	struct Node node;
	long flags;
	char cmd[ 2 ];
};

#define CF_IGNORE	0x00000001
#define CF_NOECHO	0x00000002

struct patternrule {
	struct Node node;
	char tar_pat[ MAXPATHNAME ];
	char dep_pat[ MAXPATHNAME ];
	struct target *targ;
};

struct macro {
	struct Node node;
	long flags;
	char *name;	/* dynamic */
	char *expansion; /* dynamic */
};

struct fncall {
	char *name;
	int (*call)(struct macro *, char *);
};

/* macro flags */
#define MF_EXPANDED	0x00000001
#define MF_SIMPLE	0x00000002
#define MF_ADDED	0x80000000

extern struct target *default_target;

int getline( char *buf, int sz, FILE *in );

struct target *find_target( char *targetname );
struct target *new_target( char *targetname );
int delete_target( struct target *targ );
void delete_targetlist( struct List *list );
void set_default_target( struct target *targ );

struct depend *new_depend( char *dependname );
int delete_depend( struct depend *dep );
void delete_dependlist( struct List *list );

struct command *new_command( char *cmd );
int delete_command( struct command *cmd );
void delete_commandlist( struct List *list );

int pattern_match( char *pattern, char *cmpstr );
struct patternrule *find_patternrule( char *dep_pat, char *tar_pat );
struct patternrule *new_patternrule( char *dep_pat, char *tar_pat );
int delete_patternrule( struct patternrule *rule );
void delete_patternlist( struct List *list );
int map_to_pattern( char *name, char *from_pat, char *to_pat, char *string );
struct patternrule *add_pattern_rule( struct target *tar );
struct patternrule *add_suffix_targets( char *suf );

struct target *find_macro( char *macroname );
struct macro *new_macro( char *name, char *expansion );
struct macro *set_macro( char *name, char *expansion );
struct macro *set_simplemacro( char *name, char *expansion );
void set_target_macros( char *tarname, char *depname );
int delete_macro( struct macro *mac );
void delete_macrolist( struct List *list );

struct fncall *find_fncall( char *name );
void shift_string_left( char *string, int shift );
int expand_macros( char *dest, char *src, int maxlen );

void dump_all( void );

struct target *process_targetline( char *line, struct List *cmdlist );
struct macro *process_macroline( char *line );

time_t nowtime( void );
void touch( const char *filename );

int recipe( const char *goalname, struct List *cmdlist );
