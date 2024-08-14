/* Prototypes for functions defined in
wtags.c
 */

extern ULONG windowFlagsPackTable[22];

struct Window * OpenWindowTagList(struct NewWindow * nw,
                                  struct TagItem * tags);

int fixWindowBorders(void);

