########################################
MAKEMETA=	../../../../tools/makemeta


OBJS0=\
	CloseCatalog.obj\
	CloseLocale.obj\
	ConvToLower.obj\
	ConvToUpper.obj\
	FormatDate.obj\
	FormatString.obj\
	GetCatalogStr.obj\
	GetLocaleStr.obj\
	IsAlNum.obj\
	IsAlpha.obj\
	IsCntrl.obj\
	IsDigit.obj\
	IsGraph.obj\
	IsLower.obj\
	IsPrint.obj\
	IsPunct.obj\
	IsSpace.obj\
	IsUpper.obj\
	IsXDigit.obj\
	OpenCatalogA.obj\
	OpenCatalog.obj\
	OpenLocale.obj\
	ParseDate.obj\
	StrConvert.obj\
	StrnCmp.obj

lib.timestamp:	asm.timestamp
	make objs0 ${MFLAGS} ${MARGS}
	cat *.obj >../locale.lib
	rm -f *.obj
	touch lib.timestamp

asm.timestamp:

objs0:		${OBJS0}
	rm -f *.asm
	touch asm.timestamp

.INCLUDE=${MAKEMETA}
