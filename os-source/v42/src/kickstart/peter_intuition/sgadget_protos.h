/* Prototypes for functions defined in
sgadget.c
 */

int startGadget(int (* gadgetreturn)());

static int ghResult(USHORT , LONG );

static int newActiveGadget(struct Gadget * , USHORT );

static int returnSysGadget(void);

static int turnOffGadget(LONG );

static int returnGadget(void);

int dGadget(void);

static int deferIf(int );

