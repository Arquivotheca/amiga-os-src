/* Prototypes for functions defined in c/TextTools.c */
void SetITFrontBack(int front,
                    int back);
void SetITDrawMode(int mode);
struct IntuiText *MakeIText(struct Remember **key,
                            UBYTE *text,
                            int left,
                            int top,
                            struct IntuiText *itchain,
                            struct TextAttr *ta);
struct IntuiText *MakeITList(struct Remember **key,
                             struct ITList *list);
