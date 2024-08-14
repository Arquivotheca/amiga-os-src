#include <libraries/dos.h>
int IntuitionBase;
main()
{
	IntuitionBase=OpenLibrary("intuition.library",0);
	while(! (SetSignal(0,0) & SIGBREAKF_CTRL_C))
		RemakeDisplay();
}
