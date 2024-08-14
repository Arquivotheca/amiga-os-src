;/* clockster.c - Execute me to compile me with Lattice 5.04
LC -b0 -cfistq -v -j73 clockster.c
Blink FROM LIB:rstartup.obj,clockster.o TO clockster LIBRARY LIB:Amiga.lib,LIB:LC.lib
quit
*/

#include <exec/types.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>

#ifdef LATTICE
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>
#include <clib/graphics_protos.h>
#include <stdlib.h>
#include <string.h>
#endif


#define MINARGS 1

UBYTE *vers = "\0$VER: clockster 37.5";
UBYTE *Copyright = 
  "clockster v37.5\nCopyright (c) 1990 Commodore-Amiga, Inc.  All Rights Reserved";
UBYTE *usage = "Usage: clockster";

void sprintf(UBYTE *buf, UBYTE *fmt,... );

#define THEDELAY 50
#define WIDE 100
#define HIGH 20

struct   NewWindow      nw = {
   100,10,                                /* LeftEdge and TopEdge */
   WIDE,HIGH,                             /* Width and Height */
   -1, -1,                                /* DetailPen and BlockPen */
   NULL,                                  /* IDCMP Flags with Flags below */
   SIMPLE_REFRESH|NOCAREREFRESH,
   NULL, NULL,                            /* Gadget and Image pointers */
   NULL,              			  /* Title string */
   NULL,                                  /* Screen ptr null till opened */
   NULL,                                  /* BitMap pointer */
   WIDE, HIGH,                            /* MinWidth and MinHeight */
   WIDE, HIGH,                            /* MaxWidth and MaxHeight */
   CUSTOMSCREEN                           /* Type of window */
   };

LONG year, month, day, hour, dayoweek;
LONG days, minutes, seconds, olddays, oldminutes, oldseconds;
UBYTE *ampm;
UBYTE *daynames[] = 
   {" Sunday "," Monday ","Tuesday ","Wednesday","Thursday"," Friday ","Saturday"};

struct Library *IntuitionBase, *GfxBase;

void getTime(void);
void bye(UBYTE *s, int e);
void cleanup(void);


void main(int argc, char **argv)
    {
    struct Screen *front;
    struct Window *win;
    struct RastPort *rp;
    UBYTE ybuf[8], buf[80];

    if(((argc)&&(argc<MINARGS))||(argv[argc-1][0]=='?'))
	{
	sprintf(buf,"%s\n%s\n",Copyright,usage);
	Write(Output(),buf,strlen(buf));
	bye("",RETURN_OK);
	}
    IntuitionBase = OpenLibrary("intuition.library",0);
    GfxBase = OpenLibrary("graphics.library",0);
    if((!IntuitionBase)||(!GfxBase)) bye("Can't get a library open\n",RETURN_FAIL);

    front = ((struct IntuitionBase *)IntuitionBase)->FirstScreen;
    nw.Screen = front;

    if(win = OpenWindow(&nw))
	{
	getTime();
	rp = win->RPort;
	SetAPen(rp,1);
	SetDrMd(rp,JAM2);

	sprintf(buf,"%ld:%02ld %s",hour,minutes,ampm);
	Move(rp,16,12);
	Text(rp,buf,strlen(buf));
	Delay(THEDELAY);

	sprintf(ybuf,"%ld",year);
	sprintf(buf,"%ld/%ld/%s  ",month,day,&ybuf[2]);
	Move(rp,16,12);
	Text(rp,buf,strlen(buf));
	Delay(THEDELAY);

	sprintf(buf,"%s",daynames[dayoweek]);
	Move(rp,16,12);
	Text(rp,buf,strlen(buf));
	Delay(THEDELAY);
	
	CloseWindow(win);
	}
    bye("",RETURN_OK);
    }


void bye(UBYTE *s, int e)
    {
    cleanup();
    exit(e);
    }

void cleanup()
    {
    if(GfxBase)	CloseLibrary(GfxBase);
    if(IntuitionBase)	CloseLibrary(IntuitionBase);
    }


void getTime()
   {
   struct DateStamp ds;
   LONG ticks, m, d, y, n;

   ampm = "AM";

   olddays = days;
   oldminutes = minutes;
   oldseconds = seconds;

   DateStamp(&ds);
   ticks   = ds.ds_Tick;
   days    = ds.ds_Days;
   minutes = ds.ds_Minute;

   dayoweek = days % 7;

   seconds = ticks / 50;
   hour   = minutes / 60;
   if (hour >= 12)
	{
	hour = hour % 12;
	ampm = "PM";
	}

   if (hour==0)    hour = 12;
   minutes = minutes % 60;


   n = days - 2251 ;
   y = (4 * n + 3) / 1461 ;
   n -= 1461 * (long)y / 4 ;
   y += 1984 ;
   m = (5 * n + 2) / 153 ;
   d = n - (153 * m + 2) / 5 + 1 ;
   m += 3 ;
   if (m > 12)
      {
      y++ ;
      m -= 12 ;
      }
   year = y;
   month = m;
   day = d;
   }

