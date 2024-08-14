/****** mwlib/LinkTools *******************************************************
    NAME
        LinkTools -- Tools for link management

    SYNOPSIS

        FUNCTIONS
            void *AddLinkHead(Link *head, Link *link, void *container);
            void *AddLinkTail(Link *head, Link *link, void *container);

        MACROS (see LinkTools.h)
            void InitLinkHead(Link *head);
            void InitCLinkHead(Link *head, void *container);
            Link *NextLink(Link *link);
            Link *PrevLink(Link *link);
            void *Container(Link *link);
            void DeleteLink(Link *link);
            void JoinLinks(Link *h1, Link *h2);
            void FSplitLinks(Link *h1, Link *h2);

    FUNCTION
        Provides a set of tools to manage linked lists. Based on ListTools.

    SEE ALSO
        LinkTools.h
        mwlib/ListTools

******* mwlib/LinkTools/AddLinkHead *******************************************

    NAME
        AddLinkHead -- Add given link to head of list, setting container

    SYNOPSIS
        void *AddLinkHead(Link *head, Link *link, void *container);

    FUNCTION
        Adds the given link to head of list, and sets the container (myself)
        pointer to 'container'. 'container' can be either a pointer to the
        true container, or NULL to signify the head.

    INPUTS
        head        - The 'head' of the list. Not required to be a true head, but
                      may also be another link in the list.

        link        - The link to be added.

        container   - A pointer to the container of this link. Can be NULL to
                      signify 'link' to really be a head.

    RESULT
        The value of 'container' is returned.

    SEE ALSO
        mwlib/LinkTools/AddLinkTail

******* mwlib/LinkTools/AddLinkTail *******************************************

    NAME
        AddLinkTail -- Add given link to Tail of list, setting container

    SYNOPSIS
        void *AddLinkTail(Link *head, Link *link, void *container);

    FUNCTION
        Adds the given link to Tail of list, and sets the container (myself)
        pointer to 'container'. 'container' can be either a pointer to the
        true container, or NULL to signify the head.

    INPUTS
        head        - The 'head' of the list. Not required to be a true head, but
                      may also be another link in the list.

        link        - The link to be added.

        container   - A pointer to the container of this link. Can be NULL to
                      signify 'link' to really be a head.

    RESULT
        The value of 'container' is returned.

    SEE ALSO
        mwlib/LinkTools/AddLinkHead

******************************************************************************/

#include <exec/types.h>
#include "LinkTools.h"
#include "protos.h"

void *AddLinkHead(struct Link *head, struct Link *link, void *container)
{
    AddLTHead(&head->list, &link->list);
    return link->myself = container;
}

void *AddLinkTail(struct Link *head, struct Link *link, void *container)
{
    AddLTTail(&head->list, &link->list);
    return link->myself = container;
}

