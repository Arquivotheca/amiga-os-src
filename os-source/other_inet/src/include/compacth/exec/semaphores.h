��EXEC_SEMAPHORES_H�EXEC_SEMAPHORES_H���"exec/nodes.h"����"exec/lists.h"����"exec/ports.h"��EXEC_TASKS_H�"exec/tasks.h"�
�Semaphore{
��sm_MsgPort;
�sm_Bids;
};�sm_LockMsg mp_SigTask
�SemaphoreRequest{
�MinNode sr_Link;
�Task*sr_Waiter;
};
�SignalSemaphore{
��ss_Link;
�ss_NestCount;
�MinList ss_WaitQueue;
�SemaphoreRequest ss_MultipleLink;
�Task*ss_Owner;
�ss_QueueCount;
};�