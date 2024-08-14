/* Prototypes for functions defined in
isupport.c
 */

BOOL knownScreen(struct Screen * s);

BOOL knownWindow(struct Window * w);

int internalBorderPatrol(struct Screen * s);

int BeginRefresh(struct Window * window);

int EndRefresh(struct Window * window,
               BOOL complete);

int updateOptions(USHORT * firstoption,
                  BOOL draw);

extern UBYTE InputDeviceName[13];

struct IntuitionBase * OpenIntuition(void);

