/* === ss.h ==========================================
 *
 * shared socket library main header file. All applications
 * using the shared library must use this stuff.
 * ===================================================
 */
 
#include <fd_to_fh.h>
#include <sys/types.h>
#include <devices/timer.h>
 
struct SSinfo {
	VOID *ssi_error ;          /* a ppointer to a 32-bit 'errno' value */
	fd_set *ssi_is_socket ;    /* pointer to the app's '_is_socket var */
	struct timerequest ssi_timerequest ;/* complete timerequest struct */














/* ----------------------------------------------------------------------------
 * ss.i
 *
 * INCLUDE "devices/timer.i"
 *
 * STRUCTURE  SSinfo,0
 *     ULONG  SSI_ERROR
 *     ULONG  SSI_IS_SOCKET
 *     STRUCT SSI_TIMEREQUEST,IOTV_SIZE
 *
 *
 *
 *
 *
 * 