/* Prototypes for functions defined in c/WindowTools.c */
void _InitWT(void);
struct Screen *CreateScreen(SHORT width,
                            SHORT height,
                            SHORT depth,
                            USHORT modes);
void SetScrLeftTop(int left,
                   int top);
void SetScrPens(int detail,
                int block);
void SetScrType(USHORT type);
void SetScrFont(struct TextAttr *font);
void SetScrTitle(UBYTE *title);
void SetScrGadgets(struct Gadget *gadgets);
struct TagItem *AllocScreenTag(ULONG entries);
struct TagItem *SetScreenTag(ULONG tag,
                             ULONG data);
struct Window *CreateWindow(struct Screen *scr,
                            struct Remember **key,
                            SHORT width,
                            SHORT height,
                            UBYTE *title,
                            USHORT maxvectors);
void SetWindowGadgets(struct Gadget *g);
void SetWinLeftTop(int left,
                   int top);
void SetWinIDCMP(ULONG iflags);
void SetWinFlags(ULONG flags);
void SetWinWH(int minwidth,
              int minheight,
              int maxwidth,
              int maxheight);
struct TagItem *AllocWindowTag(ULONG entries);
struct TagItem *SetWindowTag(ULONG tag,
                             ULONG data);
void SetWindowPointer(struct Window *win,
                      struct btp *btp);
void ClearWindowPointer(struct Window *win);
