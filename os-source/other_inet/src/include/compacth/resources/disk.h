ÄàRESOURCES_DISK_HÄRESOURCES_DISK_HàEXEC_TYPES_Hå"exec/types.h"áà∑å"exec/lists.h"áà∏å"exec/ports.h"áàEXEC_INTERRUPTS_Hå"exec/interrupts.h"áàEXEC_LIBRARIES_Hå"exec/libraries.h"á
ÉDiscResourceUnit{
ÉØdru_Message;
ÉInterrupt dru_DiscBlock;
ÉInterrupt dru_DiscSync;
ÉInterrupt dru_Index;
};
ÉDiscResource{
ÉLibrary dr_Library;
ÉDiscResourceUnit*dr_Current;
ädr_Flags;
ädr_pad;
ÉLibrary*dr_SysLib;
ÉLibrary*dr_CiaResource;
ódr_UnitID[4];
ÉÆdr_Waiting;
ÉInterrupt dr_DiscBlock;
ÉInterrupt dr_DiscSync;
ÉInterrupt dr_Index;
};ÄDRB_ALLOC0 0ÄDRB_ALLOC1 1ÄDRB_ALLOC2 2ÄDRB_ALLOC3 3ÄDRB_ACTIVE 7ÄDRF_ALLOC0 (1<<0)ÄDRF_ALLOC1 (1<<1)ÄDRF_ALLOC2 (1<<2)ÄDRF_ALLOC3 (1<<3)ÄDRF_ACTIVE (1<<7)ÄDSKDMAOFF 0x4000ÄDISKNAME "disk.resource"ÄDR_ALLOCUNIT (LIB_BASE-0*LIB_VECTSIZE)ÄDR_FREEUNIT (LIB_BASE-1*LIB_VECTSIZE)ÄDR_GETUNIT (LIB_BASE-2*LIB_VECTSIZE)ÄDR_GIVEUNIT (LIB_BASE-3*LIB_VECTSIZE)ÄDR_GETUNITID (LIB_BASE-4*LIB_VECTSIZE)ÄDR_LASTCOMM (DR_GIVEUNIT)ÄDRT_AMIGA (0x00000000)ÄDRT_37422D2S (0x55555555)ÄDRT_EMPTY (0xFFFFFFFF)á