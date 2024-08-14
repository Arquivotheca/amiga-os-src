#include <stdio.h>
#include <string.h>

#include <graphics/rastport.h>
#include <graphics/text.h>

struct Library *GfxBase, *DiskfontBase;
struct TextAttr ta = { "diamond.font", 9, FS_NORMAL, FPF_DISKFONT | FPF_DESIGNED };

int main (void)
{
    struct TextFont *font = NULL;
    struct RastPort rp;
    struct TextExtent tte, fte;
    char *str = "AAAAAA";
    short nfit;

    if (!(GfxBase = OpenLibrary ("graphics.library", 36))) goto clean;
    if (!(DiskfontBase = OpenLibrary ("diskfont.library", LIBRARY_MINIMUM))) goto clean;
    if (!(font = OpenDiskFont (&ta))) goto clean;

    InitRastPort (&rp);
    SetFont (&rp, font);

    TextExtent (&rp, str, strlen(str), &tte);

    printf ("TextExtent: %d chars: %d x %d  (%d,%d) - (%d,%d)\n", (int)strlen(str),
        tte.te_Width, tte.te_Height,
        tte.te_Extent.MinX, tte.te_Extent.MinY,
        tte.te_Extent.MaxX, tte.te_Extent.MaxY );

	printf("baseline=%ld\n",rp.TxBaseline);
    nfit = TextFit (&rp, str, strlen(str), &fte, &tte, 1, 32767,32767);

    /* if TextFit() works correctly, it will return strlen(str) as nfit.  For proportional
       fonts it returns strlen(str)-1. */

    printf ("TextFit Fwd: %d chars: %d x %d  (%d,%d) - (%d,%d)\n", nfit,
        fte.te_Width, fte.te_Height,
        fte.te_Extent.MinX, fte.te_Extent.MinY,
        fte.te_Extent.MaxX, fte.te_Extent.MaxY );

clean:
    if (font) CloseFont (font);
    if (DiskfontBase) CloseLibrary (DiskfontBase);
    if (GfxBase) CloseLibrary (GfxBase);

    return 0;
}
