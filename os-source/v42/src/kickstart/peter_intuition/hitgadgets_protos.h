/* Prototypes for functions defined in
hitgadgets.c
 */

BOOL hitGadgets(struct Layer * layer);

ULONG hitGGrunt(struct Gadget * g,
                struct GadgetInfo * gi,
                struct IBox * gbox,
                ULONG methodid);

int gadgetHelpTimer(void);

int HelpControl(struct Window * helpwin,
                ULONG help);

