/* Prototypes for functions defined in c/MenuTools.c */
struct MenuPac *InitMP(void);
void SetMenuHeight(struct MenuPac *m,
                   short height);
void SetMenuWidth(struct MenuPac *m,
                  short width);
void SetMenuFrontPen(struct MenuPac *m,
                     int num);
void SetMenuBackPen(struct MenuPac *m,
                    int num);
void SetMenuDrawMode(struct MenuPac *m,
                     ULONG flag);
void SetMenuHelpDirectory(struct MenuPac *pac,
                          char *directory);
void SetMenuHelpDisplay(struct MenuPac *pac,
                        void (*funct)(void));
void DInitMP(struct MenuPac *pac);
struct Menu *SetMenu(struct MenuPac *pac,
                     char *name,
                     int sep);
struct MenuPac *_FindMenu(struct MenuPac *pac,
                          int num);
struct MenuItem *_FindMenuItem(struct MenuItem *mi,
                               int num);
struct IntuiText *_CreateIText(struct MenuPac *m,
                               struct Remember **key,
                               char *text);
struct MenuItemExt *SetMenuItem(struct MenuPac *pac,
                                char *name,
                                USHORT flags,
                                LONG exc,
                                UBYTE cmd,
                                int width,
                                BOOL (*mfunct)(void),
                                ULONG itemdata,
                                char *help,
                                void (*helpfunct)(void));
struct MenuItemExt *SetSubItem(struct MenuPac *pac,
                               char *name,
                               USHORT flags,
                               LONG exc,
                               UBYTE cmd,
                               int width,
                               BOOL (*mfunct)(void),
                               ULONG itemdata,
                               char *help,
                               void (*helpfunct)(void));
void EnableMenu(struct MenuPac *mp,
                struct Window *win,
                UBYTE *s);
void EnableMenuItem(struct MenuPac *mp,
                    struct Window *win,
                    int mno,
                    UBYTE *s);
void EnableSubItem(struct MenuPac *mp,
                   struct Window *win,
                   int mno,
                   int ino,
                   UBYTE *s);
BOOL DoMenu(struct MenuPac *mp,
            ULONG code,
            APTR data1,
            APTR data2,
            BOOL boolhelp);
