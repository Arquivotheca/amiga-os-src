/* Prototypes for functions defined in
util.c
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


long copyfile __PROTO((BPTR , BPTR , long , int ));

int encode_stored_crc __PROTO((BPTR , BPTR , long , long * , long * ));

unsigned char * convdelim __PROTO((unsigned char * , unsigned char ));

boolean archive_is_msdos_sfx1 __PROTO((char * ));

boolean skip_msdos_sfx1_code __PROTO((BPTR ));

