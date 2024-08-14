
extern struct DosLibrary *DOSBase;
extern struct Library	*SysBase;

/* "asl.library"*/
/*--- functions in V36 or higher (distributed as Release 2.0) ---*/
/**/
#pragma libcall AslBase AllocFileRequest 1E 0
#pragma libcall AslBase FreeFileRequest 24 801
#pragma libcall AslBase RequestFile 2A 801
#pragma libcall AslBase AllocAslRequest 30 8002
#pragma libcall AslBase FreeAslRequest 36 801
#pragma libcall AslBase AslRequest 3C 9802
/* "console.device"*/
#pragma libcall ConsoleDevice CDInputHandler 2a 9802
#pragma libcall ConsoleDevice RawKeyConvert 30 A19804
/*--- functions in V36 or higher (distributed as Preliminary Release 2.0) ---*/
/* "diskfont.library"*/
#pragma libcall DiskfontBase OpenDiskFont 1e 801
#pragma libcall DiskfontBase AvailFonts 24 10803
/*--- functions in V34 or higher (distributed as Release 1.3) ---*/
#pragma libcall DiskfontBase NewFontContents 2a 9802
#pragma libcall DiskfontBase DisposeFontContents 30 901
/* "dos.library"*/
#pragma libcall DOSBase Open 1e 2102
#pragma libcall DOSBase Close 24 101
#pragma libcall DOSBase Read 2a 32103
#pragma libcall DOSBase Write 30 32103
#pragma libcall DOSBase Input 36 0
#pragma libcall DOSBase Output 3c 0
#pragma libcall DOSBase Seek 42 32103
#pragma libcall DOSBase DeleteFile 48 101
#pragma libcall DOSBase Rename 4e 2102
#pragma libcall DOSBase Lock 54 2102
#pragma libcall DOSBase UnLock 5a 101
#pragma libcall DOSBase DupLock 60 101
#pragma libcall DOSBase Examine 66 2102
#pragma libcall DOSBase ExNext 6c 2102
#pragma libcall DOSBase Info 72 2102
#pragma libcall DOSBase CreateDir 78 101
#pragma libcall DOSBase CurrentDir 7e 101
#pragma libcall DOSBase IoErr 84 0
#pragma libcall DOSBase CreateProc 8a 432104
#pragma libcall DOSBase Exit 90 101
#pragma libcall DOSBase LoadSeg 96 101
#pragma libcall DOSBase UnLoadSeg 9c 101
#pragma libcall DOSBase DeviceProc ae 101
#pragma libcall DOSBase SetComment b4 2102
#pragma libcall DOSBase SetProtection ba 2102
#pragma libcall DOSBase DateStamp c0 101
#pragma libcall DOSBase Delay c6 101
#pragma libcall DOSBase WaitForChar cc 2102
#pragma libcall DOSBase ParentDir d2 101
#pragma libcall DOSBase IsInteractive d8 101
#pragma libcall DOSBase Execute de 32103

/*	Device List Management*/
#pragma libcall DOSBase System 25e 2102
#pragma libcall DOSBase AssignLock 264 2102
#pragma libcall DOSBase AssignLate 26a 2102
#pragma libcall DOSBase AssignPath 270 2102
#pragma libcall DOSBase AssignAdd 276 2102
#pragma libcall DOSBase RemAssignList 27c 2102
#pragma libcall DOSBase GetDeviceProc 282 2102
#pragma libcall DOSBase FreeDeviceProc 288 101
#pragma libcall DOSBase LockDosList 28e 101
#pragma libcall DOSBase UnLockDosList 294 101
#pragma libcall DOSBase AttemptLockDosList 29a 101
#pragma libcall DOSBase RemDosEntry 2a0 101
#pragma libcall DOSBase AddDosEntry 2a6 101
#pragma libcall DOSBase FindDosEntry 2ac 32103
#pragma libcall DOSBase NextDosEntry 2b2 2102
#pragma libcall DOSBase MakeDosEntry 2b8 2102
#pragma libcall DOSBase FreeDosEntry 2be 101
#pragma libcall DOSBase IsFileSystem 2c4 101

#pragma libcall DOSBase ParsePatternNoCase 3c6 32103
#pragma libcall DOSBase MatchPatternNoCase 3cc 2102

/*------ misc ---------------------------------------------------------*/
#pragma syscall Supervisor 1e D01
/*------ special patchable hooks to internal exec activity ------------*/
/*------ module creation ----------------------------------------------*/
#pragma syscall InitCode 48 1002
#pragma syscall InitStruct 4e 0A903
#pragma syscall MakeLibrary 54 10A9805
#pragma syscall MakeFunctions 5a A9803
#pragma syscall FindResident 60 901
#pragma syscall InitResident 66 1902
/*------ diagnostics --------------------------------------------------*/
#pragma syscall Alert 6c 701
#pragma syscall Debug 72 001
/*------ interrupts ---------------------------------------------------*/
#pragma syscall Disable 78 0
#pragma syscall Enable 7e 0
#pragma syscall Forbid 84 0
#pragma syscall Permit 8a 0
#pragma syscall SetSR 90 1002
#pragma syscall SuperState 96 0
#pragma syscall UserState 9c 001
#pragma syscall SetIntVector a2 9002
#pragma syscall AddIntServer a8 9002
#pragma syscall RemIntServer ae 9002
#pragma syscall Cause b4 901
/*------ memory allocation --------------------------------------------*/
#pragma syscall Allocate ba 0802
#pragma syscall Deallocate c0 09803
#pragma syscall AllocMem c6 1002
#pragma syscall AllocAbs cc 9002
#pragma syscall FreeMem d2 0902
#pragma syscall AvailMem d8 101
#pragma syscall AllocEntry de 801
#pragma syscall FreeEntry e4 801
/*------ lists --------------------------------------------------------*/
#pragma syscall Insert ea A9803
#pragma syscall AddHead f0 9802
#pragma syscall AddTail f6 9802
#pragma syscall Remove fc 901
#pragma syscall RemHead 102 801
#pragma syscall RemTail 108 801
#pragma syscall Enqueue 10e 9802
#pragma syscall FindName 114 9802
/*------ tasks --------------------------------------------------------*/
#pragma syscall AddTask 11a BA903
#pragma syscall RemTask 120 901
#pragma syscall FindTask 126 901
#pragma syscall SetTaskPri 12c 0902
#pragma syscall SetSignal 132 1002
#pragma syscall SetExcept 138 1002
#pragma syscall Wait 13e 001
#pragma syscall Signal 144 0902
#pragma syscall AllocSignal 14a 001
#pragma syscall FreeSignal 150 001
#pragma syscall AllocTrap 156 001
#pragma syscall FreeTrap 15c 001
/*------ messages -----------------------------------------------------*/
#pragma syscall AddPort 162 901
#pragma syscall RemPort 168 901
#pragma syscall PutMsg 16e 9802
#pragma syscall GetMsg 174 801
#pragma syscall ReplyMsg 17a 901
#pragma syscall WaitPort 180 801
#pragma syscall FindPort 186 901
/*------ libraries ----------------------------------------------------*/
#pragma syscall AddLibrary 18c 901
#pragma syscall RemLibrary 192 901
#pragma syscall OldOpenLibrary 198 901
#pragma syscall CloseLibrary 19e 901
#pragma syscall SetFunction 1a4 08903
#pragma syscall SumLibrary 1aa 901
/*------ devices ------------------------------------------------------*/
#pragma syscall AddDevice 1b0 901
#pragma syscall RemDevice 1b6 901
#pragma syscall OpenDevice 1bc 190804
#pragma syscall CloseDevice 1c2 901
#pragma syscall DoIO 1c8 901
#pragma syscall SendIO 1ce 901
#pragma syscall CheckIO 1d4 901
#pragma syscall WaitIO 1da 901
#pragma syscall AbortIO 1e0 901
/*------ resources ----------------------------------------------------*/
#pragma syscall AddResource 1e6 901
#pragma syscall RemResource 1ec 901
#pragma syscall OpenResource 1f2 901
/*------ private diagnostic support -----------------------------------*/
/*------ misc ---------------------------------------------------------*/
#pragma syscall RawDoFmt 20a BA9804
#pragma syscall GetCC 210 0
#pragma syscall TypeOfMem 216 901
#pragma syscall Procure 21c 9802
#pragma syscall Vacate 222 801
#pragma syscall OpenLibrary 228 0902
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
/*------ signal semaphores (note funny registers)----------------------*/
#pragma syscall InitSemaphore 22e 801
#pragma syscall ObtainSemaphore 234 801
#pragma syscall ReleaseSemaphore 23a 801
#pragma syscall AttemptSemaphore 240 801
#pragma syscall ObtainSemaphoreList 246 801
#pragma syscall ReleaseSemaphoreList 24c 801
#pragma syscall FindSemaphore 252 901
#pragma syscall AddSemaphore 258 901
#pragma syscall RemSemaphore 25e 901
/*------ kickmem support ----------------------------------------------*/
#pragma syscall SumKickData 264 0
/*------ more memory support ------------------------------------------*/
#pragma syscall AddMemList 26a 9821005
#pragma syscall CopyMem 270 09803
#pragma syscall CopyMemQuick 276 09803
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
#pragma libcall ExpansionBase AddConfigDev 1e 801
#pragma libcall ExpansionBase AllocBoardMem 2a 001
#pragma libcall ExpansionBase AllocConfigDev 30 0
#pragma libcall ExpansionBase AllocExpansionMem 36 21003
#pragma libcall ExpansionBase ConfigBoard 3c 9802
#pragma libcall ExpansionBase ConfigChain 42 801
#pragma libcall ExpansionBase FindConfigDev 48 10803
#pragma libcall ExpansionBase FreeBoardMem 4e 1002
#pragma libcall ExpansionBase FreeConfigDev 54 801
#pragma libcall ExpansionBase FreeExpansionMem 5a 1002
#pragma libcall ExpansionBase ReadExpansionByte 60 0802
#pragma libcall ExpansionBase ReadExpansionRom 66 9802
#pragma libcall ExpansionBase RemConfigDev 6c 801
#pragma libcall ExpansionBase WriteExpansionByte 72 10803
#pragma libcall ExpansionBase ObtainConfigBinding 78 0
#pragma libcall ExpansionBase ReleaseConfigBinding 7e 0
#pragma libcall ExpansionBase SetCurrentBinding 84 0802
#pragma libcall ExpansionBase GetCurrentBinding 8a 0802
#pragma libcall ExpansionBase MakeDosNode 90 801
#pragma libcall ExpansionBase AddDosNode 96 81003
/* "graphics.library"*/
/*------ BitMap primitives ------*/
#pragma libcall GfxBase BltBitMap 1e A76543291080B
#pragma libcall GfxBase BltTemplate 24 5432910808
/*------ Text routines ------*/
#pragma libcall GfxBase ClearEOL 2a 901
#pragma libcall GfxBase ClearScreen 30 901
#pragma libcall GfxBase TextLength 36 08903
#pragma libcall GfxBase Text 3c 08903
#pragma libcall GfxBase SetFont 42 8902
#pragma libcall GfxBase OpenFont 48 801
#pragma libcall GfxBase CloseFont 4e 901
#pragma libcall GfxBase AskSoftStyle 54 901
#pragma libcall GfxBase SetSoftStyle 5a 10903
/*------	Gels routines ------*/
#pragma libcall GfxBase AddBob 60 9802
#pragma libcall GfxBase AddVSprite 66 9802
#pragma libcall GfxBase DoCollision 6c 901
#pragma libcall GfxBase DrawGList 72 8902
#pragma libcall GfxBase InitGels 78 A9803
#pragma libcall GfxBase InitMasks 7e 801
#pragma libcall GfxBase RemIBob 84 A9803
#pragma libcall GfxBase RemVSprite 8a 801
#pragma libcall GfxBase SetCollision 90 98003
#pragma libcall GfxBase SortGList 96 901
#pragma libcall GfxBase AddAnimOb 9c A9803
#pragma libcall GfxBase Animate a2 9802
#pragma libcall GfxBase GetGBuffers a8 09803
#pragma libcall GfxBase InitGMasks ae 801
/*------	General graphics routines ------*/
#pragma libcall GfxBase DrawEllipse b4 3210905
#pragma libcall GfxBase AreaEllipse ba 3210905
#pragma libcall GfxBase LoadRGB4 c0 09803
#pragma libcall GfxBase InitRastPort c6 901
#pragma libcall GfxBase InitVPort cc 801
#pragma libcall GfxBase MrgCop d2 901
#pragma libcall GfxBase MakeVPort d8 9802
#pragma libcall GfxBase LoadView de 901
#pragma libcall GfxBase WaitBlit e4 0
#pragma libcall GfxBase SetRast ea 0902
#pragma libcall GfxBase Move f0 10903
#pragma libcall GfxBase Draw f6 10903
#pragma libcall GfxBase AreaMove fc 10903
#pragma libcall GfxBase AreaDraw 102 10903
#pragma libcall GfxBase AreaEnd 108 901
#pragma libcall GfxBase WaitTOF 10e 0
#pragma libcall GfxBase QBlit 114 901
#pragma libcall GfxBase InitArea 11a 09803
#pragma libcall GfxBase SetRGB4 120 3210805
#pragma libcall GfxBase QBSBlit 126 901
#pragma libcall GfxBase BltClear 12c 10903
#pragma libcall GfxBase RectFill 132 3210905
#pragma libcall GfxBase BltPattern 138 432108907
#pragma libcall GfxBase ReadPixel 13e 10903
#pragma libcall GfxBase WritePixel 144 10903
#pragma libcall GfxBase Flood 14a 102904
#pragma libcall GfxBase PolyDraw 150 80903
#pragma libcall GfxBase SetAPen 156 0902
#pragma libcall GfxBase SetBPen 15c 0902
#pragma libcall GfxBase SetDrMd 162 0902
#pragma libcall GfxBase InitView 168 901
#pragma libcall GfxBase CBump 16e 901
#pragma libcall GfxBase CMove 174 10903
#pragma libcall GfxBase CWait 17a 10903
#pragma libcall GfxBase VBeamPos 180 0
#pragma libcall GfxBase InitBitMap 186 210804
#pragma libcall GfxBase ScrollRaster 18c 543210907
#pragma libcall GfxBase WaitBOVP 192 801
#pragma libcall GfxBase GetSprite 198 0802
#pragma libcall GfxBase FreeSprite 19e 001
#pragma libcall GfxBase ChangeSprite 1a4 A9803
#pragma libcall GfxBase MoveSprite 1aa 109804
#pragma libcall GfxBase LockLayerRom 1b0 D01
#pragma libcall GfxBase UnlockLayerRom 1b6 D01
#pragma libcall GfxBase SyncSBitMap 1bc 801
#pragma libcall GfxBase CopySBitMap 1c2 801
#pragma libcall GfxBase OwnBlitter 1c8 0
#pragma libcall GfxBase DisownBlitter 1ce 0
#pragma libcall GfxBase InitTmpRas 1d4 09803
#pragma libcall GfxBase AskFont 1da 8902
#pragma libcall GfxBase AddFont 1e0 901
#pragma libcall GfxBase RemFont 1e6 901
#pragma libcall GfxBase AllocRaster 1ec 1002
#pragma libcall GfxBase FreeRaster 1f2 10803
#pragma libcall GfxBase AndRectRegion 1f8 9802
#pragma libcall GfxBase OrRectRegion 1fe 9802
#pragma libcall GfxBase NewRegion 204 0
#pragma libcall GfxBase ClearRectRegion 20a 9802
#pragma libcall GfxBase ClearRegion 210 801
#pragma libcall GfxBase DisposeRegion 216 801
#pragma libcall GfxBase FreeVPortCopLists 21c 801
#pragma libcall GfxBase FreeCopList 222 801
#pragma libcall GfxBase ClipBlit 228 65432910809
#pragma libcall GfxBase XorRectRegion 22e 9802
#pragma libcall GfxBase FreeCprList 234 801
#pragma libcall GfxBase GetColorMap 23a 001
#pragma libcall GfxBase FreeColorMap 240 801
#pragma libcall GfxBase GetRGB4 246 0802
#pragma libcall GfxBase ScrollVPort 24c 801
#pragma libcall GfxBase UCopperListInit 252 0802
#pragma libcall GfxBase FreeGBuffers 258 09803
#pragma libcall GfxBase BltBitMapRastPort 25e 65432910809
#pragma libcall GfxBase OrRegionRegion 264 9802
#pragma libcall GfxBase XorRegionRegion 26a 9802
#pragma libcall GfxBase AndRegionRegion 270 9802
#pragma libcall GfxBase SetRGB4CM 276 3210805
#pragma libcall GfxBase BltMaskBitMapRastPort 27c A6543291080A
/*--- (2 function slots reserved here) ---*/
#pragma libcall GfxBase AttemptLockLayerRom 28e D01
/* "icon.library"*/
/*--- functions in V36 or higher (distributed as Preliminary Release 2.0) ---*/
/*	Use DiskObjects instead of obsolete WBObjects*/
#pragma libcall IconBase GetIcon 2a A9803
#pragma libcall IconBase PutIcon 30 9802
#pragma libcall IconBase FreeFreeList 36 801
#pragma libcall IconBase AddFreeList 48 A9803
#pragma libcall IconBase GetDiskObject 4e 801
#pragma libcall IconBase PutDiskObject 54 9802
#pragma libcall IconBase FreeDiskObject 5a 801
#pragma libcall IconBase FindToolType 60 9802
#pragma libcall IconBase MatchToolValue 66 9802
#pragma libcall IconBase BumpRevision 6c 9802
/* "intuition.library"*/
#pragma libcall IntuitionBase OpenIntuition 1e 0
#pragma libcall IntuitionBase Intuition 24 801
#pragma libcall IntuitionBase AddGadget 2a 09803
#pragma libcall IntuitionBase ClearDMRequest 30 801
#pragma libcall IntuitionBase ClearMenuStrip 36 801
#pragma libcall IntuitionBase ClearPointer 3c 801
#pragma libcall IntuitionBase CloseScreen 42 801
#pragma libcall IntuitionBase CloseWindow 48 801
#pragma libcall IntuitionBase CloseWorkBench 4e 0
#pragma libcall IntuitionBase CurrentTime 54 9802
#pragma libcall IntuitionBase DisplayAlert 5a 18003
#pragma libcall IntuitionBase DisplayBeep 60 801
#pragma libcall IntuitionBase DoubleClick 66 321004
#pragma libcall IntuitionBase DrawBorder 6c 109804
#pragma libcall IntuitionBase DrawImage 72 109804
#pragma libcall IntuitionBase EndRequest 78 9802
#pragma libcall IntuitionBase GetDefPrefs 7e 0802
#pragma libcall IntuitionBase GetPrefs 84 0802
#pragma libcall IntuitionBase InitRequester 8a 801
#pragma libcall IntuitionBase ItemAddress 90 0802
#pragma libcall IntuitionBase ModifyIDCMP 96 0802
#pragma libcall IntuitionBase ModifyProp 9c 43210A9808
#pragma libcall IntuitionBase MoveScreen a2 10803
#pragma libcall IntuitionBase MoveWindow a8 10803
#pragma libcall IntuitionBase OffGadget ae A9803
#pragma libcall IntuitionBase OffMenu b4 0802
#pragma libcall IntuitionBase OnGadget ba A9803
#pragma libcall IntuitionBase OnMenu c0 0802
#pragma libcall IntuitionBase OpenScreen c6 801
#pragma libcall IntuitionBase OpenWindow cc 801
#pragma libcall IntuitionBase OpenWorkBench d2 0
#pragma libcall IntuitionBase PrintIText d8 109804
#pragma libcall IntuitionBase RefreshGadgets de A9803
#pragma libcall IntuitionBase RemoveGadget e4 9802
/* The official calling sequence for ReportMouse is given below.*/
/* Note the register order.  For the complete story, read the ReportMouse*/
/* autodoc.  !!! problems: official conflicts w/ lattice prototype !!!*/
#pragma libcall IntuitionBase ReportMouse1 ea 0802
#pragma libcall IntuitionBase Request f0 9802
#pragma libcall IntuitionBase ScreenToBack f6 801
#pragma libcall IntuitionBase ScreenToFront fc 801
#pragma libcall IntuitionBase SetDMRequest 102 9802
#pragma libcall IntuitionBase SetMenuStrip 108 9802
#pragma libcall IntuitionBase SetPointer 10e 32109806
#pragma libcall IntuitionBase SetWindowTitles 114 A9803
#pragma libcall IntuitionBase ShowTitle 11a 0802
#pragma libcall IntuitionBase SizeWindow 120 10803
#pragma libcall IntuitionBase ViewAddress 126 0
#pragma libcall IntuitionBase ViewPortAddress 12c 801
#pragma libcall IntuitionBase WindowToBack 132 801
#pragma libcall IntuitionBase WindowToFront 138 801
#pragma libcall IntuitionBase WindowLimits 13e 3210805
/*--- start of next generation of names -------------------------------------*/
#pragma libcall IntuitionBase SetPrefs 144 10803
/*--- start of next next generation of names --------------------------------*/
#pragma libcall IntuitionBase IntuiTextLength 14a 801
#pragma libcall IntuitionBase WBenchToBack 150 0
#pragma libcall IntuitionBase WBenchToFront 156 0
/*--- start of next next next generation of names ---------------------------*/
#pragma libcall IntuitionBase AutoRequest 15c 3210BA9808
#pragma libcall IntuitionBase BeginRefresh 162 801
#pragma libcall IntuitionBase BuildSysRequest 168 210BA9807
#pragma libcall IntuitionBase EndRefresh 16e 0802
#pragma libcall IntuitionBase FreeSysRequest 174 801
#pragma libcall IntuitionBase MakeScreen 17a 801
#pragma libcall IntuitionBase RemakeDisplay 180 0
#pragma libcall IntuitionBase RethinkDisplay 186 0
/*--- start of next next next next generation of names ----------------------*/
#pragma libcall IntuitionBase AllocRemember 18c 10803
#pragma libcall IntuitionBase AlohaWorkbench 192 801
#pragma libcall IntuitionBase FreeRemember 198 0802
/*--- start of 15 Nov 85 names ------------------------*/
#pragma libcall IntuitionBase LockIBase 19e 001
#pragma libcall IntuitionBase UnlockIBase 1a4 801
/*--- functions in V33 or higher (distributed as Release 1.2) ---*/
#pragma libcall IntuitionBase GetScreenData 1aa 910804
#pragma libcall IntuitionBase RefreshGList 1b0 0A9804
#pragma libcall IntuitionBase AddGList 1b6 A109805
#pragma libcall IntuitionBase RemoveGList 1bc 09803
#pragma libcall IntuitionBase ActivateWindow 1c2 801
#pragma libcall IntuitionBase RefreshWindowFrame 1c8 801
#pragma libcall IntuitionBase ActivateGadget 1ce A9803
#pragma libcall IntuitionBase NewModifyProp 1d4 543210A9809
/* "layers.library"*/
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
#pragma libcall LayersBase NewLayerInfo 90 0
#pragma libcall LayersBase DisposeLayerInfo 96 801
#pragma libcall LayersBase FattenLayerInfo 9c 801
#pragma libcall LayersBase ThinLayerInfo a2 801
#pragma libcall LayersBase MoveLayerInFrontOf a8 9802
#pragma libcall LayersBase InstallClipRegion ae 9802
