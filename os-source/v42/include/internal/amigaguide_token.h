/* amigaguide_token.h
 *
 */

/*****************************************************************************/

struct AmigaGuideToken
{
    struct SignalSemaphore	 agt_Lock;		/* For exclusive access */
    struct List			 agt_XRefFileList;	/* Global cross reference file list */
    struct List			 agt_XRefList;		/* Global cross reference list */
};

/*****************************************************************************/

#define	TOKEN_NAME	"AmigaGuide®_XREF"
#define	TKSIZE		(sizeof(struct AmigaGuideToken))

/*****************************************************************************/

struct XRefFile
{
    struct Node			 xrf_Node;		/* Embedded node */
    UWORD			 xrf_Flags;		/* Status flags */
    BPTR			 xrf_Lock;		/* Directory lock */
    struct FileInfoBlock	*xrf_FIB;		/* File info block */
};

/*****************************************************************************/

#define	XRF_LOADED	0x0001

/*****************************************************************************/
