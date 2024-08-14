/****** mwhead/RequesterTools *************************************************

    Mitchell/Ware Systems, Inc          Version 1.00            19 Oct 1990

                Structs for RequesterTools

                RequesterTool Gadget List
                * An array of specifiers for the gadgets.
                  RTGLIST will cause these gadgets to be created for the
                  requester upon invokation. This array is initialized
                  for the first indicated parameters.

                * In the case of the Proportional gadget, 3 gadgets are
                  really created, chained off the first gadget referenced
                  by RTGlist->g. NOT FULLY IMPLEMENTED YET...

*****************************************************************************/
typedef enum RTRetCodes {   /* new return codes (to be adopted by RequesterTools & InterfaceTools) */
    rtr_na,                     /* not applicable - no code given */
    rtr_end,                    /* shut down */
    rtr_continue,               /* continue normal event processing (if idle routine- disable) */
    rtr_timed_recall,           /* call me upon next IntuiTick event */
    rtr_immediate_recall,       /* call me back right away  (idle routines only) */
    rtr_wait_until_next_event,  /* don't call me again until user does something (idle routines only) */
    } RTRetCodes;

typedef struct  RTGList { /* RequesterTool Gadget list - terminated with null */
                          /*                             entry    */

    /* The following will be most likely inited in a static array
    */
    short   left, top,          /* Location of Gadget in relation to requester */
            width, height,      /* Size of select box for gadget        */
            userid;             /* Special ID that user might use in
                                   identifing this gadget */
    long    type;               /* Gadget Type (Bool, String, or Prop)  */
    ULONG   flags,              /* Gadget Flags, some of which are automatic */
            activation,         /* Activation, some of which are automatic */
            general;            /* For Bool: Gadget Mutual exclusivity
                                       String: Buffer Length
                                       Prop: Prop Flags
                                       List: Background color */
    ULONG   control;            /* Control flags - see GadControl below */
    ULONG   fix;                /* NUMERICAL STRING GADGETS: Decimal
                                        point placement for LONGINT.
                                        If zero, normal longint
                                        Otherwise, alter the way
                                        the STRING gadget operates.

                                   BOOLEAN TOGGLE GADGETS: 32 more for
                                        mutual exclusivity. */

    struct  Image *norm, *sel;  /* Images to be rendered */

    BOOL    (*rfunct)();        /* Function to be called when gadget is hit
                                   Return of TRUE- End Requester
                                             FALSE- Don't End Requester- yet.
                                   Soon to be converted over to RTRetCodes */

    void    *rudata;            /* User data area */
    RTRetCodes (*ridle)();      /* Function to be called during IDLE times */

    short   custb, *custbXY;     /* Custom Border definition - relative to (r->left, r->top) */
    short   custb2, *custb2XY;   /* Custom Border definition #2 - relative to (r->left, r->top) */


    /* The following will never be initialized static array
    */
    struct  CustomGadgetClass *class;   /* class for this gadget */

    long    reserved[8];        /* reserved for future expansion- leave alone */

    long    value;              /* Parameter to/from String */
    UBYTE   *string;            /* Parameter to/from String or Prop */
    LTList  header;             /* List of elements to/from a custom gadget */

    USHORT  horizpot, vertpot;  /* Prop information        */
    USHORT  horizbody, vertbody; /* Prop information */
    BOOL    selected;           /* This gadget is selected if true */
    struct Gadget *g;           /* initialized by tools */
    } RTGLIST, RTGList, RTGEntry;   /* the RTGEntry merely denotes a single
                                       instance of RTGList */

/* NOTE: WIDTH is used to denote end of list. Everything should be set to
         NULL. */

/* Composite Gadget Types
*/
#define PROP_HORIZ  -1      /* a Horiz PROP with arrows! */
#define PROP_VERT   -2      /* a Vert PROP with arrows! */

/* Built_in Custom gadgets
*/
#define N_STATE     -3      /* an N-state gadget */
#define LIST_BOX    -4      /* a List Gadget    */
#define DISPLAY_BOX -5      /* Not a real gadget; just an area for display */

/* Event Handling (for Interface Tools)
*/
#define EVENT_ENTRY -666    /* type of a 'magic cokie' */

typedef struct CustomGadgetClass    {
    struct CustomGadgetClass *next; /* next one in chain */
    short   class;          /* Actual class value of gadget (negative numbers) */

    /* The following are called by DoRequest() and DoInterface() during normal processing
    */
    BOOL    (*create)();    /* Create an instance of this gadget class. return TRUE if successful */
    BOOL    (*handler)();   /* Real-time handler for custom gadgets */
    BOOL    (*remove)();    /* Get Rid of resources used by this gadget */

    /* The following are called whenever gadget information is modified/desired by
       the application.
    */
    BOOL    (*refresh)();   /* Refresh Gadget with new information in RTGEntry */
    BOOL    (*update)();    /* Update RTGEntry wih information from gadget */

    /* This little critter is currently not used. Set to NULL
    */
    BOOL    (*idle)();      /* What to do when no user activity is present? NIY */
    } CustomGadgetClass;

typedef union GadControl {
    struct {    /* BIT FIELD - add any new additions to BEGINNING and subtract from filler */
        unsigned int : 7,  /* filler (since this is reversed, must total 32 bits */

                      radio      : 1,   /* (bit 24) Radio button */

                      custb2     : 1,   /* (bit 23) custom border definition #2 */
                      custb      : 1,   /* (bit 22) custom border definition */

                      seperation : 4,   /* (bits 18-21) how far out to place box2 or shadow2 ( 0 defaults to 2) */
                      sizing     : 1,   /* bit for ProReq only */
                      xor        : 1,   /* bit 16 */

                      color_d    : 4,   /* dark color */

                      lower2     : 1,   /* 3-D lower recessed area extended (bit 11) */
                      raise2     : 1,

                      lower      : 1,   /* 3-D lower recessed area */
                      raise      : 1,

                      shadow2    : 1,
                      box2       : 1,

                      shadow     : 1,
                      box        : 1,

                      color_b    : 4;   /* bright color  */
        } cf;
    ULONG c;   /* control */
    } GadControl;

/* Control Flags    (first four control one of 16 colors )
*/
#define CF_COLORS       0x00000F    /* mask for colors       */
#define CF_COLORS2      0x007800    /* mask for dark colors */
#define CF_SEPERATION   0x3c0000    /* mask for seperation */

#define CF_CB(c)    ((c) & CF_COLORS)            /* color for bright */
#define CF_CD(c)    (((c) << 12) & CF_COLORS2)   /* color for dark */
#define CF_SE(c)    (((c) << 18) & CF_SEPERATION) /* how much to space out CF_xxx2 */

#define CF_0        0           /* (obsolete) draw a box in color 0 */
#define CF_1        1           /* (obsolete) draw a box in color 1 */
#define CF_2        2           /* (obsolete) draw a box in color 2 */
#define CF_3        3           /* (obsolete) draw a box in color 3 */
#define CF_4        4           /* (obsolete) draw a box in color 4 */
#define CF_5        5           /* (obsolete) draw a box in color 5 */
#define CF_6        6           /* (obsolete) draw a box in color 6 */
#define CF_7        7           /* (obsolete) draw a box in color 7 */
#define CF_UP       8           /* (obsolete) upper 8 colors */

#define CF_BOX      (1<<4)      /* draw a box around selectable area */
#define CF_SHADOW   (1<<5)      /* draw a shadow around selectable area */
#define CF_BOX2     (1<<6)      /* draw a second box around selectable area */
#define CF_SHADOW2  (1<<7)      /* draw a second shadow around selectable area */
#define CF_RAISE  (1<<8)        /* give the box that 3D look */
#define CF_LOWER  (1<<9)        /* give the box that 2nd 3D look- few pixels futher out */

#define CF_RAISE2 (1<<10)       /* give the box that 3D look */
#define CF_LOWER2 (1<<11)       /* give the box that 2nd 3D look- few pixels futher out */

#define CF_XOR      (1<<16)     /* draw mode is XOR instead of JAM1 */

#define CF_SIZING   (1<<17)     /* ProReq - display sizing handle */

#define CF_CUSTB    (1<<22)     /* Custom Border */
#define CF_CUSTB2   (1<<23)     /* Custom Border #2 */

#define CF_RADIO    (1<<24)     /* Radio button (size always controled by seperation) */

