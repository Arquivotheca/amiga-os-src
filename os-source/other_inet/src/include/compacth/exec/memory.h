��EXEC_MEMORY_H�EXEC_MEMORY_H���"exec/nodes.h"�
�MemChunk{
�MemChunk*mc_Next;
�mc_Bytes;
};
�MemHeader{
��mh_Node;
�mh_Attributes;
�MemChunk*mh_First;
�mh_Lower;
�mh_Upper;
�mh_Free;
};
�MemEntry{
�{
�meu_Reqs;
�meu_Addr;
}me_Un;
�me_Length;
};�me_un me_Un�me_Reqs me_Un.meu_Reqs�me_Addr me_Un.meu_Addr
�MemList{
��ml_Node;
�ml_NumEntries;
�MemEntry ml_ME[1];
};�ml_me ml_ME�MEMF_PUBLIC (1<<0)�MEMF_CHIP (1<<1)�MEMF_FAST (1<<2)�MEMF_CLEAR (1L<<16)�MEMF_LARGEST (1L<<17)�MEM_BLOCKSIZE 8�MEM_BLOCKMASK 7�