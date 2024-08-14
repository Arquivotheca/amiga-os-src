/* Prototypes for functions defined in c/GadgetIntTools.c */
struct Gadget *MakeIntGEList(struct Remember **key,
                             struct IntGEActive *list,
                             struct CustomGadgetClass *cgc);
void RefreshIntGEList(struct IntGEActive *list,
                      struct Window *win);
int MakeIntGEBorders(struct Remember **key,
                     struct IntGEActive *iga);
