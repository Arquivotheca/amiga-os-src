*******************************************************************************
*
*       $Id: Monitor.asm,v 39.0 91/08/21 17:26:35 chrisg Exp $
*
*******************************************************************************

	include 'exec/types.i'
	include 'graphics/view.i'

	section graphics

	xdef    _OpenMonitor
	xref    _open_monitor

******* graphics.library/OpenMonitor *****************************************
*
*   NAME
*       OpenMonitor -- open a named MonitorSpec (V36)
*
*   SYNOPSIS
*       mspc = OpenMonitor( monitor_name , display_id)
*       d0                  a1		   d0
*
*       struct MonitorSpec *OpenMonitor( char *, ULONG );
*
*   FUNCTION
*       Locate and open a named MonitorSpec.
*
*   INPUTS
*       monitor_name - a pointer to a null terminated string.
*       display_id - an optional 32 bit monitor/mode identifier 
*
*   RESULTS
*       mspc - a pointer to an open MonitorSpec structure.
*              NULL if MonitorSpec could not  be opened.
*
*   NOTE
*	if monitor_name is non-NULL, the monitor will be opened by name.
*	if monitor_name is NULL the monitor will be opened by optional ID.
*	if both monitor_name and display_id are NULL returns default monitor.
*
*   BUGS
*
*   SEE ALSO
*       CloseMonitor() graphics/monitor.h
*
******************************************************************************

_OpenMonitor:
*                         current routine calls a C subroutine to do the work
	move.l  d0,-(sp)        * push optional display ID 
	move.l  a1,-(sp)        * push ptr to name string
	jsr _open_monitor       * call C routine
	addq.l  #8,sp           * remove args from stack
	rts

	xdef    _CloseMonitor
	xref    _close_monitor

******* graphics.library/CloseMonitor ****************************************
*
*   NAME
*	CloseMonitor -- close a MonitorSpec (V36)
*
*   SYNOPSIS
*	error = CloseMonitor( monitor_spec )
*	d0                    a0
*
*	LONG CloseMonitor( struct MonitorSpec * );
*
*   FUNCTION
*	Relinquish access to a MonitorSpec.
*
*   INPUTS
*	monitor_spec - a pointer to a MonitorSpec opened via OpenMonitor(),
*	or NULL.
*
*   RESULTS
*	error - FALSE if MonitorSpec closed uneventfully.
*	        TRUE if MonitorSpec could not be closed.
*
*   BUGS
*
*   SEE ALSO
*	OpenMonitor()
*
******************************************************************************

_CloseMonitor:
*                         current routine calls a C subroutine to do the work
	move.l  a0,-(sp)        * push ptr to monitor_spec
	jsr _close_monitor      * call C routine
	addq.l  #4,sp           * remove args from stack
	rts

	end
