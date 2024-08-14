/*
 * ASL FILEREQUESTER TEST
 *
 * This arexx script will set up an automated test of the ASL File
 * Requester.
 *
 * Features:
 *
 * [ ] Setup a Test Directory with known files
 * [x] Will handle all parameters of FileRequester Test from JJSzucs.
 *
 * Changes Needed for Release as Version 1.00:
 *
 * [ ] Setup a Disk that contains the Directories and Files and Subdirectories
 *     needed for the test.
 *
 * [ ] Change all filenames that point to files unique to the current directory
 * [ ] Change all patterns to reflect the New disk Structure
 * [ ]
 *
 * Parameters:
 *
 * [ ] EARLY           -   Use tags in AllocAslRequest(); if not specified,
 *                         tags are used in AslRequest()
 * [x] WINDOW          -   Open window
 * [x] SCREEN          -   Open screen
 * [x] PUBSCREENNAME   -   Open on public screen with name <name>
 * [ ] PRIVATEIDCMP    -   Allocate private IDCMP
 *      There is no easyway to test this through a script so it is not supported.
 * [x] INTUIMSGFUNC    -   Test IntuiMessage function hook
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
 * [x] INITIALFILE <file>     - Initial file selection of <file>
 * [x] INITIALDRAWER <drawer>  - Initial drawer selection of <drawer>
 * [x] INITIALPATTERN <pattern> - Initial pattern of <pattern>
 * [x] FLAGS1 <flags 1> -  Value for ASLFR_Flags1 tag;
 *                         see asl.library/AslRequest() AutoDocs for details
 * [x] FLAGS2 <flags 2> -  Value for ASLFR_Flags2 tag;
 *                         see asl.library/AslRequest() AutoDocs for details
 * [x] DOSAVEMODE      -   Save mode
 * [x] DOMULTISELECT   -   Multi-select support
 * [x] DOPATTERNS      -   Pattern gadget support
 * [x] DRAWERSONLY     -   Drawers only
 * [x] FILTERFUNC      -   Test filter function hook
 * [x] REJECTICONS     -   Reject icons
 * [x] REJECTPATTERN   -   Reject files matching <reject pattern>
 * [x] ACCEPTPATTERN   -   Accept files matching <accept pattern>
 * [x] FILTERDRAWERS   -   Process reject and accept patterns and user-specified
 *                         pattern for drawers
 *
 * $VER: ASLFileRequester 0.20 30 Jun 1992
 *
 */
/* Setup break C handling */

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
    say 'USAGE: rx aslfilerequester [logfilename]'
    exit 0
end
/* Setup my stem variables */
fileparam.0 = 33
fileparam.1 = 'filerequester'
fileparam.2 = 'filerequester'
fileparam.3 = 'filerequester window'
fileparam.4 = 'filerequester screen'
fileparam.5 = 'filerequester userdata'
fileparam.6 = 'filerequester bold'
fileparam.7 = 'filerequester underlined'
fileparam.8 = 'filerequester italic'
fileparam.9 = 'filerequester titletext MyRequester'
fileparam.10 = 'filerequester positivetext Yay negativetext Nay'
fileparam.11 = 'filerequester initialleft 0'
fileparam.12 = 'filerequester initialtop 0'
fileparam.13 = 'filerequester initialleft 0 initialtop 0 initialwidth 640 initialheight 200'
fileparam.14 = 'filerequester initialleft 0 initialtop 0 initialwidth 640 initialheight 200'
fileparam.15 = 'filerequester initialfile filerequester'
fileparam.16 = 'filerequester initialdrawer sys:'
fileparam.17 = 'filerequester initialpattern ~(#?.info) dopatterns'
fileparam.18 = 'filerequester window sleepwindow'
fileparam.19 = 'filerequester pubscreenname MY_PUBLIC_SCREEN'
fileparam.20 = 'filerequester typeface emerald.font typesize 11'
fileparam.21 = 'filerequester locale work:asl/asllocale.prefs'
fileparam.22 = 'filerequester dosavemode'
fileparam.23 = 'filerequester domultiselect'
fileparam.24 = 'filerequester drawersonly initialdrawer sys:'
fileparam.25 = 'filerequester initialdrawer sys: rejecticons'
fileparam.26 = 'filerequester rejectpattern #?.o'
fileparam.27 = 'filerequester acceptpattern #?.c'
fileparam.28 = 'filerequester drawersonly filterdrawers rejectpattern s#? initialdrawer sys:'
fileparam.29 = 'filerequester drawersonly filterdrawers acceptpattern s#? initialdrawer sys:'
fileparam.30 = 'filerequester flags1 32'
fileparam.31 = 'filerequester initialdrawer sys: flags2 1'
fileparam.32 = 'filerequester window intuimsgfunc'
fileparam.33 = 'filerequester filterfunc s#? initialdrawer sys:'

fileinst.1 = 'Select Cancel on File Requester'
fileinst.2 = 'Select the file filerequester followed by OK'
fileinst.3 = 'A window should open with the File Requester if not, select Cancel'
fileinst.4 = 'A screen should open with the File Requester if not, select Cancel'
fileinst.5 = 'UserData test, Select OK'
fileinst.6 = 'If the text is not bold, select Cancel'
fileinst.7 = 'If the text is not underlined, select Cancel'
fileinst.8 = 'If the text is not italic, select Cancel'
fileinst.9 = 'If the Title Text is not MyRequester, select Cancel'
fileinst.10 = 'The positive text should be Yay, the negative text Nay, if not select the negative gadget.'
fileinst.11 = 'InitialLeft test. Do not move the file requester, select OK'
fileinst.12 = 'InitialTop test. Do not move the file requester, select OK'
fileinst.13 = 'InitialWidth test. Do not resize the file requester, select OK.'
fileinst.14 = 'InitialHeight test, Do not resize the file requester, select OK.'
fileinst.15 = 'InitialFile test. Select OK'
fileinst.16 = 'InitialDrawer test. Select OK'
fileinst.17 = 'If the pattern is not ~(#?.info), select Cancel, also see if there are any .info files listed in the requester.'
fileinst.18 = 'Sleepwindow Test, click in the Window that is with the Requester if there is not a busy pointer, select Cancel.'
fileinst.19 = 'PublicScreenTest. If the requester does not open on the screen called MY_PUBLIC_SCREEN, select Cancel'
fileinst.20 = 'If Emerald.font is not being used, select Cancel.'
fileinst.21 = 'If the requester is not in German, select Cancel.'
fileinst.22 = 'If the file requester is not in save mode, (inverse video), select Cancel'
fileinst.23 = 'Multi-Select Test. Select filerequester + fontrequester and Press OK'
fileinst.24 = 'DrawersOnly Test, if there are files present in listview, select Cancel'
fileinst.25 = 'RejectIcons Test. If there are .info files present in the listview, select Cancel'
fileinst.26 = 'RejectPattern Test. If there are .o file present in the listview, select Cancel'
fileinst.27 = 'AcceptPattern Test. If there are any files other than .c files present in the listview, select Cancel'
fileinst.28 = 'FilterDrawers Reject Test. If there are any drawers with names starting with the letter S present in the listview, select Cancel'
fileinst.29 = 'FilterDrawers Accept Test. If there are any drawers that start with a letter other than S present in the listview, select Cancel'
fileinst.30 = 'Flags1 Test. If the file requester is not in save mode, (inverse video), select Cancel'
fileinst.31 = 'Flags2 Test. If there are files present in listview, select Cancel'
fileinst.32 = 'IntuiMessageFunc Test. Bring the test window to front and click the close gadget three (3) times.'
fileinst.33 = 'FilterFunc Test. Wait for the Drive light to go out, select OK'

fileerror.1 = 'User did not Select Cancel'
fileerror.2 = 'User did not select filerequester and OK'
fileerror.3 = 'No window'
fileerror.4 = 'No Screen'
fileerror.5 = 'User Data incorrect'
fileerror.6 = 'Bold tag failed'
fileerror.7 = 'Underlined tag failed'
fileerror.8 = 'Italic tag failed'
fileerror.9 = 'TitleText tag failed'
fileerror.10 = 'Positive or Negative text tag failed'
fileerror.11 = 'Initialleft tag failed'
fileerror.12 = 'Initialtop tag failed'
fileerror.13 = 'InitialWidth tag Failed'
fileerror.14 = 'InitialHeight Tag failed'
fileerror.15 = 'Initialfile tag failed'
fileerror.16 = 'Initialdrawer tag failed'
fileerror.17 = 'Initialpattern tag failed'
fileerror.18 = 'SleepWindow tag failed'
fileerror.19 = 'PublicScreenName tag failed. TurboText may not be running.'
fileerror.20 = 'Typeface or TypeSize tag failure'
fileerror.21 = 'Locale tag failure'
fileerror.22 = 'DoSaveMode tag failure'
fileerror.23 = 'DoMultiSelect tag failure'
fileerror.24 = 'DrawersOnly tag failure'
fileerror.25 = 'RejectIcons tag failure'
fileerror.26 = 'RejectPattern tag failure'
fileerror.27 = 'AcceptPattern tag failure'
fileerror.28 = 'FilterDrawers Reject tag failure'
fileerror.29 = 'FilterDrawers Accept tag failure'
fileerror.30 = 'Flags1 tag failure'
fileerror.31 = 'Flags2 tag failure'
fileerror.32 = 'IntuiMsgFunc tag failure.'
fileerror.33 = 'FilterFunc tag failure'

fileresult.1 = 'File requester cancelled by user'
fileresult.2 = 'fr_File=filerequester'
fileresult.3 = 'AslRequest() return code TRUE'
fileresult.4 = 'AslRequest() return code TRUE'
fileresult.5 = 'fr_UserData=$C0EDBABE'
fileresult.6 = 'AslRequest() return code TRUE'
fileresult.7 = 'AslRequest() return code TRUE'
fileresult.8 = 'AslRequest() return code TRUE'
fileresult.9 = 'AslRequest() return code TRUE'
fileresult.10 = 'AslRequest() return code TRUE'
fileresult.11 = 'fr_LeftEdge=0'
fileresult.12 = 'fr_TopEdge=0'
fileresult.13 = 'fr_Width=640'
fileresult.14 = 'fr_Height=200'
fileresult.15 = 'fr_File=filerequester'
fileresult.16 = 'fr_Drawer=sys:'
fileresult.17 = 'AslRequest() return code TRUE'
fileresult.18 = 'AslRequest() return code TRUE'
fileresult.19 = 'AslRequest() return code TRUE'
fileresult.20 = 'AslRequest() return code TRUE'
fileresult.21 = 'AslRequest() return code TRUE'
fileresult.22 = 'AslRequest() return code TRUE'
fileresult.23 = 'fr_NumArgs=2 wa_Name = filerequester wa_Name = fontrequester'
fileresult.24 = 'AslRequest() return code TRUE'
fileresult.25 = 'AslRequest() return code TRUE'
fileresult.26 = 'AslRequest() return code TRUE'
fileresult.27 = 'AslRequest() return code TRUE'
fileresult.28 = 'AslRequest() return code TRUE'
fileresult.29 = 'AslRequest() return code TRUE'
fileresult.30 = 'AslRequest() return code TRUE'
fileresult.31 = 'AslRequest() return code TRUE'
fileresult.32 = 'IntuiMsg hook hits: 3'
fileresult.33 = 'Filter hook hits: 16'

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

if ~open('log',logfile,'W') then do
    say 'Unable to open logfile '||logfile
    exit 20
end

do i = 1 to fileparam.0
    /* This next line tells the test what to do and what to expect. */

    say fileinst.i

    /* PainIO gets output from the device, the reason for this is that
     * the AmigaShell does not have the right packets to work with execio
     * If it did then PainIO would not be necessary
     */

    myrc = PainIO(fileparam.i,'filereq')

    /* This sets filecond to the proper line in the output display. */

    filecond.1 = filereq.2
    filecond.2 = filereq.2
    filecond.3 = filereq.1
    filecond.4 = filereq.1
    filecond.5 = filereq.9
    filecond.6 = filereq.1
    filecond.7 = filereq.1
    filecond.8 = filereq.1
    filecond.9 = filereq.1
    filecond.10 = filereq.1
    filecond.11 = filereq.4
    filecond.12 = filereq.5
    filecond.13 = filereq.6
    filecond.14 = filereq.7
    filecond.15 = filereq.2
    filecond.16 = filereq.3
    filecond.17 = filereq.1
    filecond.18 = filereq.1
    filecond.19 = filereq.1
    filecond.20 = filereq.1
    filecond.21 = filereq.1
    filecond.22 = filereq.1
    filecond.23 = filereq.8 filereq.10 filereq.12
    filecond.24 = filereq.1
    filecond.25 = filereq.1
    filecond.26 = filereq.1
    filecond.27 = filereq.1
    filecond.28 = filereq.1
    filecond.29 = filereq.1
    filecond.30 = filereq.1
    filecond.31 = filereq.1
    filecond.32 = filereq.10
    filecond.33 = filereq.10

    /* DEBUG!! The line below should be uncommented for Debugging purposes */
    /* say 'filecondition = '||filecond.i||' fileresult = '||fileresult.i */
    /* END DEBUG!! */
    if i = 32 then do
        say 'User Pressed Close Gadget '||word(filereq.10, 4)||' times.'
        fileresult.32 = 'IntuiMsg hook hits: '||word(filereq.10,4)
    end
    if filecond.i ~= fileresult.i then do
        say fileerror.i
        writeln('log',fileerror.i)
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
