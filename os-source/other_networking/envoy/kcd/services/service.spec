******* xxx.service/StartService *********************************************
*
*   NAME
*	StartService -- Accept a new client connection
*
*   SYNOPSIS
*	error = StartService(taglist)
*	d0                   A0
*
*	ULONG StartService(struct TagItem *);
*
*   FUNCTION
*	This function requests that a service take whatever steps are
*	required to initiate a connection with a new client. The service
*	should fill in entityName with the name of the Entity with which
*	you will be accepting client transactions.  If you have any kind
*	of problem during startup, return with a non-zero error code. This
*	code will be passed back to the client that called FindService().
*
*   INPUTS
*	taglist - Pointer to an array of TagItem's passed in by the
*	    Services Manager.
*
*   TAGS
*	Tags defined for use with StartService():
*
*	SSVC_UserName (STRPTR) - The name of the user requesting your
*		service.
*	SSVC_Password (STRPTR) - The password of the user requesting your
*		services.
*	SSVC_HostName (STRPTR) - The hostname of the machine from which
*		the client is connecting.
*	SSVC_EntityName (STRPTR) - The buffer to fill in with the name of
*		the Entity that the client should connect to.
*
*   RESULT
*	error -	A ULONG describing why you could not start your service. This
*	    error code will be passed back to the client that called
*	    FindService().
*
******************************************************************************

******* xxx.service/AttemptShutdown ******************************************
*
*   NAME
*	AttemptShutdown -- Inform a service that the system will be going down
*
*   SYNOPSIS
*	AttemptShutdown(desc, seconds)
*			A0    D0
*
*       VOID AttemptShutdown(STRPTR, ULONG)
*
*   FUNCTION
*	This function provides a means for informing a service as a whole that
*	it should try to shutdown within a specified amount of time.
*
*   INPUTS
*	desc - Pointer to a string that contains a user-friendly reason of
*	    reason of why the service needs to be shut down.  May be NULL.
*
*	seconds - The number of seconds until the end of the world.  This may
*	    be 0 if shutdown needs to be done immediately.
*
*   NOTES
*       This function is provided as a convenience to services such as
*       filesystems that may wish to start refusing new connections and/or
*       flush any buffered data to disk.
*
******************************************************************************

******* xxx.service/GetServiceAttrsA ******************************************
*
*   NAME
*	GetServiceAttrsA -- obtain information about the service.
*	GetServiceAttrs -- varargs stub for GetServiceAttrsA.
*
*   SYNOPSIS
*	GetServiceAttrsA(taglist)
*			 A0
*
*	VOID GetServiceAttrsA(struct TagItem *);
*
*	GetServiceAttrs(tag1, ...)
*
*	VOID GetServiceAttrs(Tag, ...);
*
*   FUNCTION
*	This function provides a method for determining information about
*	the service.  Some of this data will be used by Services Manager
*	for configuration.  There will also likely be a predefined set
*	of Tags for information such as the names of the connected users,
*	the current load on the service, etc.  Service implementors will
*	also be able to define their own custom Tags for diagnostic/
*	statistical purposes.
*
*   INPUTS
*       taglist - A tagList containing the tags that you want to get the
*	    attributes for.
*
*   TAGS
*	Tags defined for GetServiceAttrsA():
*
*	SVCAttrs_Name (STRPTR) - The name of the service you are providing.
*	    (Required)
*
*
******************************************************************************

******* xxx.service/SetServiceAttrsA *****************************************
*
*   NAME
*	SetServiceAttrsA -- set attributes of a service.
*	SetServiceAttrs -- varargs stub for SetServiceAttrs
*
*   SYNOPSIS
*	SetServiceAttrsA(taglist)
*			 A0
*
*	VOID SetServiceAttrsA(struct TagItem *);
*
*	SetServiceAttrs(tag1, ...)
*
*	VOID SetServiceAttrs(Tag, ...);
*
*   FUNCTION
*	This function provides a method for setting attributes for a service.
*	For instance, a configuration editor for the service may need to change
*	the name of the service, permissions, etc.  This function provides
*	a standardized method for doing so.
*
*   INPUTS
*	taglist - A list of TagItem structures to be used by the service
*	    to modify it's operation.
*
*   TAGS
*	Tags defined for SetServiceAttrsA():
*
*	SVCAttrs_Name (STRPTR)  - The name of the service you are providing.
*
******************************************************************************
