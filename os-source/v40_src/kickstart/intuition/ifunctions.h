
/*** ifunctions.h*************************************************************
 *
 *  ifunctions.h -- intuition (internal) function type declarations
 *
 *  $Id: ifunctions.h,v 38.0 91/06/12 14:20:25 peter Exp $
 *
 *  Confidential Information: Commodore-Amiga, Inc.
 *  Copyright (c) Commodore-Amiga, Inc.
 *
 ****************************************************************************/

/** function type declarations **/
struct RastPort *obtainRP();

struct TextFont *ISetFont();

BOOL realRequest();

struct Window *hitUpfront();
BOOL hitGadgets();
ULONG hitGGrunt();	/* in fact, must return GADGETON or 0 (FALSE) */

struct Point *gadgetPoint();
struct IBox *getImageBox();
BOOL	ptInBox();

USHORT getAllMenuOK();
USHORT getMenuOK();
USHORT getOK();

BOOL sizeDrag();
ISizeWindow();
IMoveWindow();
stretchIFrame();
dragIFrame();

struct Rectangle *boxToRect();

struct Menu *grabMenu();
struct MenuItem *grabItem();
struct MenuItem *grabSub();
struct MenuItem *ItemAddress();
SHORT hitGrunt();

USHORT potInRange();
USHORT potToTop();
USHORT potIncrement();

struct Layer *getGimmeLayer();

CPTR AllocRemember();
BYTE *AllocRaster();

struct Thing *previousLink();
struct Thing * nthThing();
struct Thing * lastThing();
struct Thing * previousThing();
struct Thing * delinkThing();

BOOL	IsHires();
BOOL	IsHedley();
BOOL	AreHedley();

struct MonitorSpec *newScreenMonitorSpec();

struct RastPort	*ObtainGIRPort();
#define FreeGIRPort( rp ) 	(freeRP( rp ) )

struct Screen *defaultPubScreen();
struct Screen *openSysScreen();

ULONG	getTagData();
