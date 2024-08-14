/* Prototypes for functions defined in
vectorclass.c
 */

static LONG Scale(LONG , LONG , LONG );

extern struct VectorInfo VectorImages[13];

Class * initSysIClass(void);

static ULONG dispatchSysI(Class * , Object * , Msg );

static int drawSysI(Class * , Object * , struct impDraw * );

static BOOL setState(struct Image * , struct SysIData * , LONG , struct SysIClassData * );

static BOOL createStateImagery(struct Image * , struct SysIData * , struct SysIClassData * , struct VectorInfo * , LONG , LONG , struct DrawInfo * );

static BOOL allocRenderStuff(struct Image * , struct SysIData * , struct SysIClassData * );

static void freeRenderStuff(struct SysIClassData * );

static BOOL allocImageMem(struct Image * , struct SysIData * );

static void freeImageMem(struct SysIData * );

static void drawVectorImage(struct RastPort * , struct VectorInfo * , LONG , LONG , ULONG , struct DrawInfo * );

static void Draw1P3DBorder(UWORD * , struct RastPort * , LONG , LONG , LONG , LONG );

static void myRectangle(struct RastPort * , LONG , LONG , LONG , LONG );

static int WritePixelPen(int , int , int , int );

static int MoveDrawPen(int , int , int , int , int , int );

