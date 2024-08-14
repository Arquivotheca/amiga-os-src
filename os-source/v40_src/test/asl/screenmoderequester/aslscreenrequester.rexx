/*
 * ASLSCREENMODE TEST
 *
 * This arexx script will provide Product Assurance with a
 * semi-automated test of the ASL Screenmode Requester, and its
 * programable tags.
 *
 * [ ] EARLY               -   Use tags in AllocAslRequest(); if not specified,
 *                         tags are used in AslRequest()
 * [x] WINDOW              -   Open window
 * [x] SCREEN              -   Open screen
 * [x] PUBSCREENNAME <name>  -   Open on public screen with name <name>
 * [ ] PRIVATEIDCMP        -   Allocate private IDCMP
 * [x] INTUIMSGFUNC        -   Test IntuiMessage function hook
 * [x] SLEEPWINDOW         -   Sleep parent window
 * [x] USERDATA            -   Test user data (set to $C0EDBABE)
 * [x] TYPEFACE <type face> - Set type face to <type face>
 * [x] TYPESIZE <type size> -   Set type size to <type size>
 * [x] BOLD                -   Bold type
 * [x] UNDERLINED          -   Underlined type
 * [x] ITALIC              -   Italic type
 * [x] LOCALE <locale>     -   Use locale <locale>
 * [x] TITLETEXT <title text> - Use <title text> for title
 * [x] POSITIVETEXT <positive text> - Use <positive text> for positive text (default "OK")
 * [x] NEGATIVETEXT <negative text> - Use <negative text> for negative text (default "Cancel")
 * [x] INITIALLEFT <left>     - Initial left edge of <left>
 * [x] INITIALTOP <top>       -  Initial top edge of <top>
 * [x] INITIALWIDTH <width>   - Initial width of <width>
 * [x] INITIALHEIGHT <height> - Initial height of <height>
 * [x] INITIALDISPLAYID <display ID> - Initial display ID (decimal); from graphics/displayinfo.h
 * [x] INITIALDISPLAYWIDTH <width> - Initial display width
 * [x] INITIALDISPLAYHEIGHT <heigh> - Initial display height
 * [x] INITIALDISPLAYDEPTH <depth> - Initial display depth
 * [x] INITIALOVERSCANTYPE <overscan> - Initial overscan type (decimal); from graphics/displayinfo.h
 * [x] INITIALAUTOSCROLL <auto scroll> - Initial autoscroll
 * [x] INITIALINFOOPENED   - Open display mode information window at start-up
 * [x] INITIALINFOLEFT <left> - Left edge of display mode information window
 * [x] INITIALINFOTOP <top>- Top edge of display mode information window
 * [x] DOWIDTH             -   Do width
 * [x] DOHEIGHT            -   Do height
 * [x] DODEPTH             -   Do depth
 * [x] DOOVERSCANTYPE      -   Do overscan type selection
 * [x] DOAUTOSCROLL        -   Do auto scroll selection
 * [x] PROPERTYFLAGS <property flags> - Property flags (decimal); from graphics/displayinfo.h
 * [x] PROPERTYMASK <property mask> - Property mask (decimal); from graphics/displayinfo.h
 * [x] MINWIDTH <width>    -   Minimum width
 * [x] MAXWIDTH <width>    -   Maximum width
 * [x] MINHEIGHT <height>  -   Minimum height
 * [x] MAXHEIGHT <height>  -   Maximum height
 * [x] MINDEPTH <depth>    -   Minimum depth
 * [x] MAXDEPTH <depth>    -   Maximum depth
 * [x] FILTERFUNC          -   Test filter function
 * [x] CUSTOMSMLIST        -   Test custom screen mode list
 *
 * $VER: ASLScreenRequester 0.05 12 Jun 1992
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
if logfile = "" | logfile = '?' then do
    say 'USAGE: rx aslscreenrequester [logfilename]'
    exit 0
end

/* Setup my stem variables */
screenparam.0 = 43
screenparam.1 = 'screenmoderequester'
screenparam.2 = 'screenmoderequester'
screenparam.3 = 'screenmoderequester window'
screenparam.4 = 'screenmoderequester screen'
screenparam.5 = 'screenmoderequester userdata'
screenparam.6 = 'screenmoderequester bold'
screenparam.7 = 'screenmoderequester underlined'
screenparam.8 = 'screenmoderequester italic'
screenparam.9 = 'screenmoderequester titletext MyRequester'
screenparam.10 = 'screenmoderequester positivetext Yay negativetext Nay'
screenparam.11 = 'screenmoderequester initialleft 0'
screenparam.12 = 'screenmoderequester initialtop 0'
screenparam.13 = 'screenmoderequester initialleft 0 initialtop 0 initialwidth 640 initialheight 200'
screenparam.14 = 'screenmoderequester initialleft 0 initialtop 0 initialwidth 640 initialheight 200'
screenparam.15 = 'screenmoderequester initialdisplayid 32768'
screenparam.16 = 'screenmoderequester initialdisplaywidth 1280'
screenparam.17 = 'screenmoderequester initialdisplayheight 400'
screenparam.18 = 'screenmoderequester initialdisplaydepth 1'
screenparam.19 = 'screenmoderequester initialoverscantype 3'
screenparam.20 = 'screenmoderequester initialautoscroll 0 doautoscroll'
screenparam.21 = 'screenmoderequester initialinfoopened'
screenparam.22 = 'screenmoderequester initialinfoopened initialinfoleft 0'
screenparam.23 = 'screenmoderequester initialinfoopened initialinfotop 0'
screenparam.24 = 'screenmoderequester window sleepwindow'
screenparam.25 = 'screenmoderequester pubscreenname MY_PUBLIC_SCREEN'
screenparam.26 = 'screenmoderequester locale deutsch'
screenparam.27 = 'screenmoderequester typeface helvetica.font typesize 11'
screenparam.28 = 'screenmoderequester dowidth'
screenparam.29 = 'screenmoderequester doheight'
screenparam.30 = 'screenmoderequester dodepth'
screenparam.31 = 'screenmoderequester dooverscantype'
screenparam.32 = 'screenmoderequester doautoscroll'
screenparam.33 = 'screenmoderequester dowidth doheight dodepth dooverscantype doautoscroll'
screenparam.34 = 'screenmoderequester dowidth minwidth 640'
screenparam.35 = 'screenmoderequester dowidth maxwidth 640'
screenparam.36 = 'screenmoderequester doheight minheight 400'
screenparam.37 = 'screenmoderequester doheight maxheight 400'
screenparam.38 = 'screenmoderequester dodepth mindepth 4'
screenparam.39 = 'screenmoderequester dodepth maxdepth 4'
screenparam.40 = 'screenmoderequester filterfunc ntsc#?'
screenparam.41 = 'screenmoderequester customsmlist initialinfoopened initialinfotop 0'
screenparam.42 = 'screenmoderequester propertyflags 1 propertymask 1'
screenparam.43 = 'screenmoderequester window intuimsg'

screeninst.1 = 'Select Cancel on Screen Mode Requester'
screeninst.2 = 'Select NTSC:High Res Laced, then select OK'
screeninst.3 = 'A window should open with the Screen Mode Requester if not, select Cancel'
screeninst.4 = 'A screen should open with the Screen Mode Requester if not, select Cancel'
screeninst.5 = 'UserData test, Select OK'
screeninst.6 = 'If the text is not bold, select Cancel'
screeninst.7 = 'If the text is not underlined, select Cancel'
screeninst.8 = 'If the text is not italic, select Cancel'
screeninst.9 = 'If the Title Text is not MyRequester, select Cancel'
screeninst.10 = 'The positive text should be Yay, the negative text Nay, if not select the negative gadget.'
screeninst.11 = 'InitialLeft test. Do not move the Screen Mode requester, select OK'
screeninst.12 = 'InitialTop test. Do not move the Screen Mode requester, select OK'
screeninst.13 = 'InitialWidth test. Do not resize the Screen Mode requester, select OK.'
screeninst.14 = 'InitialHeight test, Do not resize the Screen Mode requester, select OK.'
screeninst.15 = 'InitialDisplayID test. Select OK'
screeninst.16 = 'InitialDisplayWidth test. Select OK'
screeninst.17 = 'InitialDisplayHeight Test. Select OK'
screeninst.18 = 'InitialDisplayDepth Test. Select OK'
screeninst.19 = 'InitialOverScanType Test. Select OK'
screeninst.20 = 'InitialAutoScroll Test, Select OK'
screeninst.21 = 'InitialInfoOpened Test, Select OK'
screeninst.22 = 'InitialInfoLeft Test, Select OK'
screeninst.23 = 'InitialInfoTop Test, Select OK'
screeninst.24 = 'Sleepwindow Test, click in the Window that is with the Requester if there is not a busy pointer, select Cancel.'
screeninst.25 = 'PublicScreenTest. If the requester does not open on the screen called MY_PUBLIC_SCREEN, select Cancel'
screeninst.26 = 'If the text in the requester gadgets is not in German, select Cancel'
screeninst.27 = 'TypeFace, Typesize test. The requester font should be Helvetica 11'
screeninst.28 = 'DoWidth Test. If the Width gadget is not present, select Cancel'
screeninst.29 = 'DoHeight Test. If the Height gadget is not present, select Cancel'
screeninst.30 = 'DoDepth Test. If the Depth gadget is not present, select Cancel'
screeninst.31 = 'DoOverScanType test. If the OverScanType gadget is not present, select Cancel'
screeninst.32 = 'DoAutoScroll test. If the AutoScroll gadget is not present, select Cancel'
screeninst.33 = 'DoWidth, DoHeight, DoDepth, DoOverScanType, and DoAutoScroll Test. If all five gadgets are not present select Cancel'
screeninst.34 = 'MinWidth Test. If the gadget will accept a width less than 640, select Cancel'
screeninst.35 = 'MaxWidth Test. If the gadget will accept a width greater than 640, select Cancel'
screeninst.36 = 'MinHeight Test. If the gadget will accept a height less that 400, select Cancel'
screeninst.37 = 'MaxHeight Test. If the gadget will accept a height greater than 400, select Cancel'
screeninst.38 = 'MinDepth Test. If the gadget will accept a depth less that 4, select Cancel'
screeninst.39 = 'MaxDepth Test. If the gadget will accept a depth greater than 4, select Cancel'
screeninst.40 = 'Filter Func Test. If there a monitors that are not NTSC then, Select Cancel'
screeninst.41 = 'CustomSMList test. Select Super-Duper Extra-High-Res from listview, select OK'
screeninst.42 = 'PropertyFlags, PropertyMask Test. All Interlaced Modes should be available. If not select Cancel'
screeninst.43 = 'IntuiMsg Test. Bring the attached window to the front and press the Close Gadget three (3) times. Then select OK'

screenerror.1 = 'User did not Select Cancel'
screenerror.2 = 'User did not select NTSC:High Res Laced and OK'
screenerror.3 = 'No window'
screenerror.4 = 'No Screen'
screenerror.5 = 'User Data incorrect'
screenerror.6 = 'Bold tag failed'
screenerror.7 = 'Underlined tag failed'
screenerror.8 = 'Italic tag failed'
screenerror.9 = 'TitleText tag failed'
screenerror.10 = 'Positive or Negative text tag failed'
screenerror.11 = 'Initialleft tag failed'
screenerror.12 = 'Initialtop tag failed'
screenerror.13 = 'InitialWidth tag Failed'
screenerror.14 = 'InitialHeight Tag failed'
screenerror.15 = 'InitialDisplayID tag failed'
screenerror.16 = 'InitialDisplayWidth tag failed'
screenerror.17 = 'InitialDisplayHeight tag failed'
screenerror.18 = 'InitialDisplayDepth tag failed'
screenerror.19 = 'InitialOverScanType tag failed'
screenerror.20 = 'InitialAutoScroll tag failed'
screenerror.21 = 'InitialInfoOpened tag failed'
screenerror.22 = 'InitialInfoLeft tag failed'
screenerror.23 = 'InitialInfoTop tag failed'
screenerror.24 = 'WindowSleep Tag failed'
screenerror.25 = 'PubNameScreen Tag failed'
screenerror.26 = 'Locale Tag Failed'
screenerror.27 = 'Typeface or Typesize tag failed'
screenerror.28 = 'DoWidth tag failed'
screenerror.29 = 'DoHeight Tag Failed'
screenerror.30 = 'DoDepth Tag Failed'
screenerror.31 = 'DoOverScanType tag failed'
screenerror.32 = 'DoAutoScroll tag failed'
screenerror.33 = 'DoAll tags failed'
screenerror.34 = 'MinWidth tag failed'
screenerror.35 = 'MaxWidth tag failed'
screenerror.36 = 'MinHeight tag failed'
screenerror.37 = 'MaxHeight tag failed'
screenerror.38 = 'MinDepth tag failed'
screenerror.39 = 'MaxDepth tag failed'
screenerror.40 = 'FilterFunc tag failed'
screenerror.41 = 'CustomSMList tag failed'
screenerror.42 = 'ProperyFlags or PropertyMask tag Failed'
screenerror.43 = 'IntuiMsg tag Failure'

screenresult.1 = 'Screen mode requester cancelled by user'
screenresult.2 = 'sm_DisplayID = $00019004'
screenresult.3 = 'AslRequest() return code TRUE'
screenresult.4 = 'AslRequest() return code TRUE'
screenresult.5 = 'sm_UserData = $C0EDBABE'
screenresult.6 = 'AslRequest() return code TRUE'
screenresult.7 = 'AslRequest() return code TRUE'
screenresult.8 = 'AslRequest() return code TRUE'
screenresult.9 = 'AslRequest() return code TRUE'
screenresult.10 = 'AslRequest() return code TRUE'
screenresult.11 = 'sm_LeftEdge = 0'
screenresult.12 = 'sm_TopEdge = 0'
screenresult.13 = 'sm_Width = 640'
screenresult.14 = 'sm_Height = 200'
screenresult.15 = 'sm_DisplayID = $00008000'
screenresult.16 = 'sm_DisplayWidth = 1280'
screenresult.17 = 'sm_DisplayHeight = 400'
screenresult.18 = 'sm_DisplayDepth = 1'
screenresult.19 = 'sm_OverscanType = $0003'
screenresult.20 = 'sm_AutoScroll = FALSE'
screenresult.21 = 'sm_InfoOpened = TRUE'
screenresult.22 = 'sm_InfoLeftEdge = 0'
screenresult.23 = 'sm_InfoTopEdge = 0'
screenresult.24 = 'AslRequest() return code TRUE'
screenresult.25 = 'AslRequest() return code TRUE'
screenresult.26 = 'AslRequest() return code TRUE'
screenresult.27 = 'AslRequest() return code TRUE'
screenresult.28 = 'AslRequest() return code TRUE'
screenresult.29 = 'AslRequest() return code TRUE'
screenresult.30 = 'AslRequest() return code TRUE'
screenresult.31 = 'AslRequest() return code TRUE'
screenresult.32 = 'AslRequest() return code TRUE'
screenresult.33 = 'AslRequest() return code TRUE'
screenresult.34 = 'AslRequest() return code TRUE'
screenresult.35 = 'AslRequest() return code TRUE'
screenresult.36 = 'AslRequest() return code TRUE'
screenresult.37 = 'AslRequest() return code TRUE'
screenresult.38 = 'AslRequest() return code TRUE'
screenresult.39 = 'AslRequest() return code TRUE'
screenresult.40 = 'AslRequest() return code TRUE'
screenresult.41 = 'sm_DisplayID = $FFFFBABF'
screenresult.42 = 'AslRequest() return code TRUE'
screenresult.43 = 'IntuiMsg hook hits: 3'

/* Start the Program */
say 'ASL Screen Mode Requester Test'
say 'written By Greg Givler, with test code written by John. J. Szucs'
say ' '

/* Start the process */
say  'DEBUG!! all variables are assigned, if you wish to start trace start trace then'
say 'Press Return when told by the program. END DEBUG!!'
say ' '
say 'The first part of the test you will be told what to press and what Screen Modes to select.'

call writech stdout,'Press <RETURN> '
parse pull cmd

/*
 * The plan here is to build an engine that will handle all the needed conditions.
 * I will create a log Screen Mode that will allow the user to review any failures.
 *
 */
/* Open the log Screen Mode */

open('log',logfile,'W')

do i = 1 to screenparam.0
    /* This next line tells the test what to do and what to expect. */

    say screeninst.i

    /* PainIO gets output from the device, the reason for this is that
     * the AmigaShell does not have the right packets to work with execio
     * If it did then PainIO would not be necessary
     */

    myrc = PainIO(screenparam.i,'screenreq')

    /* This sets screencond to the proper line in the output display. */

    screencond.1 = screenreq.2
    screencond.2 = screenreq.2
    screencond.3 = screenreq.1
    screencond.4 = screenreq.1
    screencond.5 = screenreq.19
    screencond.6 = screenreq.1
    screencond.7 = screenreq.1
    screencond.8 = screenreq.1
    screencond.9 = screenreq.1
    screencond.10 = screenreq.1
    screencond.11 = screenreq.10
    screencond.12 = screenreq.11
    screencond.13 = screenreq.12
    screencond.14 = screenreq.13
    screencond.15 = screenreq.2
    screencond.16 = screenreq.3
    screencond.17 = screenreq.4
    screencond.18 = screenreq.5
    screencond.19 = screenreq.6
    screencond.20 = screenreq.7
    screencond.21 = screenreq.14
    screencond.22 = screenreq.15
    screencond.23 = screenreq.16
    screencond.24 = screenreq.1
    screencond.25 = screenreq.1
    screencond.26 = screenreq.1
    screencond.27 = screenreq.1
    screencond.28 = screenreq.1
    screencond.29 = screenreq.1
    screencond.30 = screenreq.1
    screencond.31 = screenreq.1
    screencond.32 = screenreq.1
    screencond.33 = screenreq.1
    screencond.34 = screenreq.1
    screencond.35 = screenreq.1
    screencond.36 = screenreq.1
    screencond.37 = screenreq.1
    screencond.38 = screenreq.1
    screencond.39 = screenreq.1
    screencond.40 = screenreq.1
    screencond.41 = screenreq.2
    screencond.42 = screenreq.1
    screencond.43 = screenreq.20

    /* DEBUG!! The line below should be uncommented for Debugging purposes */
    /* say 'screencondition = '||screencond.i||' screenresult = '||screenresult.i */
    /* END DEBUG!! */

    if i = 43 then do
        say 'User Pressed Close Gadget '||word(screenreq.20, 4)||' times.'
        screenresult.43 = 'IntuiMsg hook hits: '||word(screenreq.20,4)
    end

    if screencond.i ~= screenresult.i then do
        say screenerror.i
        writeln('log',screenerror.i)
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

