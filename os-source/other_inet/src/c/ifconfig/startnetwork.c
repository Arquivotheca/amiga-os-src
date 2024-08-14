/* -----------------------------------------------------------------------
 * startnetwork.c  (for ifconfig) Manx36
 *
 * $Locker:  $
 *
 * $Id: startnetwork.c,v 1.1 90/11/12 14:55:54 bj Exp $
 *
 * $Revision: 1.1 $
 *
 * $Header: Hog:Other/inet/src/c/ifconfig/RCS/startnetwork.c,v 1.1 90/11/12 14:55:54 bj Exp $
 *
 *------------------------------------------------------------------------
 */



/*
 * This here to override net.lib definition.  ifconfig is the only command
 * that can really read in the network library.
 */
start_network()
{
	return	1;
}
