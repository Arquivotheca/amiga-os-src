##########################################################################
#
# $Id: makefile,v 42.0 93/06/16 11:14:46 chrisg Exp $
#
##########################################################################

VERSION=42
MODULETYPE=kickstart
SYSTEM=graphics
SUBSYSNAME=a
SUBSYSTEMS=
INCLUDEDIR=
ASMOPTS=OPTIMON 0xfdc0000 ADDSYM DEFINE VBLANK_BUG=1
INFILES=

ASMFILES1=AreaMove.asm BltClear.asm Draw.asm InitArea.asm
ASMFILES2=Move.asm MrgCop.asm MustMem.asm QBlit.asm RectFill.asm
ASMFILES3=SetRGB4.asm WaitBlit.asm WaitTOF.asm clip.asm
ASMFILES4=Flood.asm PolyDraw.asm gasm.asm ScrollRaster.asm
ASMFILES5=InitRastPort.asm LoadRGB4.asm SetRast.asm BltPattern.asm ClearMem.asm
ASMFILES6=WaitBOVP.asm SimpleSprites.asm InitTmpRas.asm
ASMFILES7=SetAPen.asm AreaDraw.asm AreaEnd.asm 
ASMFILES8=AllocRaster.asm FreeRaster.asm
ASMFILES9=InitView.asm InitVPort.asm LockLayerRom.asm UnlockLayerRom.asm
ASMFILESA=MakeVPort.asm LoadView.asm QBSBlit.asm InitBitMap.asm
ASMFILESB=FreeVPortCopLists.asm FreeCopList.asm OwnBlitter.asm
ASMFILESC=FreeCprList.asm GetColorMap.asm FreeColorMap.asm GetRGB4.asm
ASMFILESD=ScrollVPort.asm ReadPixel.asm WritePixel.asm BltRastPort.asm
ASMFILESE=BltBitMapRastPort.asm BltMaskRastPort.asm CalcIVG.asm
ASMFILESF=cdraw.asm SetRGB4CM.asm Circle.asm Ellipse.asm AttemptLockLayerRom.asm
ASMFILESG=AreaCircle.asm AreaEllipse.asm GfxMem.asm VideoControl.asm Monitor.asm
ASMFILESH=rpa8.asm wpa8.asm GetVPModeID.asm EraseRect.asm NewColorStuff.asm GetSet.asm
ASMFILESI=allocbitmap.asm GetVec.asm changevpbitmap.asm

AFILES=$(ASMFILES1) $(ASMFILES2) $(ASMFILES3) $(ASMFILES4) $(ASMFILES5) $(ASMFILES6) $(ASMFILES7)
AFILES2=$(ASMFILES8) $(ASMFILES9) $(ASMFILESA) $(ASMFILESB) $(ASMFILESC) $(ASMFILESD) $(ASMFILESE)
AFILES3=$(ASMFILESF) $(ASMFILESG) $(ASMFILESH) $(ASMFILESI)

CFILES=

INFILES=
OTHERFILES=	submacs.i makefile
TEMPDIR=
HOMEDIR=
# defaults to Vxx/src/moduletype/system/subsysname

HOMEINCL=
LINKEROPTS=
COMPILER_OPTIONS=
ASM_PREPROCESSOR=spp

.include lib:makemeta

