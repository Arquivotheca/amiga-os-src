/* special.h -- wack extensions common include */
/* $Id: special.h,v 1.6 91/04/24 20:54:13 peter Exp $	*/

typedef void (*voidfunc)();

/* amiga core address for field in sun-buffered structure */
#define CADDR(base, buffer, field) 	\
    ( (ULONG) (base) + ((ULONG) (field) - (ULONG) (buffer)) )

APTR GetMemL();
APTR FindBase();

#define REFRESH flags = -1;

#define IBADDR(a) ((APTR) ((ULONG) (a) - (ULONG)(&IBase) + (ULONG)(ibase)))

/* builds callers to dumpT(addr, ptr) where T is an exec node */
#define SHOWMIN(T) \
	Show/**/T(arg) char *arg; \
	{ struct T *w;ULONG dump/**/T(); if (AddrArg(arg, &w)) { printf("\n");\
	doMinNode(w, sizeof (struct T), dump/**/T); }\
	else puts("\nbad syntax"); }\
	\
	Show/**/T/**/List(arg) char *arg; \
	{ struct T *w;ULONG dump/**/T(); if (AddrArg(arg, &w)) { printf("\n");\
	WalkMinList(w, sizeof (struct T), dump/**/T); }\
	else puts("\nbad syntax"); }


/* builds callers to dumpT(addr, ptr) where T is not a node	*/
#define SHOWONE(T) \
	Show/**/T(arg) char *arg; \
	{ struct T *w;ULONG dump/**/T(); if (AddrArg(arg, &w)) { printf("\n");\
	doMinNode(w, sizeof (struct T), dump/**/T); }\
	else puts("\nbad syntax"); }

/* builds callers to dumpT(addr, ptr) where T is a node in a linked list */
#define SHOWSIMPLE(T) \
	Show/**/T(arg) char *arg; \
	{ struct T *w;ULONG dump/**/T(); if (AddrArg(arg, &w)) { printf("\n");\
	doMinNode(w, sizeof (struct T), dump/**/T); }\
	else puts("\nbad syntax"); }\
	\
	Show/**/T/**/List(arg) char *arg; \
	{ struct T *w;ULONG dump/**/T(); if (AddrArg(arg, &w)) { printf("\n");\
	WalkSimpleList(w, sizeof (struct T), dump/**/T); }\
	else puts("\nbad syntax"); }

#if 0 	/* Here's what an example looks like	*/

SHOWSIMPLE( Window ) yields:

ShowWindow(arg)
char	*arg;
{
    struct Window	*w;

    ULONG	dumpWindow();
    if (AddrArg(arg, &w))
    {
	printf("\n");
	doMinNode( w, sizeof (struct Window), dumpWindow);
    }
    else puts("\nbad syntax");
}

ShowWindowList(arg)
char	*arg;
{
    struct Window	*w;

    ULONG	dumpWindow();
    if (AddrArg(arg, &w))
    {
	printf("\n");
	WalkSimpleList( w, sizeof (struct Window), dumpWindow);
    }
    else puts("\nbad syntax");
}

these call dumpWindow(APTR arg, struct Window *window);

#endif
