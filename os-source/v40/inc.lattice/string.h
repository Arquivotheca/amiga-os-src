#ifndef __ARGS
#ifdef NARGS
#define __ARGS(a) ()
#else
#define __ARGS(a) a
#endif
#endif

/**
*
* Builtin function definitions
*
**/
#ifndef strlen
#define strlen __builtin_strlen
#define strcmp __builtin_strcmp
#define strcpy __builtin_strcpy
#endif
#ifndef memset
#define memset __builtin_memset
#define memcmp __builtin_memcmp
#define memcpy __builtin_memcpy
#endif

/**
*
* External definitions for string services
*
**/
extern int stcarg __ARGS((char *, char *));
extern int stccpy __ARGS((char *, char *, int));
extern int stcgfe __ARGS((char *, char *));
extern int stcgfn __ARGS((char *, char *));
extern int stcis __ARGS((char *, char *));
extern int stcisn __ARGS((char *, char *));
extern int stcd_i __ARGS((char *, int *));
extern int stcd_l __ARGS((char *, long *));
extern int stch_i __ARGS((char *, int *));
extern int stch_l __ARGS((char *, long *));
extern int stci_d __ARGS((char *, int));
extern int stci_h __ARGS((char *, int));
extern int stci_o __ARGS((char *, int));
extern int stcl_d __ARGS((char *, long));
extern int stcl_h __ARGS((char *, long));
extern int stcl_o __ARGS((char *, long));
extern int stco_i __ARGS((char *, int *));
extern int stco_l __ARGS((char *, long *));
extern int stcpm __ARGS((char *, char *, char **));
extern int stcpma __ARGS((char *, char *));
extern int stcsma __ARGS((char *, char *));
extern int astcsma __ARGS((char *, char *));
extern int stcu_d __ARGS((char *, unsigned));
extern int stcul_d __ARGS((char *, unsigned long));

#define stclen(a) strlen(a)

extern char *stpblk __ARGS((char *));
extern char *stpbrk __ARGS((char *, char *));
extern char *stpchr __ARGS((char *, int));
extern char *stpcpy __ARGS((char *, char *));
extern char *stpdate __ARGS((char *, int, char *));
extern char *stpsym __ARGS((char *, char *, int));
extern char *stptime __ARGS((char *, int, char *));
extern char *stptok __ARGS((char *, char *, int, char *));

extern int strbpl __ARGS((char **, int, char *));
extern char *strcat __ARGS((char *, char *));
extern char *strchr __ARGS((char *, int));
extern int strcmp __ARGS((char *, char *));
extern int stricmp __ARGS((char *, char *));
extern char *strcpy __ARGS((char *, char *));
extern int strcspn __ARGS((char *, char *));
extern char *strdup __ARGS((char *));
extern void strins __ARGS((char *, char *));
extern int strmid __ARGS((char *, char *, int, int));
extern int strlen __ARGS((char *));
extern char *strlwr __ARGS((char *));
extern void strmfe __ARGS((char *, char *, char *));
extern void strmfn __ARGS((char *, char *, char *, char *, char *));
extern void strmfp __ARGS((char *, char *, char *));
extern char *strncat __ARGS((char *, char *, unsigned));
extern int strncmp __ARGS((char *, char *, unsigned));
extern char *strncpy __ARGS((char *, char *, unsigned));
extern int strnicmp __ARGS((char *, char *, unsigned));
extern char *strnset __ARGS((char *, int, int));
extern char *strpbrk __ARGS((char *, char *));
extern char *strstr(char *, char *);

extern char *stpchrn __ARGS((char *, int));
extern char *strrchr __ARGS((char *, int));
extern char *strrev __ARGS((char *));
extern char *strset __ARGS((char *, int));
extern void strsfn __ARGS((char *, char *, char *, char *, char *));
extern int strspn __ARGS((char *, char *));
extern char *strtok __ARGS((char *, char *));
extern long strtol __ARGS((char *, char **, int));
extern unsigned long strtoul __ARGS((char *, char **, int));
extern char *strupr __ARGS((char *));
extern int stspfp __ARGS((char *, int *));
extern void strsrt __ARGS((char *[], int));

extern int stcgfp __ARGS((char *, char *));

#define strcmpi stricmp		/* For Microsoft compatibility */

/**
*
* External definitions for memory block services
*
**/
extern void *memccpy __ARGS((void *, void *, int, unsigned));
extern void *memchr __ARGS((void *, int, unsigned));
extern int  memcmp __ARGS((void *, void *, unsigned));
extern void *memcpy __ARGS((void *, void *, unsigned));
extern void *memset __ARGS((void *, int, unsigned));

extern void movmem __ARGS((void *, void *, unsigned));
extern void repmem __ARGS((void *, void *, int, int));
extern void setmem __ARGS((void *, unsigned, int));
extern void swmem __ARGS((void *, void *, unsigned));

