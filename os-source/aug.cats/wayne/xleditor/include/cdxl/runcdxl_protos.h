/***************

 runcdxl_protos.h

 W.D.L 930330

***************/

/*
 * COPYRIGHT: Unless otherwise noted, all files are Copyright (c) 1993
 * Commodore-Amiga, Inc.  All rights reserved.
 *
 * DISCLAIMER: This software is provided "as is".  No representations or
 * warranties are made with respect to the accuracy, reliability, performance,
 * currentness, or operation of this software, and all use is at your own risk.
 * Neither commodore nor the authors assume any responsibility or liability
 * whatsoever with respect to your use of this software.
 */

// Display.c
ULONG GetModeID( DISP_DEF * disp_def );
int ScrWinOpen( DISP_DEF * disp_def, UBYTE * ilbmfile, struct TagItem * tags );
struct ViewExtra * GetVX( ULONG ModeID );
int ViewOpen( DISP_DEF * disp_def, UBYTE * ilbmfile );
int OpenDisplay( DISP_DEF * disp_def, UBYTE * ilbmfile );
int CloseDisplay( DISP_DEF * disp_def );
VOID Display2Front( DISP_DEF * disp_def );
VOID __saveds StopCopInt( DISP_DEF * disp_def  );

// RunCDXL.c
BOOL GetCDInfo( struct CDInfo * cdi );
BOOL CDConfig( ULONG tag, ... );
VOID Pan2Display( PAN * pan, DISP_DEF * disp_def, ULONG flags, struct TagItem * inti );
VOID CDXLFreeOb( CDXLOB * cdxl_OB );
int CDXLObtain( UBYTE * CDXLFile, DISP_DEF * disp_def, CDXLOB **CDXLob, ULONG flags, struct TagItem * inti );
VOID CDDeviceTerm( CDXLOB * CDXL_ob );
BOOL CDDeviceInit( ULONG * opened, CDXLOB * CDXL_ob );
VOID FreeXList( struct MinList * xllist, CDXLOB * CDXL_ob );
VOID CDXLTerm( CDXLOB * CDXL_ob );
__interrupt __asm __saveds CDXLCallBack( register __a1 APTR intdata, register __a2 struct CDXL * xl );
BOOL SendIOR( struct IOStdReq * req, LONG cmd, ULONG off, ULONG len, APTR data);
int NewCDTV_XL( struct MinList * first, UBYTE * buf, ULONG len, PF code );
int	NewCD_XL( struct MinList * first, UBYTE * buf, ULONG len, PF code );
int PlayCDXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob );
int StartCDXL( DISP_DEF * disp_def, CDXLOB * CDXL_ob );

// CDXLAudio.c
VOID StopAudio( VOID );
VOID StartAudio( VOID );
VOID __interrupt __saveds AudioIntrCode( VOID );
VOID QuitAudio( void );
InitAudio( CDXLOB * CDXL_ob );

// blitdef.c
int InitBDef( BLITDEF * BDef, struct BitMap * Srcbm, int Srcx, int Srcy,
 struct BitMap * Dstbm, int Dstx, int Dsty,	int inwidth, int inheight,
 UBYTE * mask);
int BlitBDef( BLITDEF * BDef, struct BitMap *source, struct BitMap *dest,
 UBYTE * Mask );

// ILBMSupport.c
int DoQuery( UBYTE * filename, DISP_DEF * disp_def );
int DoILBM( UBYTE * filename, DISP_DEF * disp_def );


//AsyncXLFile.c
VOID WaitPacket( ASYNCXLFILE * );
VOID SendAsync( ASYNCXLFILE * file, APTR arg2);
LONG CloseAsyncXL( ASYNCXLFILE * file, CDXLOB * CDXL_ob );
ASYNCXLFILE * OpenAsyncXL(const STRPTR fileName, CDXLOB * CDXL_ob, LONG position);
int StepFrame( XLEDIT * xledit, BOOL Forward );
int GoToFrame( XLEDIT * xledit, ULONG frame );

//xledit.c
VOID UpDateFrame( ULONG Frame );
FileReq( struct Window * window );
CalcReq( struct Window * window, struct VisualInfo * vi, LONG * retval );
struct Gadget * GetGadget( struct Window * Window, ULONG ID );
VOID StripIntuiMessages( struct MsgPort * mp, struct Window * win );
VOID HomePanel( VOID );

//
VOID kprintf( UBYTE *, ...);