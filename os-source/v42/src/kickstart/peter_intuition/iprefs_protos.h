/* Prototypes for functions defined in
iprefs.c
 */

extern struct IFontPrefs topazprefs;

int syncIPrefs(int size);

int trackViewPos(void);

int SetIPrefs(CPTR p,
              ULONG psize,
              ULONG ptype);

static LONG setIPrefs(CPTR , ULONG , ULONG , BOOL );

int setColorPrefs(UWORD * prefcolor,
                  UWORD firstcolor,
                  UWORD numcolors);

