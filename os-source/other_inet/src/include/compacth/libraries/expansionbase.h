��LIBRARIES_EXPANSIONBASE_H�LIBRARIES_EXPANSIONBASE_H�EXEC_TYPES_H�"exec/types.h"��EXEC_LIBRARIES_H�"exec/libraries.h"��EXEC_INTERRUPTS_H�"exec/interrupts.h"��EXEC_SEMAPHORES_H�"exec/semaphores.h"��LIBRARIES_CONFIGVARS_H�"libraries/configvars.h"��TOTALSLOTS 256
�ExpansionInt
{
�IntMask;
�ArrayMax;
�ArraySize;
};
��
{
�Library LibNode;
��;
�pad;
�ExecBase;
�SegList;
�CurrentBinding CurrentBinding;
��BoardList;
��MountList;
�AllocTable[TOTALSLOTS];
�SignalSemaphore BindSemaphore;
�Interrupt Int2List;
�Interrupt Int6List;
�Interrupt Int7List;
};�