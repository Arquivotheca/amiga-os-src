/* Prototypes for functions defined in
smenu.c
 */

int startMenu(void);

static int returnMenuOK(int );

static int returnMenuDMR(int );

static int returnAllButMenuOK(int );

static int realStartMenu(void);

static int returnDMROK(int );

int startMenuKey(UWORD menunum);

static int returnMenuKey(int );

int dMenu(void);

int sendAllMenuClear(struct Screen * screen);

UWORD findMenuKey(struct Menu * menu,
                  int code);

