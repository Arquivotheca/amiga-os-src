/* Prototypes for functions defined in c/FatalRequest_BusyWait.c */
void *BusyWait(char *text);
long BusyCheck(void *handle,
               long progress,
               long goal);
void BusyClear(void *handle);
