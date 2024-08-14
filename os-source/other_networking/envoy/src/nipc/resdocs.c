/****** nipc.library/NIPCInquiryA ************************************************
*
*   NAME
*       NIPCInquiryA -- Start a nipc network query
*
*   SYNOPSIS
*       success=NIPCInquiryA(hook, maxTime, maxResponses, tagList)
*       D0                   A0    D0       D1            A1
*
*       BOOL NIPCInquiryA(struct Hook *, ULONG, ULONG, APTR);
*
*       success=NIPCInquiry(hook, maxTime, maxResponses, firsttag, ...)
*
*       BOOL NIPCInquiry(struct Hook *, ULONG, ULONG, Tag, ...);
*
*   FUNCTION
*       Starts an NIPC Network Inquiry.  This function can be used to gather
*       data about a single machine, or to gather data for a number of
*       machines.  Multiple types of inquiries may be made using this
*       function.  Please see below for a description of the types of queries
*       and the types of information you may gather.
*
*       The Hook is called for each packet returned from hosts on the
*       network.  Depending on how specific your query is, you may or may
*       not get more than one packet.  When either maxTime seconds or
*       maxResponses packets have been received, your Hook will be called
*       with a null ParamPckt parameter.  If your Hook routine decides that
*       it has all of the information it needs, it may return FALSE.  This
*       will cause nipc.library to abort the NIPCInquirr() call.
*
*       When the Hook is called, the "Object" parameter will be a pointer to
*       the Task structure for the task or process that called the
*       NIPCInquiryA() function.  This is useful if you want to signal the
*       calling task when the query is complete.  The Hook "Message"
*       parameter will be a pointer to an array of TagItem structures that
*       contain the query response data from each responding host.
*
*       If the query fails for some reason, either due to illegal parameters
*       or to a lack of resources, NIPCInquiryA() will return FALSE. In this
*       case, your Hook function will never be called, so be careful.
*
*
*   INPUTS
*
*       hook(struct Hook *)     - Pointer to a Hook structure to be called for
*                                 each response.
*       maxTime(ULONG)          - The maximum number of seconds allowed for the
*                                 query.  This is the absolute maximum time
*                                 allowed.  Nipc.library does not add anything
*                                 to this value.
*       maxResponses(ULONG)     - The maximum number of responses that you will
*                                 accept from the network.
*       tagList(struct TagItem *) - Pointer to an array of TagItem structures.
*                                   below.
*
*   TAGS
*       The Tags currently defined for NIPCInquiryA() are:
*
*       QUERY_IPADDR (ULONG) - This tag is used for querying a machine for
*                              it's Network IP address.  Note:  This function
*                              is provided purely for diagnostic purposes. Do
*                              NOT depend on this tag being available in the
*                              future.
*       MATCH_IPADDR (ULONG) - Query a host with the specified IP address
*                              address.  Please see the note above regarding
*                              QUERY_IPADDR.
*       QUERY_REALMS (STRPTR) - Query a realmserver for the names of all known
*                               Realms.
*       MATCH_REALM (STRPTR) - Only query hosts that are in a specific Realm.
*       QUERY_HOSTNAME (STRPTR) - Query a host for it's name.
*       MATCH_HOSTNAME (STRPTR) - Only query the host with the specified
*                                 hostname.
*       QUERY_SERVICES (STRPTR) - Query a host or hosts for the names of all
*                                 services on each machine.
*       MATCH_SERVICE (STRPTR) - Only query hosts that have a specific service
*                                specified by the Tag.
*       QUERY_ENTITIES (STRPTR) - Query a host or hosts for the names of all
*                                 public entities on each machine.
*       MATCH_ENTITY (STRPTR) - Query only hosts that have a public entity
*                               specified by the Tag.
*       QUERY_OWNER (STRPTR) - This Tag is currently ignored.
*       MATCH_OWNER (STRPTR) - This Tag is currently ignored.
*       QUERY_ATTNFLAGS (ULONG) - This Tag allows you to find out what bits
*                                 are set in ExecBase->AttnFlags.
*       MATCH_ATTNFLAGS (ULONG) - Query only hosts that have the specified
*                                 bits set in ExecBase->AttnFlags.
*       QUERY_LIBVERSION (VOID) - This Tag is currently ignored.
*       MATCH_LIBVERSION (VOID) - This Tag is currently ignored.
*       QUERY_CHIPREVBITS (ULONG) - This Tag alllows you to find out what
*                                   bits are set in GfxBase->ChipRevBits0.
*                                   Note: Only the lower 8 bits of the ULONG
*                                   are used.
*       MATCH_CHIPREVBITS (ULONG) - Query only hosts that have the specified
*                                   bits set in GfxBase->ChipRevBits0.
*       QUERY_MAXFASTMEM (ULONG) - Query a machine for it's maximum amount
*                                  of FAST memory.
*       MATCH_MAXFASTMEM (ULONG) -  Query only hosts with at least the
*                                   specified amount of FAST memory.
*       QUERY_AVAILFASTMEM (ULONG) - Query a machine for the amount of free
*                                    FAST memory it has available.
*       MATCH_AVAILFASTMEM (ULONG) - Query only those hosts that have at
*                                    least the specified amount of FAST
*                                    memory available.
*       QUERY_MAXCHIPMEM (ULONG) - Query a machine for it's maximum amount
*                                  of installed CHIP memory.
*       MATCH_MAXCHIPMEM (ULONG) - Query only those hosts that have at least
*                                  the specified amount of CHIP ram installed.
*       QUERY_AVAILCHIPMEM (ULONG) - Query a machine for the amount of free
*                                    CHIP memory it has available.
*       MATCH_AVAILCHIPMEM (ULONG) - Query only those hosts that have at
*                                    least the specified amount of CHIP
*                                    memory available.
*       QUERY_KICKVERSION (ULONG) - Query a machine for the version and
*                                   revision of Kickstart it is running.
*       MATCH_KICKVERSION (ULONG) - Query only those hosts that are running
*                                   at least the specified version and revision
*                                   of Kickstart.
*       QUERY_WBVERSION (ULONG) - Query a machine for the version and
*                                 revision of Workbench it is running.
*       MATCH_WBVERSION (ULONG) - Query only those hosts that are running
*                                 at least the specified version and revision
*                                 of Workbench.
*   RESULT
*       None.
*
*   EXAMPLES
*
*       Query all hosts in realm "Software" for their Hostname, CPU type, Fast
*       memory installed and Kickstart version:
*
*       NIPCInquiry(myhook,             /* The hook to call */
*                   2,                  /* 2 seconds max */
*                   15,                 /* 15 responses max */
*                   MATCH_REALM,"Software",
*                   QUERY_HOSTNAME, 0,
*                   QUERY_ATTNFLAGS, 0,
*                   QUERY_MAXFASTMEM, 0,
*                   QUERY_KICKVERSION, 0,
*                   TAG_DONE);
*
*       The Hook would then get called for each host that responds.  An example
*       TagList passed to the hook might be:
*
*               MATCH_REALM,"Software",
*               QUERY_HOSTNAME,"A2500 Test".
*               QUERY_ATTNFLAGS, AFF_68010|AFF_68020|AFF_68881,
*               QUERY_MAXFASTMEM, 4194304,      (4 Megs)
*               QUERY_KICKVERSION, 2425007,     (VERSION <<16 | REVISION)
*               TAG_DONE
*
*       ---
*       Query a server for all services it has available
*
*       NIPCInquiry(myhook,
*                   2,
*                   2,
*                   MATCH_REALM,"Marketing",
*                   MATCH_HOSTNAME,"Market Server",
*                   QUERY_SERVICES,0,
*                   TAG_DONE)
*
*       A possible response might be:
*
*                   MATCH_REALM,"Marketing",
*                   MATCH_HOSTNAME,"Market Server",
*                   QUERY_SERVICES,"EnvoyFileSystem",
*                   QUERY_SERVICES,"EnvoyPrinterService",
*                   QUERY_SERVICES,"ConferenceService",
*                   TAG_DONE)
*
*
*   NOTES
*       This function is considered very low-level and is provided for network
*       diagnostic functions.  You should probably be using the functions
*       supplied in envoy.library to do network queries.  These will provide
*       "wrappers" for the most common types of queries.
*
*******************************************************************************
