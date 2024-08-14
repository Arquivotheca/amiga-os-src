#ifndef FILEREQ_H
#define FILEREQ_H


/*****************************************************************************/


#include <exec/types.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>

#include "aslbase.h"
#include "requtils.h"


/*****************************************************************************/


/* memory allocated for the file req only when it is being displayed */
struct FRStore
{
    struct StandardPacket rs_StdPkt;
    struct AChain         rs_AC;       /* Must be longword aligned!!! */
    struct AnchorPath     rs_AP;
    struct MsgPort        rs_PacketPort;

    char                  rs_OriginalDrawer[256];
    char                  rs_OriginalFile[256];
    char                  rs_OriginalPattern[128];

    char                  rs_ParsedPattern[260];

    char                  rs_TempPattern[50];

    struct RastPort       rs_LVRPort;

    struct MsgPort        rs_AppPort;
};


/*****************************************************************************/


struct ExtFileReq
{
    /* stuff needed for all requesters */
    struct ReqInfo    fr_ReqInfo;

    /* public portion of structure, must match definition in asl.h */
    STRPTR	      fr_TitleText;	/* RESERVED 0 */
    STRPTR            fr_File;
    STRPTR            fr_Drawer;
    struct Window    *fr_Window;	/* RESERVED 1 */
    APTR	      fr_Function;	/* RESERVED 1 */
    UBYTE             fr_Options;	/* RESERVED 1 */
    UBYTE             fr_Options2;	/* RESERVED 1 */
    struct IBox       fr_Coords;	/* Coordinates of requester on exit */
    WORD              fr_MSpace;	/* RESERVED 2 */
    LONG              fr_NumArgs;	/* Number of files selected */
    struct WBArg     *fr_ArgList;	/* List of files selected */
    APTR              fr_UserData;	/* You can store your own data here */
    struct AppWindow *fr_AppWindow;	/* RESERVED 3 - AppWindow structure */
    struct FRStore   *fr_Store;
    STRPTR            fr_Pattern;	/* Contents of Pattern gadget on EXIT */

    /* private portion of structure */
    APTR              fr_RejectPattern;
    APTR              fr_AcceptPattern;

    struct Hook      *fr_FilterFunc;
    BOOL	      fr_CallOldFilter;

    /* IMPORTANT! Keep the next field NOT longword-aligned, see note below */
    char	      fr_FileString[256];
    char	      fr_DrawerString[256];
    char	      fr_PatternString[130];
    struct Hook	      fr_FileHook;

    /* for custom listview */
    struct List       fr_FileList;
    UWORD	      fr_Pad2;
    struct MinList    fr_TempFileList;
    struct MinList    fr_TempDirList;
    ULONG             fr_ItemCnt;
    ULONG             fr_TempItemCnt;
    ULONG             fr_ItemHeight;
    ULONG             fr_TopItem;
    ULONG             fr_OldTopItem;
    ULONG             fr_OldItemCnt;
    ULONG             fr_VisibleItems;
    ULONG             fr_MaxNameLen;
    ULONG             fr_MaxSizeLen;
    ULONG             fr_LVTop;
    ULONG             fr_LVLeft;
    ULONG             fr_LVWidth;
    ULONG             fr_LVHeight;
    UBYTE             fr_LVTextPen;
    UBYTE             fr_LVBackPen;
    UBYTE             fr_LVHighTextPen;
    UBYTE             fr_LVFillTextPen;
    UBYTE             fr_LVFillBackPen;
    BOOLEAN           fr_DisplayValid;
    BOOLEAN           fr_FullRender;

    WORD              fr_LightLeft;
    WORD	      fr_LightTop;
    WORD	      fr_LightWidth;
    WORD	      fr_LightHeight;

    WORD              fr_ZoomOnFileSig;
};

/* The fr_FileString field must never be longword aligned for ancient
 * compatibility.
 *
 * When we were all younger, AslRequest() used to return a pointer to the file
 * name instead of a normal BOOL. This version of ASL keeps doing
 * this, but in order to ensure folks that use the return value of the file
 * req as a bool always work, the address of the name string must never
 * fall on a multiple of 64K in memory. Otherwise, the filename's address
 * could look something like 0x12340000. Folks using this value as a BOOL
 * (which is only a WORD in length) would see such a value as FALSE, even
 * though it is not. The solution is to misalign the name buffer so as to
 * make sure it is not longword aligned, and thus cannot fall on a 64K
 * boundary. Tadam!
 */


#define PUBLIC_FR(fr) ((struct FileRequester *)((ULONG)fr+sizeof(struct ReqInfo)))


/*****************************************************************************/


struct ExtFileReq *AllocFileRequest(struct TagItem *tagList);
APTR FileRequest(struct ExtFileReq *requester, struct TagItem *tagList);
VOID FreeFileRequest(struct ExtFileReq *requester);
struct ListEntry *FindListEntry (struct ExtFileReq *fr, STRPTR name);


/*****************************************************************************/

#endif /* FILEREQ_H */
