CVERSION = 6
VERSION = 40
SYSTEM	= specialfx
SUBSYSNAME = 
AFILES	= SpecialFX.library.asm AllocFX.asm FreeFX.asm InstallFX.asm RemoveFX.asm AnimateFX.asm ColourControl.asm \
          InterruptControl.asm VideoControl.asm SpriteControl.asm LineControl.asm NewMode.asm \
	  MrgCop.asm LoadView.asm endcode.asm
INFILES	= specialfx.h specialfx.i
OTHERFILES = specialfx_internal.i specialfx_rev.rev specialfx_rev.i specialfx_rev.h \
	     specialfx_lib.sfd specialfx.doc specialfxbase.h specialfxbase.i
HOMEINCL = v42:include/libraries
HOMEDIR = v42:src/other/specialfx
POSTCOMPILE	= copyit
LINKERLIBS = lib:amiga.lib lib:debug.lib
.include MakeMeta


copyit:
	copy specialfx.ld psys:libs/specialfx.library

.PHONY: copyit

