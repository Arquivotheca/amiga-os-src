/****** mwhead/InterfaceTools *************************************************

    Mitchell/Ware Systems, Inc          Version 1.00            19 Oct 1990

                Structs for InterfaceTools

                Interface Tool Gadget/Event List

                * An array of specifiers for the gadgets & events.
                  IntGEntry will cause these gadgets to be created for the
                  interface upon invokation. This array is initialized
                  for the first indicated parameters.

                * IntGEntry is similar to RTGList and uses many of its flags,
                  etc. Eventually, IntGEntry will phase RTGList out.
*****************************************************************************/

typedef struct  IntGEntry { /* InterfaceTool Gadget/Event Entry - terminated with null */
    /* The following will be most likely inited in a static array
    */
    short   left, top,          /* Location of Gadget in relation to requester */
            width, height,      /* Size of select box for gadget        */
            userid;             /* Special ID that user might use in
                                   identifing this gadget */
    long    type;               /* Type (Bool, String, Prop, or Event)  */
    ULONG   flags,              /* Gadget Flags | IDCMP Event */
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

    short   custb, *custbXY;    /* Custom Border definition - relative to (r->left, r->top) */
    short   custb2, *custb2XY;  /* Custom Border definition #2 - relative to (r->left, r->top) */
    } IntGEntry;

typedef struct IntGEActive {    /* active correspondence to IntGEEntry */
    IntGEntry *entry;                   /* pointer (for convience) this refers to */

    struct  CustomGadgetClass *class;   /* class for this gadget */

    long    value;              /* Parameter to/from String */
    UBYTE   *string;            /* Parameter to/from String or Prop */
    LinkT   header;             /* List of elements to/from a custom gadget */

    USHORT  horizpot, vertpot;  /* Prop information        */
    USHORT  horizbody, vertbody; /* Prop information */
    BOOL    selected;           /* This gadget is selected if true */
    struct Gadget *g;           /* initialized by tools (NULL if this is an event) */
    } IntGEActive;

typedef struct IntGEHandle  {   /* handle allocated via AllocIntGEHandle()  (READ-ONLY) */
    /*------------------------------------------------------------
        Public, READ-ONLY Area
    ------------------------------------------------------------*/
    /* Vital structures
    */
    LTList loop;                    /* loop of IntGEHandle structures (coordinated) */

    struct Remember *key;           /* Key for all memory allocations */
    struct MenuPac *menu;           /* User-supplied MenuPac (we attach to window), or NULL */
    ITList *it;                     /* User Text to be displayed */
    IntGEntry *ige;                 /* User IntGEntry array */
    IntGEActive *iga;               /* Active Array allocated via AllocIntGEHandle */

    /* DoInterface() Events area for callbacks
    */
    ULONG class, code, qualifier;   /* Class & Code of last event */
    short x, y;                     /* Mouse x & y position for this event/gadget */

    ULONG reserved[8];              /* Reserved area (now inited to NULL) */

    /*-----------------------------------------------------------------------
        Private Area (subject too change. DO NOT USE IN EXTENAL ROUTINES.)
    ------------------------------------------------------------------------*/
    struct ExtNewWindow *nw;        /* Used to open the window 1.3-style */
    struct TagItem *wtags;          /* Used to open the window 2.0-style */
    struct Window *window;          /* The Window - or NULL if not opened */
    struct Gadget *gadget;          /* list of gadgets for this window */
    struct Border *border;          /* set of Borders for this window */
    struct IntuiText *text;         /* list of IntuiText structures */
    void *udata;                    /* User-supplied data */
    RTRetCodes (*idle)();           /* User-supplied idle routine */

    /* Events
    */
    struct Gadget *mgad;    /* gadget that generated the message */
    struct Window *mwin;    /* window message enamated from */
    ULONG seconds, micros;  /* intui time */

    /* User Functions area
    */
    RTRetCodes ret;
    } IntGEHandle;
