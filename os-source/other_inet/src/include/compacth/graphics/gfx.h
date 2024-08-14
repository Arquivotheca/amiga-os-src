€ˆGRAPHICS_GFX_H€GRAPHICS_GFX_H€BITSET 0x8000€BITCLR 0€AGNUS°AGNUS€TOBB(a) (()(a))€TOBB(a) (()(a)>>1)‡
ƒRectangle
{
•MinX,MinY;
•MaxX,MaxY;
};
¥ƒtPoint
{
˜x,y;
}Point;
¥Š*PLANEPTR;
ƒBitMap
{
‰BytesPerRow;
‰Rows;
Š¦;
ŠDepth;
‰pad;
PLANEPTR Planes[8];
};€RASSIZE(w,h) ((h)*((w+15)>>3&0xFFFE))‡