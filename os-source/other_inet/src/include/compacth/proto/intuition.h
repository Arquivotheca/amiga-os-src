��INTUITION_INTUITION_H�<intuition/intuition.h>�
���*�;
�OpenIntuition(�);
�Intuition(�InputEvent);
�AddGadget(�Window*,��*,�);
�ClearDMRequest(�Window*);
�ClearMenuStrip(�Window*);
�ClearPointer(�Window*);
�CloseScreen(�Screen*);
�CloseWindow(�Window*);
�CloseWorkBench(�);
�CurrentTime(�*,�*);
�DisplayAlert(�,�*,�);
�DisplayBeep(�Screen*);
�DoubleClick(�,�,�,�);
�DrawBorder(�RastPort*,�Border*,�,�);
�DrawImage(�RastPort*,�Image*,�,�);
�EndRequest(�Requester*,�Window*);
�Preferences*GetDefPrefs(�Preferences*,�);
�Preferences*GetPrefs(�Preferences*,�);
�InitRequester(�Requester*);
�MenuItem*ItemAddress(�Menu*,�);
�ModifyIDCMP(�Window*,�);
�ModifyProp(��*,�Window*,�Requester*,�,�,�,�,�);
�MoveScreen(�Screen*,�,�);
�MoveWindow(�Window*,�,�);
�OffGadget(��*,�Window*,�Requester*);
�OffMenu(�Window*,�);
�OnGadget(��*,�Window*,�Requester*);
�OnMenu(�Window*,�);
�Screen*OpenScreen(�NewScreen*);
�Window*OpenWindow(�NewWindow*);
�OpenWorkBench(�);
�PrintIText(�RastPort*,�IntuiText*,�,�);
�RefreshGadgets(��*,�Window*,�Requester*);
�RemoveGadget(�Window*,��*);
�ReportMouse(�Window*,�);
�Request(�Requester*,�Window*);
�ScreenToBack(�Screen*);
�ScreenToFront(�Screen*);
�SetDMRequest(�Window*,�Requester*);
�SetMenuStrip(�Window*,�Menu*);
�SetPointer(�Window*,�*,�,�,�,�);
�SetWindowTitles(�Window*,�*,�*);
�ShowTitle(�Screen*,�);
�SizeWindow(�Window*,�,�);
�View*ViewAddress(�);
�ViewPort*ViewPortAddress(�Window*);
�WindowToBack(�Window*);
�WindowToFront(�Window*);
�WindowLimits(�Window*,�,�,�,�);
�SetPrefs(�Preferences*,�,�);
�IntuiTextLength(�IntuiText*);
�WBenchToBack(�);
�WBenchToFront(�);
�AutoRequest(�Window*,�IntuiText*,�IntuiText*,�IntuiText*,�,�,�,�);
�BeginRefresh(�Window*);
�Window*BuildSysRequest(�Window*,�IntuiText*,�IntuiText*,�IntuiText*,�,�,�);
�EndRefresh(�Window*,�);
�FreeSysRequest(�Window*);
�MakeScreen(�Screen*);
�RemakeDisplay(�);
�RethinkDisplay(�);
�*AllocRemember(�Remember**,�,�);
�AlohaWorkbench(�);
�FreeRemember(�Remember**,�);
�LockIBase(�);
�UnlockIBase(�);
�GetScreenData(�*,�,�,�Screen*);
�RefreshGList(��*,�Window*,�Requester*,�);
�AddGList(�Window*,��*,�,�,�Requester*);
�RemoveGList(�Window*,��*,�);
�ActivateWindow(�Window*);
�RefreshWindowFrame(�Window*);
�ActivateGadget(��*,�Window*,�Requester*);
�NewModifyProp(��*,�Window*,�Requester*,�,�,�,�,�,�);�NO_PRAGMAS���OpenIntuition 1e 0���Intuition 24����AddGadget 2a 9803���ClearDMRequest 30����ClearMenuStrip 36����ClearPointer 3c����CloseScreen 42����CloseWindow 48����CloseWorkBench 4e 0���CurrentTime 54����DisplayAlert 5a 18003���DisplayBeep 60����DoubleClick 66 321004���DrawBorder 6c 109804���DrawImage 72 109804���EndRequest 78����GetDefPrefs 7e����GetPrefs 84����InitRequester 8a����ItemAddress 90����ModifyIDCMP 96����MoveScreen a2 10803���MoveWindow a8 10803���OffGadget ae a9803���OffMenu b4����OnGadget ba a9803���OnMenu c0����OpenScreen c6����OpenWindow cc����OpenWorkBench d2 0���PrintIText d8 109804���RefreshGadgets de a9803���RemoveGadget e4����ReportMouse ea����Request f0����ScreenToBack f6����ScreenToFront fc����SetDMRequest 102����SetMenuStrip 108����SetPointer 10e 32109806���SetWindowTitles 114 a9803���ShowTitle 11a����SizeWindow 120 10803���ViewAddress 126 0���ViewPortAddress 12c����WindowToBack 132����WindowToFront 138����WindowLimits 13e 3210805���SetPrefs 144 10803���IntuiTextLength 14a����WBenchToBack 150 0���WBenchToFront 156 0���BeginRefresh 162����EndRefresh 16e����FreeSysRequest 174����MakeScreen 17a����RemakeDisplay 180 0���RethinkDisplay 186 0���AllocRemember 18c 10803���AlohaWorkbench 192����FreeRemember 198����LockIBase 19e 1���UnlockIBase 1a4����GetScreenData 1aa 910804���RefreshGList 1b0 a9804���AddGList 1b6 a109805���RemoveGList 1bc 9803���ActivateWindow 1c2����RefreshWindowFrame 1c8����ActivateGadget 1ce a9803�