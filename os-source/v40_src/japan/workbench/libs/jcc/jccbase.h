/*
 *  jccbase.h - our library base...
 *
 */

struct JCCLibrary
{
    struct  Library jl_Lib;
    ULONG   jl_SegList;
    ULONG   jl_Flags;
    APTR    jl_SysBase;
    APTR    jl_UtilityBase;
    LONG    jl_Data;
};
