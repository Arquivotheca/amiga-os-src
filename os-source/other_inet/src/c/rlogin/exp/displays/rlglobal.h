/* rlglobal.h */

#define TEMPLATE "HOST/A,-L=USER/K,-C=SCREEN/S,-X=LEFT/K,-Y=TOP/K,-W=WIDTH/K,-H=HEIGHT/K,-T=TERM/K,-U=UNIT/K,-R=NORESIZE/S"

#define MAXHOST 128L
#define MAXUSER 64L
#define MAXTERMTYPE 128L 

#define OPT_HOST      0
#define OPT_USER      1
#define OPT_SCREEN    2
#define OPT_LEFT      3
#define OPT_TOP       4
#define OPT_WIDTH     5
#define OPT_HEIGHT    6
#define OPT_TERMTYPE  7
#define OPT_CONUNIT   8
#define OPT_NORESIZE  9
#define OPT_COUNT    10

struct RLGlobal {                             /* default*/
	LONG  rl_UseScreen ;                      /*   0    */
	LONG  rl_Width ;                          /*   0    */
	LONG  rl_Height ;                         /*   0    */
	LONG  rl_Unit ;                           /*   3    */
	LONG  rl_Left ;                           /*   0    */
	LONG  rl_Top ;                            /*   0    */
	ULONG rl_vpmid ;                          /*  0     */
	LONG  rl_wbscr_wide ;                     /*  0     */
	LONG  rl_wbscr_high ;                     /*  0     */
	LONG  rl_fontwide ;                       /*  8     */
	LONG  rl_fonthigh ;                       /*  8     */
	LONG  rl_80wide ;                         /*  640   */
	LONG  rl_24high ;                         /*  0     */
	LONG  rl_48high ;                         /*  0     */
	LONG  rl_rows ;                           /*  24    */
	LONG  rl_cols ;                           /*  80   */ 
	LONG  rl_borderwidth ;                    /*   0    */
	LONG  rl_bordertop ;                      /*   0    */
	LONG  rl_borderbottom ;                   /*   0    */
	LONG  rl_resize ;                         /*   1    */
	LONG  rl_sockets_are_setup ;              /*   0    */
	int   rl_socket ;                         /*   0    */
	VOID *rl_glob_ptr ;                       /*   0    */
	struct RDargs   *rl_RDargs ;              /*   0    */
	struct Screen   *rl_Screen ;              /*   0    */
	struct Window   *rl_Window ;              /*   0    */
	struct TextFont *rl_TextFont ;            /*   0    */
	struct TextAttr *rl_TextAttr ;            /*   0    */
	BYTE *rl_hostptr ;                        /*   0    */
	BYTE *rl_userptr ;                        /*   0    */
	BYTE *rl_remuserptr ;                     /*   0    */
	BYTE *rl_termtypeptr ;                    /*   0    */
	BYTE rl_Host[MAXHOST+8L] ;                /*   0    */
	BYTE rl_User[MAXUSER+8L] ;                /*   0    */
	BYTE rl_TermType[MAXTERMTYPE+8L] ;        /*   0    */
	} ;

