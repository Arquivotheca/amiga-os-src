/*
 * Amiga Grand Wack
 *
 * bind.c -- Perform all keyboard bindings.
 *
 * $Id: bind.c,v 39.15 93/11/05 14:56:54 jesup Exp $
 *
 * This file contains all the commands bound at initialization time.
 */

#include <exec/types.h>

#include "sys_proto.h"
#include "define_proto.h"
#include "symload_proto.h"
#include "decode_proto.h"
#include "special_proto.h"
#include "link_proto.h"
#include "ops_proto.h"
#include "show_proto.h"
#include "showi_proto.h"
#include "showg_proto.h"
#include "showd_proto.h"
#include "showlock_proto.h"
#include "find_proto.h"
#include "copper_proto.h"
#include "asm_proto.h"
#include "sat_proto.h"
#include "main_proto.h"
#include "rexxhandler_proto.h"
#include "wackbase_proto.h"
#include "menus_proto.h"
#include "io_proto.h"
#include "parse_proto.h"

#include "bind_proto.h"

void
InitTopMap( void )
{
    /* Miscellaneous commands */
    BindCommand( "help", ShowHelp );
    BindCommand( "?", ShowHelp );
    BindCommand( "quit", Conclude );
    BindCommand( "about", Credits );
    BindCommand( "cls", ClearScreen );
    BindCommand( "protect", ProtectAddresses );
    BindCommand( "capture", Capture  );
    BindCommand( "context", ShowContext );
    BindCommand( "go", ResumeExecution  );
    BindCommand( "call", Call  );
    BindCommand2("system", runCommand, "" );
    BindCommand2("rx", ARexx_Execute, "" );
    BindCommand( "rxs", ARexx_ExecuteString );

    /* Configuration commands */
    BindCommand( "prompt", SetWackPrompt );
    BindCommand( "setdump", setDump );
    BindCommand( "bindconstant", bindConstant );
    BindCommand( "set", bindConstant );
    BindCommand( "bindalias", bindAlias );
    BindCommand( "bindsystem", bindSystem );
    BindCommand( "bindrx", bindRexx );
    BindCommand( "bindxwack", bindXWack );
    BindCommand( "addmenu", bindMenuHeader );
    BindCommand( "addmenuitem", bindMenuItem );
    BindCommand( "addsubitem", bindMenuSub );
    BindCommand( "showhunks", showHunks );
    BindCommand( "bindhunks", bindHunks );
    BindCommand( "bindsymbols", bindSymbols );

    /* Memory commands */
    BindCommand( ",", BackFrame );
    BindCommand( ".", NextFrame );
    BindCommand( "<", BackWord );
    BindCommand( ">", NextWord );
    BindCommand( ";", Disassemble );
    BindCommand( "+", NextCount );
    BindCommand( "-", BackCount );
    BindCommand( ":", SizeFrame );
    BindCommand( "=", AssignMem );
    BindCommand( "!", swapSpareAddr );
    BindCommand( "[", Indirect );
    BindCommand( "]", Exdirect );
    BindCommand( "{", IndirectBptr );	/* } <- for editor's brace matching */
    BindCommand( "dec", ToDec );
    BindCommand( "hex", ToHex );
    BindCommand( "bptr", ShiftBPtr );
    BindCommand( "find",  Find );
    BindCommand( "/", Find );
    BindCommand( "limit",  Limit );
    BindCommand( "stacklimit",  StackLimit );
    BindCommand( "where", ApproxSym );
    BindCommand( "whereis", ApproxSymIndirect );

    /* Exec/DOS commands */
    BindCommand( "execbase", ShowExecBase );
    BindCommand( "nodes", ShowNodes );
    BindCommand( "sem",  ShowSem );
    BindCommand( "sems",  ShowSemList );
    BindCommand( "tasks", ShowTasks );
    BindCommand( "task", ShowTask );
    BindCommand( "interrupts", ShowIntVects );
    BindCommand( "ints", ShowIntVects );
    BindCommand( "devices", ShowDeviceList );
    BindCommand( "devs", ShowDeviceList );
    BindCommand( "ports", ShowPortList );
    BindCommand( "libraries", ShowLibraryList );
    BindCommand( "libs", ShowLibraryList );
    BindCommand( "resources", ShowResourceList );
    BindCommand( "res", ShowResourceList );
    BindCommand( "regions", ShowRegionList );
    BindCommand( "memory", ShowMemoryList );
    BindCommand( "mem", ShowMemoryList );
    BindCommand( "modules", ShowResModules );
    BindCommand( "mods", ShowResModules );

    BindCommand( "dosbase",  ShowDBase );
    BindCommand( "doslist",  ShowDOSList );
    BindCommand( "process", ShowProcess );
    BindCommand( "proc", ShowProcess );

    /* Intuition commands */
    BindCommand( "ibase", ShowIBase );
    BindCommand( "ascreen",  ShowAScreen );
    BindCommand( "screen",  ShowScreen );
    BindCommand( "awindow",  ShowAWindow );
    BindCommand( "window",  ShowWindow );
    BindCommand( "wflags", ShowWindowFlags );
    BindCommand( "firstgad",  ShowFirstGad );
    BindCommand( "gadget",  ShowExtGadget );
    BindCommand( "gadgets",  ShowExtGadgetList );
    BindCommand( "menu", ShowMenu );
    BindCommand( "menus", ShowMenuList );
    BindCommand( "item", ShowMenuItem );
    BindCommand( "items", ShowMenuItemList );
    BindCommand( "requester", ShowRequester );
    BindCommand( "req", ShowRequester );
    BindCommand( "propinfo", ShowPropInfo );
    BindCommand( "image", ShowImage );
    BindCommand( "itext", ShowIntuiText );
    BindCommand( "class",  ShowIClass );
    BindCommand( "object",  ShowObject );
    BindCommand( "lisems",  ShowLayerInfoSems );

    /* Graphics/Layers commands */
    BindCommand( "gfxbase",  ShowGfxBase );
    BindCommand( "view",  ShowView );
    BindCommand( "vextra",  ShowViewExtra );
    BindCommand( "viewport",  ShowViewPort );
    BindCommand( "vp",  ShowViewPort );
    BindCommand( "viewports",  ShowViewPortList );
    BindCommand( "cop", dumpCops );
    BindCommand( "copper", dumpAllCops );
    BindCommand( "copinit", dumpCopHeader );
    BindCommand( "linfo", ShowLayer_Info );
    BindCommand( "layer",  ShowLayer );

#if 0
    /* Trace commands */
    BindCommand( "toggle_trace", ToglTrace );
    BindCommand( "set_breakpoint", SetBreakPoint );
    BindCommand( "resume", Resume );
#endif

    /* Undocumented commands */
    BindCommand( "symbols", ShowTopSymMap );
    BindCommand( "magic", Nothing2 );
    BindCommand( "plugh", Nothing2 );
    BindCommand( "xyzzy", Nothing3 );
}


STRPTR
ShowHelp( char *arg )
{
    if ( arg = parseRemainder( arg ) )
    {
	if ( !stricmp( arg, "misc" ) )
	{
	    ShowMiscHelp();
	}
	else if ( !stricmp( arg, "config" ) )
	{
	    ShowConfigHelp();
	}
	else if ( !stricmp( arg, "memory" ) )
	{
	    ShowMemoryHelp();
	}
	else if ( !stricmp( arg, "exec" ) )
	{
	    ShowExecHelp();
	}
	else if ( !stricmp( arg, "intuition" ) )
	{
	    ShowIntuitionHelp();
	}
	else if ( !stricmp( arg, "graphics" ) )
	{
	    ShowGraphicsHelp();
	}
#if 0
	else if ( !stricmp( arg, "trace" ) )
	{
	    ShowTraceHelp();
	}
#endif
	else
	{
	    ShowGeneralHelp();
	}
    }
    else
    {
	ShowGeneralHelp();
    }
    return( NULL );
}


void
ShowGeneralHelp( void )
{
    PPrintf("Help is available on the following topics:\n");
    PPrintf("\tMISC      - commands to manipulate and control the Wack environment.\n");
    PPrintf("\tCONFIG    - commands to configure Wack.\n");
    PPrintf("\tMEMORY    - commands to view and manipulate memory.\n");
    PPrintf("\tEXEC      - Exec and DOS exploration commands.\n");
    PPrintf("\tINTUITION - Intuition exploration commands.\n");
    PPrintf("\tGRAPHICS  - Graphics and Layers exploration commands.\n");
#if 0
    PPrintf("\tTRACE     - commands to handle tracing and breakpoints.\n");
#endif
    PPrintf("For example, type \"help misc\" to see the environment commands.\n");
    PPrintf("\nThe help descriptions include each command's syntax.  The command is\n");
    PPrintf("shown first.  optional arguments are enclosed in parentheses.\n");
    PPrintf("Any keyword arguments are shown in all capital letters.\n");
    PPrintf("If an optional address argument (shown as \"(addr)\") is omitted,\n");
    PPrintf("the current address will be used instead.\n");
}


void
ShowMiscHelp( void )
{
#if 0
    PPrintf("help (MISC|CONFIG|MEMORY|EXEC|INTUITION|GRAPHICS|TRACE) - show help on topic.\n");
/**/PPrintf("? (MISC|CONFIG|MEMORY|EXEC|INTUITION|GRAPHICS|TRACE) - Same as help.\n");
#else
    PPrintf("help (MISC|CONFIG|MEMORY|EXEC|INTUITION|GRAPHICS) - show help on topic.\n");
/**/PPrintf("? (MISC|CONFIG|MEMORY|EXEC|INTUITION|GRAPHICS) - Same as help.\n");
#endif
    PPrintf("quit    - Exit from Wack.\n");
    PPrintf("about   - Show credits for Wack.\n");
    PPrintf("cls     - Clear the Wack display.\n");
    PPrintf("protect (ON|OFF) - Control protection of invalid addresses from access.\n");
    PPrintf("capture (filename) (OFF) (APPEND) - Capture Wack output to file.\n");
    PPrintf("context - Show context of target system (SAD only).\n");
    PPrintf("go      - Resume execution on target system (SAD only).\n");
    PPrintf("call (addr)      - call a function at the specified or current address.\n");
    PPrintf("system command   - Executes command as a shell program.\n");
    PPrintf("rx script        - Executes ARexx script.\n");
    PPrintf("rxs rexxcommands - Executes ARexx commands.\n");
}


void
ShowConfigHelp( void )
{
    PPrintf("prompt (string) - Set prompt. (*n and \\n are newline, *t and \\t are tab,\n");
    PPrintf("\t*c and \\c expand to the current address).\n");
    PPrintf("setdump (WORD|LONGWORD) (WIDE|NARROW) (ASCII|NOASCII) - Set memory dump options.\n");
    PPrintf("bindconstant label (addr) - Create a label equal to the specified address.\n");
/**/PPrintf("set label (addr) - Same as bindconstant.\n");
    PPrintf("bindalias command1 command2 - Make command1 a synonym for command2.\n");
    PPrintf("bindsystem command program (args) - Make command invoke a shell program\n");
    PPrintf("\twith the specified arguments.\n");
    PPrintf("bindrx command script (args) - Make command invoke an ARexx script\n");
    PPrintf("\twith the specified arguments.\n");
    PPrintf("bindxwack command program (args) - Make command invoke a Wack-extension\n");
    PPrintf("\tprogram passing WACKPORT=Wack's port name and the specified arguments.\n");
    PPrintf("addmenu header - Adds a menu header to the Wack window.\n");
    PPrintf("additem header item command (key) - Adds a menu item to the menu header which\n");
    PPrintf("\tinvokes the specified command, with optional menu key.\n");
    PPrintf("addsubitem header item sub command (key) - Adds a sub-item to the menu item\n");
    PPrintf("\twhich invokes the specified command, with optional menu key.\n");
}


void
ShowMemoryHelp( void )
{
    PPrintf("<Enter> - Show current memory frame.\n");
    PPrintf("(addr)  - Set current address and show that memory frame.\n");
    PPrintf(",       - Go to previous frame (multiple .'s show multiple frames).\n");
    PPrintf(".       - Go to next frame (multiple ,'s show multiple frames).\n");
    PPrintf("<       - Go to previous word (multiple <'s skip multiple words).\n");
    PPrintf(">       - Go to next word (multiple >'s skip multiple words).\n");
    PPrintf(";       - Disassemble current frame (multiple ;'s show multiple frames).\n");
    PPrintf("+h      - Go forward h bytes.\n");
    PPrintf("-h      - Go backward h bytes.\n");
    PPrintf(":h      - Set frame size to h bytes.\n");
    PPrintf("=hhhh   - Set word at current address to hhhh.\n");
    PPrintf("!       - Swap current and spare address pointers.\n");
    PPrintf("[       - Indirect (go to address pointed at by current address).\n");
    PPrintf("]       - Exdirect (undo last indirection).\n");
    PPrintf("{       - BPTR-Indirect (go to address pointed at by BPTR at current address).\n"); /* } */
    PPrintf("dec h   - Converts h (hex) to decimal value.\n");
    PPrintf("hex n   - Converts n (decimal) to hex value.\n");
    PPrintf("bptr b  - Converts BCPL pointer b to hex value.\n");
    PPrintf("find (h)           - Find 16 or 32 bit number between current address and limit.\n");
/**/PPrintf("/ (h)              - Same as find.\n");
    PPrintf("limit (addr)       - Set limit address for find operation.\n");
    PPrintf("stacklimit (addr)  - Set current address and find limit based on specified task.\n");
    PPrintf("where (addr)       - Show nearest symbol to specified address.\n");
    PPrintf("whereis (addr)     - Show nearest symbol to contents of specified address.\n");

    PPrintf("These commands require SegTracker to be running on the target machine:\n");
    PPrintf("showhunks (module) - Show hunks of specified (or all) module(s).\n");
    PPrintf("bindhunks (module) - Make symbols for hunks of specified (or all) module(s).\n");
    PPrintf("bindsymbols symfile (module) - Define every symbol in symfile using\n");
    PPrintf("\tthe actual values of the hunks of the specified module.\n");

    PPrintf("\nThe [, ], {, and ! commands may be followed by another command for\n"); /* } */
    PPrintf("immediate action, for example, \"[task\" would indirect and show that task.\n");
}


void
ShowExecHelp( void )
{
    PPrintf("execbase     - Show ExecBase.\n");
    PPrintf("nodes (addr) - Show nodes on the same list as the specified one.\n");
    PPrintf("sem (addr)   - Show specified semaphore.\n");
    PPrintf("sems (addr)  - Show semaphore list at specified address.\n");
    PPrintf("tasks        - Show all tasks.\n");
    PPrintf("task (addr|name) - Show task at specified address or whose task name\n");
    PPrintf("\tor cli_CommandName partially matches the specified name.\n");
    PPrintf("interrupts   - Show interrupt vectors.\n");
/**/PPrintf("ints         - Same as interrupts.\n");
    PPrintf("devices      - Show all devices.\n");
/**/PPrintf("devs         - Same as devices.\n");
    PPrintf("ports        - Show all public ports.\n");
    PPrintf("libraries    - Show all libraries.\n");
/**/PPrintf("libs         - Same as libraries.\n");
    PPrintf("resources    - Show all resources.\n");
/**/PPrintf("res          - Same as resources.\n");
    PPrintf("regions      - Show all memory regions.\n");
    PPrintf("memory       - Show all memory headers.\n");
/**/PPrintf("mem          - Same as memory.\n");
    PPrintf("modules      - Show all resident modules.\n");
/**/PPrintf("mods         - Same as modules.\n");

    PPrintf("dosbase      - Show DOSBase.\n");
    PPrintf("doslist      - Show DOS device/volume/assign list.\n");
    PPrintf("process (addr|name) - Show task at specified address or whose task name\n");
    PPrintf("\tor cli_CommandName partially matches the specified name.\n");
/**/PPrintf("proc (addr|name)    - Same as process.\n");
}


void
ShowIntuitionHelp( void )
{
    PPrintf("ibase            - Show IntuitionBase.\n");
    PPrintf("ascreen          - Show address of active screen.\n");
    PPrintf("screen (addr)    - Show screen.\n");
    PPrintf("awindow          - Show address of active window.\n");
    PPrintf("window (addr)    - Show window.\n");
    PPrintf("wflags (addr)    - Describe flags of window.\n");
    PPrintf("firstgad (addr)  - Show address of first gadget of window.\n");
    PPrintf("gadget (addr)    - Show gadget.\n");
    PPrintf("gadgets (addr)   - Show all gadgets on this list.\n");
    PPrintf("menu (addr)      - Show Menu header.\n");
    PPrintf("menus (addr)     - Show chain of Menu headers.\n");
    PPrintf("item (addr)      - Show MenuItem or SubItem.\n");
    PPrintf("items (addr)     - Show chain of MenuItems or SubItems.\n");
    PPrintf("requester (addr) - Show requester.\n");
/**/PPrintf("req (addr)       - Same as requester.\n");
    PPrintf("propinfo (addr)  - Show PropInfo structure.\n");
    PPrintf("image (addr)     - Show Image structure.\n");
    PPrintf("itext (addr)     - Show IntuiText structure.\n");
    PPrintf("class (addr)     - Show boopsi class.\n");
    PPrintf("object (addr)    - Show boopsi object.\n");
    PPrintf("lisems           - Show LayerInfo semaphores for all screens.\n");
}


void
ShowGraphicsHelp( void )
{
    PPrintf("gfxbase          - Show GfxBase.\n");
    PPrintf("view (addr)      - Show View structure.\n");
    PPrintf("vextra (addr)    - Show ViewExtra structure.\n");
    PPrintf("viewport (addr)  - Show ViewPort structure.\n");
/**/PPrintf("vp (addr)        - Same as viewport.\n");
    PPrintf("viewports (addr) - Show ViewPort list.\n");
    PPrintf("cop              - Show copper list (omit color table).\n");
    PPrintf("copper           - Show copper list (include color table).\n");
    PPrintf("copinit          - Show copinit copper list.\n");
    PPrintf("linfo (addr)     - Show LayerInfo structure.\n");
    PPrintf("layer (addr)     - Show Layer structure.\n");
}

#if 0
void
ShowTraceHelp( void )
{
    PPrintf("No Trace help is currently available.\n");
}
#endif
