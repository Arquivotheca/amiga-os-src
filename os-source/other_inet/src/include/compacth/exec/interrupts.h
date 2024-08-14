€ˆEXEC_INTERRUPTS_H€EXEC_INTERRUPTS_HˆµŒ"exec/nodes.h"‡ˆ·Œ"exec/lists.h"‡
ƒInterrupt{
ƒ¬is_Node;
”is_Data;
VOID(*is_Code)();
};
ƒIntVector{
”iv_Data;
VOID(*iv_Code)();
ƒ¬*iv_Node;
};
ƒSoftIntList{
ƒ®sh_List;
‰sh_Pad;
};€SIH_PRIMASK (0xf0)€INTB_NMI 15€INTF_NMI (1L<<15)‡