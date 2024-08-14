/* Prototypes for functions defined in
wbench.c
 */

extern UBYTE WBPUBNAME[10];

struct Screen * findWorkBench(void);

struct Screen * openwbscreen(void);

struct Screen * openWorkBench(void);

int pokeWorkbench(void);

BOOL GetScreenData(struct Screen * buffer,
                   UWORD size,
                   UWORD type,
                   struct Screen * customsc);

