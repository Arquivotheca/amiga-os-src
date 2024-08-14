/* ====================================================================== */

/*
	SetCPU V1.5
	by Copyright 1989 by Dave Haynie

	COOLHAND MODULE

	This module contains the code used to display a cool looking 
	KickStart prompt hand on the screen.
*/

#include "setcpu.h"

/* ====================================================================== */

/* This is the KickStart hand design as in the A1000.  Only, Intuition makes
   it considerably easier to build than they way the Kernel ROM does it. */
   
/* Define the vectors.  The hand is a drawing composed of a series of line
   vectors.  Intuition provides a handy structure, called a boarder, that'll
   render this whole mess with a single function call. */

static WORD vect1[] = {  0,  0, 23,  0, 23, 22, 78, 22, 78,  0, 90,  0,101, 11,
                       101, 83, 92, 83, 92, 45, 29, 45, 27, 43, 18, 43, 17, 45,
                        10, 45, 10, 54,  0, 61,  0,  0 };
static WORD vect2[] = {  0,  0,  0,-62, 93,-62,105,-50,105, 23, 53, 23, 53, 28,
                        49, 36, 45, 41, 41, 44, 35, 53, 27, 57, 27, 68,-23, 68,
                       -23, 37,-24, 37,-24,  9,-20,  3,-13, -3,-12, -7, -8,-14,
                        -3,-17,  0,-18,  0,-18, -3,-16, -7,-14,-11, -7,-12, -3,
                       -19,  3,-23,  9,-23, 36,-22, 37,-22, 67,  7, 67,  7, 46,
                        15, 46, 19, 44, 19, 23, 17, 20, 17, 10, 32, -3, 32,-15,
                        29,-17, 26,-17, 29,-14, 29, -7, 28, -6, 21, -6, 18, -9,
                         9, -2, -3,  4,-15, 13,-15, 12, -3,  3, -7,  2,-10, -1,
                        -7,  1, -3,  2,  0,  0 };
static WORD vect3[] = {  0,  0,  2, -7, 10, -6, 11, -3, 11,  4,  4,  4,  0,  0 };
static WORD vect4[] = {  0,  0,  0,-10, 15,-23, 15,-35, 74,-35, 74,  2,  1,  2,
                         0, -2 };
static WORD vect5[] = {  0,  0, 51,  0, 51, 21,  0, 21,  0,  0 };
static WORD vect6[] = {  0,  0, 11,  0, 11, 14,  0, 14,  0,  0 };
static WORD vect7[] = {  0,  0,  7,  0,  7, 12,  0, 12,  0,  0 };
static WORD vect8[] = {  0,  0,  3,  0,  3,  2,  0,  6,  0,  0 };
static WORD vect9[] = {  0,  0,  0,-20,  7,-20, 12,-25, 12,-34, 16,-32, 24,-32,
                        24,-30, 28,-25, 32,-25, 32,-23, 26,-14, 18,-11, 18,  0,
                         0,  0 };
static WORD vect10[] = { 0,  0,  0,  5,  5,  2,  0,  0 };
static WORD vect11[] = { 0,  0, -4,  2, -4, -3, 17, -3,  7,  7,  6,  7,  8,  5,
                         3,  0,  0,  0 };

static WORD vect12[] = { 0,  0,  7,  0,  7,  5,  3, 13,  0, 17, -5, 18, -8, 18,
                       -11, 14,-11, 11,  0,  0 };

static WORD vect13[] = { 0,  0,  0,  3,  2,  6,  2,  6,  8,  5,  9,  2,  5, -1,
                         0,  0 };
static WORD vect14[] = { 0,  0,  8,  2, 10, -1,  6, -5,  3, -5,  0, -3,  0,  0 };
static WORD vect15[] = { 0,  0, 11, 11, 11, 83 };

static struct Border HandLines[] = {
   { 105, 51,1,0,JAM1,18, &vect1[0], &HandLines[1] },
   { 103,112,1,0,JAM1,59, &vect2[0], &HandLines[2] },
   { 120,101,1,0,JAM1, 7, &vect3[0], &HandLines[3] },
   { 121,132,1,0,JAM1, 8, &vect4[0], &HandLines[4] },
   { 130, 51,1,0,JAM1, 5, &vect5[0], &HandLines[5] },
   { 166, 54,1,0,JAM1, 5, &vect6[0], &HandLines[6] },
   { 168, 55,1,0,JAM1, 5, &vect7[0], &HandLines[7] },
   { 117, 97,1,0,JAM1, 5, &vect8[0], &HandLines[8] },
   { 111,179,1,0,JAM1,15, &vect9[0], &HandLines[9] },
   { 123,135,1,0,JAM1, 4,&vect10[0],&HandLines[10] },
   { 127,138,1,0,JAM1, 9,&vect11[0],&HandLines[11] },
   { 148,135,1,0,JAM1,10,&vect12[0],&HandLines[12] },
   { 138,146,1,0,JAM1, 8,&vect13[0],&HandLines[13] },
   { 124,144,1,0,JAM1, 7,&vect14[0],&HandLines[14] },   
   { 196, 51,1,0,JAM1, 3,&vect15[0],NULL },
};

/* This is a list of area fills to perform; the first number is the "A" pen
   to set, the next two numbers are the screen position to start filling
   from. */

static WORD FillList[][3] = {
   { 2, 107,109 }, { 3, 132, 71 }, { 2, 169, 66 }, { 1, 104, 51 },
   { 1, 129, 51 }, { 1, 167, 55 }, { 1, 176, 67 }, { 1, 182, 55 },
   { 1, 196,134 }, { 1, 145,136 }, { 1, 116, 97 }
};

#define FILLCNT	11

/* Finally, I've got a few images to render, which I do using the standard
   Intuition Image structures. */

UWORD image0[] = {	/* AMIGA */
   0x1f9f,0xfe3f,0xff77,0xfe7c, 0x070c,0xef8f,0x1e71,0xdc30,
   0x07f8,0xe7ce,0x1cf3,0x1fe0, 0x0731,0xf3ce,0x1df6,0x1cc0,
   0x0760,0x03dc,0x3ffc,0x1d80, 0x07c0,0x879c,0x3f78,0x1f00,
   0x0780,0xcf78,0x7e78,0x1e00, 0x0700,0xfeff,0xfc7c,0x1c00,
   0x1001,0x8220,0x4050,0x0004, 0x0404,0x8889,0x1210,0x5010,
   0x0408,0x2448,0x1090,0x1020, 0x0410,0x1042,0x0510,0x1040,
   0x0420,0x0050,0x2690,0x1080, 0x0440,0x0484,0x0900,0x1100,
   0x0480,0x4960,0x4208,0x1200, 0x0500,0x0683,0x8404,0x1400,
};

UWORD image1[] = {	/* Kick */
   0xc71f,0x1e63,0x8000, 0x6631,0x8c63,0x0000, 0x3e01,0x8c33,0x0000, 
   0x6631,0x8c1f,0x0000, 0xc61f,0x0e33,0x0000, 0x0600,0x0063,0x0000,
   0x0700,0x0c63,0x8000
};

UWORD image2[] = {	/* Start */
   0x703c,0xdc38,0x3e00, 0x9818,0x664c,0x6000, 0x1998,0x780c,0x3c00,
   0x19b8,0x600c,0x0600, 0x7cdc,0x3c3e,0x7c00, 0x1800,0x000c,0x0000,
   0x1000,0x0008,0x0000
};

struct Image NameImage[] = {
   { 126,122,64,8,2,&image0[0],3,0,&NameImage[1] },
   { 144,112,48,7,1,&image1[0],2,0,&NameImage[2] },
   { 140,102,48,7,1,&image2[0],2,0,NULL }   
};

/* Here's the color scheme. */

static WORD coolColor[] = { 0x0fff,0x0000,0x077c,0x0bbb };

/* ====================================================================== */

/* This section contains data for the mechanics of the CoolHand. */

static struct NewScreen newScreen = {
   0,0,320,200,2,0,1,0,CUSTOMSCREEN,NULL,NULL,NULL,NULL
};

static struct NewWindow newWindow = {
   0,0,320,200,2,0,0,BORDERLESS|ACTIVATE,NULL,NULL,NULL,NULL,NULL,0,0,0,0,
   CUSTOMSCREEN
};

static struct Screen	*coolScreen;
static struct Window	*coolWindow, *coolShade;

struct GfxBase		*GfxBase;
struct IntuitionBase	*IntuitionBase;

/* ====================================================================== */

/* This is the main section.  Since I consider the CoolHand prompt the end
   of this life and the beginning of the next, I'm not going to worry about
   freeing stuff up. */
   
struct Window *CoolHand() {
   struct RastPort *rp;
   struct ViewPort *vp;
   struct Image *image;
   LONG size;
   UWORD i,*bits,*pointer;
   BOOL ok = TRUE;

   GfxBase = (struct GfxBase *)OpenLibrary("graphics.library",0L);
   IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library",0L);
   if (!GfxBase || !IntuitionBase) return;
   if (!(coolScreen = OpenScreen(&newScreen))) return;
   ShowTitle(coolScreen,0L);
   LoadRGB4(vp = &coolScreen->ViewPort,&coolColor[0],4L);
   newWindow.Screen = coolScreen;
   pointer = (UWORD *)AllocMem(64L,MEMF_CHIP|MEMF_CLEAR);
   if (!(coolShade = OpenWindow(&newWindow))) return;
   if (!(coolWindow = OpenWindow(&newWindow))) return;
   WindowToFront(coolShade);
   SetPointer(coolWindow,pointer,2L,2L,0L,0L);
   Delay(25L);
   DrawBorder(rp = coolWindow->RPort,&HandLines[0],0L,0L);

   SetOPen(rp,1L);
   for (i = 0; i < FILLCNT; ++i) {
      SetAPen(rp,(long)FillList[i][0]);
      Flood(rp,0L,(long)FillList[i][1],(long)FillList[i][2]);
   }
   for (image = &NameImage[0]; image; image = image->NextImage) {
      size = (LONG) (((image->Width+15)/16)*image->Height)*2*image->Depth;
      movmem((char *)image->ImageData,
             (char *)(bits = (UWORD *)AllocMem(size,MEMF_CHIP)),
             (int)size);
      image->ImageData = bits;
   }
   DrawImage(rp,&NameImage[0],0L,0L);
   CloseWindow(coolShade);
   return coolWindow;
}
