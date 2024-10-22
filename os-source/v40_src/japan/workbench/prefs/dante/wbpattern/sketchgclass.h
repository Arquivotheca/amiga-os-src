/* sketchgclass.h
 * Written by David N. Junod
 *
 */

#define SKETCHCLASSID	NULL

/* Tags that our boopsi object understands */
#define	SPA_Width	(TAG_USER + 1)
#define	SPA_Height	(TAG_USER + 2)
#define	SPA_Depth	(TAG_USER + 3)

#define	SPA_MagX	(TAG_USER + 10)
#define	SPA_MagY	(TAG_USER + 11)

#define	SPA_APen	(TAG_USER + 20)

#define	SPA_RastPort	(TAG_USER + 30)
#define	SPA_BitMap	(TAG_USER + 31)

#define	SPA_Store	(TAG_USER + 40)
#define	SPA_Restore	(TAG_USER + 41)
#define	SPA_Clear	(TAG_USER + 42)
#define	SPA_Erase	(TAG_USER + 43)
#define	SPA_Update	(TAG_USER + 44)

/*****************************************************************************/

#define	GM_EXTRENDER	20

#define	GME_STORE	1	/* Store work area to undo buffer */
#define	GME_RESTORE	2	/* Restore undo buffer to work area */
#define	GME_CLEAR	3	/* Clear the work area to foreground color */
#define	GME_ERASE	4	/* Clear the work area to background color */

/* GM_EXTRENDER	*/
struct gpExtRender
{
    ULONG		MethodID;
    struct GadgetInfo	*gpr_GInfo;	/* gadget context		*/
    struct RastPort	*gpr_RPort;	/* all ready for use		*/
    LONG		gpr_Command;	/* might be a "highlight pass"	*/
};

