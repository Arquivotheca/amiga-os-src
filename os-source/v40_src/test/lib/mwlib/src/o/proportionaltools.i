/* Prototypes for functions defined in c/ProportionalTools.c */
struct Gadget *MakeVertProp(struct Remember **key,
                            struct Gadget *gadlist);
struct Gadget *MakeHorizProp(struct Remember **key,
                             struct Gadget *gadlist);
struct Gadget *MakeProps(struct Remember **key,
                         struct Gadget *gadlist);
struct Gadget *MakeIndHorizProp(struct Remember **key,
                                struct Image *im,
                                int Left,
                                int Top,
                                int Length,
                                ULONG Flags,
                                ULONG Activation);
struct Gadget *MakeIndVertProp(struct Remember **key,
                               struct Image *im,
                               int Left,
                               int Top,
                               int Length,
                               ULONG Flags,
                               ULONG Activation);
