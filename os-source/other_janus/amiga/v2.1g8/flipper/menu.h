struct IntuiText IText1 = {
    3,1,COMPLEMENT,    /* front and back text pens, drawmode and fill byte */
    0,0,    /* XY origin relative to container TopLeft */
    NULL,    /* font pointer or NULL for default */
    MAKEDATE,    /* pointer to text */
    NULL    /* next IntuiText structure */
};

struct MenuItem MenuItem2 = {
    NULL,    /* next MenuItem structure */
    0,9,    /* XY of Item hitbox relative to TopLeft of parent hitbox */
    88,8,    /* hit box width and height */
    ITEMTEXT+HIGHCOMP,    /* Item flags */
    0,    /* each bit mutually-excludes a same-level Item */
    (APTR)&IText1,    /* Item render  (IntuiText or Image or NULL) */
    NULL,    /* Select render */
    NULL,    /* alternate command-key */
    NULL,    /* SubItem list */
    MENUNULL    /* filled in by Intuition for drag selections */
};

struct IntuiText IText2 = {
    3,1,COMPLEMENT,    /* front and back text pens, drawmode and fill byte */
    0,0,    /* XY origin relative to container TopLeft */
    NULL,    /* font pointer or NULL for default */
    VERSION, 
    /* "Version 0.2", */   /* pointer to text */
    NULL    /* next IntuiText structure */
};

struct MenuItem MenuItem1 = {
    &MenuItem2,    /* next MenuItem structure */
    0,0,    /* XY of Item hitbox relative to TopLeft of parent hitbox */
    88,8,    /* hit box width and height */
    ITEMTEXT+HIGHCOMP,    /* Item flags */
    0,    /* each bit mutually-excludes a same-level Item */
    (APTR)&IText2,    /* Item render  (IntuiText or Image or NULL) */
    NULL,    /* Select render */
    NULL,    /* alternate command-key */
    NULL,    /* SubItem list */
    MENUNULL    /* filled in by Intuition for drag selections */
};

struct Menu FlipperMenu = {
    NULL,    /* next Menu structure */
    0,0,    /* XY origin of Menu hit box relative to screen TopLeft */
    129,0,    /* Menu hit box width and height */
    MENUENABLED,    /* Menu flags */
    "About Flipper",    /* text of Menu name */
    &MenuItem1    /* MenuItem linked list pointer */
};

