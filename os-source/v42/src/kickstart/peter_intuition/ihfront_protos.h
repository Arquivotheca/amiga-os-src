/* Prototypes for functions defined in
ihfront.c
 */

static struct InputToken * newIEToken(struct InputEvent * , enum ITCommand );

int createInputTokens(struct InputEvent * ie);

static int convertRawKey(struct InputEvent * );

static int rawkeyButtonUpClone(struct InputEvent * );

static int createMouseToken(struct InputEvent * , UWORD , enum ITCommand );

static int rawkeyButtonDown(struct InputEvent * );

static int rawkeyCommandKeys(struct InputEvent * );

static int rawkeyCursorKeys(struct InputEvent * );

static int convertPointerPos(struct InputEvent * , struct LongPoint * );

static int newPointerPos(struct LongPoint * , struct InputEvent * , struct LongPoint * , int , int , struct TabletData * );

static int convertMouse(struct InputEvent * );

static int addButtonEvent(struct InputToken * , struct TabletData * );

static int metaDrag(struct InputEvent * );

static int initCursorInc(void);

static int repeatCursorInc(void);

static int kbdVMouse(struct InputEvent * );

static int kbdHMouse(struct InputEvent * );

