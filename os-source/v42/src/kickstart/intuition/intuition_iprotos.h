
/* activate.c */
LONG IActivateGadget ( struct InputToken *it );
BOOL ActivateGadget ( struct Gadget *gadget , struct Window *window , struct Requester *request );

/* alert.c */
ULONG timedDisplayAlert ( ULONG alertnum , UBYTE *string , LONG height , ULONG timeout );

/* bhooks.c */
ULONG boolHook ( struct Hook *h , struct Gadget *g , ULONG *cgp );

/* buttongclass.c */
Class *initButtonGClass ( void );

/* classes.c */
void initIClasses ( void );
ULONG IDoGadgetMethodA ( struct Gadget *gad , struct Window *win , Msg message );
void sendNotifyIDCMP ( struct opUpdate *msg );
ULONG DoGadgetMethodA ( struct Gadget *gad , struct Window *win , struct Requester *req , Msg message );
ULONG SetGadgetAttrsA ( struct Gadget *g , struct Window *window , struct Requester *req , struct TagItem *taglist );
ULONG SetAttrsA ( APTR o , struct TagItem *tags );
ULONG GetAttr ( ULONG AttrID , APTR o , ULONG *StoragePtr );
APTR NewObjectA ( struct IClass *cl , UBYTE *classid , struct TagItem *tags );
void DisposeObject ( APTR o );
APTR NextObject ( APTR ptr );

/* coercion.c */
void setupMonitor ( ULONG force );
void coerceScreens ( struct Screen *onescreen , ULONG force );
ULONG setScreenDisplayID ( struct Screen *sc , ULONG displayID , ULONG *ecodeptr , ULONG force );

/* dclip.c */
void autoScroll ( struct Screen *sc );
void rethinkMouseLimits ( void );
void screenLegalPosition ( struct Screen *screen , struct Rectangle *limitrect , WORD flags );
LONG QueryOverscan ( ULONG displayID , struct Rectangle *rect , LONG ostype );

/* drawgadgets.c */
void commonGadgetRender ( struct RastPort *rp , struct Gadget *g , struct GadgetInfo *gi , struct IBox *gbox , LONG suppress_selected );
void commonGadgetTextAndGhost ( struct RastPort *rp , struct Gadget *g , struct GadgetInfo *gi , struct IBox *gbox );
ULONG gadgetDrawState ( struct Gadget *g , struct GadgetInfo *gi , LONG suppress_selected );
void drawGadgets ( APTR ptr , struct Gadget *gadget , LONG numgad , LONG filter_flags );
LONG callGadgetHookPkt ( struct Hook *hook , struct Gadget *gad , Msg message );
LONG callGadgetHook ( struct Hook *hook , struct Gadget *gad , ULONG method , struct GadgetInfo *ginfo, ...);
LONG propRender ( struct Gadget *g , struct gpRender *cgp );
void renderNewKnob ( struct Gadget *g , struct GadgetInfo *gi , struct PGX *pgx , struct RastPort *rp );
void drawKnob ( struct Gadget *g , struct GadgetInfo *gi , struct RastPort *rp , struct PGX *pgx );

/* ezreq.c */
void FreeSysRequest ( struct Window *window );
LONG easyRequestArgs ( struct Window *rwin , struct EasyStruct *ez , ULONG *idcmp_ptr , UWORD *args );
BOOL autoRequest ( struct Window *rwin , struct IntuiText *bodyt , struct IntuiText *goodt , struct IntuiText *badt , ULONG goodif , ULONG badif , WORD width , WORD height );
LONG SysReqHandler ( struct Window *w , ULONG *idcmp_ptr , LONG wait_first );
struct Window *buildEasyRequestArgs ( struct Window *rwin , struct EasyStruct *ez , ULONG iflags , UWORD *args );
APTR buildSysRequest ( struct Window *rwin , struct IntuiText *bodyt , struct IntuiText *retryt , struct IntuiText *cancelt , ULONG iflags , LONG width , LONG height );

/* fillrectclass.c */
Class *initFillRectClass ( void );

/* fonts.c */
void copyTAttr ( struct TextAttr *ta , struct TAttrBuff *tab );
struct TextFont *ISetFont ( struct RastPort *rp , struct TextAttr *font );
void ICloseFont ( struct TextFont *fp );
LONG IntuiTextLength ( struct IntuiText *itext );
LONG RastPortITextLength ( struct RastPort *rp , struct IntuiText *itext );
void RastPortITextExtent ( struct RastPort *rp , struct IntuiText *itext , struct TextExtent *te );

/* format.c */
struct ITList *formatITList ( UBYTE *fmt , UWORD *arglist , UWORD **nextargp , struct Remember **remkey , LONG separator );

/* frameiclass.c */
Class *initFrameIClass ( void );
void interiorBox ( struct RastPort *rp , struct IBox *b , LONG xw , LONG yw , LONG pen );

/* frbuttonclass.c */
Class *initFramedButtonClass ( void );

/* gadclass.c */
Class *initGadgetClass ( void );
LONG getGadgetAttr ( LONG cl , LONG o , LONG msg );

/* gadgets.c */
UWORD AddGList ( struct Window *window , struct Gadget *gadget , ULONG pos , LONG numgad , struct Requester *requester );
UWORD RemoveGList ( struct Window *window , struct Gadget *gadget , LONG numgad );
LONG delinkGadgetList ( struct Gadget *gadget , struct Gadget *firstgadget , LONG numgad );
LONG inGList ( struct Gadget *gadget , struct Gadget *firstgadget , LONG numgad );
void OnGadget ( struct Gadget *gadget , struct Window *w , struct Requester *req );
void OffGadget ( struct Gadget *gadget , struct Window *w , struct Requester *req );
void NewModifyProp ( struct Gadget *gad , struct Window *win , struct Requester *req , ULONG flags , ULONG hpot , ULONG vpot , ULONG hbody , ULONG vbody , LONG numgad );
void IModifyProp ( struct Gadget *gad , struct Window *win , struct PropPacket *pp );
void sniffWindowGadgets ( struct Window *w , struct Gadget *g , LONG numgad );
void initGadgets ( struct Window *win , struct Gadget *g , LONG numgad );

/* genv.c */
void gadgetBox ( struct Gadget *g , struct GadgetInfo *gi , struct IBox *box );
void lockGDomain ( struct GListEnv *ge );
void unlockGDomain ( struct GListEnv *ge );
struct Point *gadgetPoint ( struct Gadget *g , struct GadgetInfo *gi , struct Point windowcoord , struct Point *gadgetcoord , struct IBox *gbox );
struct Point *gadgetMouse ( struct Gadget *g , struct GadgetInfo *gi , struct Point *mouse , struct IBox *gbox );
void GadgetMouse ( struct Gadget *g , struct GadgetInfo *gi , WORD *mouse );
struct Hook *gadgetHook ( struct Gadget *g );
void gadgetInfo ( struct Gadget *g , struct GListEnv *ge );
void clearWords ( UWORD *ptr , ULONG numwords );
BOOL gListDomain ( struct Gadget *g , struct Window *window , struct GListEnv *ge );
void gadgetBoundingBox ( struct Gadget *g , struct GadgetInfo *gi , struct IBox *box );
struct Requester *findGadgetRequest ( struct Gadget *gadget , struct Window *window );
struct IBox *getImageBox ( struct IBox *im , struct IBox *box );
void newImageBox ( struct Image *im , struct IBox *box );

/* ggclass.c */
Class *initGGClass ( void );

/* hitgadgets.c */
BOOL hitGadgets ( struct Layer *layer );
ULONG hitGGrunt ( struct Gadget *g , struct GadgetInfo *gi , struct IBox *gbox , ULONG methodid );
void gadgetHelpTimer ( void );
void HelpControl ( struct Window *helpwin , ULONG help );

/* icclass.c */
Class *initICClass ( void );

/* idcmp.c */
ULONG translateIEtoIDCMP ( UBYTE ieclass , UWORD iecode );
BOOL ModifyIDCMP ( struct Window *window , ULONG flags );
void IModifyIDCMP ( struct Window *window , ULONG flags );
void reclaimWBMsg ( void );
void reclaimMessages ( struct Window *window );
LONG snoopVerifyReply ( void );
struct IntuiMessage *initIMsg ( ULONG class , ULONG code , struct InputEvent *ie , struct Window *window , struct TabletData *tablet );
void sendAllAll ( ULONG class , ULONG sendflag );

/* iequeue.c */
void initIEvents ( void );
void reclaimIEvents ( void );
struct InputEvent *cloneIEvent ( struct InputEvent *ie );
struct InputEvent *queueIEvent ( void );
struct InputEvent *returnIEvents ( void );

/* ih.c */
struct InputEvent *IntuitionHandler ( struct InputEvent *ie );
void Intuition ( struct InputEvent *ie );
void activeEvent ( ULONG ieclass , ULONG code );
void windowEvent ( struct Window *sendwindow , ULONG ieclass , ULONG iecode );
void broadcastIEvent ( ULONG ieclass );
BOOL sendIDCMP ( ULONG ieclass , ULONG code , struct InputToken *it , struct Window *window , APTR iaddress );
BOOL PutMsgSafely ( struct MsgPort *port , struct IntuiMessage *imsg );

/* ihfront.c */
void createInputTokens ( struct InputEvent *ie );

/* ihooks.c */
void initHook ( struct Hook *hook , ULONG (*dispatch )());
void InitIIHooks ( void );

/* ilock.c */
void lockIBase ( ULONG isem );
void unlockIBase ( ULONG isem );
LONG fakeLockLayers ( APTR x );
LONG fakeUnlockLayers ( APTR x );
LONG fakeLockLayerInfo ( APTR x );
LONG fakeUnlockLayerInfo ( APTR x );
LONG fakeLockLayerRom ( APTR x );
LONG fakeUnlockLayerRom ( APTR x );
LONG CheckLock ( UBYTE *str , LONG sem );
LONG AssertLock ( UBYTE *str , LONG sem );
LONG LockFree ( UBYTE *str , LONG sem );
LONG badOrder ( struct Task *task , LONG sem );

/* imageclass.c */
Class *initImageClass ( void );
void DrawImage ( struct RastPort *rport , struct Image *image , LONG xoffset , LONG yoffset );
void DrawImageState ( struct RastPort *rport , struct Image *image , LONG xoffset , LONG yoffset , ULONG state , struct DrawInfo *drinfo );
BOOL PointInImage ( ULONG point , struct Image *image );
void EraseImage ( struct RastPort *rport , struct Image *image , LONG xoffset , LONG yoffset );
ULONG sendCompatibleImageMessage ( struct Image *image , ULONG msg1, ... );

/* images.c */

/* init.c */
void InitRequester ( struct Requester *req );
void IntuiRomInit ( void );

/* iprefs.c */
void syncIPrefs ( LONG size );
void trackViewPos ( void );
LONG SetIPrefs ( CPTR p , ULONG psize , ULONG ptype );
void setColorPrefs ( UWORD *prefcolor , UWORD firstcolor , UWORD numcolors );

/* isdefault.c */
void doDefault ( void );
void sendWindowMouse ( void );

/* ism.c */
void initTokens ( void );
struct InputToken *newToken ( void );
ULONG doISM ( enum ITCommand command , CPTR obj1 , CPTR obj2 , ULONG subcommand );
struct InputToken *preSendISM ( enum ITCommand command , CPTR obj1 , CPTR obj2 , ULONG subcommand );
void sendISM ( enum ITCommand command , CPTR obj1 , CPTR obj2 , ULONG subcommand );
void sendISMNoQuick (enum ITCommand command, CPTR obj1, CPTR obj2, ULONG subcommand);
void beginToken ( struct InputToken *it );
void deferToken ( void );
void reuseToken ( void );

/* isupp2.c */
void stretchIFrame ( struct Point delta );
void dragIFrame ( struct Point delta );
BOOL isTick ( struct InputEvent *ie );
BOOL isSelectup ( struct InputEvent *ie );
void drawFrames ( struct RastPort *RP , struct Rectangle *oldrect , struct Rectangle *newrect );

/* isupport.c */
BOOL knownScreen ( struct Screen *s );
BOOL knownWindow ( struct Window *w );
void internalBorderPatrol ( struct Screen *s );
void BeginRefresh ( struct Window *window );
void EndRefresh ( struct Window *window , LONG complete );
void updateOptions ( UWORD *firstoption , BOOL draw );
struct IntuitionBase *OpenIntuition ( void );

/* itexticlass.c */
Class *initITextIClass ( void );

/* layout.c */
struct Gadget *createITGadget ( struct Screen *screen , struct IntuiText *gtext , struct Remember **remkey , struct Gadget *prevgad , ULONG id , ULONG frame );
void spreadLayoutGadgets ( struct Gadget *g , LONG gadcount , struct IBox *box , LONG totalwidth , LONG gadspace );
LONG glistExtentBox ( struct Gadget *g , struct IBox *box , LONG gadspace );
LONG ITextLayout ( struct RastPort *rp , struct IntuiText *it , struct IBox *box , BOOL do_layout , LONG xoffset , LONG yoffset );

/* menu.c */
LONG checkmarkMenu ( struct Menu *menu , struct MenuItem *firstitem , BOOL sub );
struct Menu *grabMenu ( struct Menu *menu , UWORD menuNum );
struct MenuItem *grabItem ( struct Menu *menu , WORD menuNum );
struct MenuItem *grabSub ( struct MenuItem *item , WORD menuNum );
void eraseItem ( void );
void eraseSub ( void );
void highItem ( void );
void highSub ( void );
void showMStrip ( struct Window *w );
void removeMStrip ( struct Screen *s );
void resetMenu (struct Window *w, WORD mflags, WORD iflags, WORD sflags);
void drawItem ( UWORD num );
void drawSub ( UWORD num );
BOOL doSetMenuStrip ( struct Window *window , struct Menu *menu , LONG recalc );
void realSetMenuStrip (struct Window *window, struct Menu *menu, LONG precalc_sizes);
void ClearMenuStrip ( struct Window *window );
void IOnOffMenu ( struct Window *window , UWORD menuNum , BOOL onOrOff );
BOOL OnMenu ( struct Window *window , ULONG menuNum );
BOOL OffMenu ( struct Window *window , ULONG menuNum );
void getMenu ( void );

/* misc.c */
void DisplayBeep (struct Screen *screen);
APTR AllocRemember ( struct Remember **remember , ULONG size , ULONG flags );
void FreeRemember ( struct Remember **remember , LONG reallyforget );
struct Layer *getGimmeLayer ( struct Window *w );
void cloneRP ( struct RastPort *clonerp , struct RastPort *rp );
void BlastPatternBox ( struct RastPort *rp , struct IBox *box , UWORD *pat , LONG size , UBYTE pena , UBYTE penb , UBYTE mode );
void BlastPattern ( struct RastPort *RP , LONG left , LONG top , LONG right , LONG bottom , UWORD *pattern , LONG size , UBYTE pena , UBYTE penb , UBYTE mode );
void drawBox ( struct RastPort *rp , LONG x , LONG y , LONG width , LONG height , UBYTE apen , UBYTE mode , UBYTE hthick , UBYTE vthick );
void xorbox ( struct RastPort *rp , LONG left , LONG top , LONG width , LONG height );
void xorboxmask ( struct RastPort *rp , LONG left , LONG top , LONG width , LONG height );
void outbox ( struct RastPort *rp , LONG left , LONG top , LONG width , LONG height );
void drawBlock ( struct RastPort *rp , LONG left , LONG up , LONG right , LONG down , UBYTE apen );
void drawImageGrunt ( struct RastPort *rport , struct Image *image , LONG xoffset , LONG yoffset , UBYTE minterm );
void DrawBorder ( struct RastPort *rp , struct Border *border , LONG left , LONG top );
LONG PrintIText ( struct RastPort *rp , struct IntuiText *itext , LONG left , LONG top );
void printIText ( struct RastPort *rp , struct IntuiText *itext , LONG left , LONG top , LONG use_it_pens_and_mode );
struct MenuItem *ItemAddress ( struct Menu *menustrip , ULONG menunum );
BOOL DoubleClick ( ULONG startsec , ULONG startmicro , ULONG endsec , ULONG endmicro );
void CurrentTime ( ULONG *secs , ULONG *micros );
struct View *ViewAddress ( void );
struct ViewPort *ViewPortAddress ( struct Window *window );
void IDisplayBeep ( struct Screen *screen );
struct RastPort *obtainRP ( struct RastPort *clonee , struct Layer *layer );
void ReleaseGIRPort ( struct RastPort *rp );
void freeRP ( struct RastPort *rp );
struct RastPort *ObtainGIRPort ( struct GadgetInfo *gi );

/* modclass.c */
Class *initModelClass ( void );

/* mouse.c */
void acceloMouse ( struct InputEvent *ie );
void correctoMouse ( struct InputEvent *ie );
void correctOne ( WORD *value , UBYTE *state );
void mouseError ( WORD *errorsum , WORD *pos , WORD *delta );
void freeMouse ( void );
void moverastermouse ( void );
void rastermouse ( LONG x , LONG y );
void screenMouse ( struct Screen *s );
void windowMouse ( struct Window *w );
LONG SetMouseQueue ( struct Window *w , ULONG qlen );

/* newlook.c */
struct Gadget *createNewSysGadget ( struct XScreen *s , UWORD res , UWORD gindex , BOOL gimme );
void embossedBoxTrim ( struct RastPort *rp , struct IBox *b , UBYTE ulpen , UBYTE lrpen , UBYTE meeting_type );
void getSizeGDims ( struct Screen *sc , UWORD dims [2 ]);

/* phooks.c */
ULONG propHook ( struct Hook *h , struct Gadget *g , ULONG *cgp );
LONG propSetupInfo ( struct Gadget *g , struct CGPSetupInfo *cgp );

/* pointer.c */
void initMousePointer ( void );
struct BitMap *setDefaultMousePointer ( struct BitMap *bm , LONG xoffset , LONG yoffset , LONG xresn , LONG yresn , LONG wordwidth , LONG type );
LONG ISetWindowPointer ( struct Window *win , struct MousePointer *newMousePointer );
LONG setMousePointer ( void );
void updateMousePointer ( struct ExtSprite *newSprite );
void notePointerKill ( struct IntuitionBase *passedIBase );
void reinstatePointer ( void );
void setSpriteSpeed ( struct Screen *sc );
void LIB_SetWindowPointerA ( struct Window *win , struct TagItem *taglist );
void SetPointer ( struct Window *win , UWORD *pointer , LONG height , LONG width , LONG xoffset , LONG yoffset );
Class *initPointerClass ( void );

/* pools.c */
void poolInit ( struct List *lh , LONG size , LONG initnum );
struct Node *poolGet ( struct List *lh );
void poolReturn ( struct Node *ln );

/* prefs.c */
struct Preferences *GetDefPrefs ( struct Preferences *preferences , LONG size );
struct Preferences *GetPrefs ( struct Preferences *preferences , LONG size );
struct Preferences *setPrefs ( struct Preferences *preferences , LONG size , BOOL RealThing );
void setWBColorsGrunt ( struct Screen *screen , BOOL allcolors );

/* prop.c */
void containerBumpPots ( struct Gadget *g , struct GadgetInfo *gi , struct Point coord , struct IBox *knobbox );
UWORD potToTop ( UWORD pot , UWORD body );
void positionNewKnobForPots ( struct Gadget *g , struct GadgetInfo *gi , struct PGX *pgx );
BOOL dragNewKnob ( struct Gadget *g , struct GadgetInfo *gi , struct Point mousept , struct IBox *gbox );
void syncKnobs ( struct Gadget *g , struct GadgetInfo *gi , struct PGX *pgx );
BOOL borderlessProp ( struct Gadget *g );
void setupPGX ( struct Gadget *g , struct GadgetInfo *gi , struct PGX *pgx );

/* propgadclass.c */
Class *initPropGadgetClass ( void );

/* pubclasses.c */
void InitClassList ( void );
struct List *lockClassList ( void );
void unlockClassList ( void );
Class *FindClass ( ClassID classid );
struct IClass *MakeClass ( UBYTE *classid , UBYTE *superid , struct IClass *superclass , ULONG instsize , ULONG flags );
void AddClass ( Class *cl );
Class *makePublicClass ( ClassID classid , ClassID superid , UWORD instsize , ULONG (*dispatch )());
Class *makeMyClass ( LONG myclassid , ClassID *superid , Class *superclass , UWORD instsize , ULONG (*dispatch )());
void RemoveClass ( Class *cl );
BOOL FreeClass ( Class *cl );

/* pubscreen.c */
struct Screen *LockPubScreen ( UBYTE *name );
void UnlockPubScreen ( UBYTE *name , struct Screen *screen );
struct List *LockPubScreenList ( void );
void UnlockPubScreenList ( void );
UBYTE *NextPubScreen ( struct Screen *screen , UBYTE *namebuf );
void SetDefaultPubScreen ( UBYTE *name );
struct Screen *GetDefaultPubScreen ( UBYTE *Namebuff );
UWORD SetPubScreenModes ( UWORD modes );
UWORD PubScreenStatus ( struct Screen *screen , UWORD status );
struct Screen *windowPubScreen ( UBYTE *pubname , BOOL fallback );
struct Screen *defaultPubScreen ( void );
struct PubScreenNode *initPubScreen ( char *pubname , struct Task *pubtask , UBYTE pubsig , LONG *errorptr );
void linkPubScreen ( struct Screen *sc , struct PubScreenNode *pubnode );
LONG freePubScreenNode ( struct PubScreenNode *psn );
LONG bumpPSNVisitor ( struct PubScreenNode *psn );
LONG decPSNVisitor ( struct PubScreenNode *psn );

/* rect.c */
void eraseRect ( struct RastPort *rp , struct Rectangle *r );
void eraseRemnants ( struct RastPort *rp , struct IBox *oldbox , struct IBox *newbox , struct Point offset , UBYTE pen , UBYTE patternpen );
void fillAround ( struct RastPort *rp , struct Rectangle *yesrect , struct Rectangle *norect , UBYTE pen , UBYTE patternpen );
void boxModernize ( void (*func )(struct RastPort *,LONG,LONG,LONG,LONG), struct RastPort *rp , struct IBox *box );

/* requester.c */
void drawRequester ( struct Requester *req , struct Window *w );
BOOL ClearDMRequest ( struct Window *w );
BOOL SetDMRequest ( struct Window *w , struct Requester *req );
BOOL Request ( struct Requester *req , struct Window *window );
BOOL IRequest ( struct Requester *req , struct Window *window , LONG isdmr );
BOOL IEndRequest ( struct Window *window , struct Requester *req );
void EndRequest ( struct Requester *req , struct Window *window );

/* rootclass.c */
Class *initRootClass ( void );

/* scale.c */
void screenToMouse ( struct Point *pt , struct Screen *screen );
void screenToLongMouse ( struct LongPoint *pt , struct Screen *screen );
void longmouseToScreen ( struct LongPoint *pt , struct Screen *screen );
void scaleConversion ( struct Point *pt , struct Point res1 , struct Point res2 );
void mouseToSprite ( struct Point *pt );
void scaleScreenMouse ( struct Rectangle *rect , struct Screen *screen );

/* screens.c */
struct Screen *openScreen ( struct NewScreen *ns );
struct Screen *openScreenTagList ( struct NewScreen *ns , struct TagItem *tags );
LONG IOpenScreen ( struct Screen *sc , struct TagItem *taglist );
ULONG displayOScan ( CPTR drecord , ULONG displayID , LONG ostype , struct Rectangle *rect );
BOOL CloseScreen ( struct Screen *OScreen );
void AlohaWorkbench ( struct MsgPort *wbport );
LONG CloseWorkBench ( void );
void initWBMessage ( struct IntuiMessage *imsg , struct MsgPort *replyport , UWORD code );
BOOL ICloseScreen ( struct Screen *OScreen );
struct Screen *openSysScreen ( UWORD type );

/* scsupp.c */
void forceBarFront ( register struct Screen *s );
void forceBarBack ( register struct Screen *s );
void forceBarCenter ( register struct Screen *s );
LONG screenGadgets ( struct Screen *s );
void ScreenPosition ( struct Screen *screen , ULONG flags , LONG x1 , LONG y1 , LONG x2 , LONG y2 );
void IPreMoveScreen ( struct Screen *screen , struct Rectangle *rect , ULONG flags );
void IMoveScreen ( struct Screen *screen , struct Point *delta , WORD flags );
void LIB_ScreenDepth ( struct Screen *screen , ULONG flags , struct Screen *reserved );
void IScreenDepth ( struct Screen *screen , LONG whichway );
struct DrawInfo *GetScreenDrawInfo ( struct Screen *s );
void FreeScreenDrawInfo ( struct Screen *s , struct DrawInfo *dri );
void setScreen ( struct Screen *s );
void drawScreenBar ( struct RastPort *rp , struct Screen *s , ULONG blockpen );
void refreshScreenBar ( struct Screen *sc );
void screenbar ( struct Screen *s );
void ShowTitle ( struct Screen *screen , BOOL showit );
void IShowTitle ( struct Screen *screen , BOOL showit );
struct ScreenBuffer *AllocScreenBuffer ( struct Screen *sc , struct BitMap *bm , ULONG flags );
void ICopyScreenBuffer ( struct Screen *sc , struct ScreenBuffer *sb );
void FreeScreenBuffer ( struct Screen *sc , struct ScreenBuffer *sb );
LONG ChangeScreenBuffer ( struct Screen *sc , struct ScreenBuffer *sb );
void IChangeScreenBuffer ( struct Screen *screen , struct ScreenBuffer *sb );
LONG parentOriginY ( struct Screen *child , struct Screen *parent );
struct Screen *screenFamily ( struct Screen *s );
LONG attachScreen ( struct Screen *child , struct Screen *parent );
void relinkScreen ( struct Screen *screen , ULONG flags );
void delinkScreen ( struct Screen *OScreen );
LONG canSlide ( struct Screen *s , ULONG override );
BOOL IsHires ( struct Screen *s );

/* sdmrtimeout.c */
void startDMRTimeout ( void (*dmrreturn )(LONG), LONG okcode );
void dDMRTimeout ( void );

/* sgadget.c */
void startGadget ( void (*gadgetreturn )());
void dGadget ( void );

/* shooks.c */
ULONG stringHook ( struct Hook *h , struct Gadget *g , ULONG *cgp );
LONG stringSetupInfo ( struct Gadget *g , struct CGPSetupInfo *cgp );
LONG stringHitTest ( void );

/* sidlewindow.c */
void startIdleWindow ( void );
void dIdleWindow ( void );

/* sinput.c */
LONG stringInput ( struct InputEvent *ie , struct Gadget *g , struct GadgetInfo *gi , BOOL *redisplay , LONG *gadgetupcode );
void iEdit ( CPTR hook , struct SGWork *sgw );
LONG iMouse ( struct InputEvent *ie , struct Gadget *g , struct GadgetInfo *gi , struct StringExtend *sex , BOOL *redisplay , struct IBox *gbox );
LONG copybuf ( BYTE *src , LONG size , BYTE *dest );
struct Hook *SetEditHook ( struct Hook *edithook );

/* size.c */
void sizeDamage ( struct Window *w , LONG dx , LONG dy );

/* smenu.c */
void startMenu ( void );
void startMenuKey ( UWORD menunum );
void dMenu ( void );
void sendAllMenuClear ( struct Screen *screen );
UWORD findMenuKey ( struct Menu *menu , LONG code );

/* snowindow.c */
void startNoWindow ( void );
void dNoWindow ( void );

/* srequester.c */
void startRequester ( void );
void dRequester ( void );

/* sscreendrag.c */
void startScreenDrag (void (*sdreturn )(void));
void dScreenDrag ( void );

/* strgadclass.c */
Class *initStrGadgetClass ( void );

/* strings.c */
void displayString ( struct RastPort *rp , struct Gadget *sg , struct GadgetInfo *gi , struct IBox *gbox , struct StringExtend *sex );
struct StringExtend *getSEx ( struct Gadget *g );

/* sverify.c */
void startVerify ( void (*vreturn )(LONG), struct Window *window , ULONG class , UWORD code );
void dVerify ( void );

/* swsizedrag.c */
void startWSizeDrag ( void (*wsdreturn )(void), LONG size_drag );
void dWSizeDrag ( void );

/* thing.c */
void delinkThing ( struct Thing *thing , struct Thing *prevthing );
struct Thing *previousThing ( struct Thing *thing , struct Thing *prevthing , LONG *pos );
struct Thing *lastThing ( struct Thing *thing );
struct Thing *nthThing ( struct Thing *thing , LONG pos , UWORD *realn );
struct Thing *previousLink ( struct Thing *first , struct Thing *current );

/* timing.c */
void doTiming ( void );

/* varargs.c */
ULONG SetGadgetAttrs ( struct Gadget *g , struct Window *w , struct Requester *req , ULONG tag1, ...);
ULONG SetAttrs ( Object *o , ULONG tag1, ...);
Object *NewObject ( Class *cl , ClassID classid , ULONG tag1 , ...);
ULONG NotifyAttrChanges ( Object *o , void *ginfo , ULONG flags , ULONG tag1, ...);
ULONG mySetSuperAttrs ( Class *cl , Object *o , ULONG t1, ...);

/* vectorclass.c */
Class *initSysIClass ( void );

/* view.c */
void MakeScreen ( struct Screen *screen );
LONG newMakeScreen ( struct Screen *screen , BOOL rethink );
LONG RemakeDisplay ( void );
LONG RethinkDisplay ( void );
LONG modeVerify ( LONG force );

/* wbench.c */
struct Screen *findWorkBench ( void );
struct Screen *openwbscreen ( void );
struct Screen *openWorkBench ( void );
void pokeWorkbench ( void );
BOOL GetScreenData ( struct Screen *buffer , UWORD size , UWORD type , struct Screen *customsc );

/* windows.c */
LONG addSWGadgets ( struct Window *w , LONG addzoom );
void freeSysGadgets ( struct Gadget *g );
struct Window *hitUpfront ( struct Layer **layerhit );
struct Screen *hitScreen ( void );
LONG IOpenWindow ( struct Window *w , struct BitMap *superbmap , struct Hook *backfill );
void closeWindowCommon ( struct Window *window , ULONG failure );
LONG embossedFillPen ( struct Window *w );
void drawEmbossedBorder ( struct Window *w , LONG draw_flags );
void SafeRectFill ( struct RastPort *rp , WORD xmin , WORD ymin , WORD xmax , WORD ymax );
void setWindow ( struct Window *w , LONG initial );
LONG windowObscured ( struct Window *win );
LONG intersectBox ( struct IBox *box1 , struct IBox *box2 );
struct Window *OpenWindow ( struct ExtNewWindow *nw );
void CloseWindow ( struct Window *window );
void ActivateWindow ( struct Window *window );
void RefreshWindowFrame ( struct Window *w );
void LendMenus ( struct Window *fromwindow , struct Window *towindow );
void SetWindowTitles ( struct Window *window , UBYTE *windowtitle , UBYTE *screentitle );
BOOL WindowLimits ( struct Window *window , WORD minwidth , WORD minheight , UWORD maxwidth , UWORD maxheight );
void MoveWindow ( struct Window *window , LONG dx , LONG dy );
void SizeWindow ( struct Window *window , WORD dx , WORD dy );
void ChangeWindowBox ( struct Window *window , LONG l , LONG t , LONG w , LONG h );
void WindowToFront ( struct Window *window );
void MoveWindowInFrontOf ( struct Window *w , struct Window *behindw );
void WindowToBack ( struct Window *window );
void ScrollWindowRaster ( struct Window *win , LONG dx , LONG dy , LONG xmin , LONG ymin , LONG xmax , LONG ymax );

/* wtags.c */
struct Window *OpenWindowTagList ( struct NewWindow *nw , struct TagItem *tags );
void fixWindowBorders ( void );

/* zoom.c */
void ZipWindow ( struct Window *window );
void IZoomWindow ( struct Window *window , BOOL interactive );
LONG IChangeWindow ( struct Window *w , struct IBox *newbox , LONG subcommand );
void IWindowDepth ( struct Window *w , LONG whichway , struct Window *behindwindow );
LONG setupLCS ( struct Window *w , struct LayerChangeSpec *lcs , struct LayerEntry *le , struct IBox *wbox );
BOOL establishReqLayer3 ( struct Requester *req , struct IBox *windowbox , struct Point *gzzdims );
