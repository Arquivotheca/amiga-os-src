/* Prototypes for functions defined in c/FatalRequest.c */
struct Library *_SetIntuition(void);
void _UndoIntuition(struct Library *IntuitionBase);
void SetFatalWindow(struct Window *win);
int _GetResponse(struct Library *IntuitionBase,
                 struct Window *win);
struct Window *_FRBuildEasyRequest(struct Library *IntuitionBase,
                                   struct Window *win,
                                   struct EasyStruct *es,
                                   ULONG idcmp, ...);
