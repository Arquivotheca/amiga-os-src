########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	OpenIntuition.obj\
	Intuition.obj\
	AddGadget.obj\
	ClearDMRequest.obj\
	ClearMenuStrip.obj\
	ClearPointer.obj\
	CloseScreen.obj\
	CloseWindow.obj\
	CloseWorkBench.obj\
	CurrentTime.obj\
	DisplayAlert.obj\
	DisplayBeep.obj\
	DoubleClick.obj\
	DrawBorder.obj\
	DrawImage.obj\
	EndRequest.obj\
	GetDefPrefs.obj\
	GetPrefs.obj\
	InitRequester.obj\
	ItemAddress.obj\
	ModifyIDCMP.obj\
	ModifyProp.obj\
	MoveScreen.obj\
	MoveWindow.obj\
	OffGadget.obj\
	OffMenu.obj\
	OnGadget.obj\
	OnMenu.obj\
	OpenScreen.obj\
	OpenWindow.obj\
	OpenWorkBench.obj\
	PrintIText.obj\
	RefreshGadgets.obj\
	RemoveGadget.obj\
	ReportMouse.obj\
	ReportMouse1.obj\
	Request.obj\
	ScreenToBack.obj\
	ScreenToFront.obj\
	SetDMRequest.obj\
	SetMenuStrip.obj\
	SetPointer.obj\
	SetWindowTitles.obj\
	ShowTitle.obj\
	SizeWindow.obj\
	ViewAddress.obj\
	ViewPortAddress.obj\
	WindowToBack.obj\
	WindowToFront.obj\
	WindowLimits.obj\
	SetPrefs.obj\
	IntuiTextLength.obj\
	WBenchToBack.obj\
	WBenchToFront.obj\
	AutoRequest.obj\
	BeginRefresh.obj\
	BuildSysRequest.obj\
	EndRefresh.obj\
	FreeSysRequest.obj\
	MakeScreen.obj\
	RemakeDisplay.obj\
	RethinkDisplay.obj\
	AllocRemember.obj\
	AlohaWorkbench.obj\
	FreeRemember.obj\
	LockIBase.obj\
	UnlockIBase.obj\
	GetScreenData.obj\
	RefreshGList.obj\
	AddGList.obj\
	RemoveGList.obj\
	ActivateWindow.obj\
	RefreshWindowFrame.obj\
	ActivateGadget.obj\
	NewModifyProp.obj\
	QueryOverscan.obj\
	MoveWindowInFrontOf.obj\
	ChangeWindowBox.obj\
	SetEditHook.obj\
	SetMouseQueue.obj\
	ZipWindow.obj\
	LockPubScreen.obj\
	UnlockPubScreen.obj\
	LockPubScreenList.obj\
	UnlockPubScreenList.obj\
	NextPubScreen.obj\
	SetDefaultPubScreen.obj\
	SetPubScreenModes.obj\
	PubScreenStatus.obj\
	ObtainGIRPort.obj\
	ReleaseGIRPort.obj\
	GadgetMouse.obj\
	SetIPrefs.obj\
	GetDefaultPubScreen.obj\
	EasyRequestArgs.obj\
	EasyRequest.obj\
	BuildEasyRequestArgs.obj\
	BuildEasyRequest.obj\
	SysReqHandler.obj\
	OpenWindowTagList.obj

OBJS1=\
	OpenWindowTags.obj\
	OpenScreenTagList.obj\
	OpenScreenTags.obj\
	DrawImageState.obj\
	PointInImage.obj\
	EraseImage.obj\
	NewObjectA.obj\
	NewObject.obj\
	DisposeObject.obj\
	SetAttrsA.obj\
	SetAttrs.obj\
	GetAttr.obj\
	SetGadgetAttrsA.obj\
	SetGadgetAttrs.obj\
	NextObject.obj\
	FindClass.obj\
	MakeClass.obj\
	AddClass.obj\
	GetScreenDrawInfo.obj\
	FreeScreenDrawInfo.obj\
	ResetMenuStrip.obj\
	RemoveClass.obj\
	FreeClass.obj\
	lockPubClass.obj\
	unlockPubClass.obj\
	AllocScreenBuffer.obj\
	FreeScreenBuffer.obj\
	ChangeScreenBuffer.obj\
	ScreenDepth.obj\
	ScreenPosition.obj\
	ScrollWindowRaster.obj\
	LendMenus.obj\
	DoGadgetMethodA.obj\
	DoGadgetMethod.obj\
	SetWindowPointerA.obj\
	SetWindowPointer.obj\
	TimedDisplayAlert.obj\
	HelpControl.obj

lib.timestamp:	asm.timestamp
	make objs0 objs1 ${MFLAGS} ${MARGS}
	cat *.obj >../intuition.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}

objs1:		${OBJS1}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
