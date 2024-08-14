
struct PrivateListReq {
	WORD			LeftEdge;
	WORD			TopEdge;
	WORD			Width;
	WORD			Height;
	APTR			UserData;
	char			*TitleText;
	char			*PosText;
	char			*NegText;
	char			*Buffer;
	ULONG			(*HookFunc)(ULONG, void *, void *);
	struct Screen	*Screen;
	struct Window	*Window;
	struct Gadget	*OKGadget,
					*CancelGadget,
					*ListGadget,
					*StrGadget;
	struct List		*List;
	LONG			Active;
	UWORD			BufferSize;
	UWORD			Flags;
};

#define LISTREQF_NEWIDCMP	(1<<0)

