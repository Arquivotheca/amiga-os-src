/* lhex function prototypes */

// util.c
extern long copyfile(FILE *f1, FILE *f2, long size, int crc_flg);
extern int encode_stored_crc (FILE *ifp, FILE *ofp, long size, long *original_size_var,long *write_size_var);
extern unsigned char *convdelim(unsigned char *path, unsigned char delim);
extern boolean archive_is_msdos_sfx1(char *name);
extern boolean skip_msdos_sfx1_code (FILE *fp);

#ifdef NOSTRDUP
extern char *strdup ( char *buf);
#endif

#if defined(NOBSTRING) && !defined(__STDC__)
extern memmove(register char *dst , register char *src, register int cnt);
#endif

#ifdef NOFTRUNCATE
extern int i_rename(char *, char *);
#endif

extern int rmdir(char *path);
extern int mkdir(char *path, int mode);

#ifndef USESTRCASECMP
extern int strucmp(register char *s, register char *t);
#endif

#ifdef NOMEMSET
extern char *memset (char *s, int c, int n);
#endif


// lharc.c
extern void message (char *subject, char *name);
extern void warning (char *subject, char *name);
extern void error (char *subject, char *msg);
extern void fatal_error (char *msg);
extern void write_error (void);
extern void read_error (void);
extern void interrupt (int signo);
extern char *xmalloc (int size);
extern char *xrealloc (char *old, int size);
extern void init_sp (struct string_pool *sp);
extern void add_sp (struct string_pool *sp, char *name, int len);
extern void finish_sp (register struct string_pool *sp, int *v_count, char ***v_vector);
extern void free_sp (char **vector);
extern void cleaning_files (int *v_filec, char ***v_filev);
#ifdef NODIRECTORY
extern boolean find_files (char *name, int *v_filec, char ***v_filev);
extern void free_files (int filec, char **filev);
#else
extern boolean find_files (char *name, int *v_filec, char ***v_filev);
extern void free_files (int filec, char **filev);
#endif
extern void build_temporary_name (void);
extern void build_backup_name (char *buffer, char *original);
extern void build_standard_archive_name (char *buffer, char *orginal);
extern boolean need_file (char *name);
extern FILE *xfopen (char *name, char *mode);
extern FILE *open_old_archive (void);
extern int inquire (char *msg, char *name, char *selective);
extern void write_archive_tail (FILE *nafp);
extern void copy_old_one (FILE *oafp, FILE *nafp, LzHeader *hdr);

// patmatch.c
int patmatch(register char *p, register char *s,int f);

#ifdef AMIGA
// amiga.c
extern int getpid(void);
extern int getuid(void);
extern int umask(int numask);
extern int chown(char *name, int uid, int gid);
extern int chmod(const char *name, int mode);
extern int i_chmod(const char *name, unsigned long amiga_mode);
extern int link(char *name1, char *name2);
extern void mktemp(char *tempname);
extern utime(char *file, time_t times[]);
//extern i_utime(char *file, struct utimbuf *times);
extern i_utime(char *file, time_t times[]);
extern void kill(int pid, int signo);
extern void generic_to_ados_filename( unsigned char *name, int len );
extern void ados_convert(struct LzHeader *hdr);
#endif

// crcio.c
extern void make_crctable(void);
extern unsigned short calccrc( unsigned char *p, unsigned int n);
extern void fillbuf(unsigned char n);
extern unsigned short getbits(unsigned char n);
extern void putcode(unsigned char n, unsigned short x);
extern void putbits(unsigned char n, unsigned short x);
extern int fread_crc( unsigned char *p, int n, FILE *fp);
extern void fwrite_crc( unsigned char *p, int n, FILE *fp);
extern void init_code_cache(void);
extern void init_getbits(void);
extern void init_putbits(void);
extern void putc_euc(int c, FILE *fd);
extern int fwrite_txt( unsigned char *p, int n, FILE *fp);
extern int fread_txt( unsigned char *p, int n, FILE *fp);

// lhadd.c

extern FILE *append_it (char *name, FILE *oafp, FILE *nafp);
#ifdef TMP_FILENAME_TEMPLATE
extern void temporary_to_new_archive_file (long new_archive_size);
#else
extern void temporary_to_new_archive_file (long new_archive_size);
#endif

extern void cmd_add (void);
extern void cmd_delete (void);

// lhlist.c
extern void cmd_list (void);

// lhext.c
extern void cmd_extract (void);

// header.c

extern int calc_sum (register char *p, register int len);
extern boolean get_header (FILE *fp, register LzHeader *hdr);
extern void init_header (char *name, struct stat *v_stat, LzHeader *hdr);
extern void write_header (FILE *nafp, LzHeader *hdr);

// extract.c
extern int decode_lzhuf (FILE *infp, FILE *outfp, long original_size,\
			 long packed_size, char *name, int method);

// append.c
#ifndef LHEXTRACT

extern int encode_lzhuf (FILE *infp, FILE *outfp, long size,\
	long *original_size_var, long *packed_size_var, char *name,\
	 char *hdr_method);
#endif

extern void start_indicator (char *name, long size, char *msg,\
		 long def_indicator_threshold);
extern void finish_indicator2 (char *name, char *msg, int pcnt);
extern void finish_indicator (char *name, char *msg);


// slide.c
#ifndef LHEXTRACT
extern int encode_alloc(int method);
extern void get_next_match(void);
extern void encode(struct interfacing *interface);
#endif

extern void decode(struct interfacing *interface);

// dhuf.h
extern void start_c_dyn(void);
extern void decode_start_dyn(void);
extern unsigned short decode_c_dyn(void);
extern unsigned short decode_p_dyn(void);
extern void output_dyn( int code, unsigned int pos);
extern void encode_end_dyn(void);

// shuf.c
extern void decode_start_st0(void);
extern void encode_p_st0(unsigned short j);
extern void encode_start_fix(void);
extern void decode_start_fix(void);
extern unsigned short decode_c_st0(void);
extern unsigned short decode_p_st0(void);


// huf.c
extern void output_st1(unsigned short c, unsigned short p);
extern unsigned char *alloc_buf(void);
extern void encode_start_st1(void);
extern void encode_end_st1(void);

extern unsigned short decode_c_st1(void);
extern unsigned short decode_p_st1(void);
extern void decode_start_st1(void);


// larc.c
extern unsigned short decode_c_lzs(void);
extern unsigned short decode_p_lzs(void);
extern void decode_start_lzs(void);
extern unsigned short decode_c_lz5(void);
extern unsigned short decode_p_lz5(void);
extern void decode_start_lz5(void);

// maketbl.c
extern void make_table(short nchar, unsigned char bitlen[], short tablebits,\
	        unsigned short table[]);

// maketree.c
extern void make_code(int n, unsigned char len[], unsigned short code[]);
extern short make_tree(int nparm, unsigned short freqparm[],\
		 unsigned char lenparm[],unsigned short codeparm[]);
