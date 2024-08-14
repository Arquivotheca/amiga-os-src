struct IntuiText IText1 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	19,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Save Icons?",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem1 = {
	NULL,	/* next MenuItem structure */
	0,0,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	107,8,	/* hit box width and height */
	CHECKIT+ITEMTEXT+HIGHCOMP+CHECKED,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText1,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	NULL,	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct Menu Menu3 = {
	NULL,	/* next Menu structure */
	137,0,	/* XY origin of Menu hit box relative to screen TopLeft */
	75,0,	/* Menu hit box width and height */
	MENUENABLED,	/* Menu flags */
	"Options",	/* text of Menu name */
	&MenuItem1	/* MenuItem linked list pointer */
};

struct IntuiText IText2 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Restore",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem4 = {
	NULL,	/* next MenuItem structure */
	0,18,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	128,8,	/* hit box width and height */
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText2,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	'R',	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText3 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Last Saved",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem3 = {
	&MenuItem4,	/* next MenuItem structure */
	0,9,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	128,8,	/* hit box width and height */
	ITEMTEXT+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText3,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	NULL,	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText4 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Reset to default",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem2 = {
	&MenuItem3,	/* next MenuItem structure */
	0,0,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	128,8,	/* hit box width and height */
	ITEMTEXT+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText4,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	NULL,	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct Menu Menu2 = {
	&Menu3,	/* next Menu structure */
	82,0,	/* XY origin of Menu hit box relative to screen TopLeft */
	48,0,	/* Menu hit box width and height */
	MENUENABLED,	/* Menu flags */
	"Edit",	/* text of Menu name */
	&MenuItem2	/* MenuItem linked list pointer */
};

struct IntuiText IText5 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Quit",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem11 = {
	NULL,	/* next MenuItem structure */
	0,54,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText5,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	'Q',	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText6 = {
	3,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem10 = {
	&MenuItem11,	/* next MenuItem structure */
	0,45,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText6,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	NULL,	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText7 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Save As Default",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem9 = {
	&MenuItem10,	/* next MenuItem structure */
	0,36,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText7,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	'D',	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText8 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Save As ...",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem8 = {
	&MenuItem9,	/* next MenuItem structure */
	0,27,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText8,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	'A',	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText9 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Save",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem7 = {
	&MenuItem8,	/* next MenuItem structure */
	0,18,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText9,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	'S',	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText10 = {
	3,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem6 = {
	&MenuItem7,	/* next MenuItem structure */
	0,9,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+HIGHCOMP+HIGHBOX,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText10,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	NULL,	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct IntuiText IText11 = {
	0,1,JAM2,	/* front and back text pens, drawmode and fill byte */
	0,0,	/* XY origin relative to container TopLeft */
	&TOPAZ80,	/* font pointer or NULL for default */
	"Open ...",	/* pointer to text */
	NULL	/* next IntuiText structure */
};

struct MenuItem MenuItem5 = {
	&MenuItem6,	/* next MenuItem structure */
	0,0,	/* XY of Item hitbox relative to TopLeft of parent hitbox */
	160,8,	/* hit box width and height */
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,	/* Item flags */
	0,	/* each bit mutually-excludes a same-level Item */
	(APTR)&IText11,	/* Item render  (IntuiText or Image or NULL) */
	NULL,	/* Select render */
	'O',	/* alternate command-key */
	NULL,	/* SubItem list */
	MENUNULL	/* filled in by Intuition for drag selections */
};

struct Menu Menu1 = {
	&Menu2,	/* next Menu structure */
	0,0,	/* XY origin of Menu hit box relative to screen TopLeft */
	75,0,	/* Menu hit box width and height */
	MENUENABLED,	/* Menu flags */
	"Project",	/* text of Menu name */
	&MenuItem5	/* MenuItem linked list pointer */
};

#define MenuList1 Menu1
