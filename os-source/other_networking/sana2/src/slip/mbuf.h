#include <exec/types.h>
#include <exec/ports.h>
#include <clib/exec_protos.h>

struct mbuf
{
    char *m_off;
    long m_len;
};

