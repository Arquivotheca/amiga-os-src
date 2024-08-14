
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/io.h>
#include <exec/resident.h>

#include <dos/dosextens.h>

#include <pragmas/exec_pragmas.h>
#include <clib/exec_protos.h>

#define LOWLEVEL_PRIVATE_PRAGMAS
#include <pragmas/lowlevel_pragmas.h>
#include <clib/lowlevel_protos.h>

#include "nonvolatile.h"
#include "nonvolatilebase.h"

#define SysBase NVBase->nv_ExecBase
#define LowLevelBase  NVBase->nv_LowLevelBase

/* NVRAM/ */
extern APTR __asm ReadNVRAM (register __a0 STRPTR AppName, register __a1 STRPTR ItemName, register __a6 struct NVBase *NVBase);
extern BOOL __asm DeleteNVRAM (register __a0 STRPTR AppName, register __a1 STRPTR ItemName, register __a6 struct NVBase *NVBase);

/* Disk/ */
extern APTR __asm ReadDisk (register __a0 STRPTR AppName, register __a1 STRPTR ItemName, register __a6 struct NVBase *NVBase);
extern BOOL __asm DeleteDisk (register __a0 STRPTR AppName, register __a1 STRPTR ItemName, register __a6 struct NVBase *NVBase);

/* Lowlevel.library */
extern void KillReq (void);
extern void RestoreReq (void);


/****** nonvolatile.library/GetCopyNV ****************************************
*
*   NAME
*	GetCopyNV -- return a copy of an item stored in nonvolatile storage.
*		     (V40)
*
*   SYNOPSIS
*	data = GetCopyNV(appName, itemName, killRequesters);
*	D0		 A0	  A1	    D1
*
*	APTR GetCopyNV(STRPTR, STRPTR, BOOL);
*
*   FUNCTION
*	Searches the nonvolatile storage for the indicated appName and
*	itemName. A pointer to a copy of this data will be returned.
*
*	The strings appName and itemName may not contain the '/' or ':'
*	characters. It is recommended that these characters be blocked
*	from user input when requesting appName and itemName strings.
*
*   INPUTS
*	appName - NULL terminated string indicating the application name
*		  to be found. Maximum length is 31.
*	itemName - NULL terminated string indicated the item within the
*		   application to be found. Maximum length is 31.
*	killRequesters - Suppress system requesters flag. TRUE if all system
*			 requesters are to be suppressed during this function.
*			 FALSE if system requesters are allowed.
*
*   RESULT
*	data - pointer to a copy of the data found in the nonvolatile storage
*	       assocated with appName and itemName. NULL will be returned
*	       if there is insufficient memory or the appName/itemName does
*	       not exist.
*
*   SEE ALSO
*	FreeNVData(), <libraries/nonvolatile.h>
*
******************************************************************************/

APTR __asm GetCopyNV(register __a0 STRPTR AppName, register __a1 STRPTR ItemName,
                       register __d1 BOOL KillRequesters, register __a6 struct NVBase *NVBase)
{
	APTR result;

	if ( (AppName != NULL) && (ItemName !=NULL) )
	{
		if (KillRequesters) KillReq();
		if ((result = ReadDisk(AppName, ItemName, NVBase)) == NULL) result = ReadNVRAM(AppName,ItemName,NVBase);
		if (KillRequesters) RestoreReq();
	}
	else
	{
		result = NULL;
	};
	return(result);

} /* GetCopyNV */




/****** nonvolatile.library/FreeNVData ***************************************
*
*   NAME
*	FreeNVData -- release the memory allocated by a function of this
*		      library. (V40)
*
*   SYNOPSIS
*	FreeNVData(data);
*		   A0
*
*	VOID FreeNVData(APTR);
*
*   FUNCTION
*	Frees a block of memory that was allocated by any of the following:
*	GetCopyNV(), GetNVInfo(), GetNVList().
*
*   INPUTS
*	data - pointer to the memory block to be freed. If passed NULL,
*	       this function does nothing.
*
*   SEE ALSO
*	GetCopyNV(), GetNVInfo(), GetNVList()
*
******************************************************************************/
void __asm FreeNVData(register __a0 void *Data, register __a6 struct NVBase *NVBase)
{

	if (Data != NULL) FreeMem(&((ULONG *)Data)[-1], ((ULONG *)Data)[-1]);

} /* FreeNVData */




/****** nonvolatile.library/DeleteNV *****************************************
*
*   NAME
*	DeleteNV -- remove an entry from nonvoltatile storage. (V40)
*
*   SYNOPSIS
*	success = DeleteNV(appName, itemName, killRequesters);
*	D0		   A0	    A1	      D1
*
*	BOOL DeleteNV(STRPTR, STRPTR, BOOL);
*
*   FUNCTION
*	Searches the nonvolatile storage for the indicated entry and removes
*	it.
*
*	The strings appName and itemName may not contain the '/' or ':'
*	characters. It is recommended that these characters be blocked
*	from user input when requesting AppName and ItemName strings.
*
*   INPUTS
*	appName - NULL terminated string identifing the application that
*		  created the data. Maximum length is 31.
*	ItemName - NULL terminated string uniquely identifing the data
*		   within the application. Maximum length is 31.
*	killRequesters - suppress system requesters flag. TRUE if all system
*			 requesters are to be suppressed during this function.
*			 FALSE if system requesters are allowed.
*
*   RESULT
*	success - TRUE will be returned if the entry is found and deleted.
*		  If the entry is not found, FALSE will be returned.
*
******************************************************************************/


BOOL __asm DeleteNV(register __a0 STRPTR AppName, register __a1 STRPTR ItemName,
                      register __d1 BOOL KillRequesters, register __a6 struct NVBase *NVBase)
{
	BOOL	result;
	if ( (AppName != NULL) && (ItemName !=NULL) )
	{
		if (KillRequesters) KillReq();
		result = DeleteNVRAM(AppName, ItemName, NVBase) | DeleteDisk(AppName, ItemName, NVBase);
		if (KillRequesters) RestoreReq();
	}
	else
	{
		result = NULL;
	};
	return(result);
}




