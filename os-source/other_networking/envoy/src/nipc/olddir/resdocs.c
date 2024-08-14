/****** nipc.library/NetQueryA ************************************************
*
*   NAME
*       NetQueryA -- Start a nipc network query
*
*   SYNOPSIS
*       NetQueryA(hook, queryClass, maxTime, tagList)
*                 A0    D0          D1       A1
*
*       VOID NetQueryA(struct Hook *, ULONG, ULONG, struct TagItem *);
*
*       NetQuery(hook, queryClass, maxTime, firsttag, ...)
*
*       VOID NetQuery(sutrct Hook *, ULONG, ULONG, Tag, ...);
*
*   FUNCTION
*       Starts an NIPC network query.  This function can be used to gather
*       data about a single machine, or to gather data for a given network.
*       Only one class of query may be performed at a time.  The classes of
*       queries you can do are:
*
*       NHClass_Hosts    - Query the local network or a given realm for the
*                          names of all hosts matching the data given in the
*                          tagList.  See below.
*       NHClass_Services - Query a single machine for a list of all services
*                          running on that machine.
*       NHClass_Entities - Query a single machine for a list of all public
*                          entities in use on a machine.
*       NHClass_Realms   - In an internetworked environment, returns a list
*                          of all Realms defined on your network.
*
*       The Hook is called for each packet returned from the network.
*       Depending on the class of the query, you may get more than one
*       packet.  For the NHClass_Hosts query, you will get one packet for
*       each machine matching your request.  For the other three classes,
*       you will get one packet containing all of the data you are looking
*       for.
*
*       A pointer to the packet is passed in A2 (the "object").  This
*       is the format of the data:
*
*           struct NameHeader
*           {
*               ULONG   nh_Reserved0;   /* Reserved */
*               UWORD   nh_ID;          /* Unique ID of this query */
*               UBYTE   nh_Type;        /* Query type (private) */
*               UBYTE   nh_Class;       /* Query Class */
*               UWORD   nh_Length;      /* Response packet length */
*               UBYTE   nh_Realms;      /* Number of Realms in the answer */
*               UBYTE   nh_Hosts;       /* Number of hosts in the answer */
*               UBYTE   nh_Services;    /* Number of services in the answer */
*               UBYTE   nh_Entities;    /* Number of entities in the answer */
*           };
*
*       The answer data is found immediately following the NameHeader. The
*       format is a series of null-terminated strings, the number of which
*       is specified in the fields above.  The strings will not be in any
*       specific order.
*
*       Do *NOT* free the NameHeader, it will be freed by nipc.library.
*
*       NetQueryA() returns when either maxTime seconds have passed, or in
*       the case of NHClass_Services or NHClass_Entities, the single
*       response packet is processed by the Hook function.
*
*   INPUTS
*       hook             - Pointer to a Hook structure to be called for each
*                          response.
*       queryClass       - The Class of the query, one of NHClass_Hosts,
*                          NHClass_Services, NHClass_Entities or
*                          NHClass_Realms.
*       maxTime          - The maximum number of seconds allowed for the
*                          query.  This is the absolute maximum time
*                          allowed.  nipc.library does not add anything to
*                          this value.
*       tagList          - Pointer to an array of TagItem structures. See
*                          below.
*
*   TAGS
*       The Tags defined for NetQueryA() are:
*
*       NQ_Realm (STRPTR) - Optional name of Realm to query.
*       NQ_Host (STRPTR) - Name of the host to query for a Services or
*                          Entities query.
*       NQ_Service (STRPTR) - Optional name of a Service that you want hosts
*                             to try to match.
*       NQ_Entity (STRPTR) - Option name of an Entity that you want hosts to
*                            to try to match.
*
*   RESULT
*       None.
*
*******************************************************************************
