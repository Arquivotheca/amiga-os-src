struct HAND_MESSAGE
		{
		struct Message mesI;
		ULONG m_type;
		ULONG pid;
		struct DosPacket *pkt_pointer;
		ULONG error;
		ULONG monitor;
		};

#define STARTUP 712
#define SENDMG  782
