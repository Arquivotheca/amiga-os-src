#include "exec/types.h"
#include <intuition/intuition.h>
#include "gadgets.h"
#include "display.h"

extern struct Image UpImage;
extern struct Image DownImage;

SHORT chip CnXY[] = {0,0, CNWIDTH,0, CNWIDTH,CNHEIGHT, 0,CNHEIGHT, 0,0};

struct Border chip CnBorder = {
   -1, -1,   /* LeftEdge, TopEdge */
   1, 0,   /* FrontPen, BackPen */
   JAM2,   /* DrawMode */
   5,      /* Count */
   CnXY,   /* XY pairs */
   NULL   /* NextBorder */
};

struct IntuiText chip CnText = {
   1, 0,      /* FrontPen, BackPen */
   JAM2,      /* DrawMode */
   0, 0,      /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   " CANCEL ",   /* IText */
   NULL      /* NextText */
};

struct Gadget chip CnGadget = {
   NULL,            /* Next Gadget */
   -2-CNWIDTH,         /* LeftEdge */
   1,               /* TopEdge */
   CNWIDTH-1, CNHEIGHT-1,      /* Width, Height */
   GRELRIGHT|GADGHCOMP,   /* Flags */
   RELVERIFY|TOPBORDER,   /* Activation Flags */
   BOOLGADGET,            /* Gadget Type */
   (APTR)&CnBorder,            /* Gadget Render */
   NULL,               /* Select Render */
   &CnText,            /* Gadget Text */
   NULL,               /* Mutual Exclude */
   NULL,               /* SpecialInfo */
   CNID,               /* Gadget ID */
   NULL               /* User Data */
};

SHORT chip OkXY[] = {0,0, OKWIDTH,0, OKWIDTH,OKHEIGHT, 0,OKHEIGHT, 0,0};

struct Border chip OkBorder = {
   -1, -1,   /* LeftEdge, TopEdge */
   1, 0,   /* FrontPen, BackPen */
   JAM2,   /* DrawMode */
   5,      /* Count */
   OkXY,   /* XY pairs */
   NULL   /* NextBorder */
};

struct IntuiText chip OkText = {
   1, 0,      /* FrontPen, BackPen */
   JAM2,      /* DrawMode */
   0, 0,      /* LeftEdge, TopEdge */
   NULL,      /* ITextFont */
   "   OK   ",   /* IText */
   NULL      /* NextText */
};

struct Gadget chip OkGadget = {
   NULL,            /* Next Gadget */
   -2-CNWIDTH-40-OKWIDTH,   /* LeftEdge */
   1,               /* TopEdge */
   OKWIDTH-1, OKHEIGHT-1,   /* Width, Height */
   GRELRIGHT|GADGHCOMP,   /* Flags */
   RELVERIFY|TOPBORDER,   /* Activation Flags */
   BOOLGADGET,            /* Gadget Type */
   (APTR)&OkBorder,            /* Gadget Render */
   NULL,               /* Select Render */
   &OkText,            /* Gadget Text */
   NULL,               /* Mutual Exclude */
   NULL,               /* SpecialInfo */
   OKID,               /* Gadget ID */
   NULL               /* User Data */
};

struct PropInfo chip PrInfo = {
   AUTOKNOB|FREEVERT,   /* Flags */
   0,0,            /* HorizPot, VertPot */
   0xffff,0xffff,      /* HorizBody, VertBody */
   0,0,            /* CWidth, CHeight */
   0,0,            /* HPotRes, VPotRes */
   0,0               /* LeftBorder, TopBoder */
};

struct Image chip PrImage;

struct Gadget chip PrGadget = {
   NULL,               /* Next Gadget */
   -PRWIDTH-1,         /* LeftEdge */
   TOPOFFSET+UPHEIGHT+SPACING,      /* TopEdge */
   PRWIDTH,      /* Width */
   -TOPOFFSET-UPHEIGHT-SPACING-SPACING-DNHEIGHT,/* Height */
   GRELRIGHT|GRELHEIGHT,            /* Flags */
   RELVERIFY,            /* Activation Flags */
   PROPGADGET,            /* Gadget Type */
   (APTR)&PrImage,            /* Gadget Render */
   NULL,               /* Select Render */
   NULL,               /* Gadget Text */
   NULL,               /* Mutual Exclude */
   (APTR)&PrInfo,            /* SpecialInfo */
   PRID,               /* Gadget ID */
   NULL               /* User Data */
};

/*
SHORT chip DnXY[] = {0,0, DNWIDTH,0, DNWIDTH,DNHEIGHT, 0,DNHEIGHT, 0,0};

struct Border chip DnBorder = {
   -1, -1,
   1, -1,
   JAM1,
   5,
   DnXY,
   NULL
};

struct IntuiText chip DnText = {
   1, -1,
   JAM1,
   4, 8,
   NULL,
   "DN",
   NULL
};
*/

struct Gadget chip DnGadget = {
   &PrGadget,            /* Next Gadget */
   -DNWIDTH,         /* LeftEdge */
   -DNHEIGHT,   /* TopEdge */
   DNWIDTH-1, DNHEIGHT-1,      /* Width, Height */
/*   GADGHCOMP|GRELRIGHT|GRELBOTTOM,   /* Flags */
   GADGIMAGE|GADGHCOMP|GRELRIGHT|GRELBOTTOM,   /* Flags */
   RELVERIFY,            /* Activation Flags */
   BOOLGADGET,            /* Gadget Type */
/*   &DnBorder,            /* Gadget Render */
   (APTR)&DownImage,            /* Gadget Render */
   NULL,               /* Select Render */
/*   &DnText,            /* Gadget Text */
   NULL,               /* Gadget Text */
   NULL,               /* Mutual Exclude */
   NULL,               /* SpecialInfo */
   DNID,               /* Gadget ID */
   NULL               /* User Data */
};

/*
SHORT chip UpXY[] = {0,0, UPWIDTH,0, UPWIDTH,UPHEIGHT, 0,UPHEIGHT, 0,0};

struct Border chip UpBorder = {
   -1, -1,
   1, -1,
   JAM1,
   5,
   UpXY,
   NULL
};

struct IntuiText chip UpText = {
   1, -1,
   JAM1,
   4, 8,
   NULL,
   "UP",
   NULL
};
*/

struct Gadget chip UpGadget = {
   &DnGadget,            /* Next Gadget */
   -UPWIDTH-1, TOPOFFSET+4,   /* LeftEdge, TopEdge */
   UPWIDTH-1, UPHEIGHT-1,      /* Width, Height */
/*   GADGHCOMP|GRELRIGHT,   /* Flags */
   GADGIMAGE|GADGHCOMP|GRELRIGHT,   /* Flags */
   RELVERIFY,            /* Activation Flags */
   BOOLGADGET,            /* Gadget Type */
/*   &UpBorder,            /* Gadget Render */
   (APTR)&UpImage,            /* Gadget Render */
   NULL,               /* Select Render */
/*   &UpText,            /* Gadget Text */
   NULL,               /* Gadget Text */
   NULL,               /* Mutual Exclude */
   NULL,               /* SpecialInfo */
   UPID,               /* Gadget ID */
   NULL               /* User Data */
};
