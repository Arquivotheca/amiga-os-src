#include <exec/types.h>
#include <exec/devices.h>
#include <exec/io.h>
#include <devices/trackdisk.h>

void main()
	{
	struct IORequest IOReq;
	struct TDU_PublicUnit *TrackDiskBase;
	int i;

	for (i=0; i<4; i++)
		{
		if (!OpenDevice(TD_NAME, i, &IOReq, 0))
			{
			if (IOReq.io_Device->dd_Library.lib_Version >=36)
				{
				TrackDiskBase = (struct TDU_PublicUnit *)IOReq.io_Unit;
				TrackDiskBase->tdu_PubFlags |= TDPF_NOCLICK;
				}
			CloseDevice(&IOReq);
			}
		}
	}
