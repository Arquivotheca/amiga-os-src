��GRAPHICS_GFX_H�GRAPHICS_GFX_H�BITSET 0x8000�BITCLR 0�AGNUS�AGNUS�TOBB(a) ((�)(a))��TOBB(a) ((�)(a)>>1)�
�Rectangle
{
�MinX,MinY;
�MaxX,MaxY;
};
��tPoint
{
�x,y;
}Point;
��*PLANEPTR;
�BitMap
{
�BytesPerRow;
�Rows;
��;
�Depth;
�pad;
PLANEPTR Planes[8];
};�RASSIZE(w,h) ((h)*((w+15)>>3&0xFFFE))�