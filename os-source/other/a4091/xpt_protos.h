/* Prototypes for functions defined in
xpt.c
 */

LONG xpt_init(void);

CCB * xpt_ccb_alloc(void);

void xpt_ccb_free(CCB * );

LONG xpt_action(CCB * );

LONG xpt_bus_register(CAM_SIM_ENTRY * );

LONG xpt_bus_deregister(UBYTE );

void xpt_async(LONG , LONG , LONG , LONG , void * , LONG );

static struct xpt_Sim * find_sim(UBYTE );

static LONG reply_ccb(CCB * , LONG );

static LONG default_action(CCB * );

static struct xpt_LunInfo * find_unit(struct xpt_Sim * , CCB_HEADER * );

static LONG add_unit(struct xpt_Sim * , UBYTE , UBYTE , UBYTE * , ULONG );

static void __stdargs call_hook(CALLBACK * , LONG , ...);

static void spawn_scan(UBYTE );

static void __asm scan_callback(register __a0 CALLBACK * , register __a1 CCB * );

static UBYTE wait_callback(CCB * , UBYTE );

static void scan_path(void);

