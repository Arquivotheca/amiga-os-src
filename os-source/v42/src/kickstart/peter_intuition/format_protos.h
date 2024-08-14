/* Prototypes for functions defined in
format.c
 */

struct ITList * formatITList(UBYTE * fmt,
                             UWORD * arglist,
                             UWORD ** nextargp,
                             struct Remember ** remkey,
                             int separator);

static struct IntuiText * getITextList(int , struct Remember ** );

