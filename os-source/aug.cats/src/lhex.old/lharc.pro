/* Prototypes for functions defined in
lharc.c
 */


#ifndef __NOPROTO

#ifndef __PROTO
#define __PROTO(a) a
#endif

#else
#ifndef __PROTO
#define __PROTO(a) ()

#endif
#endif


extern char ** cmd_filev;

extern int cmd_filec;

extern char * archive_name;

extern char expanded_archive_name[1024];

extern char temporary_name[1024];

extern char backup_archive_name[1024];

extern int S_T_S;

extern char STS_Buff[8];

extern boolean quiet;

extern boolean text_mode;

extern boolean verbose;

extern boolean noexec;

extern boolean force;

extern boolean prof;

extern int compress_method;

extern int header_level;

extern boolean verbose_listing;

extern boolean output_to_stdout;

extern boolean new_archive;

extern boolean update_if_newer;

extern boolean delete_after_append;

extern boolean generic_format;

extern boolean remove_temporary_at_error;

extern boolean recover_archive_when_interrupt;

extern boolean remove_extracting_file_when_inte;

extern boolean get_filename_from_stdin;

extern boolean ignore_directory;

extern boolean verify_mode;

extern char * extract_directory;

extern char ** xfilev;

extern int xfilec;

extern char * Keywords[32];

extern struct RDArgs * Args;

extern LONG Result[32];

extern long __stack;

extern long __STKNEED;

extern char __stdiowin[55];

void main __PROTO((int , char ** ));

extern BPTR openfiles[32];

extern int maxopen;

void closeatmyexit __PROTO((BPTR ));

void clearcame __PROTO((BPTR ));

void myexit __PROTO((int ));

void message __PROTO((char const * , char const * ));

void warning __PROTO((char const * , char const * ));

void error __PROTO((char const * , char const * ));

void fatal_error __PROTO((char const * , char const * ));

extern char const * writting_filename;

extern char const * reading_filename;

void write_error __PROTO((void));

void read_error __PROTO((void));

void interrupt __PROTO((int ));

char * xmalloc __PROTO((int ));

char * xcalloc __PROTO((int , int ));

char * xstrdup __PROTO((char * ));

char * xrealloc __PROTO((char * , int ));

void init_sp __PROTO((struct string_pool * ));

void add_sp __PROTO((struct string_pool * , char * , int ));

void finish_sp __PROTO((struct string_pool * , int * , char *** ));

void free_sp __PROTO((char ** ));

void cleaning_files __PROTO((int * , char *** ));

void build_temporary_name __PROTO((void));

void build_backup_name __PROTO((char * , char * ));

void build_standard_archive_name __PROTO((char * , char * ));

boolean need_file __PROTO((char * ));

BPTR xfopen __PROTO((char * , int ));

extern int archive_file_mode;

extern int archive_file_gid;

BPTR open_old_archive __PROTO((void));

int inquire __PROTO((char const * , char const * , char const * ));

void write_archive_tail __PROTO((BPTR ));

void copy_old_one __PROTO((BPTR , BPTR , LzHeader * ));

int find_files __PROTO((char * , int * , char *** ));

void free_files __PROTO((int , char ** ));

int is_directory __PROTO((struct FileInfoBlock * ));

int is_regularfile __PROTO((struct FileInfoBlock * ));

