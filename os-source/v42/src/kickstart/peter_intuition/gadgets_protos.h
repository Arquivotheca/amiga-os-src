/* Prototypes for functions defined in
gadgets.c
 */

USHORT AddGList(struct Window * window,
                struct Gadget * gadget,
                UWORD pos,
                UWORD numgad,
                struct Requester * requester);

USHORT RemoveGList(struct Window * window,
                   struct Gadget * gadget,
                   WORD numgad);

int delinkGadgetList(struct Gadget * gadget,
                     struct Gadget * firstgadget,
                     int numgad);

int inGList(struct Gadget * gadget,
            struct Gadget * firstgadget,
            int numgad);

int OnGadget(struct Gadget * gadget,
             APTR ptr,
             struct Requester * req);

int OffGadget(struct Gadget * gadget,
              APTR ptr,
              struct Requester * req);

int NewModifyProp(struct Gadget * gad,
                  struct Window * win,
                  struct Requester * req,
                  USHORT flags,
                  USHORT hpot,
                  USHORT vpot,
                  USHORT hbody,

         USHORT vbody,
                  WORD numgad);

int IModifyProp(struct Gadget * gad,
                struct Window * win,
                struct PropPacket * pp);

int sniffWindowGadgets(struct Window * w,
                       struct Gadget * g,
                       int numgad);

int initGadgets(struct Window * win,
                struct Gadget * g,
                LONG numgad);

