/* these are the dos packet actions that are not defined in the standard
   libraries/dosextens.h file */

#define		ACTION_MORE_CACHE	18
#define		ACTION_FLUSH		27
#define		ACTION_SET_DATE		34

#define		ACTION_DUMMY		1000
#define		ACT_READ		1001	/* already have another read */
#define		ACT_WRITE		1002	/* and another write */
#define		ACTION_SEEK		1008

#define		ACTION_FIND_UPDATE	1004
#define		ACTION_FIND_INPUT	1005
#define		ACTION_FIND_OUTPUT	1006
#define		ACTION_END		1007
#define		ACTION_FORMAT		1020

