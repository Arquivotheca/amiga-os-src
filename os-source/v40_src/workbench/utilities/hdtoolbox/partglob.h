/* HandlePart's globals - PartGlob.h */

extern struct PartitionBlock *CurrentPart,*PrevPart,*NextPart,*save_part;
extern struct PartitionBlock savecurrpart; /* for handlefs */

extern LONG PrevPMax,NextPMin,Cylinders;

extern ULONG DefaultEnvironment[17];

extern LONG PartUpdate,FSUpdate;

extern WORD Init;

extern UWORD *PartPattern;

extern struct IntuiText PartIText[];

extern BYTE use_advanced;
