
#ifdef  D
 #undef D
#endif

#ifdef  P
 #undef P
#endif

#ifdef  Q
 #undef Q
#endif

#ifdef	X
 #undef	X
#endif

#ifdef  PD
 #undef PD
#endif

#ifdef  PF
 #undef PF
#endif

#ifdef  PP
 #undef PP
#endif

#ifdef  PRINTF
 #undef PRINTF
#endif

#ifdef KPRINTF
 #define    PRINTF  kprintf
#else
 #define    PRINTF  printf
#endif

#define D(x)    {x;};
#define Q(x)    {x;fflush(stdout);pause();}
#define P(x)    {PRINTF("%ls",x);};
#define PF(x)   {PRINTF("%ls\n",x);};
#define PD(x,y) {PRINTF("%ls Delay ...\n",x);Delay(y);};
#define PP(x)   {PRINTF("%ls\n",x);pause();};
#define	X()		{PRINTF("%ls @ %ld\n", __FILE__, __LINE__ );}

#ifndef DEBUG
#define DEBUG
#endif

#ifndef DEBUG_UTILS
#define DEBUG_UTILS

#define	PFSTRING	"*no*string*"
#define PFSTR(x) ((x) ? x : PFSTRING)

#endif

