
******* version.library/FreeVersion ******************************************
*
*   NAME
*	FreeVersion -- release resources allocated by GetVersion(). (V40)
*
*   SYNOPSIS
*	FreeVersion(versionInfo);
*		    A0
*
*	VOID FreeVersion(struct VersionInfo *);
*
*   FUNCTION
*	This function frees any memory or other resources allocated by a
*	call to GetVersion().
*
*   INPUTS
*	versionInfo - pointer to the VersionInfo structure to free. May be
*		      NULL in which case this function does nothing.
*
*   SEE ALSO
*	GetVersion(), <libraries/version.h>
*
******************************************************************************

******* version.library/GetVersion *******************************************
*
*   NAME
*	GetVersionA -- get version information from an object. (V40)
*	GetVersion -- varargs stub for GetVersionA(). (V40)
*
*   SYNOPSIS
*	versionInfo = GetVersionA(name,tagList);
*	D0			  A0   A1
*
*	struct VersionInfo *GetVersionA(STRPTR, struct TagItem *);
*
*	versionInfo = GetVersion(name,firstTag, ...);
*
*	struct VersionInfo *GetVersion(STRPTR, Tag, ...);
*
*   FUNCTION
*       This function attempt to find version information on a given named
*	object. The object can be a module in ROM, memory-resident library or
*	device, a disk-based file.
*
*   INPUTS
*	name - the NULL-terminated name of the object to get version
*	       information on. This may be a simple name, or a full AmigaDOS
*	       path. Passing NULL here indicates you wish to obtain the version
*	       and revision of the current Workbench disk.
*	tagList - pointer to an array of tags providing optional extra
*		  parameters, or NULL
*
*   TAGS
*	GV_Kickstart (BOOL) - Setting this tag to TRUE causes the version of
*			      the current Kickstart ROM to be returned.
*			      The name parameter is ignored in such a case.
*			      Default for this tag is FALSE. (V40)
*	GV_Location (ULONG) - This tag defines places where this function is
*		              to look for "name". See the VILOCF_XXX constants
*			      in <libraries/version.h> for the possible values.
*			      Default for this tag is 0xfffffff which means
*			      to accept all possible sources. (V40)
*
*   RESULT
*	versionInfo - a pointer to a filled-in VersionInfo structure, or
*		      NULL if no memory could be allocated for the structure.
*		      Also check the vi_Flags field to see the status of the
*		      operation.
*
*   SEE ALSO
*	FreeVersion(), <libraries/version.h>
*
******************************************************************************
