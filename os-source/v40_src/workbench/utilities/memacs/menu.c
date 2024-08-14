/* menu.c */

#include "ed.h"

#if !AMIGA
#define JAM2 0
#endif

#define MENUPROJECT 0
#define MENUEDIT 1 
#define MENUWINDOWS 2  
#define MENUMOVES 3
#define MENULINES 4
#define MENUWORDS 5
#define MENUSEARCH 6
#define MENUMACRO 7

#define BLUE 0
#define WHITE 1

#define SIXTYADJUST 30

struct TextAttr topaz80 = {
    "topaz.font",
    8,
    FS_NORMAL,
    FPF_ROMFONT
};

struct IntuiText cmdtext[] = {
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Echo",		/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Bind-key",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Unbind-key",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Describe-key",		/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Tab",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Back-char",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Forw-del-char",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Forw-char",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Back-del-char",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Newline",		/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText projectext[] = {
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Rename       ^X F",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Read-file    ^X^R",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Visit-file   ^X^V",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Insert-file  ^X^I",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Save-file    ^X^S",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Save-as-file ^X^W",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Save-mod     ^X^M",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Save-exit    ^X^F",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "New-Cli      ^- ",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Cli-command  ^X!",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Quit         ^C",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "About ...",	/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText editext[] = {
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Kill-region   ^W",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Yank          ^Y",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Set-mark      ^@",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Copy-region   ESCw",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Upper-region  ^X^U",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Lower-region  ^X^L",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "List-buffers  ^X^B",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Select-buffer ^Xb",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Insert-buffer ESC^Y",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Kill-buffer   ^Xk",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Justify-buffer ^XJ",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Redisplay     ^L",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Quote-char    ^Q",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Indent        ^J",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Transpose     ^T",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Cancel        ^G",	/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText windowtext[] = {
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "One-window    ^X1",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Split-window  ^X2",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Next-window   ^Xn",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Prev-window   ^Xp",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Expand-window ^Xz",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Shrink-window ^X^Z",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Next-w-page * ESC^V",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Prev-w-page * ^Xv",/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText movetext[] = {
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Top-of-buffer ESC<",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "End-of-buffer ESC>",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Top-of-window ESC,",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "End-of-window ESC.",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Goto-line     ^X^G",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Swap-dot&mark ^X^X",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Next-page     ^V",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Prev-page     ESCv",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Next-word     ESCf",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Previous-word ESCb",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Scroll-up     ^Z",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Scroll-down   ESCz",/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText linetext[]= {
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Open-line     ^O",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Kill-line     ^X^D",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Kill-to-eol   ^K",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Start-of-line ^A",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "End-of-line   ^E",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Next-line     ^N",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Previous-line ^P",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Line-to-top   ESC!",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Delete-blanks ^X^O",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE, WHITE, JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Show-Line#    ^X=",	/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText wordtext[] = {
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "delete-forw  ESCd",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "delete-back  ESCh",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Upper-word   ESCu",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Lower-word   ESCl",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Cap-word     ESCc",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Switch-case  ESC^",/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText searchtext[] = {
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Search-forward   ^S",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Search-backward  ^R",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Search-replace ESCr",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Query-s-r      ESCq",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Fence-match    ESC^F",/* IText */
    NULL		/* NextText */
  }
};

struct IntuiText macrotext[] = {
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Set-arg        ^U",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Set          ESCs",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Start-macro   ^X(",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Stop-macro    ^X)",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Execute-macro ^Xe",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Set-key      ^X^K",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Reset-keys   ESCk",	/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Execute-file ESCe",/* IText */
    NULL		/* NextText */
  },
  {
    BLUE,WHITE,JAM2,	/* FrontPen, BackPen, DrawMode */
    0, 0,		/* LeftEdge, TopEdge */
    &topaz80,		/* ITextFont */
    "Execute-line ^[^[",	/* IText */
    NULL		/* NextText */
  }
};

#if AMIGA

struct MenuItem projectitems[] = {
  {
    &projectitems[1],		/* next item */
    0, 0, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[0],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[2],		/* next item */
    0, 10, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[3],		/* next item */
    0, 20, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[4],		/* next item */
    0, 30, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[5],	/* next item */
    0, 40, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,	/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[6],	/* next item */
    0, 50, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[5],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,	/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[7],		/* next item */
    0, 60, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[6],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[8],		/* next item */
    0, 70, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[7],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[9],		/* next item */
    0, 80, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[8],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[10],		/* next item */
    0, 90, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[9],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &projectitems[11],	/* next item */
    0, 100, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[10],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    0, 110, 130+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&projectext[11],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct MenuItem edititems[] = {
  {
    &edititems[1],	/* next item */
    0, 0, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[0],  /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[2],		/* next item */
    0, 10, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[3],	/* next item */
    0, 20, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[4],	/* next item */
    0, 30, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[5],	/* next item */
    0, 40, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[6],	/* next item */
    0, 50, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[5],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[7],	/* next item */
    0, 60, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[6],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[8],	/* next item */
    0, 70, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[7],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[9],	/* next item */
    0, 80, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[8],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[10],	/* next item */
    0, 90, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[9],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[11],	/* next item */
    0, 100, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[10],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[12],	/* next item */
    0, 110, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[11],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[13],		/* next item */
    0, 120, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[12],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[14],		/* next item */
    0, 130, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[13],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[15],		/* next item */
    0, 140, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[14],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[16],		/* next item */
    0, 150, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&editext[15],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &edititems[17],		/* next item */
    0, 150, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | HIGHNONE,
    0,			/* mutual exclude */
    NULL,		/* ItemFill */
    NULL,		/* SelectFill */
    'C',		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    0, 150, 158+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | HIGHNONE,
    0,			/* mutual exclude */
    NULL,		/* ItemFill */
    NULL,		/* SelectFill */
    'V',		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

/* COMMSEG */

struct MenuItem windowitems[] = {
  {
    &windowitems[1],	/* next item */
    0, 0, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[0],  /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &windowitems[2],		/* next item */
    0, 10, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &windowitems[3],		/* next item */
    0, 20, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &windowitems[4],		/* next item */
    0, 30, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &windowitems[5],		/* next item */
    0, 40, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &windowitems[6],		/* next item */
    0, 50, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[5],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &windowitems[7],		/* next item */
    0, 60, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[6],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    0, 70, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&windowtext[7],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct MenuItem moveitems[] = {
  {
    &moveitems[1],	/* next item */
    0, 0, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[0],  /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[2],		/* next item */
    0, 10, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[3],		/* next item */
    0, 20, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[4],		/* next item */
    0, 30, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[5],		/* next item */
    0, 40, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[6],		/* next item */
    0, 50, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[5],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[7],		/* next item */
    0, 60, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[6],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[8],	/* next item */
    0, 70, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[7],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[9],		/* next item */
    0, 80, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[8],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[10],		/* next item */
    0, 90, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[9],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &moveitems[11],		/* next item */
    0, 100, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[10],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    0, 110, 156+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&movetext[11],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct MenuItem lineitems[] = {
  {
    &lineitems[1],	/* next item */
    0, 0, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[0],  /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[2],		/* next item */
    0, 10, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[3],		/* next item */
    0, 20, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[4],	/* next item */
    0, 30, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[5],	/* next item */
    0, 40, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[6],	/* next item */
    0, 50, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[5],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[7],	/* next item */
    0, 60, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[6],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[8],	/* next item */
    0, 70, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[7],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &lineitems[9],	/* next item */
    0, 80, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[8],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    0, 90, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&linetext[9],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct MenuItem worditems[] = {
  {
    &worditems[1],	/* next item */
    0, 0, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED|ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&wordtext[0], /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &worditems[2],	/* next item */
    0, 10, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&wordtext[1],	/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &worditems[3],	/* next item */
    0, 20, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&wordtext[2],	/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &worditems[4],	/* next item */
    0, 30, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&wordtext[3],	/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &worditems[5],	/* next item */
    0, 40, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&wordtext[4],	/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    0, 50, 148+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&wordtext[5],	/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct MenuItem searchitems[] = {
  {
    &searchitems[1],	/* next item */
    -40, 0, 140+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED|ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&searchtext[0],  /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &searchitems[2],		/* next item */
    -40, 10, 140+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&searchtext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &searchitems[3],		/* next item */
    -40, 20, 140+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&searchtext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &searchitems[4],		/* next item */
    -40, 30, 140+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&searchtext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    -40, 40, 140+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&searchtext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct MenuItem macroitems[] = {
  {
    &macroitems[1],	/* next item */
    -90, 0, 120+SIXTYADJUST, 9,	/* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[0],  /* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[2],		/* next item */
    -90, 10, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[1],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[3],		/* next item */
    -90, 20, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[2],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[4],		/* next item */
    -90, 30, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[3],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[5],		/* next item */
    -90, 40, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[4],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[6],		/* next item */
    -90, 50, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[5],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[7],		/* next item */
    -90, 60, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[6],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    &macroitems[8],		/* next item */
    -90, 70, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[7],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  },
  {
    NULL,		/* next item */
    -90, 80, 120+SIXTYADJUST, 9, /* LeftEdge, TopEdge, Width, Height */
    ITEMENABLED | ITEMTEXT | HIGHCOMP,
    0,			/* mutual exclude */
    (APTR)&macrotext[8],/* ItemFill */
    NULL,		/* SelectFill */
    NULL,		/* Command */
    NULL,		/* SubItem */
    NULL		/* NextSelect */
  }
};

struct Menu menus[] = {
  {
    &menus[1],		/* next menu */
    0+0, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Project",		/* name */
    projectitems	/* first item */
  },
  {
    &menus[2],		/* next menu */
    0+80, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,		/* flags */
    " Edit",		/* name */
    edititems		/* first item */
  },
  {
    &menus[3],		/* next menu */
    0+160, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Window",		/* name */
    windowitems		/* first item */
  },
  {
    &menus[4],		/* next menu */
    0+240, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Move",		/* name */
    moveitems		/* first item */
  },
  {
    &menus[5],		/* next menu */
    0+320, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Line",		/* name */
    lineitems		/* first item */
  },
  {
    &menus[6],		/* next menu */
    0+400, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Word",		/* name */
    worditems		/* first item */
  },
  {
    &menus[7],		/* next menu */
    0+480, 0, 80, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Search",		/* name */
    searchitems		/* first item */
  },
  {
    NULL,		/* next menu */
    0+560, 0, 70, 0,	/* LeftEdge, TopEdge, Width, Height */
    MENUENABLED,	/* flags */
    "Extras",		/* name */
    macroitems		/* first item */
  }
};
#endif

