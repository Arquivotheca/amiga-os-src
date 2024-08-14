/* $Id: display_protos.h,v 1.4 93/03/23 10:32:59 peter Exp $ */

VOID FreeIBuffer( struct IBuffer *ib );
VOID SwapIBuffer( VOID );
void MakeWorkIBSafe( void );
long StartPPScreen(struct GadDir *gl, struct BuildBg_Info *bg, UBYTE *buttondata);
VOID EndPPScreen(VOID);
UWORD GetIDCMPEvent( void );
void StripIDCMPEvents( void );
void FadeIn(void);
void FadeOut(void);
void togglegnum( void );
void DoBoxMove(UWORD event);

void AbleColorChanger ( LONG on );

