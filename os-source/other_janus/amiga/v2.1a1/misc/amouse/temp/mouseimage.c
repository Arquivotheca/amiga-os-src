/************************************************************************
 *
 * MouseImage.c
 *
 ************************************************************************/
#include <exec/types.h>
#include <exec/memory.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <graphics/sprite.h>

#define REV      0L
#define SPRHEIGHT   16
#define WORDSPERSPR   (2 * SPRHEIGHT + 4)

static   UWORD   Colour1 = -1,
      Colour3 = -1;
static   int   SpriteStart;

static   short   StandardImage[] =
   {
   0x3fff,
   0x1fff,
   0x0fff,
   0x07ff,
   0x03ff,
   0x01ff,
   0x00ff,
   0x007f,
   0x003f,
   0x001f,
   0x01ff,
   0x10ff,
   0x30ff,
   0xf87f,
   0xf87f,
   0xfc3f,
   
   0x0000,
   0x4000,
   0x6000,
   0x7000,
   0x7800,
   0x7c00,
   0x7e00,
   0x7f00,
   0x7f80,
   0x78c0,
   0x7c00,
   0x4600,
   0x0600,
   0x0300,
   0x0300,
   0x0180,
   
   };   /* end of StandardImage */


struct SimpleSprite      MouseImage;
UWORD            *SpriteData;
UWORD            *NullData;
extern   struct   Window      *Window;
extern   struct   IntuitionBase   *IntuitionBase;
static   struct   Window      *MouseWindow = NULL;
static   struct   ViewPort   *MouseVPort = NULL;
static   short         Shown;
static   short         TextMode = FALSE;

void MoveMouseImage();
void DelMouseImage ();
void die();
void ToggleMouseImage();
void SetMouseImage();
void SetVideoMode();
 
/************************************************************************
 *
 * MoveMouseImage
 *
 ************************************************************************/
void MoveMouseImage(X,Y)
int   X,
   Y;
   {
   if( TextMode )
      return;
   if( MouseWindow
       &&
       MouseWindow->WScreen->ViewPort.Modes & HIRES
      )
      X &= 0xFFFFFFFE;   /* even increments in HIRES */
   else
      X >>= 1;
   WaitTOF();
   if( MouseWindow )
      {
      X += MouseWindow->LeftEdge+ MouseWindow->BorderLeft;
      Y += MouseWindow->TopEdge+  MouseWindow->BorderTop;
      }
   MoveSprite(MouseVPort,&MouseImage,X,Y);
   }   /* end of MoveMouseImage */


/************************************************************************
 *
 * DelMouseImage
 *
 ************************************************************************/
void DelMouseImage ()
   {

   if(MouseImage.num)
      FreeSprite((long) MouseImage.num);

   if (SpriteData)
      FreeMem(SpriteData, 2L * (WORDSPERSPR+6));
   }

/************************************************************************
 *
 * die
 *
 ************************************************************************/
void die(str)
char *str;
   {
   if( Window )
      {
      SetWindowTitles(Window, str, -1L);
      WaitPort(Window->UserPort);
      }
   else
      puts (str);
   DelMouseImage();
   }   /* end of die */

/************************************************************************
 *
 * InitMouseImage
 *
 ************************************************************************/
InitMouseImage()
   {
   if( !(SpriteData = (UWORD *)AllocMem (2L * (WORDSPERSPR+6) ,
          MEMF_CHIP|MEMF_CLEAR)))
      {
      die("Can't allocate sprite buffer.\n");
      return(TRUE);
      }
   NullData = SpriteData+(WORDSPERSPR);   /* 6 WORDS 0 */
   if( (SpriteStart = GetSprite( &MouseImage, -1)) == -1)
      {
      die("Sprite allocation failed.\n");
      return(TRUE);
      }
   SpriteStart = ( (SpriteStart & 0x06) << 1 ) + 16;
   Shown = FALSE;
   MouseImage.x = 320;
   MouseImage.y = 100;
   NewMouseImage(NULL);
   SetMouseImage(FALSE);   /* initially off */
   TextMode = FALSE;   
   return(FALSE);
   }   /* end of InitMouseImage */

/************************************************************************
 *
 * ToggleMouseImage
 *
 ************************************************************************/
void ToggleMouseImage()
   {
   if( Shown )
      {
      MouseImage.height = 1;
      ChangeSprite(MouseVPort, &MouseImage, NullData);
      }
   else
      {
      MouseImage.height = SPRHEIGHT;
      ChangeSprite(MouseVPort, &MouseImage, SpriteData);
      }
   Shown = !Shown;
   }   /* end of ToggleMouseImage */

/************************************************************************
 *
 * NewMouseImage
 *
 ************************************************************************/
NewMouseImage(ImageData)
short   *ImageData;
   {
   
   UWORD *cw;   /* current position in buffer of sprite images */
   int scan;
   register   short   Work;
   if( !ImageData )
      ImageData = StandardImage;

   cw = SpriteData;   /* current position at top of buffer */
   *cw++ = 0;      /* poscntl */
   *cw++ = 0;
   for(   scan=0;
      scan<SPRHEIGHT;
      scan++)
      {
      Work = ~ImageData[scan];
      *cw++ = (Work & 0xC000 ? 0x8000 : 0 ) |
         (Work & 0x3000 ? 0x4000 : 0 ) |
         (Work & 0x0C00 ? 0x2000 : 0 ) |
         (Work & 0x0300 ? 0x1000 : 0 ) |
         (Work & 0x00C0 ? 0x0800 : 0 ) |
         (Work & 0x0030 ? 0x0400 : 0 ) |
         (Work & 0x000C ? 0x0200 : 0 ) |
         (Work & 0x0003 ? 0x0100 : 0 );
      Work  = ImageData[scan+SPRHEIGHT];
      *cw++ = (Work & 0xC000 ? 0x8000 : 0 ) |
         (Work & 0x3000 ? 0x4000 : 0 ) |
         (Work & 0x0C00 ? 0x2000 : 0 ) |
         (Work & 0x0300 ? 0x1000 : 0 ) |
         (Work & 0x00C0 ? 0x0800 : 0 ) |
         (Work & 0x0030 ? 0x0400 : 0 ) |
         (Work & 0x000C ? 0x0200 : 0 ) |
         (Work & 0x0003 ? 0x0100 : 0 );
      }
   *cw++ = 0;   /* termination */
   *cw++ = 0;
   if( Shown && !TextMode)
      {
      MouseImage.height = SPRHEIGHT;
      ChangeSprite(MouseVPort, &MouseImage, SpriteData);
      }

   return(0);

   }   /* end of NewMouseImage */

/************************************************************************
 *
 * SetMouseImage
 *
 ************************************************************************/
static   int   CurrentMode;

void SetMouseImage(On)
int   On;
   {
   UWORD      C1,
         C2;
   if( TextMode )
      return;
   if( On )
      {

         /* call SetVideoMode in case the Screen/Window has
            changed without the PC Side telling us. i.e. when
            the user/program changes the number of Colours.

      >>>>>>> SetVideoMode calls SetMouseImage(FALSE) <<<<<<<<

          */
      SetVideoMode(CurrentMode);
      MouseImage.height = SPRHEIGHT;
      ChangeSprite(MouseVPort, &MouseImage, SpriteData);
      if( MouseVPort )
         {   /* got a viewport - so change its colours */
         Colour1 = GetRGB4(MouseVPort->ColorMap,SpriteStart+1);
         Colour3 = GetRGB4(MouseVPort->ColorMap,SpriteStart+3);
         C1 = GetRGB4(MouseVPort->ColorMap,0);
         C2 = GetRGB4(MouseVPort->ColorMap,1);
         SetRGB4(MouseVPort,
            SpriteStart+1,
            (C1 >> 8) & 0x0f,
            (C1 >> 4) & 0x0f,
            (C1 ) & 0x0f);
         SetRGB4(MouseVPort,
            SpriteStart+3,
            (C2 >> 8) & 0x0f,
            (C2 >> 4) & 0x0f,
            (C2 ) & 0x0f);
         }
      }
   else
      {
      MouseImage.height = 1;
      ChangeSprite(MouseVPort, &MouseImage, NullData);
      if( MouseVPort 
          &&
          Colour1 != -1)
         {   /* got a viewport-so have changed its colours */
         SetRGB4(MouseVPort,
            SpriteStart+1,
            (Colour1 >> 8) & 0x0f,
            (Colour1 >> 4) & 0x0f,
            (Colour1 ) & 0x0f);
         SetRGB4(MouseVPort,
            SpriteStart+3,
            (Colour3 >> 8) & 0x0f,
            (Colour3 >> 4) & 0x0f,
            (Colour3 ) & 0x0f);
         Colour1 = -1;
         }
      }
   Shown = On;
   }   /* end of SetMouseImage */

/************************************************************************
 *
 * SetVideoMode
 *
 ************************************************************************/
static   char   PCMono[]   = " PC Monochrome Display ";
static   char   PCColour[]   = " PC Color Display  ";

void SetVideoMode(Mode)
int   Mode;
   {
   struct Screen   *WorkScreen;
   struct Window   *WorkWindow;
   char      *Title;
   short      Found = FALSE;
   if( CurrentMode != Mode )
      SetMouseImage(FALSE);   /* turn it off */
   CurrentMode = Mode;
   TextMode = FALSE;
   if( Mode == 7 )
      {
      Title    = PCMono;
      TextMode = TRUE;
      }
   else
      {
      Title    = PCColour;
      TextMode = Mode < 4;
      }

   MouseWindow = NULL;
   MouseVPort  = NULL;

   Forbid();

   for(   WorkScreen = IntuitionBase->FirstScreen;
      WorkScreen
      &&
      !Found;
      WorkScreen = WorkScreen->NextScreen)
      {
      for(   WorkWindow = WorkScreen->FirstWindow;
         WorkWindow
         &&
         !Found;
         WorkWindow = WorkWindow->NextWindow)
         {
         if( strcmp(WorkWindow->Title,Title) == 0 )
            {
            MouseWindow = WorkWindow;
            MouseVPort  = &(MouseWindow->WScreen->ViewPort);
            Found = TRUE;
            }
         }
      }
   
   Permit();
   if( TextMode )
      SetMouseImage(FALSE);      /* OFF */
   }   /* end of SetVideoMode */
