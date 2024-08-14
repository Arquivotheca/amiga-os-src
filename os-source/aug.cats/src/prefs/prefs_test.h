
/* prefs_test.c */
BOOL OpenLibraries ( int VOID );
VOID CloseLibraries ( int VOID );
struct Screen *OpenPreferScreen ( struct NewScreen *ns , struct ScreenModePref *smp , struct PalettePref *pp , struct TagItem *attrs );
VOID CloseEnv ( int VOID );
BOOL OpenEnv ( int VOID );
VOID FreshenColors ( VOID *pr );
VOID FreshenScreen ( VOID *s );
int main ( int argc , char **argv );
