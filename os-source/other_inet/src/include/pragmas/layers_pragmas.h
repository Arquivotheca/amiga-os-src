/*
**	$Id: layers_pragmas.h,v 38.7 92/03/26 16:25:36 mks Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "layers.library" */
#pragma libcall LayersBase InitLayers 1e 801
#pragma libcall LayersBase CreateUpfrontLayer 24 A432109808
#pragma libcall LayersBase CreateBehindLayer 2a A432109808
#pragma libcall LayersBase UpfrontLayer 30 9802
#pragma libcall LayersBase BehindLayer 36 9802
#pragma libcall LayersBase MoveLayer 3c 109804
#pragma libcall LayersBase SizeLayer 42 109804
#pragma libcall LayersBase ScrollLayer 48 109804
#pragma libcall LayersBase BeginUpdate 4e 801
#pragma libcall LayersBase EndUpdate 54 0802
#pragma libcall LayersBase DeleteLayer 5a 9802
#pragma libcall LayersBase LockLayer 60 9802
#pragma libcall LayersBase UnlockLayer 66 801
#pragma libcall LayersBase LockLayers 6c 801
#pragma libcall LayersBase UnlockLayers 72 801
#pragma libcall LayersBase LockLayerInfo 78 801
#pragma libcall LayersBase SwapBitsRastPortClipRect 7e 9802
#pragma libcall LayersBase WhichLayer 84 10803
#pragma libcall LayersBase UnlockLayerInfo 8a 801
#pragma libcall LayersBase NewLayerInfo 90 00
#pragma libcall LayersBase DisposeLayerInfo 96 801
#pragma libcall LayersBase FattenLayerInfo 9c 801
#pragma libcall LayersBase ThinLayerInfo a2 801
#pragma libcall LayersBase MoveLayerInFrontOf a8 9802
#pragma libcall LayersBase InstallClipRegion ae 9802
#pragma libcall LayersBase MoveSizeLayer b4 3210805
#pragma libcall LayersBase CreateUpfrontHookLayer ba AB432109809
#pragma libcall LayersBase CreateBehindHookLayer c0 AB432109809
#pragma libcall LayersBase InstallLayerHook c6 9802
/*--- functions in V39 or higher (Release 3) ---*/
#pragma libcall LayersBase InstallLayerInfoHook cc 9802
#pragma libcall LayersBase SortLayerCR d2 10803
#pragma libcall LayersBase DoHookClipRects d8 A9803
