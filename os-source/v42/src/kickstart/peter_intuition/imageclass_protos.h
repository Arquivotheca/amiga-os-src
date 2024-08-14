/* Prototypes for functions defined in
imageclass.c
 */

Class * initImageClass(void);

extern ULONG imagePackTable[9];

static ULONG imageDispatch(Class * , Object * , Msg );

static int drawImage(Class * , Object * , struct impDraw * );

static int eraseImage(Class * , Object * , struct impErase * );

static int hittestImage(Class * , Object * , struct impHitTest * );

static int getImageAttr(Class * , Object * , struct opGet * );

int DrawImage(struct RastPort * rport,
              struct Image * image,
              int xoffset,
              int yoffset);

int DrawImageState(struct RastPort * rport,
                   struct Image * image,
                   int xoffset,
                   int yoffset,
                   int state,
                   struct DrawInfo * drinfo);

int PointInImage(struct Point point,
                 struct Image * image);

int EraseImage(struct RastPort * rport,
               struct Image * image,
               int xoffset,
               int yoffset);

int sendCompatibleImageMessage(struct Image * image,
                               ULONG msg1);

