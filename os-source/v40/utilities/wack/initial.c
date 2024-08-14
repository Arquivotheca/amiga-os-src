
/***********************************************************************
*
*			   G R A N D W A C K
*
************************************************************************
*
*	Copyright (C) 1985, Commodore-Amiga. All rights reserved.
*
************************************************************************
*
*   Source Control:
*
*	$Id: initial.c,v 1.10 92/08/27 12:14:59 peter Exp $
*
*	$Locker:  $
*
*	$Log:	initial.c,v $
 * Revision 1.10  92/08/27  12:14:59  peter
 * "task" and "proc" are now shortcuts for showtask and showprocess
 * respectively.  Added bindings for new show stack-pointer ("sp") and
 * find ROM address ("findrom", "rom") features.
 * 
 * Revision 1.9  91/05/29  08:47:34  peter
 * Added "itext" binding.
 * 
 * Revision 1.8  91/04/24  20:49:36  peter
 * New bindings awindow, awin, ascreen, asc, sc, win, gad, gads, spareaddr,
 * fgad, fgadget.
 * "!" now invokes spareaddr instead of newhelp.
 * 
 * Revision 1.7  90/05/30  11:08:32  kodiak
 * change where function to $ key and make @ alphanumeric
 * 
 * Revision 1.6  90/02/21  11:44:05  kodiak
 * rename dbase to dosbase, and add dos device listing to it
 * 
 * Revision 1.5  90/01/15  17:17:55  jimm
 * change Window to XWindow to pick up internal structs
 * 
 * Revision 1.4  89/09/21  19:35:28  jimm
 * bringing rcs up to sync
 * 
 * Revision 1.3  88/01/22  13:45:23  jimm
 * new functions viewport (vp) viewports (vps) view
 * 
 * Revision 1.2  88/01/21  13:37:10  root
 * kodiak's copy of jimm's version, snapshot jan 21
 * 
*
***********************************************************************/

extern Help();
extern Conclude();

extern ShowFrame();
extern NextWord();
extern BackWord();
extern NextFrame();
extern BackFrame();
extern NextCount();
extern BackCount();
extern DisAsm();
extern Whereis();
extern Indirect();
extern IndirectBptr();
extern Exdirect();
extern SizeFrame();
extern Resume();
extern ToglTrace();
extern SetBreakPoint();
extern AssignMem();
extern GatherKeys();

extern ShowNodes();
extern ShowTasks();
extern ShowTask();
extern ShowSP();
extern FindROMAddrs();
extern ShowProcess();
extern ShowDeviceList();
extern ShowPortList();
extern ShowIntVects();
extern ShowLibraryList();
extern ShowResourceList();
extern ShowRegionList();
extern ShowMemoryList();
extern ShowResModules();
extern ShowTopKeyMap();
extern ShowTopSymMap();
extern ShowExecBase();
extern AttachHunks();
extern BindHunks();
extern Interpret();
extern ShiftBPtr();
extern ToHex();
extern ToDec();
extern ClearScreen();
extern ApproxSym();
extern Nothing();

/* jimm: */
extern	ShowMenuList();
extern	ShowMenu();
extern	ShowMenuItemList();
extern	ShowMenuItem();
extern	ShowRequesterList();
extern	ShowRequester();
extern	SpecialHelp();	/* help for these new functions */
extern	ShowDBase();
extern	ShowIBase();
extern	ShowAWindow();
extern	ShowAScreen();
extern	ShowFirstGad();
extern	ShowGfxBase();
extern	ShowLayer_Info();
extern	ShowXScreen();
extern	ShowXScreenList();
extern	ShowSem();
extern	ShowSemList();
extern  ShowISems();
extern  ShowLayerInfoSems();
extern  Find();
extern  Limit();
extern	StackLimit();
extern	ShowLayer();
extern	ShowView();
extern	ShowViewExtra();
extern	ShowViewPort();
extern	ShowViewPortList();
extern	ShowXWindow();
extern	ShowXWindowList();
extern	ShowGadget();
extern	ShowGadgetList();
extern  ShowGEnv();
extern  ShowPropInfo();
extern  ShowImage();
extern	ShowIntuiText();
extern  ShowThing();
extern  ShowThingList();
extern	dumpCops();
extern	dumpAllCops();
extern	dumpCopHeader();
extern	swapSpareAddr();

NewDisAsm()
{
    printf ("\07\nstart using ';' to disassemble");
    DisAsm();
}

InitTopMap()
{
    BindPrim ("help", Help);
    BindPrim ("quit", Conclude);

    BindPrim ("`GatherKeys", GatherKeys);

    /* jimm: */
    BindPrim("newhelp", SpecialHelp);
    BindPrim("spareaddr", swapSpareAddr);
    BindPrim("dosbase",  ShowDBase);
    BindPrim("ibase",  ShowIBase);
    BindPrim("awindow",  ShowAWindow);
    BindPrim("awin",  ShowAWindow);
    BindPrim("asc",  ShowAScreen);
    BindPrim("ascreen",  ShowAScreen);
    BindPrim("gfxbase",  ShowGfxBase);
    BindPrim("linfo", ShowLayer_Info);
    BindPrim("screen",  ShowXScreen);
    BindPrim("sc",  ShowXScreen);
    BindPrim("screens",  ShowXScreenList);
    BindPrim("sem",  ShowSem);
    BindPrim("sems",  ShowSemList);
    BindPrim("isems",  ShowISems);
    BindPrim("lisems",  ShowLayerInfoSems);
    BindPrim("limit",  Limit);
    BindPrim("find",  Find);
    BindPrim("stacklimit",  StackLimit);
    BindPrim("sl",  StackLimit);
    BindPrim("view",  ShowView);
    BindPrim("vext",  ShowViewExtra);
    BindPrim("viewport",  ShowViewPort);
    BindPrim("vp",  ShowViewPort);
    BindPrim("viewports",  ShowViewPortList);
    BindPrim("vps",  ShowViewPortList);
    BindPrim("window",  ShowXWindow);
    BindPrim("win",  ShowXWindow);
    BindPrim("windows",  ShowXWindowList);
    BindPrim("gadget",  ShowGadget);
    BindPrim("gad",  ShowGadget);
    BindPrim("gads",  ShowGadgetList);
    BindPrim("gadgets",  ShowGadgetList);
    BindPrim("fgadget",  ShowFirstGad);
    BindPrim("fgad",  ShowFirstGad);
    BindPrim("layer",  ShowLayer);
    BindPrim("genv",  ShowGEnv);
    BindPrim("menu", ShowMenu);
    BindPrim("menus", ShowMenuList);
    BindPrim("item", ShowMenuItem);
    BindPrim("items", ShowMenuItemList);
    BindPrim("requester", ShowRequester);
    BindPrim("requesters", ShowRequesterList);
    BindPrim("pinfo", ShowPropInfo);
    BindPrim("image", ShowImage);
    BindPrim("itext", ShowIntuiText);
    BindPrim("thing", ShowThing);
    BindPrim("things", ShowThingList);
    BindPrim("cop", dumpCops);
    BindPrim("Cop", dumpAllCops);
    BindPrim("copinit", dumpCopHeader);

    BindPrim ("show_frame", ShowFrame);
    BindPrim ("next_frame", NextFrame);
    BindPrim ("back_frame", BackFrame);
    BindPrim ("next_word", NextWord);
    BindPrim ("back_word", BackWord);
    BindPrim ("next_count", NextCount);
    BindPrim ("back_count", BackCount);
    BindPrim ("disassemble", DisAsm);
    BindPrim ("newdisasm", NewDisAsm);
    BindPrim ("whereis", Whereis);
    BindPrim ("indirect", Indirect);
    BindPrim ("indirect_bptr", IndirectBptr);
    BindPrim ("exdirect", Exdirect);
    BindPrim ("size_frame", SizeFrame);
    BindPrim ("toggle_trace", ToglTrace);
    BindPrim ("set_breakpoint", SetBreakPoint);
    BindPrim ("resume", Resume);
    BindPrim ("assign_mem", AssignMem);
    
    BindPrim ("nodes", ShowNodes);
    BindPrim ("execbase", ShowExecBase);
    BindPrim ("tasks", ShowTasks);
    BindPrim ("showtask", ShowTask);
    BindPrim ("task", ShowTask);
    BindPrim ("sp", ShowSP);
    BindPrim ("findrom", FindROMAddrs );
    BindPrim ("rom", FindROMAddrs );
    BindPrim ("ints", ShowIntVects);
    BindPrim ("showprocess", ShowProcess);
    BindPrim ("proc", ShowProcess);
    BindPrim ("devs", ShowDeviceList);
    BindPrim ("devices", ShowDeviceList);
    BindPrim ("ports", ShowPortList);
    BindPrim ("libs", ShowLibraryList);
    BindPrim ("libraries", ShowLibraryList);
    BindPrim ("rsrcs", ShowResourceList);
    BindPrim ("resources", ShowResourceList);
    BindPrim ("regions", ShowRegionList);
    BindPrim ("mem", ShowMemoryList);
    BindPrim ("memory", ShowMemoryList);
    BindPrim ("mods", ShowResModules);
    BindPrim ("modules", ShowResModules);
    BindPrim ("keys", ShowTopKeyMap);
    BindPrim ("symbols", ShowTopSymMap);
    BindPrim ("where", ApproxSym);
    BindPrim ("magic", Nothing);
    BindPrim ("attach", AttachHunks);
    BindPrim ("bindcli", AttachHunks);
    BindPrim ("bindhunks", BindHunks);
    BindPrim ("interpret", Interpret);
    BindPrim ("bptr", ShiftBPtr);
    BindPrim ("dec", ToDec);
    BindPrim ("hex", ToHex);
    BindPrim ("cls", ClearScreen);

    BindKey ('!', "TopMap", "spareaddr");
    BindKey ('?', "TopMap", "help");
    BindKey ('\n', "TopMap", "show_frame");
    BindKey ('.', "TopMap", "next_frame");
    BindKey (',', "TopMap", "back_frame");
    BindKey ('>', "TopMap", "next_word");  /* move like cursor keys */
    BindKey ('<', "TopMap", "back_word");
    BindKey ('\010', "TopMap", "back_word");
    BindKey (' ', "TopMap", "next_word");
    BindKey (';', "TopMap", "disassemble");
    BindKey ('/', "TopMap", "find");		/* jimm: */
    BindKey ('*', "TopMap", "whereis");		/* jimm	*/
    BindKey ('[', "TopMap", "indirect");
    BindKey ('{', "TopMap", "indirect_bptr");
    BindKey (']', "TopMap", "exdirect");
    BindKey (':', "TopMap", "size_frame");
    BindKey ('+', "TopMap", "next_count");
    BindKey ('-', "TopMap", "back_count");
    BindKey ('=', "TopMap", "assign_mem");
    BindKey ('$', "TopMap", "where");		/* kodiak (was '@') */
    BindKey ('(', "TopMap", "interpret");
    BindKey ('@', "TopMap", "`GatherKeys");
    BindKey ('`', "TopMap", "`GatherKeys");
    BindKey ('_', "TopMap", "`GatherKeys");
    BindSet ('0', '9', "TopMap", "`GatherKeys");
    BindSet ('a', 'z', "TopMap", "`GatherKeys");
    BindSet ('A', 'Z', "TopMap", "`GatherKeys");
}
