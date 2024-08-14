
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <intuition/gadgetclass.h>
#include <exec/lists.h>
#include <exec/nodes.h>
#include <graphics/displayinfo.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <dos/dos.h>
#include <stdio.h>

#include <pragmas/dos_pragmas.h>
#include <pragmas/exec_pragmas.h>
#include <pragmas/utility_pragmas.h>
#include <pragmas/iffparse_pragmas.h>

#define MIN(a,b) ((a)<=(b)?(a):(b))

#define STRINGSIZE        84

#define TYPE_VOID          0
#define TYPE_WINDOW        1
#define TYPE_GADGET        2
#define TYPE_INTEGER       3
#define TYPE_STRING        4
#define TYPE_STRINGLIST    5
#define TYPE_FUNCTION      6
#define TYPE_CHARACTER     7
#define TYPE_PLACE         8
#define TYPE_FREEDOM       9
#define TYPE_WORDLIST     10
#define TYPE_USER         11
#define TYPE_SCREEN       12
#define TYPE_LINKEDLIST   13
#define TYPE_IGNORE       14

#define ID_TMUI MAKE_ID('T','M','U','I')  /* FORM: Project file */
#define ID_TMSE MAKE_ID('T','M','S','E')  /* FORM: Settings file */

#define ID_THDR MAKE_ID('T','H','D','R')  /* Chunk: Header */
#define ID_ANNO MAKE_ID('A','N','N','O')  /* Chunk: Annotation */

#define ID_SATB MAKE_ID('S','A','T','B')  /* Chunk: Auto Top Border setting */
#define ID_SCOM MAKE_ID('S','C','O','M')  /* Chunk: Comments setting */
#define ID_SGSZ MAKE_ID('S','G','S','Z')  /* Chunk: Grid Size setting */
#define ID_SPRA MAKE_ID('S','P','R','A')  /* Chunk: Pragmas setting */
#define ID_SICN MAKE_ID('S','I','C','N')  /* Chunk: Create Icons setting */
#define ID_SUSG MAKE_ID('S','U','S','G')  /* Chunk: User Signal setting */
#define ID_SARX MAKE_ID('S','A','R','X')  /* Chunk: SimpleRexx setting */
#define ID_SCHP MAKE_ID('S','C','H','P')  /* Chunk: __chip Keyword setting */
#define ID_SUDA MAKE_ID('S','U','D','A')  /* Chunk: UserData setting */
#define ID_SFRQ MAKE_ID('S','F','R','Q')  /* Chunk: ASL File Requester setting */

#define ID_TSCR MAKE_ID('T','S','C','R')  /* FORM: Screen */
#define ID_TSDA MAKE_ID('T','S','D','A')  /* Chunk: Screen data */
#define ID_CMAP MAKE_ID('C','M','A','P')  /* Chunk: Color map */

#define ID_TFON MAKE_ID('T','F','O','N')  /* FORM: Font */
#define ID_TTAT MAKE_ID('T','T','A','T')  /* Chunk: Text Attr */

#define ID_TWIN MAKE_ID('T','W','I','N')  /* FORM: Window */
#define ID_TWDA MAKE_ID('T','W','D','A')  /* Chunk: Window data */

#define ID_TMLS MAKE_ID('T','M','L','S')  /* FORM: Menu list */
#define ID_TMEN MAKE_ID('T','M','E','N')  /* FORM: Menu */
#define ID_TMDA MAKE_ID('T','M','D','A')  /* Chunk: Menu data */
#define ID_TITE MAKE_ID('T','I','T','E')  /* FORM: Item */
#define ID_TIDA MAKE_ID('T','I','D','A')  /* Chunk: Item data */
#define ID_TSID MAKE_ID('T','S','I','D')  /* Chunk: Subitem data */

#define ID_TGAD MAKE_ID('T','G','A','D')  /* FORM: Gadtools Gadget */
#define ID_TGDA MAKE_ID('T','G','D','A')  /* Chunk: Gadtools Gadget data */

#define ID_TTAG MAKE_ID('T','T','A','G')  /* FORM: Toolmaker tags */
#define ID_TNIN MAKE_ID('T','N','I','N')  /* Chunk: Non-interactive tag */
#define ID_TCHA MAKE_ID('T','C','H','A')  /* Chunk: Character tag */
#define ID_TINT MAKE_ID('T','I','N','T')  /* Chunk: Integer tag */
#define ID_TWLS MAKE_ID('T','W','L','S')  /* Chunk: Word List tag */
#define ID_TSTR MAKE_ID('T','S','T','R')  /* Chunk: String tag */
#define ID_TSLS MAKE_ID('T','S','L','S')  /* Chunk: String List tag */
#define ID_TLLS MAKE_ID('T','L','L','S')  /* Chunk: Linked List tag */

#define TMFILEERR_NONE      0
#define TMFILEERR_OPEN      1
#define TMFILEERR_WRONGTYPE 2
#define TMFILEERR_OUTOFMEM  3
#define TMFILEERR_READ      4
#define TMFILEERR_WRITE     5

#define MODE_DEFAULT       0x00000000
#define MODE_NTSC          0x00000001
#define MODE_PAL           0x00000002
#define MODE_INTERLACE     0x00000004
#define MODE_HIRES         0x00000008
#define MODE_SUPERHIRES    0x00000010
#define MODE_PRODUCTIVITY  0x00000020
#define MODE_A202410HZ     0x00000040
#define MODE_A202415HZ     0x00000080
#define MODE_OSCANTEXT     0x00000100
#define MODE_OSCANSTANDARD 0x00000200
#define MODE_OSCANMAX      0x00000400
#define MODE_OSCANVIDEO    0x00000800
#define MODE_OSCANNONE     0x00001000
#define MODE_CUSTOMCOLORS  0x00002000
#define MODE_DEFAULTWIDTH  0x00004000
#define MODE_DEFAULTHEIGHT 0x00008000
#define MODE_WORKBENCH     0x80000000

#define WINDOWFLAG_GZZ          0x0001
#define WINDOWFLAG_DISABLED     0x0002
#define WINDOWFLAG_OPENATSTART  0x0004

#define SCREENFLAG_OPENATSTART  0x0004

#define MENUFLAG_AUTOEXCLUDE    0x0001

