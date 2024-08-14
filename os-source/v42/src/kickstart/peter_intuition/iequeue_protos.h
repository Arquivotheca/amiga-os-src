/* Prototypes for functions defined in
iequeue.c
 */

int initIEvents(void);

int reclaimIEvents(void);

struct InputEvent * cloneIEvent(struct InputEvent * ie);

struct InputEvent * queueIEvent(void);

struct InputEvent * returnIEvents(void);

