/*
**	$Id: player_pragmas.h,v 1.2 92/09/21 13:19:28 jerryh Exp $
**
**	SAS/C format pragma files.
**
**	Contains private definitions. COMMODORE INTERNAL USE ONLY!
*/

/* "player.library" */
#pragma libcall PlayerBase GetOptions 1e 901
#pragma libcall PlayerBase SetOptions 24 901
#pragma libcall PlayerBase GetPlayerState 2a 901
#pragma libcall PlayerBase ModifyPlayList 30 101
#pragma libcall PlayerBase ObtainPlayList 36 00
#pragma libcall PlayerBase SubmitKeyStroke 3c 101
#pragma libcall PlayerBase Analyze 42 00
