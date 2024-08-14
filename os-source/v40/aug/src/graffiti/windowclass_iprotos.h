
/* windowclass.c */
Class *initViewWindowClass ( void );
ULONG freeViewWindowClass ( Class *cl );
struct Window *OpenViewWindow ( Class *cl , Tag tag1 , ...);
void SetViewWindowAttrs ( Class *cl , struct Window *w , Tag tag1 , ...);
void GetCurrentTopValues ( Class *cl , struct Window *w , ULONG *toph , ULONG *topv );
void CloseViewWindow ( Class *cl , struct Window *w );
