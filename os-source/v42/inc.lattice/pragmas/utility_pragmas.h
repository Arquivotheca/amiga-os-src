/* "utility.library" */
/*--- functions in V36 or higher (Release 2.0) ---*/

/* Tag item functions */

#pragma libcall UtilityBase FindTagItem 1e 8002
#pragma libcall UtilityBase GetTagData 24 81003
#pragma libcall UtilityBase PackBoolTags 2a 98003
#pragma libcall UtilityBase NextTagItem 30 801
#pragma libcall UtilityBase FilterTagChanges 36 09803
#pragma libcall UtilityBase MapTags 3c 09803
#pragma libcall UtilityBase AllocateTagItems 42 001
#pragma libcall UtilityBase CloneTagItems 48 801
#pragma libcall UtilityBase FreeTagItems 4e 801
#pragma libcall UtilityBase RefreshTagItemClones 54 9802
#pragma libcall UtilityBase TagInArray 5a 8002
#pragma libcall UtilityBase FilterTagItems 60 09803

/* Hook functions */

#pragma libcall UtilityBase CallHookPkt 66 9A803
/*--- (1 function slot reserved here) ---*/

/* Date functions */

/*--- (1 function slot reserved here) ---*/
#pragma libcall UtilityBase Amiga2Date 78 8002
#pragma libcall UtilityBase Date2Amiga 7e 801
#pragma libcall UtilityBase CheckDate 84 801

/* 32 bit integer muliply functions */

#pragma libcall UtilityBase SMult32 8a 1002
#pragma libcall UtilityBase UMult32 90 1002

/* 32 bit integer division funtions. The quotient and the remainder are */
/* returned respectively in d0 and d1 */

#pragma libcall UtilityBase SDivMod32 96 1002
#pragma libcall UtilityBase UDivMod32 9c 1002
/*--- functions in V37 or higher (Release 2.04) ---*/

/* International string routines */

#pragma libcall UtilityBase Stricmp a2 9802
#pragma libcall UtilityBase Strnicmp a8 09803
#pragma libcall UtilityBase ToUpper ae 001
#pragma libcall UtilityBase ToLower b4 001
/*--- functions in V39 or higher (Release 3) ---*/

/* More tag Item functions */

#pragma libcall UtilityBase ApplyTagChanges ba 9802
/*--- (1 function slot reserved here) ---*/

/* 64 bit integer muliply functions. The results are 64 bit quantities */
/* returned in D0 and D1 */

#pragma libcall UtilityBase SMult64 c6 1002
#pragma libcall UtilityBase UMult64 cc 1002

/* Structure to Tag and Tag to Structure support routines */

#pragma libcall UtilityBase PackStructureTags d2 A9803
#pragma libcall UtilityBase UnpackStructureTags d8 A9803

/* New, object-oriented NameSpaces */

#pragma libcall UtilityBase AddNamedObject de 9802
#pragma libcall UtilityBase AllocNamedObjectA e4 9802
#ifdef __SASC_60
#pragma tagcall UtilityBase AllocNamedObject e4 9802
#endif
#pragma libcall UtilityBase AttemptRemNamedObject ea 801
#pragma libcall UtilityBase FindNamedObject f0 A9803
#pragma libcall UtilityBase FreeNamedObject f6 801
#pragma libcall UtilityBase NamedObjectName fc 801
#pragma libcall UtilityBase ReleaseNamedObject 102 801
#pragma libcall UtilityBase RemNamedObject 108 9802

/* Unique ID generator */

#pragma libcall UtilityBase GetUniqueID 10e 00
/*--- (4 function slots reserved here) ---*/
