��EXEC_INTERRUPTS_H�EXEC_INTERRUPTS_H���"exec/nodes.h"����"exec/lists.h"�
�Interrupt{
��is_Node;
�is_Data;
VOID(*is_Code)();
};
�IntVector{
�iv_Data;
VOID(*iv_Code)();
��*iv_Node;
};
�SoftIntList{
��sh_List;
�sh_Pad;
};�SIH_PRIMASK (0xf0)�INTB_NMI 15�INTF_NMI (1L<<15)�