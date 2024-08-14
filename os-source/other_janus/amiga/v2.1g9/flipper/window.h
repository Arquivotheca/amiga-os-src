struct NewWindow FlipperWindow = {
    400,10,    /* window XY origin relative to TopLeft of screen */
    140,45,    /* window width and height */
    0,1,    /* detail and block pens */
    GADGETUP+CLOSEWINDOW,    /* IDCMP flags */
    WINDOWDRAG+WINDOWDEPTH+WINDOWCLOSE,    /* other window flags */
    NULL,    /* first gadget in gadget list */
    NULL,    /* custom CHECKMARK imagery */
    "Flipper",    /* window title */
    NULL,    /* custom screen pointer */
    NULL,    /* custom bitmap */
    -1,-1,    /* minimum width and height */
    -1,-1,    /* maximum width and height */
    WBENCHSCREEN    /* destination screen type */
};


