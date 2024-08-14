struct NIPCPrefs
{
	ULONG	np_RealmServer;
	ULONG	np_RealmName;
	UBYTE	np_UserName[16];
	UBYTE	np_AmigaName[64];
	ULONG	np_RealmServerName[64];
};

struct NIPCDevicePrefs
{
	ULONG	ndp_IPType;
	ULONG	ndp_ARPType;
	UBYTE	ndp_HardAddress[16];
	ULONG	ndp_IPAddress;
	ULONG	ndp_IPSubnet;
	UBYTE	ndp_Reserved0;
	UBYTE	ndp_DevName[128];
};

struct NIPCRoutePrefs
{
	ULONG	nrp_DestNetwork;
	ULONG	nrp_Gateway;
	ULONG	nrp_Mask;
	UWORD	nrp_Hops;
};