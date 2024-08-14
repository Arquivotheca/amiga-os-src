/***************************************************************************
*			COMMODORE
*			-----------
*
*	FILE: mainloop.c
*
*	HISTORY		AUTHOR COMMENTS
*	92-11-18	creation H.v.K.
*   93-03-24    addition for testing the commo interface
*
***************************************************************************/
#include "defs.h"

extern void	player_init(void);
extern void 	player(void);
extern void 	Dispatcher(void); 
extern void 	Init_command_handler(void);
extern void 	command_handler(void);
extern byte	get_servo_proces_state(void);
extern void	servo_start(void);
extern void	enable_scor_counter(void);


/* main entry point: */
void main(void)
{

   player_init();
   COMMO_INIT();
   Init_command_handler();
   enable_scor_counter();


   EA = 1; /* general enabling of interrupts */

   for(;;)
   {
    
      command_handler();
      COMMO_INTERFACE();
      Dispatcher();  
      player();

   }  /* do forever */
}
