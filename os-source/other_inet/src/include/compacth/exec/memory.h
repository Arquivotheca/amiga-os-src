ÄàEXEC_MEMORY_HÄEXEC_MEMORY_Hàµå"exec/nodes.h"á
ÉMemChunk{
ÉMemChunk*mc_Next;
ómc_Bytes;
};
ÉMemHeader{
É¨mh_Node;
âmh_Attributes;
ÉMemChunk*mh_First;
îmh_Lower;
îmh_Upper;
ómh_Free;
};
ÉMemEntry{
´{
ómeu_Reqs;
îmeu_Addr;
}me_Un;
óme_Length;
};Äme_un me_UnÄme_Reqs me_Un.meu_ReqsÄme_Addr me_Un.meu_Addr
ÉMemList{
É¨ml_Node;
âml_NumEntries;
ÉMemEntry ml_ME[1];
};Äml_me ml_MEÄMEMF_PUBLIC (1<<0)ÄMEMF_CHIP (1<<1)ÄMEMF_FAST (1<<2)ÄMEMF_CLEAR (1L<<16)ÄMEMF_LARGEST (1L<<17)ÄMEM_BLOCKSIZE 8ÄMEM_BLOCKMASK 7á