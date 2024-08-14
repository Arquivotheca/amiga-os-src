/*
** Unix signal emulation.  We don't have the luxury of interruptable system
** calls or any resource tracking so this is about the best we can really
** do.
*/

#include <signal.h>

void	(*sig_array[_NUMSIG+1])();

int (*signal(which, func))()
	int	which;
	void	(*func)();
{
	if(which < _NUMSIG){
		sig_array[which] = func;
		return 0;
	}
	return -1;
}

/*
** take_signal is called typically from the program's main loop when
** a particular event is returned from Wait().
*/
void
take_signal(which)
{
	if(which < _NUMSIG && sig_array[which]){
		(*sig_array[which])(which);
	}
}
