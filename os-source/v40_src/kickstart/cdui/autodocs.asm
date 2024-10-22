
******* cdui.library/StartCDUI ***********************************************
*
*   NAME
*	StartCDUI -- start the CD game system user-interface module. (V40)
*
*   SYNOPSIS
*	port = StartCDUI();
*	D0
*
*	struct MsgPort *StartCDUI(VOID);
*
*   FUNCTION
*	This function initiates the CD game system user-interface tasks.
*	A public port is created for communication with the UI tasks. Its
*	name is STARTUP_ANIM_PORT, as defined in <internal/cdui.h>.
*
*	The UI port accepts standard Exec messages. The mn_Length field of
*	these messages indicates commands to perform. The commands currently
*	supported are:
*
*		ANIMMSG_STARTANIM   - Initiate the startup animation sequence
*		ANIMMSG_DOORCLOSED  - User has inserted a CD
*		ANIMMSG_RESTARTANIM - The CD the user inserted is bogus, resume
*		ANIMMSG_BOOTING     - User has inserted a game
*		ANIMMSG_SHUTDOWN    - Game is ready to run, get out
*		ANIMMSG_RED_BUTTON  - Do the action associated with the red button
*		ANIMMSG_BLUE_BUTTON - Do the action associated with the blue button
*		ANIMMSG_NOP         - NOP, ignored
*
*   RESULTS
*	port - the port where to send commands to. This port can also be
*	       found by FindPort(STARTUP_ANIM_PORT).
*
*   NOTES
*	cdui.library only allows a single opener at any given time.
*
*   SEE ALSO
*	<internal/cdui.h>
*
******************************************************************************

