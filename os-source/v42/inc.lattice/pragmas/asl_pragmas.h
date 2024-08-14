/* "asl.library" */
/*--- functions in V36 or higher (Release 2.0) ---*/

/* OBSOLETE -- Please use the generic requester functions instead */

#pragma libcall AslBase AllocFileRequest 1e 00
#pragma libcall AslBase FreeFileRequest 24 801
#pragma libcall AslBase RequestFile 2a 801
#pragma libcall AslBase AllocAslRequest 30 8002
#ifdef __SASC_60
#pragma tagcall AslBase AllocAslRequestTags 30 8002
#endif
#pragma libcall AslBase FreeAslRequest 36 801
#pragma libcall AslBase AslRequest 3c 9802
#ifdef __SASC_60
#pragma tagcall AslBase AslRequestTags 3c 9802
#endif
