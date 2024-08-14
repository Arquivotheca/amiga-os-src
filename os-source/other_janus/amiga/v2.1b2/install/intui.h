/* things needed for intui.c */

#define FWIDTH	640
#define FHEIGHT	(200 - 10)


/* globals ***********************/

extern VOID Notice(struct Window *w,UBYTE *string1, UBYTE *string2);

extern VOID PrintMsg(UBYTE *msg);

extern LONG mark_file_list(struct List *list,	/* the list */
		    LONG *list_blocks,		/* blocks in list */
		    LONG *src_blocks,		/* blocks going in */
		    LONG *dest_blocks,		/* blocks available */
		    int del);

extern struct Screen *s;		/* our screen */
extern struct Window *w;		/* our window */
extern struct RastPort *rp;		/* our rastport */

