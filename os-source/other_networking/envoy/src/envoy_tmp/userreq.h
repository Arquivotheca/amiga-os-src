struct IntUserInfo
{
	struct Node	Node;
	UBYTE		UserName[32];
};

struct IntGroupInfo
{
	struct Node	Node;
	UBYTE		GroupName[32];
	UBYTE	   	ListName[48];
};

struct UserReqData
{
	struct Window	*urd_Window;
	struct Window	*urd_BlockWindow;
	struct Requester *urd_Requester;
	struct MsgPort	*urd_RefreshPort;
	struct Hook	*urd_Hook;
	struct Screen 	*urd_Screen;
	struct Library	*urd_AccountsBase;
	APTR		 urd_VisualInfo;

	struct TextFont *urd_SysFont;
	struct TextAttr *urd_SysFontAttr;
	UWORD		 urd_MinWidth;
	UWORD		 urd_MinHeight;
	UWORD		 urd_TopOffset;
	UWORD		 urd_BottomBorder;

	struct Gadget	*urd_GadList;
	struct Gadget	*urd_LastGadget;
	struct Gadget	*urd_ListView;
	struct Gadget	*urd_OK;
	struct Gadget	*urd_Cancel;
        struct TagItem	*urd_TagItem;
        struct TagItem	*urd_TagList;
        struct IntuiText urd_IText;
	STRPTR		 urd_UserBuff;
	ULONG		 urd_UserBuffLen;
	STRPTR		 urd_GroupBuff;
	ULONG		 urd_GroupBuffLen;

	ULONG		 urd_Seconds;
	ULONG		 urd_Micros;
	ULONG		 urd_ICode;

	UBYTE		 urd_LVFmt[32];

	struct MinList	 urd_UserList;
	struct MinList   urd_GroupList;
	struct MinList   urd_ListviewList;

	struct UserInfo	*urd_UserInfo;
	struct GroupInfo *urd_GroupInfo;

	struct TextAttr  urd_LVTextAttr;
	ULONG		 urd_Pad0;
	UBYTE		 urd_LVTextName[64];
	STRPTR		 urd_Title;

	ULONG		 urd_Mask;
	ULONG		 urd_BlockMask;

	UBYTE		 urd_NameStr[32];

	UWORD		 urd_Left;
	UWORD		 urd_Top;
	UWORD		 urd_Width;
	UWORD		 urd_Height;

	UWORD		 urd_SizeGadget;
	UWORD		 urd_DragBar;
	BOOL		 urd_CustomScreen;
	BOOL		 urd_IsUP;

	ULONG		 urd_Status;
};
