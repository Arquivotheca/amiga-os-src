
/* main.c */
LONG main ( void );
struct Gadget *creategadget ( struct GlobalData *gd , ULONG kind , struct Gadget *prev , struct NewGadget *ng , ULONG data , ...);
void setgadgetattrs ( struct GlobalData *gd , ULONG id , ULONG data , ...);
struct Menu *createmenus ( struct GlobalData *gd , struct NewMenu *nm , ULONG data , ...);
BOOL layoutmenus ( struct GlobalData *gd , ULONG data , ...);
struct Window *openwindowtags ( struct GlobalData *gd , ULONG data , ...);
void lprintf ( struct GlobalData *gd , STRPTR fmt , void *arg1 , ...);
VOID FindLayers ( struct GlobalData *gd );
void GetIconArgs ( struct GlobalData *gd , struct WBArg *wa );
BOOL CreateMsgPorts ( struct GlobalData *gd );
void DeleteMsgPorts ( struct GlobalData *gd );
void ProcessCommand ( struct GlobalData *gd , ULONG id , struct IntuiMessage *imsg );
void CheckMemory ( struct GlobalData *gd , ULONG flags );
void ShowMungBlock ( struct GlobalData *gd , struct MemCheck *mc );
ULONG CheckAllocatedMemory ( struct GlobalData *gd , ULONG *ustart , ULONG usize );

/* freemem.c */
void ASM newFreeMem ( REG (a1 )UBYTE *memb , REG (d0 )ULONG size , REG (a2 )ULONG *stackptr );

/* window.c */
BOOL OpenCWindow ( struct GlobalData *gd );
void CloseCWindow ( struct GlobalData *gd );

/* private.c */
struct MemHeader *AllocMemHeader ( struct GlobalData *gd );
void FreePrivateList ( struct GlobalData *gd );
void FreeMemHeader ( struct GlobalData *gd , struct MemHeader *mh );

/* info.c */
void ASM MemoryDump ( REG (a0 )struct GlobalData *gd , REG (d0 )STRPTR prefix , REG (a1 )char *ptr , REG (d1 )ULONG size );
BOOL ASM GetHunkInfo ( REG (a0 )struct GlobalData *gd , REG (a1 )struct Task *tc , REG (a2 )ULONG *stackptr , REG (a3 )ULONG *h , REG (a4 )ULONG *o );
void ASM ShowProcessInfo ( REG (a0 )struct GlobalData *gd , REG (a1 )ULONG *stackptr , REG (a2 )struct MemCheck *mc );
