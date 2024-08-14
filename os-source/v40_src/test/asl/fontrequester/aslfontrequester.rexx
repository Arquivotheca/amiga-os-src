/*
 * ASL FONTREQUESTER TEST
 *
 * This arexx script will provide Product Assurance with a
 * semi-automated test of the ASL Font Requester, and its Programming
 * tags.
 *
 * [x] Will handle all parameters of fontrequester Test from JJSzucs.
 *
 * Changes Needed for Release as Version 1.00:
 *
 * [ ] Implement unimplemented parameters of fontrequester.
 *
 *
 * Parameters:
 *
 * [ ] EARLY           -   Use tags in AllocAslRequest(); if not specified,
 *                         tags are used in AslRequest()
 * [x] WINDOW          -   Open window
 * [x] SCREEN          -   Open screen
 * [x] PUBSCREENNAME   -   Create and open on public screen with name <name>
 * [ ] PRIVATEIDCMP    -   Allocate private IDCMP
 * [x] INTUIMSGFUNC    -   Test IntuiMessage function hook;
 *                         outputs hook, object, and message to debugging termal;
 *                         counts hook hits, bad hooks, bad objects, and bad messages
 * [x] SLEEPWINDOW     -   Sleep parent window
 * [x] USERDATA        -   Test user data (set to $C0EDBABE)
 * [x] TYPEFACE        -   Set type face to <type face>
 * [x] TYPESIZE        -   Set type size to <type size>
 * [x] BOLD            -   Bold type
 * [x] UNDERLINED      -   Underlined type
 * [x] ITALIC          -   Italic type
 * [x] LOCALE <locale> -   Use locale <locale>
 * [x] TITLETEXT <title text> - Use <title text> for title
 * [x] POSITIVETEXT <positive text> - Use <positive text> for positive text (default "OK")
 * [x] NEGATIVETEXT <negative text> - Use <negative text> for negative text (default "Cancel")
 * [x] INITIALLEFT <left>     - Initial left edge of <left>
 * [x] INITIALTOP <top>       -  Initial top edge of <top>
 * [x] INITIALWIDTH <width>   - Initial width of <width>
 * [x] INITIALHEIGHT <height> - Initial height of <height>
 * [x] INITIALNAME <name>  -   Initial typeface name
 * [x] INITIALSIZE <size>  -   Initial type size
 * [x] INITIALBOLD         -   Initial type bold
 * [x] INITIALUNDERLINED   -   Initial type underlined
 * [x] INITIALITALIC       -   Initial type italic
 * [ ] INITIALFLAGS <flags>    -   Initial type flags; see graphics/text.h for definitions
 * [x] INITIALFRONTPEN <pen>   -   Initial front pen
 * [x] INITIALBACKPEN <pen>    -   Initial back pen
 * [x] INITIALJAM1         -   Initial JAM1 drawing mode
 * [x] INITIALJAM2         -   Initial JAM2 drawing mode
 * [x] INITIALCOMPLEMENT   -   Initial COMPLEMENT drawing mode
 * [ ] INITIALINVERSVID    -   Initial INVERSVID drawing mode
 * [x] FLAGS <flags>           -   Decimal value for ASLFO_Flags; see libraries/asl.h for definitions
 * [x] DOFRONTPEN          -   Include front pen selection in font requester
 * [x] DOBACKPEN           -   Include back pen selection in font requester
 * [x] DOSTYLE             -   Include style selection in font requester
 * [x] DODRAWMODE          -   Include drawing mode selection in font requester
 * [x] FIXEDWIDTHONLY      -   Only select from fixed-width fonts
 * [x] MINHEIGHT           -   Minimum height of font to select
 * [x] MAXHEIGHT           -   Maximum height of font to select
 * [x] FILTERFUNC <pattern> -   Test filter function; only accepts fonts with name
 *                             matching <pattern>
 * [ ] HOOKFUNC            -   Test hook function
 *      Not yet implemented
 * [x] MODELIST            -   Test custom drawing mode list
 *
 * $VER: ASLFontRequester 0.04 12 Jun 1992
 *
 */


signal on break_c

/* Open the rexxsupport.library if it is not already open */

if ~show('L',"rexxsupport.library") then do
    if ~addlib("rexxsupport.library",0,-30,0) then do
        say "Unable to open rexxsupport.library"
        exit 20
        end
    end
/* Check the command line */

parse arg logfile
if logfile = "" | logfile = '?'then do
    say'USAGE: rx aslfontrequester [logfilename]'
    exit 0
end
/* Setup my stem variables */
fontparam.0 = 40
fontparam.1 = 'fontrequester'
fontparam.2 = 'fontrequester'
fontparam.3 = 'fontrequester window'
fontparam.4 = 'fontrequester screen'
fontparam.5 = 'fontrequester userdata'
fontparam.6 = 'fontrequester bold'
fontparam.7 = 'fontrequester underlined'
fontparam.8 = 'fontrequester italic'
fontparam.9 = 'fontrequester titletext MyRequester'
fontparam.10 = 'fontrequester positivetext Yay negativetext Nay'
fontparam.11 = 'fontrequester initialleft 0'
fontparam.12 = 'fontrequester initialtop 0'
fontparam.13 = 'fontrequester initialleft 0 initialtop 0 initialwidth 640 initialheight 200'
fontparam.14 = 'fontrequester initialleft 0 initialtop 0 initialwidth 640 initialheight 200'
fontparam.15 = 'fontrequester initialname ruby'
fontparam.16 = 'fontrequester initialname ruby initialsize 12'
fontparam.17 = 'fontrequester initialname ruby initialsize 12 initialbold'
fontparam.18 = 'fontrequester initialname ruby initialsize 12 initialunderlined'
fontparam.19 = 'fontrequester initialname ruby initialsize 12 initialitalic'
fontparam.20 = 'fontrequester window sleepwindow'
fontparam.21 = 'fontrequester pubscreenname MY_PUBLIC_SCREEN'
fontparam.22 = 'fontrequester typeface emerald.font typesize 11'
fontparam.23 = 'fontrequester locale deutsch'
fontparam.24 = 'fontrequester initialfrontpen 2'
fontparam.25 = 'fontrequester initialbackpen 2'
fontparam.26 = 'fontrequester dofrontpen'
fontparam.27 = 'fontrequester dobackpen'
fontparam.28 = 'fontrequester dostyle'
fontparam.29 = 'fontrequester dodrawmode'
fontparam.30 = 'fontrequester dofrontpen dobackpen dostyle dodrawmode'
fontparam.31 = 'fontrequester fixedwidthonly'
fontparam.32 = 'fontrequester minheight 10'
fontparam.33 = 'fontrequester maxheight 8'
fontparam.34 = 'fontrequester filterfunc t#?'
fontparam.35 = 'fontrequester dodrawmode modelist'
fontparam.36 = 'fontrequester dodrawmode initialjam1'
fontparam.37 = 'fontrequester dodrawmode initialjam2'
fontparam.38 = 'fontrequester dodrawmode initialcomplement'
fontparam.39 = 'fontrequester flags 1'
fontparam.40 = 'fontrequester screen window intuimsgfunc'

fontinst.1 = 'Select Cancel on File Requester'
fontinst.2 = 'Select the font sapphire, followed by OK'
fontinst.3 = 'A window should open with the File Requester if not, select Cancel'
fontinst.4 = 'A screen should open with the File Requester if not, select Cancel'
fontinst.5 = 'UserData test, Select OK'
fontinst.6 = 'If the text is not bold, select Cancel'
fontinst.7 = 'If the text is not underlined, select Cancel'
fontinst.8 = 'If the text is not italic, select Cancel'
fontinst.9 = 'If the Title Text is not MyRequester, select Cancel'
fontinst.10 = 'The positive text should be Yay, the negative text Nay, if not select the negative gadget.'
fontinst.11 = 'InitialLeft test. Do not move the file requester, select OK'
fontinst.12 = 'InitialTop test. Do not move the file requester, select OK'
fontinst.13 = 'InitialWidth test. Do not resize the file requester, select OK.'
fontinst.14 = 'InitialHeight test, Do not resize the file requester, select OK.'
fontinst.15 = 'InitialName test. Select OK'
fontinst.16 = 'InitialSize test. Select OK'
fontinst.17 = 'InitialBold Test. Select OK'
fontinst.18 = 'InitialUnderlined Test. Select OK'
fontinst.19 = 'InitialItalic Test. Select OK'
fontinst.20 = 'Sleepwindow Test, click in the Window that is with the Requester if there is not a busy pointer, select Cancel.'
fontinst.21 = 'PublicScreenTest. If the requester does not open on the screen called MY_PUBLIC_SCREEN, select Cancel'
fontinst.22 = 'If the font in the requester gadgets is not emerald, select Cancel'
fontinst.23 = 'If the text in the requester gadgets is not in German, select Cancel'
fontinst.24 = 'InitialFrontPen Test. Select OK'
fontinst.25 = 'InitialBackPen Test. Select OK'
fontinst.26 = 'DoFrontPen Test. If the FrontPen gadget is not present, select Cancel'
fontinst.27 = 'DoBackPen Test. If the BackPen gadget is not present, select Cancel'
fontinst.28 = 'DoStyle Test. If the Style gadget is not present, select Cancel'
fontinst.29 = 'DoDrawMode Test. If the DrawMode gadget is not present, select Cancel'
fontinst.30 = 'DoFontPen, DoBackPen, DoStyle, DoDrawMode Test. If all four gadgets are not present, select Cancel'
fontinst.31 = 'FixedWidth Font Test. If there are proportional fonts present in the list view, select Cancel'
fontinst.32 = 'MinHeight Test. If there are font sizes less than 10, select Cancel'
fontinst.33 = 'MaxHeight Test. If there are font sizes greater than 8, select Cancel'
fontinst.34 = 'FilterFunc Test. All fonts listed should start with the letter T, if not select Cancel'
fontinst.35 = 'ModeList Test. The Drawmode gadget will have JAM1, JAM2, and Complement. If not select Cancel'
fontinst.36 = 'InitialJam1 Test. The Drawmode gadget should be set to Text, if not select Cancel'
fontinst.37 = 'InitialJam2 Test. The Drawmode gadget should be set to Text+Field, if not select Cancel'
fontinst.38 = 'InitialComplement Test. The Drawmode gadget should be set to Complement, if not select Cancel'
fontinst.39 = 'Flags Test.The front pen gadget should be present in the requester, if not, select Cancel'
fontinst.40 = 'IntuiMsgFunc Test. Bring the attached window to front and press the Close Gadget three (3) times'

fonterror.1 = 'User did not Select Cancel'
fonterror.2 = 'User did not select the font sapphire and OK'
fonterror.3 = 'No window'
fonterror.4 = 'No Screen'
fonterror.5 = 'User Data incorrect'
fonterror.6 = 'Bold tag failed'
fonterror.7 = 'Italic tag failed'
fonterror.8 = 'Underlined tag failed'
fonterror.9 = 'TitleText tag failed'
fonterror.10 = 'Positive or Negative text tag failed'
fonterror.11 = 'Initialleft tag failed'
fonterror.12 = 'Initialtop tag failed'
fonterror.13 = 'InitialWidth tag Failed'
fonterror.14 = 'InitialHeight Tag failed'
fonterror.15 = 'InitialName tag failed'
fonterror.16 = 'InitialSize tag failed'
fonterror.17 = 'InitialBold tag failed'
fonterror.18 = 'InitialUnderlined tag failed'
fonterror.19 = 'InitialItalic tag failed.'
fonterror.20 = 'WindowSleep Tag failed'
fonterror.21 = 'PubNameScreen Tag failed'
fonterror.22 = 'Typeface or Typesize tag failed'
fonterror.23 = 'Locale Tag Failed'
fonterror.24 = 'InitialFrontPen tag failed'
fonterror.25 = 'InitialBackPen tag failed'
fonterror.26 = 'DoFrontPen tag failed'
fonterror.27 = 'DoBackPen tag failed'
fonterror.28 = 'DoStyle tag failed'
fonterror.29 = 'DoDrawMode tag failed'
fonterror.30 = 'DoFrontPen, DoBackPen, DoStyle or DoDrawMode tag failed'
fonterror.31 = 'FixedWidthOnly tag failed'
fonterror.32 = 'MinHeight tag failed'
fonterror.33 = 'MaxHeight tag failed'
fonterror.34 = 'FilterFunc tag failed'
fonterror.35 = 'ModeList tag failed'
fonterror.36 = 'InitialJam1 tag failed'
fonterror.37 = 'InitialJam2 tag failed'
fonterror.38 = 'InitialComplement tag failed'
fonterror.39 = 'Flags tag failed'
fonterror.40 = 'IntuiMsgFunc tag failed'

fontresult.1 = 'Font requester cancelled by user'
fontresult.2 = 'fo_Attr.ta_Name=sapphire.font'
fontresult.3 = 'AslRequest() return code TRUE'
fontresult.4 = 'AslRequest() return code TRUE'
fontresult.5 = 'fo_UserData=$C0EDBABE'
fontresult.6 = 'AslRequest() return code TRUE'
fontresult.7 = 'AslRequest() return code TRUE'
fontresult.8 = 'AslRequest() return code TRUE'
fontresult.9 = 'AslRequest() return code TRUE'
fontresult.10 = 'AslRequest() return code TRUE'
fontresult.11 = 'fo_LeftEdge=0'
fontresult.12 = 'fo_TopEdge=0'
fontresult.13 = 'fo_Width=640'
fontresult.14 = 'fo_Height=200'
fontresult.15 = 'fo_Attr.ta_Name=ruby.font'
fontresult.16 = 'fo_Attr.ta_YSize=12'
fontresult.17 = 'fo_Attr.ta_Style=$02'
fontresult.18 = 'fo_Attr.ta_Style=$01'
fontresult.19 = 'fo_Attr.ta_Style=$04'
fontresult.20 = 'AslRequest() return code TRUE'
fontresult.21 = 'AslRequest() return code TRUE'
fontresult.22 = 'AslRequest() return code TRUE'
fontresult.23 = 'AslRequest() return code TRUE'
fontresult.24 = 'fo_FrontPen=2'
fontresult.25 = 'fo_BackPen=2'
fontresult.26 = 'AslRequest() return code TRUE'
fontresult.27 = 'AslRequest() return code TRUE'
fontresult.28 = 'AslRequest() return code TRUE'
fontresult.29 = 'AslRequest() return code TRUE'
fontresult.30 = 'AslRequest() return code TRUE'
fontresult.31 = 'AslRequest() return code TRUE'
fontresult.32 = 'AslRequest() return code TRUE'
fontresult.33 = 'AslRequest() return code TRUE'
fontresult.34 = 'AslRequest() return code TRUE'
fontresult.35 = 'AslRequest() return code TRUE'
fontresult.36 = 'AslRequest() return code TRUE'
fontresult.37 = 'AslRequest() return code TRUE'
fontresult.38 = 'AslRequest() return code TRUE'
fontresult.39 = 'AslRequest() return code TRUE'
fontresult.40 = 'IntuiMsg hook hits: 3'

/* Start the Program */
say 'ASL File Requester Test'
say 'written By Greg Givler, with test code written by John. J. Szucs'
say ' '

/* Start the process */
say 'DEBUG!! all variables are assigned, if you wish to start trace start trace then'
say 'Press Return when told by the program. END DEBUG!!'
say ' '
say 'The first part of the test you will be told what to press and what files to select.'

call writech stdout,'Press <RETURN> '
parse pull cmd

/*
 * The plan here is to build an engine that will handle all the needed conditions.
 * I will create a log file that will allow the user to review any failures.
 *
 */
/* Open the log file */

open('log',logfile,'W')

do i = 1 to fontparam.0
    /* This next line tells the test what to do and what to expect. */

    say fontinst.i

    /* PainIO gets output from the device, the reason for this is that
     * the AmigaShell does not have the right packets to work with execio
     * If it did then PainIO would not be necessary
     */

    myrc = PainIO(fontparam.i,'fontreq')

    /* This sets fontcond to the proper line in the output display. */

    fontcond.1 = fontreq.2
    fontcond.2 = fontreq.2
    fontcond.3 = fontreq.1
    fontcond.4 = fontreq.1
    fontcond.5 = fontreq.9
    fontcond.6 = fontreq.1
    fontcond.7 = fontreq.1
    fontcond.8 = fontreq.1
    fontcond.9 = fontreq.1
    fontcond.10 = fontreq.1
    fontcond.11 = fontreq.10
    fontcond.12 = fontreq.11
    fontcond.13 = fontreq.12
    fontcond.14 = fontreq.13
    fontcond.15 = fontreq.2
    fontcond.16 = fontreq.3
    fontcond.17 = fontreq.4
    fontcond.18 = fontreq.4
    fontcond.19 = fontreq.4
    fontcond.20 = fontreq.1
    fontcond.21 = fontreq.1
    fontcond.22 = fontreq.1
    fontcond.23 = fontreq.1
    fontcond.24 = fontreq.6
    fontcond.25 = fontreq.7
    fontcond.26 = fontreq.1
    fontcond.27 = fontreq.1
    fontcond.28 = fontreq.1
    fontcond.29 = fontreq.1
    fontcond.30 = fontreq.1
    fontcond.31 = fontreq.1
    fontcond.32 = fontreq.1
    fontcond.33 = fontreq.1
    fontcond.34 = fontreq.1
    fontcond.35 = fontreq.1
    fontcond.36 = fontreq.1
    fontcond.37 = fontreq.1
    fontcond.38 = fontreq.1
    fontcond.39 = fontreq.1
    fontcond.40 = fontreq.13

    /* DEBUG!! The line below should be uncommented for Debugging purposes */
    /* say 'fontcondition = '||fontcond.i||' fontresult = '||fontresult.i */
    /* END DEBUG!! */

    if i = 40 then do
        say 'User Pressed Close Gadget '||word(fontreq.10, 4)||' times.'
        fontresult.40 = 'IntuiMsg hook hits: '||word(fontreq.10,4)
    end

    if fontcond.i ~= fontresult.i then do
        say fonterror.i
        writeln('log',fonterror.i)
    end
end

close('log')
/* Exit program */

exit 0


PainIO:
/* This function was written by John Lockhart, thanks John this will
make my life much easier */

  parse arg PainCmd,PainStem               /* parse arg preserves lowercase */

/*  say "Paincmd = '"||PainCmd||"' PainStem = '"||PainStem||"'" */
  PainStem = strip(PainStem)   /* no #@$%#^ spaces!! */
  if (pos('>',PainCmd) ~= 0) then do        /* fatal error trap */
     call writeln STDOUT,"PainIO: Cannot redirect command" PainCmd
     call writeln STDOUT,"No extra >'s allowed!  Exiting program, sorry."
     exit(20)
     end
  xpain.1 = wordindex(PainCmd,2)                   /* get insert point */
  if xpain.1 = 0 then do
    xpain.1 = length(PainCmd)              /* get correct insert point */
    xpain.xpad = ' '
    end
  else do
    xpain.1 = xpain.1 - 1
    xpain.xpad = ''
    end
  xpain.2 = time('s')     /* get reasonably unique number for temp file */
  PainCmd = insert(xpain.xpad||'>t:painio.'||xpain.2||' ',PainCmd,xpain.1)
  xpain.f = 't:painio.'||xpain.2      /* FILENAME */
  options results   /* if not already on */
  xpain.3 = trace()  /* previous trace mode */
  call trace('off')  /* turn OFF to prevent errors from stopping this program */
  address command PainCmd
  xpain.r = rc              /* return code */
  call trace(xpain.3)  /* restore previous trace mode */
  if ~open('painfile',xpain.f,'r') then do
     call writeln stdout, "PainIO: cannot open redirected file.  Exiting..."
     exit(20)
     end
  xpain.xndx = 0   /* index */
  do while ~eof('painfile')
     xpain.xndx = xpain.xndx + 1
     xpain.icmd = PainStem||"."||xpain.xndx "= readln('painfile')"
     interpret xpain.icmd
     end
  /* OK, what we probably want is xpain.xndx - 1, which avoids EOF */
  xpain.icmd = PainStem||".0 = xpain.xndx - 1"
  interpret xpain.icmd

  /* we're done now --- cleanup time */
  call close('painfile')                   /* close and delete temp file */
  address command 'delete quiet' xpain.f
  PainCmd = xpain.r                        /* save return code       */
  drop xpain.                              /* drop all that space    */
return PainCmd                             /* return RC from command */

break_c:
    say 'Detected CTRL-C exiting...'
    exit 20

