#ifndef DRIVER_H
#define DRIVER_H


/*****************************************************************************/


/* Printer buffers */
#define MAX_WIDTH 1536
#define MAX_FONTS (9*4*3)

/* Default margins, intended to handle the unprintable area of the page */
#define MARG_LEFT   20000
#define MARG_RIGHT  20000
#define MARG_TOP    20000
#define MARG_BOTTOM 20000

typedef LONG __stdargs (*WRITEFUNC)(APTR,LONG);
typedef LONG __stdargs (*READYFUNC)(VOID);

#define PWRITE(a,b) (*((WRITEFUNC)(PD->pd_PWrite)))(a,b)
#define PWAIT()     (*((READYFUNC)(PD->pd_PBothReady)))()
#define PREF        (PD->pd_Preferences)

#define BOOLEAN UBYTE


/*****************************************************************************/


LONG __saveds __stdargs DriverOpen(VOID);
LONG __saveds __stdargs DriverClose(VOID);

VOID SetPED(VOID);


/*****************************************************************************/


#endif /* DRIVER_H */
