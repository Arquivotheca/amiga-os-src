#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/expansionbase.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/sprite.h>
#include <graphics/gfxmacros.h>
#include <libraries/filehandler.h>
#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <devices/input.h>
#include "all.h"

void kprintf(char *,...);
void sprintf(char *,char *,...);
#define  D(x) ;
#define printf kprintf

#define DEPTH     2
#define WIDTH     640
#define HEIGHT    200
#define VPMODES   HIRES|SPRITES

extern struct Custom custom;

struct Display {
   struct View v;
   struct ViewPort vp;
   struct ColorMap *cm;
   struct RasInfo ri;
   struct BitMap b;
   struct RastPort rp;
   BOOL   Success;
   struct TextFont *font8;
   struct TextFont *font9;
   };

struct IH {
   struct MsgPort    *port;
   struct IOStdReq   *msg;
   struct Interrupt  hstuff;
   };

struct Sprite {
   SHORT num;
   struct SimpleSprite sprite;
   APTR   image;
   SHORT  x,y;
   };

void NewList(struct List *);


struct IOStdReq  *CreateStdIO(struct MsgPort *ioReplyPort);
void              DeleteStdIO(struct IOStdReq *ioStdReq);
struct IORequest *CreateExtIO(struct MsgPort *ioReplyPort,LONG size);
void              DeleteExtIO(struct IORequest *ioExt);
struct MsgPort   *CreatePort(char *name,long pri);
void              DeletePort(struct MsgPort *port);
BOOL              DisplayOn(struct World *world, USHORT c0, USHORT c1, USHORT c2, USHORT c3);
void              DisplayOff(struct World *world);
void              OpenSprite(struct World *world);
void              CloseSprite(struct World *world);
void              DisSprite(struct World *world,int num);
BOOL              OpenIH(struct World *world,APTR mycode);
void              CloseIH(struct World *world);
void              DrawBox(struct World *world,SHORT l,SHORT t,SHORT w,SHORT h,SHORT m);
void              UpdateMouse(struct World *,struct InputEvent *); 
