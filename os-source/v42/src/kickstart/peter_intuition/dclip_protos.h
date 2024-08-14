/* Prototypes for functions defined in
dclip.c
 */

int autoScroll(struct Screen * sc);

int rethinkMouseLimits(void);

int QueryOverscan(ULONG displayID,
                  struct Rectangle * rect,
                  WORD ostype);

int screenLegalPosition(struct Screen * screen,
                        struct Rectangle * limitrect,
                        WORD flags);

