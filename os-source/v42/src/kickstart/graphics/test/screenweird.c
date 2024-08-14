#include <graphics/displayinfo.h>
#include <intuition/intuition.h>
 
UWORD Pens[] = {0,1,1,2,1,3,1,0,3};
 
struct TagItem ScreenTags[] = {
        {SA_Width,100},
        {SA_Height,100},
        {SA_Depth,2},
        {SA_SysFont,0},
        {SA_Type,CUSTOMSCREEN},
        {SA_FullPalette,1},
        {SA_Title,&"Testing"},
        {SA_Pens,&Pens},
        {SA_DisplayID,HIRESLACE_KEY}};
 
struct IntuitionBase *IntuitionBase;
 
struct Screen *MyScreen;
 
int main()
        {
        register int p = 10000000;
        
        if (!(IntuitionBase = (struct IntuitionBase *) OpenLibrary("intuition.library",36)
))
                {
                puts("Couldn't open Intuition!");
                exit(10);
                }
        if (!(MyScreen = OpenScreenTagList(NULL,ScreenTags)))
                {
                CloseLibrary(IntuitionBase);
                puts("Couldn't open custom screen!");
                exit(10);
                }
        while (--p); /* (how lazy..) */
        CloseScreen(MyScreen);
        }
