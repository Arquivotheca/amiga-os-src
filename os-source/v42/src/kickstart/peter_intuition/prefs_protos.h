/* Prototypes for functions defined in
prefs.c
 */

extern struct Preferences DefaultPreferences;

struct Preferences * GetDefPrefs(struct Preferences * preferences,
                                 int size);

struct Preferences * GetPrefs(struct Preferences * preferences,
                              int size);

struct Preferences * setPrefs(struct Preferences * preferences,
                              int size,
                              BOOL RealThing);

static int copyprefs(struct Preferences * , int , struct Preferences * );

static int setIBasePrefs(void);

int setWBColorsGrunt(struct Screen * screen,
                     int allcolors);

static int setWBColorsGruntGrunt(struct Screen * , int , int , int );

