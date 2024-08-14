
/* dnh.c */
LONG Tokenize ( struct GlobalData *gd , STRPTR name , ULONG *address );
ULONG ASM nodehost ( REG (a0 )struct Hook *h , REG (a2 )STRPTR o , REG (a1 )Msg msg );

/* screen.c */
STRPTR ScreenType ( struct GlobalData *gd , struct Screen *scr );
STRPTR DumpScreenFlags ( struct GlobalData *gd , struct Screen *scr );
void showscreenlist ( struct GlobalData *gd );
void showscreen ( struct GlobalData *gd , ULONG address );

/* bitmap.c */
STRPTR DumpBitMapFlags ( struct GlobalData *gd , struct Screen *scr );

/* main.c */
int main ( void );
APTR openamigaguide ( struct GlobalData *gd , struct NewAmigaGuide *nag , Tag tag1 , ...);
void bprintf ( struct GlobalData *gd , STRPTR fmt , void *arg1 , ...);
void vprintf ( struct GlobalData *gd , STRPTR fmt , void *arg1 , ...);
void vfprintf ( struct GlobalData *gd , BPTR fh , STRPTR fmt , void *arg1 , ...);
VOID DisplayError ( struct GlobalData *gd , LONG err );

/* class.c */
struct MinList *FindHead ( struct MinNode *node );
void showclasslist ( struct GlobalData *gd );

/* device.c */
void showdevicelist ( struct GlobalData *gd );
void showdevice ( struct GlobalData *gd , ULONG address );

/* library.c */
STRPTR ShowLibFlags ( struct GlobalData *gd , struct Library *lib );
void showlibrarylist ( struct GlobalData *gd );
void showlibrarybase ( struct GlobalData *gd , struct Library *lib );
void showlibrary ( struct GlobalData *gd , ULONG address );

/* window.c */
STRPTR DumpWindowFlags ( struct GlobalData *gd , struct Window *win );
void showwindow ( struct GlobalData *gd , ULONG address );

/* memory.c */
void showmemory ( struct GlobalData *gd , ULONG address );
