void ArgArrayDone( void );
UBYTE **ArgArrayInit( LONG arg1, UBYTE **arg2 );
LONG ArgInt( UBYTE **arg1, UBYTE *arg2, LONG arg3 );
UBYTE *ArgString( UBYTE **arg1, UBYTE *arg2, UBYTE *arg3 );
CxObj *HotKey( UBYTE *arg1, struct MsgPort *arg2, LONG arg3 );
struct InputEvent *InvertString( UBYTE *arg1, ULONG *arg2 );
/**/
/* Macros */
/**/
/* CxObj  *CxCustom(LONG(*)(),LONG)                   (A0,D0) */
/* CxObj  *CxDebug(LONG)                              (D0) */
/* CxObj  *CxFilter(BYTE *)                           (A0) */
/* CxObj  *CxSender(struct MsgPort *,LONG)            (A0,D0) */
/* CxObj  *CxSignal(struct Task *,LONG)               (A0,D0) */
/* CxObj  *CxTranslate(struct InputEvent *)           (A0) */
/* CxObj  *CxTypeFilter(LONG)                         (D0) */
