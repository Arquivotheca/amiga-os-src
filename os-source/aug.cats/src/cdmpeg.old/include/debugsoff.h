
#ifdef	D
 #undef	D
#endif

#ifdef	P
 #undef	P
#endif

#ifdef	Q
 #undef	Q
#endif

#ifdef	X
 #undef	X
#endif

#ifdef	PD
 #undef	PD
#endif

#ifdef	PF
 #undef	PF
#endif

#ifdef	PP
 #undef	PP
#endif

#ifdef	K
 #undef	K
#endif

#ifdef	KP
 #undef	KP
#endif

#ifdef	KPF
 #undef	KPF
#endif

#ifdef	PRINTF
 #undef	PRINTF
#endif

#define	D(x)	;
#define	Q(x)	;
#define	P(x)	;
#define	PF(x)	;
#define	PD(x,y)	;
#define	PP(x)	;
#define	X()	;

#define	KP(x)	;
#define	KPF(x)	;
#define	K(x)	;

#define	PRINTF	printf

#ifndef	DEBUG_UTILS
#define	DEBUG_UTILS

#define	PFSTRING	"*no*string*"
#define PFSTR(x) ((x) ? x : PFSTRING)

#endif


