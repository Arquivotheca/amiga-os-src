
******* services.library/FindService *****************************************
*
*   NAME
*	FindServiceA -- Connect to a Service.
*	FindService -- varargs stub for FindServiceA().
*
*   SYNOPSIS
*	remote_entity = FindServiceA(remotehost, svcname, srcentity, taglist);
*	D0                           A0          A1       A2         A3
*
*   	struct Entity *FindServiceA(STRPTR, STRPTR, struct Entity *,
*		struct TagItem *);
*
*	remote_entity = FindService(remoteHost, svcname,srcentity,tag1, ...);
*
*	struct Entity *FindServiceA(STRPTR, STRPTR, struct Entity *,
*		Tag, ...);
*
*   FUNCTION
*	Attemps to locate a certain service on a given host.
*
*   INPUTS
*	remotehost - Pointer to a NULL-terminated string that is the name
*	    of the host on which the service you want to use is provided.
*	    NULL implies the local host.
*	servicename - Pointer to a NULL-terminated string that is the name
*           of the service you want to connect to.
*	srcentity - an Entity returned by nipc.library/CreateEntity() that
*	    you wish to use as the 'near' side of the communications path.
*
*   TAGS
*       Tags for use with FindService():
*
*	FSVC_UserName (STRPTR) - Specifies the name of the user who is trying
*	    to use the service.  If NULL, or the tag is not specified, the
*	    call will only be able to connect to a service marked as public
*	    on the remote machine.
*
*	FSVC_PassWord (STRPTR) - The password of the user who is trying to use
*	    the service.  Only useful in conjunction with FSVC_UserName.
*
*	FSVC_Error (ULONG *) - If specified, a pointer to a ULONG in which a
*	    error code describing the failure if there was one.
*
*   RESULT
*	remote_entity - NULL if the given service cannot be found or access to
*	    the service was denied.  Otherwise, a magic cookie describing the
*	    service that you found.
*
*   NOTES
*	All of the rules for FindEntity() apply here as well.  Please read the
*	Autodoc for FindEntity() for further information.
*
*	Each SUCCESSFUL FindService() REQUIRES an associated LoseService().
*
*   SEE ALSO
*	nipc.library/FindEntity(), nipc.library/LoseEntity()
*
******************************************************************************

******* services.library/LoseService *****************************************
*
*   NAME
*	LoseService -- Free up any resources allocated from FindService().
*
*   SYNOPSIS
*	LoseEntity(entity)
*		   A0
*
*   	VOID LoseEntity(struct Entity *)
*
*   FUNCTION
*	This will merely free up any resources allocated with a successful
*	FindService() call.
*
*   INPUTS
*	entity - A pointer to the Entity to use when communication with the
*	    server process.
*
*   RESULT
*	None
*
*   NOTES
*	LoseService() should only be use on entities returned by FindService.
*	Attempting to use LoseService() on entities created by CreateEntity
*	is asking for trouble.
*
*   BUGS
*	None known.
*
*   SEE ALSO
*	FindService()
*
******************************************************************************
