/* Prototypes for functions defined in
newlook.c
 */

extern UWORD fourColorPens[13];

extern UWORD twoColorPens[13];

extern struct TagItem sdragtags[6];

extern struct TagItem wdragtags[6];

extern struct TagItem closetags[5];

extern struct TagItem zoomtags[5];

extern struct TagItem depthtags[5];

extern struct TagItem sdepthtags[5];

extern struct TagItem sizetags[5];

extern struct TagItem lclosetags[5];

extern struct TagItem lzoomtags[5];

extern struct TagItem ldepthtags[5];

extern struct TagItem lsdepthtags[5];

extern struct TagItem lsizetags[5];

extern struct TagItem * gtaglists[8][2];

struct Gadget * createNewSysGadget(struct XScreen * s,
                                   USHORT res,
                                   USHORT gindex,
                                   BOOL gimme);

static struct Image * getScreenSysImage(struct Screen * , int );

void embossedBoxTrim(struct RastPort * rp,
                     struct IBox * b,
                     ULONG ulpen,
                     ULONG lrpen,
                     ULONG meeting_type);

int getSizeGDims(struct Screen * sc,
                 UWORD * dims);

