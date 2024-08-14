#include <exec/types.h>
#include <graphics/gfx.h>
#include "/view.h"
#include <intuition/intuition.h>
#include <intuition/screens.h>

struct Library *GfxBase,*IntuitionBase;
struct Screen *myscreen;
struct BitMap *extrabitmap;
struct DBufInfo *myDBI;

struct DBufInfo *AllocDBufInfo(struct ViewPort *vp);
VOID FreeDBufInfo(struct DBufInfo *dbi);

/*#define DEBUG printf */
#define DEBUG if (0) printf

void Fail(char *msg)
{
	int i;
	if (msg) printf("%s\n",msg);
	DEBUG("freeing frames\n");
	DEBUG("freeing 2nd buffer");
	if (extrabitmap) FreeBitMap(extrabitmap);
	DEBUG("freedbi\n");
	FreeDBufInfo(myDBI);	/* null free OK */
	DEBUG("closing screen\n");
	if (myscreen) CloseScreen(myscreen);
	if (GfxBase) CloseLibrary(GfxBase);
	if (IntuitionBase) CloseLibrary(IntuitionBase);
	exit(0);
}


struct Library *openlib(char *name,ULONG version)
{
	struct Library *t1;
	t1=OpenLibrary(name,version);
	if (! t1)
	{
		printf("error- needs %s version %d\n",name,version);
		Fail(0l);
	}
	else return(t1);
}



#define SETCOLOR(c) *((UWORD *)0xdff180)=c

main()
{
	ULONG time[2],newtime[2];
	ULONG i,j;
	struct BitMap *bptrs[2];
	GfxBase=openlib("graphics.library",39);
	IntuitionBase=openlib("intuition.library",37);

	myscreen=OpenScreenTags(0l,SA_Width,320,SA_Height,200,SA_Depth,8,
			SA_DisplayID,0x800);
	if (! myscreen) Fail("couldn't open screen");
	myDBI=AllocDBufInfo(&(myscreen->ViewPort));
	if (! myDBI) Fail("Coudn't allocate a DBufInfo");
	if (! (extrabitmap=AllocBitMap(320,200,8,BMF_DISPLAYABLE,0l)))
			Fail("couldn't allocate second bitmap");

	bptrs[0]=&(myscreen->BitMap);
	bptrs[1]=extrabitmap;
	CurrentTime(time,time+1);
	for(i=0;i<40000;i++)
	{
		SETCOLOR(0xf00);
		ChangeVPBitMap(&(myscreen->ViewPort),bptrs[i & 1],myDBI);
		SETCOLOR(0);
		for(j=0;j<5000;j++) ;
	}
	CurrentTime(newtime,newtime+1);
	printf("secs=%d\n",newtime[0]-time[0]);
	Fail("done");
}
