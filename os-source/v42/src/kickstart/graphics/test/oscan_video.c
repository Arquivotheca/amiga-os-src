#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/displayinfo.h>

int IntuitionBase;

main()
{
	struct Screen *screen;
	struct Window *window;
	IntuitionBase=OpenLibrary("intuition.library",0);
    screen = OpenScreenTags ( NULL,
                              SA_DisplayID,   HIRES_KEY,
                              SA_Overscan,    OSCAN_VIDEO,
                              SA_Depth,       2L,
                              SA_Title,       "My Screen",
                              TAG_END );

    window = OpenWindowTags ( NULL,
                              WA_CustomScreen,  screen,
                              WA_Flags,         WFLG_CLOSEGADGET | WFLG_SMART_REFRESH | WFLG_NOCAREREFRESH,
                              WA_IDCMP,         IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY,
                              TAG_END );
	Delay(50*30);
	CloseWindow(window);
	CloseScreen(screen);
}
