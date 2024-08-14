/* Prototypes for functions defined in
packmoncommon.c
 */

void strCpy(STRPTR , STRPTR );

LONG strLen(STRPTR );

struct MsgPort * createPort(STRPTR , LONG );

void deletePort(struct MsgPort * );

void newList(struct List * );

