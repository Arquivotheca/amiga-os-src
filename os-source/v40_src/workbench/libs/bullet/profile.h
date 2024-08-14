/* profile.h */

/*--------------------------------------*/
/*    Timing Profiles                   */
/*--------------------------------------*/
/*  Exclude profile feature when time_val >= equates below */

extern  int  prof_val;

#define PROF_metrics       1   /* metric */
#define PROF_margin        2
#define PROF_fill          3   /* fill */
#define PROF_rast_tran     4   /* transition production */
#define PROF_rast_loop     5   /* tran direction and count */
#define PROF_nxy           6   /* nxy() */
#define PROF_nxt_pt        7   /* nxt_pt() */
#define PROF_raster        8   /* raster init and nxt_pt() */
#define PROF_xskel         9   /* xskel */
#define PROF_yskel         10  /* yskel */
#define PROF_yclass        11  /* y lines */
#define PROF_txy_skel_init 12  /* scale skels to working bm space */
#define PROF_skeletal_init 13  /* loop overhead of above */
#define PROF_MAKbitMap     14
#define PROF_MAKfontSize   15
#define PROFILE_CT         16

