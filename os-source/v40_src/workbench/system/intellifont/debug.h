 /*----------------------------DEBUG.H--------------------------------------
  *  DEBUG.H  This header file contains the statements necessary for the
  *           conditional compilation of debug statements
  *------------------------------------------------------------------------*/

#ifdef DEBUG
#define  DBGS(statements)	{ statements; }
#else
#define  DBGS(statements)	;
#endif

#define DBG(user_text) \
    DBGS(kprintf(user_text))

#define DBG1(user_text,a1) \
    DBGS(kprintf(user_text,a1))

#define DBG2(user_text,a1,a2) \
    DBGS(kprintf(user_text,a1,a2))

#define DBG3(user_text,a1,a2,a3) \
    DBGS(kprintf(user_text,a1,a2,a3))

#define DBG4(user_text,a1,a2,a3,a4) \
    DBGS(kprintf(user_text,a1,a2,a3,a4))

#define DBG5(user_text,a1,a2,a3,a4,a5) \
    DBGS(kprintf(user_text,a1,a2,a3,a4,a5))

#define DBG6(user_text,a1,a2,a3,a4,a5,a6) \
    DBGS(kprintf(user_text,a1,a2,a3,a4,a5,a6))
