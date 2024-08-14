
#include <exec/types.h>
#include <graphics/gfx.h>
#include <stdlib.h>

struct GfxBase *GfxBase;
void PrintFirst(struct BitMap *BitMap)
{
int i,j;
   for (i=0; i<BitMap->Rows; i++)
   {
      for (j=0; j<BitMap->BytesPerRow/2; j++)
      {
         printf("%04x", ((UWORD *)BitMap->Planes[0])[i/2]);
      }
      printf("\n");
   }
}

void main(void)
{
   if ((GfxBase=(struct GfxBase *)OpenLibrary("graphics.library", 0))!=NULL)
   {
   struct BitMap *SrcBitMap, *BitMap;
      if ((SrcBitMap=AllocBitMap(16, 16, 1, BMF_CLEAR, NULL))!=NULL)
      {
         if ((BitMap=AllocBitMap(64, 16, 1, BMF_CLEAR, NULL))!=NULL)
         {
            printf("Source bitplane\n");
            PrintFirst(SrcBitMap);
            printf("Destination bitplane\n");
            PrintFirst(BitMap);
            BltBitMap(BitMap, 0, 0,
                      BitMap, 5, 0,
                      6, 16, 0xf0, ~0, NULL);
            WaitBlit();
            
            printf("After blitting\n");
            PrintFirst(BitMap);
            
            FreeBitMap(BitMap);
         }
         FreeBitMap(SrcBitMap);
      }
      CloseLibrary((struct Library *)GfxBase);
   }
}
