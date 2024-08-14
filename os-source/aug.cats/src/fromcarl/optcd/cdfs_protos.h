void	ResetCDTV( void );
void	SetDebug( LONG DebugLevel );
LONG	ValidDisk( void );
void	MountFS( void );
struct	TMInfo *GetTMInfo( void );
LONG	IsNoPrefs( void );
void	SetSpeed( LONG Speed );

struct	MsgPort *InitFSE( UBYTE *Mem, ULONG Length, ULONG Monitor );
void	QuitFSE( void );
void	ReplyFSE( struct Message *Msg );
void	SetFSEMonitor( ULONG Monitor );
void	StartFSE( void );
void	StopFSE( void );
