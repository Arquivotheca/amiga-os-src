/*****************************************************************************/

struct GlobalData
{
    struct Library		*gd_DOSBase;
    struct Library		*gd_UtilityBase;
    struct GfxBase		*gd_GfxBase;
    struct IntuitionBase	*gd_IntuitionBase;
    struct Library		*gd_AmigaGuideBase;

    struct Process		*gd_Process;		/* Our process address */
    struct Screen		*gd_Screen;		/* Screen that our application will open on */
    LONG			 gd_NHID;		/* Unique id */
    ULONG			 gd_NHFlags;		/* Help related flags */
    UBYTE			 gd_NHName[64];		/* Unique name */
    struct Hook			 gd_NHHook;		/* Dynamic node host hook */
    struct AmigaGuideHost	*gd_NH;			/* Dynamic node host */
    AMIGAGUIDECONTEXT		 gd_AmigaGuide;		/* Pointer to the AmigaGuide context */
    struct NewAmigaGuide	 gd_NAG;		/* Used to start AmigaGuide */
    UBYTE			 gd_Buffer[1024];	/* Temporary buffer */
    UBYTE			 gd_FBuffer[512];	/* Temporary string buffer */
    UBYTE			 gd_Node[12288];	/* Node buffer */
};

/*****************************************************************************/

#define	DOSBase			 gd->gd_DOSBase
#define	UtilityBase		 gd->gd_UtilityBase
#define	GfxBase			 gd->gd_GfxBase
#define	IntuitionBase		 gd->gd_IntuitionBase
#define	AmigaGuideBase		 gd->gd_AmigaGuideBase
