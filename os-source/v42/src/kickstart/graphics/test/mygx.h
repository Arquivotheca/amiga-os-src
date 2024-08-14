#define SETCOORDS (4096+512+(3<<19))

#define SETXY (SETCOORDS+8*3)
#define SETXY0 (SETXY+64*0)
#define SETXY1 (SETXY+64*1)
#define SETXY2 (SETXY+64*2)
#define SETXY3 (SETXY+64*3)

#define STATUS (3<<19)

#define DOBLIT (4+(3<<19))

#define DoReg(r,x) vp[(r)>>2]=x

#define SETCOORD(r,x,y) DoReg(r,(x<<16)+y)

#define fground 0
#define bground 1
#define pmask 2
#define draw_mode 3
#define pat_originx 4
#define pat_originy 5
#define raster 6
#define pixel8_reg 7
#define w_min 8
#define w_max 9

#define SetDrReg(r,v) DoReg(r+512+(3<<19),v)

