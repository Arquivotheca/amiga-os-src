ÄàEXEC_SEMAPHORES_HÄEXEC_SEMAPHORES_Hàµå"exec/nodes.h"áà∑å"exec/lists.h"áà∏å"exec/ports.h"áàEXEC_TASKS_Hå"exec/tasks.h"á
ÉSemaphore{
É©sm_MsgPort;
òsm_Bids;
};Äsm_LockMsg mp_SigTask
ÉSemaphoreRequest{
ÉMinNode sr_Link;
ÉTask*sr_Waiter;
};
ÉSignalSemaphore{
É¨ss_Link;
ïss_NestCount;
ÉMinList ss_WaitQueue;
ÉSemaphoreRequest ss_MultipleLink;
ÉTask*ss_Owner;
ïss_QueueCount;
};á