#include <exec/types.h>
#include <clib/exec_protos.h>
#include "/view.h"
#include <graphics/videocontrol.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <stdio.h>

/* A3000+ benchmarks:
	320x200x256  0 colors loaded ks 39.1234 test 2761 / 4528  ( NoICLUpdate / normal ) 
	1280x200x256 0 colors loaded ks 39.1234 test 3930 / 5785
	320x200x256  0 colors loaded ks 39.116  test 3741 / 3741
	1280x200x256 0 colors loaded ks 39.116  test 6961 / 6961
*/



struct Library *GfxBase,*IntuitionBase;
struct Screen *myscreen;

int oldp;

void Fail(char *msg)
{
	SetTaskPri(FindTask(0),oldp);
	if (msg) printf("%s\n",msg);
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



ULONG ar1=0;

main(argc,argv)
int argc;
char **argv;
{
	int i;
	ULONG bp1=0;
	GfxBase=openlib("graphics.library",39);
	IntuitionBase=openlib("intuition.library",37);
	myscreen=OpenScreenTags(0l,SA_Width,320,SA_Height,200,SA_Depth,8,
				SA_DisplayID,0x0,TAG_END);

//	VideoControlTags(myscreen->ViewPort.ColorMap,VC_IntermediateCLUpdate,0,0);
	printf("auxflags=%ld\n",myscreen->ViewPort.ColorMap->AuxFlags);
	oldp=SetTaskPri(FindTask(0),30);
	WaitTOF(); WaitTOF();
	for(i=0;i<100;i++)
	{
		WaitTOF();
		LoadRGB32(&(myscreen->ViewPort),&ar1);
		bp1+=VBeamPos();
	}
	printf("sum=%ld\n",bp1);

	Fail(0);
}

