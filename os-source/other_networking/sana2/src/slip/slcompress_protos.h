/* Prototypes for functions defined in
slcompress.c
 */

void sl_compress_init(struct slcompress * comp);

u_char sl_compress_tcp(struct mbuf * m,
                       register struct ip * ip,
                       struct slcompress * comp,
                       ULONG compress_cid);

ULONG sl_uncompress_tcp(u_char ** bufp,
                        ULONG len,
                        ULONG type,
                        struct slcompress * comp);

