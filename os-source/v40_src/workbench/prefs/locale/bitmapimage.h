struct BitMapImage
{
    UWORD LeftEdge;
    UWORD TopEdge;
    UWORD Width;
    UWORD Height;
    struct BitMap *BitMap;
    UBYTE *Compress;
};

struct Images
{
    UWORD i_Offset;
    UWORD i_XPad;
    UWORD i_Width;
    WORD i_Data;
};
