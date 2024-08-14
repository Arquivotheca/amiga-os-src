/* Prototypes for functions defined in
header.c
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


int calc_sum __PROTO((UBYTE * , int ));

extern unsigned int dsboy[12];

unsigned long ados_to_generic_stamp __PROTO((struct DateStamp ));

unsigned long ados_to_unix_stamp __PROTO((struct DateStamp ));

unsigned long unix_to_ados_mode __PROTO((int ));

boolean readhead_0 __PROTO((BPTR , LzHeader * ));

void writehead_0 __PROTO((BPTR , LzHeader * ));

boolean readhead_1 __PROTO((BPTR , LzHeader * ));

void writehead_1 __PROTO((BPTR , LzHeader * ));

boolean readhead_2 __PROTO((BPTR , LzHeader * ));

void writehead_2 __PROTO((BPTR , LzHeader * ));

extern struct hdrrdr Header_Levels[4];

boolean get_header __PROTO((BPTR , LzHeader * ));

void init_header __PROTO((UBYTE * , struct FileInfoBlock * , LzHeader * ));

void write_header __PROTO((BPTR , LzHeader * ));

