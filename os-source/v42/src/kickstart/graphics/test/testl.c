#include <exec/types.h>
#include <intuition/intuition.h>
extern ULONG DOSBase;

struct GfxBase *GfxBase;
struct IntuitionBase *IntuitionBase;

void main()
{
    if (IntuitionBase = OpenLibrary( "intuition.library", 0 ))
        {
            struct Window *w;
            if (w = OpenWindowTags( NULL,
                    WA_Left,        0,
                    WA_Top,         0,
                    WA_Width,       320,
                    WA_Height,      256,
                    WA_IDCMP,       MOUSEBUTTONS,
					WA_Borderless,-1,
                    TAG_END ))
                {
                Wait( 1L << w->UserPort->mp_SigBit );
                CloseWindow( w );
                }
        CloseLibrary( IntuitionBase );
        }
}
