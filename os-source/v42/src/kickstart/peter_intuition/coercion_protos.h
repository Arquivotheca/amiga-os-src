/* Prototypes for functions defined in
coercion.c
 */

int setupMonitor(int force);

int coerceScreens(struct Screen * onescreen,
                  int force);

int setScreenDisplayID(struct Screen * sc,
                       ULONG displayID,
                       ULONG * ecodeptr,
                       int force);

static int coerceScreenGrunt(struct Screen * , ULONG );

