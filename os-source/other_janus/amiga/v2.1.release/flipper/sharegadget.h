
USHORT __chip ImageData3[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xE000,0x0000,
   0x0000,0xE000,0x1830,0x0000,0xE000,0x3C18,0x0000,0xE000,
   0x3C0C,0x0000,0xE000,0x6606,0x0000,0xE000,0x7E0C,0x0000,
   0xE000,0xC318,0x0000,0xE000,0xC330,0x0000,0xE000,0x0000,
   0x0000,0xE000,0x0000,0x0000,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x0000,0x0000,0x0038,0x0000,0x0000,0x0038,0x0000,0x0204,
   0x0038,0x0000,0x0302,0x0038,0x0000,0x0901,0x0038,0x0000,
   0x0181,0x8038,0x0000,0x1C83,0x0038,0x0000,0x30C6,0x0038,
   0x0000,0x30CC,0x0038,0x0000,0x0000,0x0038,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image3 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData3, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

USHORT __chip ImageData4[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xFFFF,0xFFFF,
   0xFFC0,0xFFFF,0xFFFF,0xFFC0,0xFFFF,0xFBF7,0xFFC0,0xFFFF,
   0xF9FB,0xFFC0,0xFFFF,0xEDFD,0xFFC0,0xFFFF,0xFCFC,0xFFC0,
   0xFFFF,0xC6F9,0xFFC0,0xFFFF,0x9E73,0xFFC0,0xFFFF,0x9E67,
   0xFFC0,0xFFFF,0xFFFF,0xFFC0,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x1FFF,0xFFFF,0xFFF8,0x1FFF,0xCF9F,0xFFF8,0x1FFF,0x87CF,
   0xFFF8,0x1FFF,0x87E7,0xFFF8,0x1FFF,0x33F3,0xFFF8,0x1FFF,
   0x03E7,0xFFF8,0x1FFE,0x79CF,0xFFF8,0x1FFE,0x799F,0xFFF8,
   0x1FFF,0xFFFF,0xFFF8,0x1FFF,0xFFFF,0xFFF8,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image4 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData4, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

struct Gadget AGadget = {
   NULL,   /* next gadget */
   72,27,   /* origin XY of hit box relative to window TopLeft */
   45,14,   /* hit box width and height */
   GADGHIMAGE+GADGIMAGE,   /* gadget flags */
   RELVERIFY,  /* activation flags */
   BOOLGADGET, /* gadget type flags */
   (APTR)&Image3, /* gadget border or image to be rendered */
   (APTR)&Image4, /* alternate imagery for selection */
   NULL, /* first IntuiText structure */
   NULL, /* gadget mutual-exclude long word */
   NULL, /* SpecialInfo structure */
   NULL, /* user-definable data */
   NULL  /* pointer to user-definable data */
};

USHORT __chip ImageData5[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xE000,0x0000,
   0x0000,0xE0F8,0xFE18,0x0000,0xE06C,0x6638,0x1800,0xE066,
   0x6018,0x1800,0xE066,0x7818,0x0000,0xE066,0x6018,0x0000,
   0xE06C,0x6018,0x1800,0xE0F8,0xF07E,0x1800,0xE000,0x0000,
   0x0000,0xE000,0x0000,0x0000,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x0000,0x0000,0x0038,0x0000,0x0000,0x0038,0x0012,0x1986,
   0x0038,0x0019,0x1986,0x0638,0x0019,0x8006,0x0638,0x0019,
   0x9E06,0x0038,0x0011,0x9806,0x0038,0x0003,0x0800,0x0638,
   0x003E,0x3C1F,0x8638,0x0000,0x0000,0x0038,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image5 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData5, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

USHORT __chip ImageData6[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xFFFF,0xFFFF,
   0xFFC0,0xFFFF,0xFFFF,0xFFC0,0xFFDB,0xCCF3,0xFFC0,0xFFCD,
   0xCCF3,0xF3C0,0xFFCC,0xFFF3,0xF3C0,0xFFCC,0xC3F3,0xFFC0,
   0xFFDC,0xCFF3,0xFFC0,0xFFF9,0xEFFF,0xF3C0,0xFF83,0x87C0,
   0xF3C0,0xFFFF,0xFFFF,0xFFC0,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x1FFF,0xFFFF,0xFFF8,0x1E0E,0x03CF,0xFFF8,0x1F27,0x338F,
   0xCFF8,0x1F33,0x3FCF,0xCFF8,0x1F33,0x0FCF,0xFFF8,0x1F33,
   0x3FCF,0xFFF8,0x1F27,0x3FCF,0xCFF8,0x1E0E,0x1F03,0xCFF8,
   0x1FFF,0xFFFF,0xFFF8,0x1FFF,0xFFFF,0xFFF8,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image6 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData6, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

struct Gadget DF0Gadget = {
   &AGadget,   /* next gadget */
   72,12,   /* origin XY of hit box relative to window TopLeft */
   45,14,   /* hit box width and height */
   GADGHIMAGE+GADGIMAGE,   /* gadget flags */
   RELVERIFY,  /* activation flags */
   BOOLGADGET, /* gadget type flags */
   (APTR)&Image5, /* gadget border or image to be rendered */
   (APTR)&Image6, /* alternate imagery for selection */
   NULL, /* first IntuiText structure */
   NULL, /* gadget mutual-exclude long word */
   NULL, /* SpecialInfo structure */
   NULL, /* user-definable data */
   NULL  /* pointer to user-definable data */
};

USHORT __chip ImageData7[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xE000,0x0000,
   0x0000,0xE082,0x18C6,0x0000,0xE0C6,0x3CE6,0x0000,0xE0EE,
   0x3CF6,0x0000,0xE0FE,0x66DE,0x0000,0xE0D6,0x7ECE,0x0000,
   0xE0C6,0xC3C6,0x1800,0xE0C6,0xC3C6,0x1800,0xE000,0x0000,
   0x0000,0xE000,0x0000,0x0000,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x0000,0x0000,0x0038,0x0000,0x0000,0x0038,0x0020,0x8211,
   0x8038,0x0011,0x8309,0x8038,0x0001,0x8921,0x8038,0x0029,
   0x8131,0x8038,0x0031,0x1C31,0x8038,0x0031,0x3031,0x8638,
   0x0031,0xB0F1,0x8638,0x0000,0x0000,0x0038,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image7 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData7, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

USHORT __chip ImageData8[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xFFFF,0xFFFF,
   0xFFC0,0xFFFF,0xFFFF,0xFFC0,0xFFDF,0x7DEE,0x7FC0,0xFFEE,
   0x7CF6,0x7FC0,0xFFFE,0x76DE,0x7FC0,0xFFD6,0x7ECE,0x7FC0,
   0xFFCE,0xE3CE,0x7FC0,0xFFCE,0xCFCE,0x79C0,0xFFCE,0x4F0E,
   0x79C0,0xFFFF,0xFFFF,0xFFC0,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x1FFF,0xFFFF,0xFFF8,0x1F7D,0xE739,0xFFF8,0x1F39,0xC319,
   0xFFF8,0x1F11,0xC309,0xFFF8,0x1F01,0x9921,0xFFF8,0x1F29,
   0x8131,0xFFF8,0x1F39,0x3C39,0xE7F8,0x1F39,0x3C39,0xE7F8,
   0x1FFF,0xFFFF,0xFFF8,0x1FFF,0xFFFF,0xFFF8,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image8 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData8, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

struct Gadget ManGadget = {
   &DF0Gadget, /* next gadget */
   24,27,   /* origin XY of hit box relative to window TopLeft */
   45,14,   /* hit box width and height */
   GADGHIMAGE+GADGIMAGE,   /* gadget flags */
   RELVERIFY,  /* activation flags */
   BOOLGADGET, /* gadget type flags */
   (APTR)&Image7, /* gadget border or image to be rendered */
   (APTR)&Image8, /* alternate imagery for selection */
   NULL, /* first IntuiText structure */
   NULL, /* gadget mutual-exclude long word */
   NULL, /* SpecialInfo structure */
   NULL, /* user-definable data */
   NULL  /* pointer to user-definable data */
};

USHORT __chip ImageData1[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xE000,0x0000,
   0x0000,0xE061,0x99F8,0xE000,0xE0F1,0x9969,0xB000,0xE0F1,
   0x9863,0x1800,0xE199,0x9863,0x1800,0xE1F9,0x9863,0x1800,
   0xE30D,0x9861,0xB000,0xE30C,0xF8F0,0xE000,0xE000,0x0000,
   0x0000,0xE000,0x0000,0x0000,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x0000,0x0000,0x0038,0x0000,0x0000,0x0038,0x0008,0x6616,
   0x0838,0x000C,0x6618,0x6438,0x0024,0x6618,0xC638,0x0006,
   0x6618,0xC638,0x0072,0x6618,0x4638,0x00C3,0x0608,0x0C38,
   0x00C3,0x3E3C,0x3838,0x0000,0x0000,0x0038,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image1 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData1, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

USHORT __chip ImageData2[] = {
   0xFFFF,0xFFFF,0xFFF0,0xFFFF,0xFFFF,0xFFE0,0xFFFF,0xFFFF,
   0xFFC0,0xFFFF,0xFFFF,0xFFC0,0xFFF7,0x99E9,0xF7C0,0xFFF3,
   0x99E7,0x9BC0,0xFFDB,0x99E7,0x39C0,0xFFF9,0x99E7,0x39C0,
   0xFF8D,0x99E7,0xB9C0,0xFF3C,0xF9F7,0xF3C0,0xFF3C,0xC1C3,
   0xC7C0,0xFFFF,0xFFFF,0xFFC0,0xC000,0x0000,0x0000,0x8000,
   0x0000,0x0000,0x0000,0x0000,0x0008,0x0000,0x0000,0x0018,
   0x1FFF,0xFFFF,0xFFF8,0x1F9E,0x6607,0x1FF8,0x1F0E,0x6696,
   0x4FF8,0x1F0E,0x679C,0xE7F8,0x1E66,0x679C,0xE7F8,0x1E06,
   0x679C,0xE7F8,0x1CF2,0x679E,0x4FF8,0x1CF3,0x070F,0x1FF8,
   0x1FFF,0xFFFF,0xFFF8,0x1FFF,0xFFFF,0xFFF8,0x3FFF,0xFFFF,
   0xFFF8,0x7FFF,0xFFFF,0xFFF8
};

struct Image Image2 = {
   0,0,  /* XY origin relative to container TopLeft */
   45,14,   /* Image width and height in pixels */
   2, /* number of bitplanes in Image */
   ImageData2, /* pointer to ImageData */
   0x0003,0x0000, /* PlanePick and PlaneOnOff */
   NULL  /* next Image structure */
};

struct Gadget AutoGadget = {
   &ManGadget, /* next gadget */
   24,12,   /* origin XY of hit box relative to window TopLeft */
   45,14,   /* hit box width and height */
   GADGHIMAGE+GADGIMAGE,   /* gadget flags */
   RELVERIFY,  /* activation flags */
   BOOLGADGET, /* gadget type flags */
   (APTR)&Image1, /* gadget border or image to be rendered */
   (APTR)&Image2, /* alternate imagery for selection */
   NULL, /* first IntuiText structure */
   NULL, /* gadget mutual-exclude long word */
   NULL, /* SpecialInfo structure */
   NULL, /* user-definable data */
   NULL  /* pointer to user-definable data */
};

struct NewWindow NewWindowStructure1 = {
   450,120,   /* window XY origin relative to TopLeft of screen */
   141,44,  /* window width and height */
   0,1,  /* detail and block pens */
   GADGETUP+CLOSEWINDOW,   /* IDCMP flags */
   WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE /* +ACTIVATE */ , /* other window flags */
   &AutoGadget, /* first gadget in gadget list */
   NULL, /* custom CHECKMARK imagery */
   "Flipper",  /* window title */
   NULL, /* custom screen pointer */
   NULL, /* custom bitmap */
   -1, -1,  /* minimum width and height */
   -1, -1, /* maximum width and height */
   WBENCHSCREEN   /* destination screen type */
};


/* end of PowerWindows source generation */


#define AUTOSELECT { AutoGadget.Flags |= SELECTED; \
ManGadget.Flags &= ~SELECTED; \
RefreshGList (&AutoGadget, muxer, NULL, 2);}

#define MANSELECT {AutoGadget.Flags &= ~SELECTED; \
ManGadget.Flags |= SELECTED; \
RefreshGList (&AutoGadget, muxer, NULL, 2);}

#define DF0SELECT {DF0Gadget.Flags |= SELECTED; \
AGadget.Flags &= ~SELECTED; \
RefreshGList (&DF0Gadget, muxer, NULL, 2);}

#define ASELECT {DF0Gadget.Flags &= ~SELECTED; \
AGadget.Flags |= SELECTED; \
RefreshGList (&DF0Gadget, muxer, NULL, 2);}

