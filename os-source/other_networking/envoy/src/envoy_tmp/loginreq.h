
struct LoginReqData
{
	struct Window	*lrd_Window;
	struct Window	*lrd_BlockWindow;
	struct Requester *lrd_Requester;
	struct MsgPort	*lrd_RefreshPort;
	struct Hook	*lrd_Hook;
	struct Screen 	*lrd_Screen;
	APTR		*lrd_VisualInfo;

	struct TextFont *lrd_SysFont;
	struct TextAttr *lrd_SysFontAttr;
	struct TextFont *lrd_ReqFont;
	struct TextAttr *lrd_ReqFontAttr;
	UWORD		 lrd_FontSpace;
	UWORD		 lrd_FontHeight;
	UWORD		 lrd_BigSpace;
	UWORD		 lrd_BigHeight;
	UWORD		 lrd_OptimalWidth;
	UWORD		 lrd_OptimalHeight;
	UWORD		 lrd_MinWidth;
	UWORD		 lrd_MinHeight;
	UWORD		 lrd_TopOffset;
	UWORD		 lrd_BottomBorder;

	struct Gadget	*lrd_GadList;
	struct Gadget	*lrd_LastGadget;
	struct Gadget	*lrd_UserName;
	struct Gadget	*lrd_Password;
	struct Gadget	*lrd_OK;
	struct Gadget	*lrd_Cancel;
        struct TagItem	*lrd_TagItem;
        struct TagItem	*lrd_TagList;
        struct IntuiText lrd_IText;
	STRPTR		 lrd_NameBuff;
	ULONG		 lrd_NameBuffLen;
	STRPTR		 lrd_PassBuff;
	ULONG		 lrd_PassBuffLen;
	STRPTR		 lrd_Title;

	ULONG		 lrd_Mask;
	ULONG		 lrd_BlockMask;

	UBYTE		 lrd_NameStr[32];
	UBYTE		 lrd_PassStr[32];

	UWORD		 lrd_Left;
	UWORD		 lrd_Top;
	UWORD		 lrd_Width;
	UWORD		 lrd_Height;

	UWORD		 lrd_SizeGadget;
	UWORD		 lrd_DragBar;
	BOOL		 lrd_CustomScreen;
	BOOL		 lrd_IsUP;

	ULONG		 lrd_Status;

	struct Hook	 lrd_EditHook;
};
