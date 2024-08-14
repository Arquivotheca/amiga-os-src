/*
**	$Id: datatypes_pragmas.h,v 39.4 93/05/27 09:41:37 davidj Exp Locker: davidj $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "datatypes.library" */
/*--- functions in V40 or higher (Release 3.1) ---*/
/* ARexx function host entry point */
#ifdef DATATYPES_PRIVATE_PRAGMAS
#pragma libcall DataTypesBase RLDispatch 1e 9802
#endif

/* Public entries */

#pragma libcall DataTypesBase ObtainDataTypeA 24 98003
#ifdef __SASC_60
#pragma tagcall DataTypesBase ObtainDataType 24 98003
#endif
#pragma libcall DataTypesBase ReleaseDataType 2a 801
#pragma libcall DataTypesBase NewDTObjectA 30 8002
#ifdef __SASC_60
#pragma tagcall DataTypesBase NewDTObject 30 8002
#endif
#pragma libcall DataTypesBase DisposeDTObject 36 801
#pragma libcall DataTypesBase SetDTAttrsA 3c BA9804
#ifdef __SASC_60
#pragma tagcall DataTypesBase SetDTAttrs 3c BA9804
#endif
#pragma libcall DataTypesBase GetDTAttrsA 42 A802
#ifdef __SASC_60
#pragma tagcall DataTypesBase GetDTAttrs 42 A802
#endif
#pragma libcall DataTypesBase AddDTObject 48 0A9804
#pragma libcall DataTypesBase RefreshDTObjectA 4e BA9804
#ifdef __SASC_60
#pragma tagcall DataTypesBase RefreshDTObjects 4e BA9804
#endif
#pragma libcall DataTypesBase DoAsyncLayout 54 9802
#pragma libcall DataTypesBase DoDTMethodA 5a BA9804
#ifdef __SASC_60
#pragma tagcall DataTypesBase DoDTMethod 5a BA9804
#endif
#pragma libcall DataTypesBase RemoveDTObject 60 9802
#pragma libcall DataTypesBase GetDTMethods 66 801
#pragma libcall DataTypesBase GetDTTriggerMethods 6c 801
#pragma libcall DataTypesBase PrintDTObjectA 72 BA9804
#ifdef __SASC_60
#pragma tagcall DataTypesBase PrintDTObject 72 BA9804
#endif
#ifdef DATATYPES_PRIVATE_PRAGMAS
#pragma libcall DataTypesBase ObtainDTDrawInfoA 78 9802
#endif
#ifdef DATATYPES_PRIVATE_PRAGMAS
#ifdef __SASC_60
#pragma tagcall DataTypesBase ObtainDTDrawInfo 78 9802
#endif
#endif
#ifdef DATATYPES_PRIVATE_PRAGMAS
#pragma libcall DataTypesBase DrawDTObjectA 7e A5432109809
#endif
#ifdef DATATYPES_PRIVATE_PRAGMAS
#ifdef __SASC_60
#pragma tagcall DataTypesBase DrawDTObject 7e A5432109809
#endif
#endif
#ifdef DATATYPES_PRIVATE_PRAGMAS
#pragma libcall DataTypesBase ReleaseDTDrawInfo 84 9802
#endif
#pragma libcall DataTypesBase GetDTString 8a 001
