#include	<exec/types.h>

/* --- GF64 --- */
#define	GF64_SIZE	64
typedef unsigned char	GF;

/* --- P & Q codes --- */
#define	P_LEN	24
#define	Q_LEN	4
#define	P_RANK	4
#define	Q_RANK	2

/* --- RS decoding constants --- */
#define	LOCLEN	4
#define	ERRNUM	2

/* --- use inline GF macros --- */
#define	GF_MACS

#ifdef	GF_MACS
#define	GF_Add(a,b)	((a)^(b))
#define	GF_Mul(a,b)	(MulTab[((a)<<6)+(b)])
#define	GF_Inv(a)	(Inv[(a)])
#else
GF GF_Add(GF a, GF b);
GF GF_Mul(GF a, GF b);
GF GF_Inv(GF a);
#endif

/* --- globale variable --- */
#ifdef	GF_MACS
extern	GF	MulTab[GF64_SIZE*GF64_SIZE];
#else
extern	GF	MulTab[GF64_SIZE][GF64_SIZE];
#endif

extern	GF 	Log[GF64_SIZE],Exp[GF64_SIZE],Inv[GF64_SIZE];
extern	GF	Hp[P_RANK][P_LEN],Hq[Q_RANK][Q_LEN];
extern	GF	SynP[P_RANK],SynQ[Q_RANK];

extern	UWORD	IrrPol;
extern  UWORD	GFDim,GFSize,GFLastIndex,GFZeroIndex;

#include	"ecc_protos.h"
#include	"synpoly_protos.h"