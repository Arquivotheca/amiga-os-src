########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	CreateGadgetA.obj\
	CreateGadget.obj\
	FreeGadgets.obj\
	GT_SetGadgetAttrsA.obj\
	GT_SetGadgetAttrs.obj\
	CreateMenusA.obj\
	CreateMenus.obj\
	FreeMenus.obj\
	LayoutMenuItemsA.obj\
	LayoutMenuItems.obj\
	LayoutMenusA.obj\
	LayoutMenus.obj\
	GT_GetIMsg.obj\
	GT_ReplyIMsg.obj\
	GT_RefreshWindow.obj\
	GT_BeginRefresh.obj\
	GT_EndRefresh.obj\
	GT_FilterIMsg.obj\
	GT_PostFilterIMsg.obj\
	CreateContext.obj\
	DrawBevelBoxA.obj\
	DrawBevelBox.obj\
	GetVisualInfoA.obj\
	GetVisualInfo.obj\
	FreeVisualInfo.obj\
	GT_GetGadgetAttrsA.obj\
	GT_GetGadgetAttrs.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../gadtools.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
