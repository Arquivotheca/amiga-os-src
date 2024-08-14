#include <exec/types.h>
struct Library *GfxBase;

int main (void)
{
    struct ColorMap *cm;
    ULONG rgb[3];

    if (!(GfxBase = OpenLibrary ("graphics.library", 39))) goto clean;

    if (cm = GetColorMap (16)) {
        SetRGB32CM (cm, 0, 0x18181818, 0x19191919, 0x1a1a1a1a);
        GetRGB32 (cm, 0, 1, rgb);
        printf ("%08lx %08lx %08lx\n", rgb[0], rgb[1], rgb[2]);
    }

clean:
    if (cm) FreeColorMap (cm);
    if (GfxBase) CloseLibrary (GfxBase);
    return 0;
}
