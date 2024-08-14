/* suck.h -- suck debugging tool includes */

#define printf  kprintf

/* visuals parameters */
#define WLEFT	20
#define WTOP	20
#define WWIDTH	200
#define WHEIGHT	110

#define STRGWIDTH	WIDTH1
#define STRGHEIGHT	9

#define WIDTH1	 80
#define HEIGHT1	 14

#define TLEFT1	4
#define TLEFT2	-40
#define TTOP1	2
#define TTOP2	2

#define DOTSPERROW 	15
#define TOPBORD 	15
#define ROW(n)	((n) * DOTSPERROW + TOPBORD)	/* pixel top for row 'n' */
#define LEFTBORD   	10
#define MIDLEFTBORD	110

/* string buffer sizes */
#define CKSIZEBUFLEN	9
#define CKFREEBUFLEN	9
#define CKMSGLEN	19
#define SUCKMSGLEN	23
#define AVAILMSGLEN	12

/* gadget text */
#define SUCKGT	"  Suck"
#define FREEGT	"Surrender"
#define AVAILGT	" Avail:"
#define CKSIZEGT "Chunk Size"
#define CKFREEGT "Chunks Free"

/* chunk-suckers */
WORD	chunkSuck();	/* suck them chunks		*/
VOID	chunkFree();	/* spit 'em back		*/
VOID	chunkMsg();	/* announce recent suck activity*/

VOID	suckMsg();	/* message into status area	*/
VOID	suckAvail();	/* call Avail() and update disp.*/

#define FREEALL  ((ULONG) ~0)

struct ChunkState {
/*    struct List cs_List;*/
    struct	ChunkHunk *cs_next;
    ULONG	cs_count;
    ULONG	cs_size;
    };

struct ChunkHunk {
    struct ChunkHunk *ch_next;
    ULONG ch_size;
    };

#define DEFCHUNKSIZE	0x1000

/* Gadget ID's */
#define CKSIZEG		9	/* size of chunks	*/
#define CKFREEG		8	/* number of chunks freed after suck */
#define SUCKG		2	/* go suck 		*/
#define FREEG		3	/* free allchunks	*/
#define AVAILG		4	/* refresh avail. msg.	*/

#define FIRSTGADGET &cksizeg

/* gadgets */
extern	struct Gadget cksizeg;
    extern	struct StringInfo cksizesi;
extern	struct Gadget ckfreeg;
    extern	struct StringInfo ckfreesi;
extern	struct Gadget suckg;
extern	struct Gadget freeg;
extern	struct Gadget availg;

/* gadget text */
extern	struct IntuiText ckfreegt;
extern	struct IntuiText cksizegt;
extern	struct IntuiText freegt;
extern	struct IntuiText suckgt;
extern	struct IntuiText availgt;

/* string gadget buffers */
extern	UBYTE cksizebuf[];
extern	UBYTE ckfreebuf[];

/* other (message) text */
extern	struct IntuiText ckmsgt;
extern	struct IntuiText suckmsgt;
extern	struct IntuiText availmsgt;

extern	UBYTE	ckmsgbuf[];
extern	UBYTE	suckmsgbuf[];
extern	UBYTE	availmsgbuf[];

extern  struct Window *window;
extern	struct ChunkState s;
