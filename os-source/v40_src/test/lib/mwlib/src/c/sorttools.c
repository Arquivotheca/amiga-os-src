/******************************************************************************
    SortTools.c

    Mitchell/Ware Systems           Version 1.00            2/23/90

    Sort routine(s) that make use of the LTList construct. Sort fields must
    be present after the LTList construct, and must be referenced as a function
    of the distance from the LTList struct.
-------------------------------------------------------------------------------

    ZSortLT(head, ptr_index, buf_index, buf_size)
        struct LTList *head;   -Head of the list for sorting
        long ptr_index; -Index of pointer to buffer from LTList. If zero, then
                         the buffer is considered a part of the nodes. Normally,
                         ptr_index cannot be less than 2.

        long buf_index; -Index of character to sort on. When ptr_index is
                         zero, then buf_index is a part of the list nodes
                         themselves, and cannot be less than 8 (or
                         sizeof(struct LTList)).

        long buf_size;  -Size of the sort buffer (or field).

        WARN- ZSortLT is RECURSIVE. The recursion level is directly related
              to buf_size. On each recursion, ZSortLT will allocate approx.
              2048 bytes of memory, so total worst-case memory requirement
              will be 2048 * buf_size.

              On 68xxx hardware, the bytes in words and long words are placed
              most-significant first(lowest) in memory. Therefore, on such
              machines ZSortLT may be used to sort such integers.

        BUGS- At present, there is alot of overhead involved with initializing
              the 256 buckets.

******************************************************************************/

#include <exec/types.h>
#include "Tools.h"

void ZSortLT(head, ptr_index, buf_index, buf_size)
struct LTList *head;
long ptr_index, buf_index;
long buf_size;
{
    struct LTList *bucket;
    ULONG i;
    union data {
        struct LTList l;
        unsigned char *s[1];
        unsigned char c[1];
        } *d;

    if (bucket = calloc(256, sizeof(struct LTList)))
    {
        /* initialize buckets
        */
        for (i = 0; i < 256; ++i)
            InitLTHead(bucket + i);


        /* fill up buckets
        */
        while ((d = (void *) head->next) != (void *) head)
        {
            DeleteLT(&d->l);
            AddLTTail(bucket + ((ptr_index)
                                    ? d->s[ptr_index][buf_index]
                                    : d->c[buf_index]),
                                            &d->l);
        }

        /* Sort buckets
        */
        if (++buf_index, buf_size--)
            for (i = 0; i < 256; ++i)
                if (bucket[i].next != bucket[i].prev) /* if more than one in chain */
                    ZSortLT(&bucket[i], ptr_index, buf_index, buf_size);

        /* Recreate List
        */
        for (i = 0; i < 256; ++i)
            if (bucket[i].next != &bucket[i])
            {
                AddLTTail(head, &bucket[i]);
                DeleteLT(&bucket[i]);
            }

        free(bucket);
    }
}
