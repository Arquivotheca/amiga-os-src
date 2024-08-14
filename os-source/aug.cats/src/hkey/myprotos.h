#ifndef MYPROTOS_H
#define MYPROTOS_H

void handleCxMsg(struct Message *);
BOOL setupCX(char **);
void shutdownCX(void);
void   handleIMsg(struct IntuiMessage *);
struct Window *getNewWindow(void);
int    addGadgets(struct Window *, void *);
void   removeGadgets(void);
void setupCustomGadgets(struct Gadget **, void *);
void HandleGadget(ULONG,ULONG);
void setupCustomMenu(void);
void handleCustomMenu(UWORD);
void refreshWindow(void);
BOOL setupCustomCX(void);
void shutdownCustomCX(void);
void handleCustomCXMsg(ULONG);
void setupWindow(void);
void shutdownWindow(void);
void terminate(void);
BOOL MySetupCX(void);
void MyShutdownCX(void);
void MyHandleGadgets(ULONG,ULONG);
void MyHandleCXMsg(ULONG);
void FlushGadgets(void);
void refreshGadgets(void);
void MyaddGadgets(struct Gadget **);
void doExternal(UBYTE *);
void handleCustomCXCommand(ULONG);

/* hkinternal functions */
void doInternal(UBYTE *);
void makesize(int);
void cyclebackward(void);
void cycleforward(void);
BOOL shelltofront(void);

#endif /* MYPROTOS_H */
