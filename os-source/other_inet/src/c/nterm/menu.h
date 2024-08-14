#include "serial.h"
#include "meta.h"

/* MENU STRIPS */

#define MENUPROJECT 0
#define MENUTERMINAL 1
#define MENUSETUP 2

/*----------------------------------------------------------------------*/
/* MENUITEMS */

/* MENUPROJECT MENUITEMS */
#define ITEMSAVE	0
#define ITEMLOAD	1
#define ITEMRESET	2
#define ITEMBREAK 	3
#define ITEMWHO 	4
#define ITEMQUIT 	5


/* MENUTERMINAL MENUITEMS */
#define ITEMTTY 0
#define ITEMVT52 1
#define ITEMVT100 2
#define ITEMAMIGA 3
#define NUMTERMINAL 4


/* MENUSETUP MENUITEMS */
#define ITEMLINE_WRAP	0
#define ITEMCTRL_CHARS	1
#define ITEMTABS		2
#define ITEMBELL		3
#define ITEMDELANDBKSP	4
#define ITEMCOLUMN 		5
#define ITEMROW 		6
#define ITEMCOLOR		7
#define ITEMFUNC		8

/*----------------------------------------------------------------------*/
/* SUBITEMS */

/* LINE WRAP */
#define SUBITEMLINEWRAPON 0
#define SUBITEMLINEWRAPOFF 1
#define NUMWRAP 2

/* CTRL CHARS */
#define SUBITEMEXECUTED 0
#define SUBITEMVISIBLE 1
#define NUMCTRL 2

/* BELL */
#define AUDIBLE 0
#define SILENT 1
#define NOFLASH 2
#define NUMBELL 3

/* COLUMNS */
#define COLUMN80	0
#define COLUMN128	1
#define COLOK		2
#define NUMCOLUMN	2

/* ROWS */
#define ROW24 0
#define ROW49 1
#define ROWOK 2
#define NUMROW 2

/* DEL and BACKSPACE */
#define SUBITEMDELNORMAL	0
#define SUBITEMDELSWAPPED	1
#define NUMDEL			2

#define SEND 0
#define RECEIVE 1
