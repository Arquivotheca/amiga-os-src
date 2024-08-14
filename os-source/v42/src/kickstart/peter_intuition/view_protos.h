/* Prototypes for functions defined in
view.c
 */

static ULONG CalcISG(struct Screen * );

int MakeScreen(struct Screen * screen);

int newMakeScreen(struct Screen * screen,
                  BOOL rethink);

static int modeSnoop(struct Screen * , int );

static int fixBitMapPokers(struct Screen * );

int RemakeDisplay(void);

int RethinkDisplay(void);

static int rethinkVPorts(int );

int modeVerify(int force);

static int laceCheck(void);

static int monitorCheck(void);

static int remakeCopList(void);

