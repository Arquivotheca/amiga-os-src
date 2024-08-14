/*
 * Dummy daemon_init() in case the user doesn't want any initialization to
 * happen during startup.
 */
daemon_init(msg)
	struct INETDmsg *msg;
{
	return 1;
}
