
******* asl.library/AllocAslRequest ******************************************
*
*   NAME
*	AllocAslRequest -- allocate an ASL requester. (V36)
*	AllocAslRequestTags -- varargs stub for AllocAslRequest(). (V36)
*
*   SYNOPSIS
*	requester = AllocAslRequest(reqType,tagList);
*	D0                          D0      A0
*
*	APTR AllocAslRequest(ULONG,struct TagItem *);
*
*	requester = AllocAslRequestTags(reqType,firstTag, ...);
*
*	APTR AllocAslRequestTags(ULONG,Tag,...);
*
*   FUNCTION
*	Allocates an ASL requester data structure of the specified type.
*
*   INPUTS
*	type - type of requester to allocate. Currently defined types
*	       are ASL_FileRequest, ASL_FontRequest and ASL_ScreenModeRequest.
*	       ASL_ScreenModeRequest is available starting with asl.library
*	       V38 only.
*	tags - pointer to an optional tag list specifying how to initialize the
*	       data structure returned by this function. See the
*	       documentation for AslRequest() for an explanation of
*	       how to use the currently defined tags.
*
*   RESULT
*	requester - an initialized requester data structure, or NULL on
*	            failure. The data structure returned corresponds to the
*		    requested type;
*			ASL_FileRequest       -> (struct FileRequester *)
*			ASL_FontRequest       -> (struct FontRequester *)
*			ASL_ScreenModeRequest -> (struct ScreenModeRequester *)
*
*   WARNING
*	All ASL requester data structures are READ-ONLY and can only be
*	modified by using tags!
*
*   SEE ALSO
*	AslRequest(), FreeAslRequest()
*
******************************************************************************


******* asl.library/AslRequest ***********************************************
*
*   NAME
*	AslRequest -- get input from user using an ASL requester. (V36)
*	AslRequestTags -- varargs stub for AslRequest(). (V36)
*
*   SYNOPSIS
*	result = AslRequest(requester,tags);
*	D0                  A0        A1
*
*	BOOL AslRequest(APTR,struct TagItem *);
*
*	result = AslRequestTags(requester,firstTag,...);
*
*	BOOL AslRequestTags(APTR,Tag,...);
*
*   FUNCTION
*	Prompts the user for input, based on the specific type of
*	requester and modifying tags. The actions and results
*	are specific to the type of requester, but in general the action is
*	to open a requesting window prompting the user for a specific input.
*	If the user cancels or the system aborts the request, FALSE
*	is returned, otherwise the requester's data structure reflects the
*	user input.
*
*	Note that tag values stay in effect for each use of the
*	requester until they are cleared or modified by passing the
*	same tag with a new value.
*
*   INPUTS
*	requester - requester structure allocated with AllocAslRequest(). If
*		    this parameter is NULL, this function will always return
*		    FALSE with a dos.library/IoErr() result of
*		    ERROR_NO_FREE_STORE.
*	tags - pointer to an optional tag list which may be used to
*	       control features of the requester.
*
*   TAGS
*	Tags for use with the file requester:
*
*	ASLFR_Window (struct Window *) - Parent window of requester. If no
*			ASLFR_Screen tag is specified, the window structure
*			is used to determine on which screen to open the
*			requesting window. (V36)
*
*	ASLFR_PubScreenName (STRPTR) - Name of a public screen to open on.
*			This overrides the screen used by ASLFR_Window. (V38)
*
*	ASLFR_Screen (struct Screen *) - Screen on which to open the requester.
*			This overrides the screen used by ASLFR_Window or by
*			ASLFR_PubScreenName. (V38)
*
*	ASLFR_PrivateIDCMP (BOOL) - When set to TRUE, this tells ASL to
*			allocate a new IDCMP port for the requesting window.
*			If not specified or set to FALSE, and if ASLFR_Window
*			is provided, the requesting window will share
*			ASLFR_Window's IDCMP port. (V38)
*
*	ASLFR_IntuiMsgFunc (struct Hook *) - A function to call whenever an
*			unknown Intuition message arrives at the message port
*			being used by the requesting window. The function
*			receives the following parameters:
*				A0 - (struct Hook *)
*				A1 - (struct IntuiMessage *)
*				A2 - (struct FileRequester *)
*			(V38)
*
*	ASLFR_SleepWindow (BOOL) - When set to TRUE, this tag will cause the
*			window specified by ASLFR_Window to be "put to
*			sleep". That is, a busy pointer will be displayed in
*			the parent window, and no gadget or menu activity will
*			be allowed. This is done by opening an invisible
*			Intuition Requester in the parent window. (V38)
*
*	ASLFR_UserData (APTR) - A 32-bit value that is simply copied in the
*			fr_UserData field of the requester structure. (V38)
*
*	ASLFR_TextAttr (struct TextAttr *) - Font to be used for the requesting
*			window's gadgets and menus. If this tag is not provided
*			or its value is NULL, the default font of the screen
*			on which the requesting window opens will be used.
*			This font must already be in memory as ASL calls
*			OpenFont() and not OpenDiskFont(). As of V38, the font
*			used in the file requester's file list must be
*			monospaced. If the font provided with this tag is
*			not monospaced, then the file list will use the current
*			system default font as chosen in preferences. (V38)
*
*	ASLFR_Locale (struct Locale *) - Locale to use for the requesting
*			window. This determines the language used for the
*			requester's gadgets and menus. If this tag is not
*			provided or its value is NULL, the system's current
*			default locale will be used. As of V38, items such as
*			the date format always uses the system default locale.
*			(V38)
*
*	ASLFR_TitleText (STRPTR) - Title to use for the requesting window.
*			Default is no title. (V36)
*
*	ASLFR_PositiveText (STRPTR) - Label of the positive gadget in the
*			requester. English default is "OK". Prior to V38, this
*			string could not be longer than 6 characters. (V36)
*
*	ASLFR_NegativeText (STRPTR) - Label of the negative gadget in the
*			requester. English default is "Cancel". Prior to V38,
*			this string could not be longer than 6 characters.
*			(V36)
*
*	ASLFR_InitialLeftEdge (WORD) - Suggested left edge of requesting
*			window. (V36)
*
*	ASLFR_InitialTopEdge (WORD) - Suggested top edge of requesting
*			window. (V36)
*
*	ASLFR_InitialWidth (WORD) - Suggested width of requesting window. (V36)
*
*	ASLFR_InitialHeight (WORD) - Suggested height of requesting window.
*			(V36)
*
*	ASLFR_InitialFile (STRPTR) - Initial contents of the file requester's
*			File text gadget. Default is empty. (V36)
*
*	ASLFR_InitialDrawer (STRPTR) - Initial contents of the file
*			requester's Drawer text gadget. Default is empty. (V36)
*
*	ASLFR_InitialPattern (STRPTR) - Initial contents of the file
*			requester's Pattern text gadget. Default is #?. (V36)
*
*	ASLFR_Flags1 (ULONG) - Bulk initialization of many of the requester's
*			options. See <libraries/asl.h> for the possible
*			flag values. Default is 0. (V36)
*
*	ASLFR_Flags2 (ULONG) - Bulk initialization of many of the requester's
*			options. See <libraries/asl.h> for the possible
*			flag values. Default is 0. (V36)
*
*	ASLFR_DoSaveMode (BOOL) - Set this tag to TRUE when the file requester
*			is being used for saving. Default is FALSE. (V38)
*
*	ASLFR_DoMultiSelect (BOOL) - Set this tag to TRUE to let the user
*			select multiple files at once. This tag excludes
*			save mode. Default is FALSE. (V38)
*
*	ASLFR_DoPatterns (BOOL) - Set this tag to TRUE to cause a pattern
*			gadget to be displayed. Default is FALSE. (V38)
*
*	ASLFR_DrawersOnly (BOOL) - Set this tag to TRUE to cause the requester
*			to only display drawers, and have no File gadget. This
*			is useful to let the user choose a destination
*			directory. Default is FALSE. (V38)
*
*	ASLFR_FilterFunc (struct Hook *) - A function to call for each file
*			encountered. If the function returns TRUE, the file is
*			included in the file list, otherwise it is rejected
*			and not displayed. The function receives the following
*			parameters:
*				A0 - (struct Hook *)
*				A1 - (struct AnchorPath *)
*				A2 - (struct FileRequester *)
*			(V38)
*
*	ASLFR_RejectIcons (BOOL) - Set this tag to TRUE to cause the requester
*			not to display Workbench icons. This flag should
*			normally be set. Default is FALSE. (V38)
*
*	ASLFR_RejectPattern (UBYTE *) - Specifies an AmigaDOS pattern that is
*			used to reject files. That is, any files with names
*			matching this pattern are not included in the file
*			list. Note that the pattern must have been
*			parsed by dos.library/ParsePatternNoCase(). Default
*			is ~(#?) which matches nothing. (V38)
*
*	ASLFR_AcceptPattern (UBYTE *) - Specifies an AmigaDOS pattern that is
*			used to accept files. That is, only files with names
*			matching this pattern are included in the file list.
*			Note that the pattern must have been parsed by
*			dos.library/ParsePatternNoCase(). Default is #? which
*			matches everything. (V38)
*
*	ASLFR_FilterDrawers (BOOL) - Set this tag to TRUE if you want the
*			ASLFR_RejectPattern, ASLFR_AcceptPattern, and the
*			Pattern text gadget to also apply to drawer names.
*			Normally, drawers are always displayed. Default is
*			FALSE. (V38)
*
*	ASLFR_HookFunc (APTR) - A function that is called if the
*			FRF_FILTERFUNC or FRF_INTUIFUNC flags are specified in
*			the ASLFR_Flags1 tag. The function will be called like
*			so, with the arguments on the stack using the standard
*			C pushing order:
*
*				ULONG function(ULONG mask, APTR object,
*					       struct FileRequester *fr)
*
*			The Mask value is a copy of the specific ASLFR_Flags1
*			value the callback is for, either FRF_FILTERFUNC or
*			FRF_INTUIFUNC; object is a pointer to a data object
*			(a (struct IntuiMessage *) for FRF_INTUIFUNC, and a
*			(struct AnchorPath *) for FRF_FILTERFUNC).
*
*			For FRF_FILTERFUNC, the function should return 0 if the
*			file should be added to the list, non-zero if it
*			shouldn't. For FRF_INTUIFUNC, the function should
*			return the original (struct IntuiMessage *) it was
*			passed.
*
*	Tags used for the font requester:
*
*	ASLFO_Window (struct Window *) - Parent window of requester. If no
*			ASLFO_Screen tag is specified, the window structure
*			is used to determine on which screen to open the
*			requesting window. (V36)
*
*	ASLFO_PubScreenName (STRPTR) - Name of a public screen to open on.
*			This overrides the screen used by ASLFO_Window. (V38)
*
*	ASLFO_Screen (struct Screen *) - Screen on which to open the requester.
*			This overrides the screen used by ASLFO_Window or by
*			ASLFO_PubScreenName. (V38)
*
*	ASLFO_PrivateIDCMP (BOOL) - When set to TRUE, this tells ASL to
*			allocate a new IDCMP port for the requesting window.
*			If not specified or set to FALSE, and if ASLFO_Window
*			is provided, the requesting window will share
*			ASLFO_Window's IDCMP port. (V38)
*
*	ASLFO_IntuiMsgFunc (struct Hook *) - A function to call whenever an
*			unknown Intuition message arrives at the message port
*			being used by the requesting window. The function
*			receives the following parameters:
*				A0 - (struct Hook *)
*				A1 - (struct IntuiMessage *)
*				A2 - (struct FontRequester *)
*			(V38)
*
*	ASLFO_SleepWindow (BOOL) - When set to TRUE, this tag will cause the
*			window specified by ASLFO_Window to be "put to
*			sleep". That is, a busy pointer will be displayed in
*			the parent window, and no gadget or menu activity will
*			be allowed. This is done by opening an invisible
*			Intuition Requester in the parent window. (V38)
*
*	ASLFO_UserData (APTR) - A 32-bit value that is simply copied in the
*			fo_UserData field of the requester structure. (V38)
*
*	ASLFO_TextAttr (struct TextAttr *) - Font to be used for the requesting
*			window's gadgets and menus. If this tag is not provided
*			or its value is NULL, the default font of the screen
*			on which the requesting window opens will be used.
*			This font must already be in memory as ASL calls
*			OpenFont() and not OpenDiskFont().
*			(V38)
*
*	ASLFO_Locale (struct Locale *) - Locale to use for the requesting
*			window. This determines the language used for the
*			requester's gadgets and menus. If this tag is not
*			provided or its value is NULL, the system's current
*			default locale will be used. (V38)
*
*	ASLFO_TitleText (STRPTR) - Title to use for the requesting window.
*			Default is no title. (V36)
*
*	ASLFO_PositiveText (STRPTR) - Label of the positive gadget in the
*			requester. English default is "OK". Prior to V38, this
*			string could not be longer than 6 characters. (V36)
*
*	ASLFO_NegativeText (STRPTR) - Label of the negative gadget in the
*			requester. English default is "Cancel". Prior to V38,
*			this string could not be longer than 6 characters.
*			(V36)
*
*	ASLFO_InitialLeftEdge (WORD) - Suggested left edge of requesting
*			window. (V36)
*
*	ASLFO_InitialTopEdge (WORD) - Suggested top edge of requesting
*			window. (V36)
*
*	ASLFO_InitialWidth (WORD) - Suggested width of requesting window. (V38)
*
*	ASLFO_InitialHeight (WORD) - Suggested height of requesting window.
*			(V38)
*
*	ASLFO_InitialName (STRPTR) - Initial contents of the font requester's
*			Name text gadget. Default is none. (V36)
*
*	ASLFO_InitialSize (UWORD) - Initial contents of the font requester's
*			Size numeric gadget (fo_Attr.ta_YSize). Default is 8.
*			(V36)
*
*	ASLFO_InitialStyle (UBYTE) - Initial setting of the font requester's
*			Style gadget (fo_Attr.ta_Style). Default is FS_NORMAL.
*			(V36)
*
*	ASLFO_InitialFlags (UBYTE) - Initial setting of the font requester's
*			fo_Flags field (fo_Attr.ta_Flags). Default is
*			FPF_ROMFONT. (V36)
*
*	ASLFO_InitialFrontPen (UBYTE) - Initial setting of the font requester's
*			Front Color gadget (fo_FrontPen). This value also
*			determines the color used to render the text in the
*			sample font area. Default is 1. (V36)
*
*	ASLFO_InitialBackPen (UBYTE) - Initial setting of the font requester's
*			Back Color gadget (fo_BackPen). This value also
*			determines the color used to render the background of
*			the sample font area. Default is 0. (V36)
*
*	ASLFO_InitialDrawMode (UBYTE) - Initial setting of the font
*			requester's Mode gadget (fo_DrawMode). Default is
*			JAM1. (V38)
*
*	ASLFO_Flags (ULONG) - Bulk initialization of many of the requester's
*			options. See <libraries/asl.h> for the possible
*			flag values. Default is 0. (V36)
*
*	ASLFO_DoFrontPen (BOOL) - Set this tag to TRUE to cause the requester
*			to display the Front Color selection gadget. Default is
*			FALSE. (V38)
*
*	ASLFO_DoBackPen (BOOL) - Set this tag to TRUE to cause the requester
*			to display the Back Color selection gadget. Default
*			is FALSE. (V38)
*
*	ASLFO_DoStyle (BOOL) - Set this tag to TRUE to cause the requester
*			to display the Style checkboxes. Default is FALSE.
*			(V38)
*
*	ASLFO_DoDrawMode (BOOL) - Set this tag to TRUE to cause the requester
*			to display the Mode cycle gadget. Default is FALSE.
*			(V38)
*
*	ASLFO_FixedWidthOnly (BOOL) - Set this tag to TRUE to cause the
*			requester to only display fixed-width fonts. Default
*			is FALSE. (V38)
*
*	ASLFO_MinHeight (UWORD) - The minimum font height to let the user
*			select. Default is 5. (V36)
*
*	ASLFO_MaxHeight (UWORD) - The maximum font height to let the user
*			select. Default is 24. (V36)
*
*	ASLFO_FilterFunc (struct Hook *) - A function to call for each font
*			encountered. If the function returns TRUE, the font is
*			included in the font list, otherwise it is rejected
*			and not displayed. The function receives the following
*			parameters:
*				A0 - (struct Hook *)
*				A1 - (struct TextAttr *)
*				A2 - (struct FontRequester *)
*			(V38)
*
*	ASLFO_HookFunc (APTR) - A function that is called if the
*			FOF_FILTERFUNC or FOF_INTUIFUNC flags are specified in
*			the ASLFO_Flags tag. The function will be called like
*			so, with the arguments on the stack using the standard
*			C pushing order:
*
*				ULONG function(ULONG mask, APTR object,
*					       struct FontRequester *fr)
*
*			The Mask value is a copy of the specific ASLFO_Flags
*			value the callback is for, either FOF_FILTERFUNC or
*			FOF_INTUIFUNC; object is a pointer to a data object
*			(a (struct IntuiMessage *) for FOF_INTUIFUNC, and a
*			(struct TextAttr *) for FOF_FILTERFUNC).
*
*			For FOF_FILTERFUNC, the function should return 1 if the
*			font should be added to the list, 0 if it
*			shouldn't. For FOF_INTUIFUNC, the function should
*			return the original (struct IntuiMessage *) it was
*			passed.
*
*	ASLFO_MaxFrontPen (UBYTE) - The maximum number of pens that are offered
*			to the user when the ASLFO_DoFrontPen tag is specified.
*			Default is 255. (V40)
*
*	ASLFO_MaxBackPen (UBYTE) - The maximum number of pens that are offered
*			to the user when the ASLFO_DoBackPen tag is specified.
*			Default is 255. (V40)
*
*	ASLFO_ModeList (STRPTR *) - Replacement list of modes to display in
*			Mode cycle gadget. The first string pointer is used
*			as the name of the gadgets, and following strings are
*			used as the different labels of the cycle gadget,
*			until a NULL string pointer which denotes the end of
*			the string pointer array. The first string in this
*			array corresponds to JAM1, the second is JAM2, the
*			third to COMPLEMENT. The array can be early terminated
*			by not providing all entries. English default for
*			the array is "Text" "Text+Field" and "Complement".
*			(V36)
*
*	ASLFO_FrontPens (UBYTE *) - Pointer to a table of pen numbers
*			indicating  which colors should be used and edited
*			by the palette gadget when the ASLFO_DoFrontPen tag is
*			specified. This array must contain as many entries as
*			there are colors displayed in the palette gadget. The
*			Default is NULL, which causes a 1-to-1 mapping of pen
*			numbers. (V40)
*
*	ASLFO_BackPens (UBYTE *) - Pointer to a table of pen numbers
*			indicating  which colors should be used and edited
*			by the palette gadget when the ASLFO_DoBackPen tag is
*			specified. This array must contain as many entries as
*			there are colors displayed in the palette gadget. The
*			Default is NULL, which causes a 1-to-1 mapping of pen
*			numbers. (V40)
*
*	Tags used for the screen mode requester (V38):
*
*	ASLSM_Window (struct Window *) - Parent window of requester. If no
*			ASLSM_Screen tag is specified, the window structure
*			is used to determine on which screen to open the
*			requesting window. (V38)
*
*	ASLSM_PubScreenName (STRPTR) - Name of a public screen to open on.
*			This overrides the screen used by ASLSM_Window. (V38)
*
*	ASLSM_Screen (struct Screen *) - Screen on which to open the requester.
*			This overrides the screen used by ASLSM_Window or by
*			ASLSM_PubScreenName. (V38)
*
*	ASLSM_PrivateIDCMP (BOOL) - When set to TRUE, this tells ASL to
*			allocate a new IDCMP port for the requesting window.
*			If not specified or set to FALSE, and if ASLSM_Window
*			is provided, the requesting window will share
*			ASLSM_Window's IDCMP port. (V38)
*
*	ASLSM_IntuiMsgFunc (struct Hook *) - A function to call whenever an
*			unknown Intuition message arrives at the message port
*			being used by the requesting window. The function
*			receives the following parameters:
*				A0 - (struct Hook *)
*				A1 - (struct IntuiMessage *)
*				A2 - (struct ScreenModeRequester *)
*			(V38)
*
*	ASLSM_SleepWindow (BOOL) - When set to TRUE, this tag will cause the
*			window specified by ASLSM_Window to be "put to
*			sleep". That is, a busy pointer will be displayed in
*			the parent window, and no gadget or menu activity will
*			be allowed. This is done by opening an invisible
*			Intuition Requester in the parent window. (V38)
*
*	ASLSM_UserData (APTR) - A 32-bit value that is simply copied in the
*			sm_UserData field of the requester structure. (V38)
*
*	ASLSM_TextAttr (struct TextAttr *) - Font to be used for the requesting
*			window's gadgets and menus. If this tag is not provided
*			or its value is NULL, the default font of the screen
*			on which the requesting window opens will be used.
*			This font must already be in memory as ASL calls
*			OpenFont() and not OpenDiskFont().
*			(V38)
*
*	ASLSM_Locale (struct Locale *) - Locale to use for the requesting
*			window. This determines the language used for the
*			requester's gadgets and menus. If this tag is not
*			provided or its value is NULL, the system's current
*			default locale will be used. (V38)
*
*	ASLSM_TitleText (STRPTR) - Title to use for the requesting window.
*			Default is no title. (V38)
*
*	ASLSM_PositiveText (STRPTR) - Label of the positive gadget in the
*			requester. English default is "OK". Prior to V38, this
*			string could not be longer than 6 characters. (V36)
*
*	ASLSM_NegativeText (STRPTR) - Label of the negative gadget in the
*			requester. English default is "Cancel". Prior to V38,
*			this string could not be longer than 6 characters.
*			(V36)
*
*	ASLSM_InitialLeftEdge (WORD) - Suggested left edge of requesting
*			window. (V38)
*
*	ASLSM_InitialTopEdge (WORD) - Suggested top edge of requesting
*			window. (V38)
*
*	ASLSM_InitialWidth (WORD) - Suggested width of requesting window. (V38)
*
*	ASLSM_InitialHeight (WORD) - Suggested height of requesting window.
*			(V38)
*
*	ASLSM_InitialDisplayID (ULONG) - Initial setting of the Mode list view
*			gadget (sm_DisplayID). Default is 0 (LORES_KEY). (V38)
*
*	ASLSM_InitialDisplayWidth (ULONG) - Initial setting of the Width gadget
*			(sm_DisplayWidth). Default is 640. (V38)
*
*	ASLSM_InitialDisplayHeight (ULONG) - Initial setting of the Height
*			gadget (sm_DisplayHeight). Default is 200. (V38)
*
*	ASLSM_InitialDisplayDepth (UWORD) - Initial setting of the Colors
*			gadget (sm_DisplayDepth). Default is 2. (V38)
*
*	ASLSM_InitialOverscanType (UWORD) - Initial setting of the
*			Overscan Type cycle gadget (sm_OverscanType).
*			These values are the OSCAN_#? values from
*			<intuition/screens.h>.
*
*			User-space naming of overscan types is different than
*			programmer-space:
*
*			Under V38:
*			     0               -->  "Regular Size"
*			     OSCAN_TEXT      -->  "Text Size"
*			     OSCAN_STANDARD  -->  "Graphics Size"
*			     OSCAN_MAXIMUM   -->  "Maximum Size"
*
*			Under V39 and above:
*			     OSCAN_TEXT      -->  "Text Size"
*			     OSCAN_STANDARD  -->  "Graphics Size"
*			     OSCAN_MAXIMUM   -->  "Extreme Size"
*			     OSCAN_VIDEO     -->  "Maximum Size"
*
*			OSCAN_VIDEO is not supported prior to V39. The value
*			0 corresponds to "Regular Size" under V38. Since this
*			value isn't supported by Intuition's SA_Overscan
*			tag, it was removed from ASL in V39. Asking for 0
*			under V39 will yield OSCAN_TEXT.
*
*			Default is OSCAN_TEXT. (V38)
*
*	ASLSM_InitialAutoScroll (BOOL) - Initial setting of the AutoScroll
*			cycle gadget (sm_AutoScroll). Default is TRUE. (V38)
*
*	ASLSM_InitialInfoOpened (BOOL) - Whether to open the property
*			information window automatically. Default is FALSE.
*			(V38)
*
*	ASLSM_InitialInfoLeftEdge (WORD) - Initial left edge of information
*			window. (V38)
*
*	ASLSM_InitialInfoTopEdge (WORD) - Initial top edge of information
*			window. (V38)
*
*	ASLSM_DoWidth (BOOL) - Set this tag to TRUE to cause the requester
*			to display the Width numeric gadget. Default is FALSE.
*			(V38)
*
*	ASLSM_DoHeight (BOOL) - Set this tag to TRUE to cause the requester
*			to display the Height numeric gadget. Default is FALSE.
*			(V38)
*
*	ASLSM_DoDepth (BOOL) - Set this tag to TRUE to cause the requester to
*			display the Colors slider gadget. Default is FALSE.
*			(V38)
*
*	ASLSM_DoOverscanType (BOOL) - Set this tag to TRUE to cause the
*			requester to display the Overscan Type cycle gadget.
*			Default is FALSE. (V38)
*
*	ASLSM_DoAutoScroll (BOOL) - Set this tag to TRUE to cause the requester
*			to display the AutoScroll checkbox gadget. Default is
*			FALSE. (V38)
*
*	ASLSM_PropertyFlags (ULONG) - A mode must have these property flags to
*			appear in the list of modes. Only the bits set in
*			ASLSM_PropertyMask are considered. See the description
*			of ASLSM_PropertyMask for more info. Default is
*			DIPF_IS_WB. (V38)
*
*	ASLSM_PropertyMask (ULONG) - A mask to apply to ASLSM_PropertyFlags
*			to determine which bits to consider. The use of these
*			two values is identical in concept to how Exec uses
*			the two flag paraneters in exec.library/SetSignal().
*			Default is DIPF_IS_WB. This is how this tag and the
*			ASLSM_PropertyFlags tag interact:
*
*			if ((displayInfo.PropertyFlags & propertyMask) ==
*			   (propertyFlags & propertyMask))
*			{
*			    \* Mode accepted */
*			}
*			else
*			{
*			    \* Mode rejected */
*			}
*
*			where "displayInfo" is a filled in DisplayInfo
*			structure for a given mode. (V38)
*
*	ASLSM_MinWidth (ULONG) - The minimum display width to let the user
*			choose. Default is 16. (V38)
*
*	ASLSM_MaxWidth (ULONG) - The maximum display width to let the user
*			choose. Default is 16368. (V38)
*
*	ASLSM_MinHeight (ULONG) - The minimum display height to let the user
*			choose. Default is 16. (V38)
*
*	ASLSM_MaxHeight (ULONG) - The maximum display height to let the user
*			choose. Default is 16384. (V38)
*
*	ASLSM_MinDepth (UWORD) - The minimum display depth to let the user
*			choose. Default is 1. (V38)
*
*	ASLSM_MaxDepth (UWORD) - The maximum display depth to let the user
*			choose. Default is 24. (V38)
*
*	ASLSM_FilterFunc (struct Hook *) - A function to call for each mode
*			encountered. If the function returns TRUE, the mode is
*			included in the file list, otherwise it is rejected
*			and not displayed. The function receives the following
*			parameters:
*				A0 - (struct Hook *)
*				A1 - (ULONG) mode id
*				A2 - (struct ScreenModeRequester *)
*			(V38)
*
*	ASLSM_CustomSMList (struct List *) - Custom list of modes to let
*			the user choose from. This is a list of DisplayNode
*			nodes, see <libraries/asl.h> for the definition. (V38)
*
*   RESULT
*	result - FALSE if the user cancelled the requester or if something
*		 prevented the requester from opening. If TRUE, values in the
*		 requester structure will be set depending on the particular
*		 type of requester.
*
*		 Under V38, If the return value is FALSE, you can look
*		 at the result from the dos.library/IoErr() function to
*		 determine whether the requester was cancelled or simply
*		 failed to open. If dos.library/IoErr() returns 0, then the
*		 requester was cancelled, any other value indicates a failure
*		 to open. Current possible failure codes are
*		 ERROR_NO_FREE_STORE which indicates there was not enough
*		 memory, and ERROR_NO_MORE_ENTRIES which indicates no modes
*		 were available for display in the screen mode requester
*		 (usually because the application filter hook filtered them
*		 all away)
*
*   WARNING
*	All ASL requester data structures are READ-ONLY and can only be
*	modified by using tags!
*
*   BUGS
*	Under V38 and V39, the filtering functions for the file requester
*	were only asked to filter drawer selections if the ASLFR_FilterDrawers
*	tag was set to TRUE. The compatible behavior, reestablished in V40,
*	is to always pass drawers for filtering.
*
*   SEE ALSO
*	AllocAslRequest(), FreeAslRequest()
*
******************************************************************************


******* asl.library/FreeAslRequest *******************************************
*
*   NAME
*	FreeAslRequest -- frees resources allocated by AllocAslRequest(). (V36)
*
*   SYNOPSIS
*	FreeAslRequest(requester);
*		       A0
*
*	VOID FreeAslRequest(APTR);
*
*   FUNCTION
*	FreeAslRequest() frees any system resources allocated by
*	AllocAslRequest(). Once a requester has been freed, it can no
*	longer be used with other calls to AslRequest().
*
*   INPUTS
*	requester - requester obtained from AllocAslRequest(), or NULL in
*		    which case this function does nothing.
*
*   SEE ALSO
*	AllocAslRequest()
*
******************************************************************************


******* asl.library/AllocFileRequest *****************************************
*
*   NAME
*	AllocFileRequest -- allocates an ASL FileRequester structure. (V36)
*
*   SYNOPSIS
*	requester = AllocFileRequest()
*	D0
*
*	struct FileRequester *AllocFileRequest(VOID);
*
*   NOTES
*	This function is obsolete, please use AllocAslRequest() instead.
*
*   SEE ALSO
*	AllocAslRequest()
*
******************************************************************************


******* asl.library/FreeFileRequest ******************************************
*
*   NAME
*	FreeFileRequest -- free file requester structure. (V36)
*
*   SYNOPSIS
*	FreeFileRequest(requester);
*			A0
*
*	VOID FreeFileRequest(struct FileRequester *);
*
*   NOTES
*	This function is obsolete, please use AllocAslRequest() instead.
*
*   SEE ALSO
*	FreeAslRequest()
*
******************************************************************************


******* asl.library/RequestFile **********************************************
*
*   NAME
*	RequestFile -- request user to select files. (V36)
*
*   SYNOPSIS
*	result = RequestFile(requester);
*	D0                   A0
*
*	BOOL RequestFile(struct FileRequester *);
*
*   NOTES
*	This function is obsolete, please use AslRequest() instead.
*
*   SEE ALSO
*	AslRequest()
*
******************************************************************************

