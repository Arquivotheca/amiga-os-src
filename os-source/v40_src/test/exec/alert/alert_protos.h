/* Prototypes for functions defined in
alert.c
 */

extern struct lookup theAlerts[159];

extern struct Opts opts;

extern struct RDArgs * rdargs;

extern struct Library * UtilityBase;

void main(int , UBYTE ** );

STRPTR alertToEng(ULONG );

ULONG engToAlert(STRPTR );

BOOL xStrToULong(STRPTR , ULONG * );

