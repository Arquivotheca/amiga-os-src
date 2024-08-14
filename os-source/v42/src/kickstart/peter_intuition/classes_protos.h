/* Prototypes for functions defined in
classes.c
 */

ULONG DoGadgetMethodA(struct Gadget * gad,
                      struct Window * win,
                      struct Requester * req,
                      Msg message);

ULONG IDoGadgetMethodA(struct Gadget * gad,
                       struct Window * win,
                       Msg message);

int SetGadgetAttrsA(struct Gadget * g,
                    struct Window * window,
                    struct Requester * req,
                    struct TagItem * taglist);

int SetAttrsA(Object * o,
              struct TagItem * tags);

int GetAttr(ULONG AttrID,
            Object * o,
            ULONG * StoragePtr);

extern ULONG (* classinitfuncs[16])(void);

int initIClasses(void);

extern UBYTE * ButtonGClassName;

extern UBYTE * FillRectClassName;

extern UBYTE * FrameIClassName;

extern UBYTE * FrButtonClassName;

extern UBYTE * GadgetClassName;

extern UBYTE * GroupGClassName;

extern UBYTE * ICClassName;

extern UBYTE * ImageClassName;

extern UBYTE * ITextIClassName;

extern UBYTE * ModelClassName;

extern UBYTE * PropGClassName;

extern UBYTE * RootClassName;

extern UBYTE * StrGClassName;

extern UBYTE * SysIClassName;

extern UBYTE * PointerClassName;

Object * NewObjectA(Class * cl,
                    ClassID classid,
                    struct TagItem * tags);

int DisposeObject(Object * o);

Object * NextObject(struct Node ** lnptr);

int sendNotifyIDCMP(struct opUpdate * msg);

