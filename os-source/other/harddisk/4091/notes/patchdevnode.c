Relay-Version: version B 2.10.3 4.3bsd-beta 6/6/85; site cbmvax.commodore.com
Path: cbmvax!cbmehq!cbmger!amiux!sourcery!olsen
From: olsen@sourcery.uucp (Olaf Barthel)
Newsgroups: adsp.adsp
Subject: Re: filesystem resource
Message-ID: <olsen.7841@sourcery.uucp>
Date: 1 Sep 91 20:06:47 GMT
Date-Received: 2 Sep 91 06:26:57 GMT
References: <michael.3502@phoenix.UUCP>
Organization: None
Lines: 119

In article <michael.3502@phoenix.UUCP> michael@phoenix.UUCP (Michael Warner) writes:

>- Can anyone tell me what DOS\2 and DOS\3 are about? They all have the same
>  id string. Are they just there for compatibility?

I suggest that you ask Randell Jessup about that (I remember that DOS2 and DOS3
were the international versions SFS and FFS which use an open hashing function
different from the standard one understood by pre-2.x system software).

>- Why are the stack sizes garbage? Are they ignored by DOS entry builders?
>- Why does the DOS\1 entry have a seglist of 0? This is the one I want
>  to boot from under V2!

Remember that the entries listed in the FileSystem.resource are actually
patches to apply to a DeviceNode returned by MakeDosNode(). The following
piece of code will handle the setup for you (note: this is not an officially
endorsed code fragment, use at your own risk):

#include <libraries/filehandler.h>
#include <resources/filesysres.h>

	/* PatchDeviceNode(DeviceNode):
	 *
	 *	Patch a DeviceNode returned from MakeDosNode() using
	 *	data from a FileSystem.resource entry whose
	 *	DosType matches the one in the corresponding
	 *	DosEnvec structure linked to the DeviceNode.
	 *
	 *	Returns TRUE if successful, FALSE otherwise.
	 */

BYTE
PatchDeviceNode(struct DeviceNode *DevNode)
{
	struct FileSysResource		*FSR;
	struct FileSysEntry		*FSE;
	BYTE				 Success = FALSE;
	struct DosEnvec			*DosEnvec;
	struct FileSysStartupMsg	*Startup;

		/* Obtain a pointer to the DosEnvec structure
		 * associated with the DeviceNode (one could do
		 * that in a single line of code, though).
		 */

	Startup = (struct FileSysStartup *)DevNode -> dn_Startup;

	DosEnvec = (struct DosEnvec *)BADDR(Startup -> fssm_Environ);

		/* Open FileSystem.resource. */

	if(FSR = (struct FileSysResource *)OpenResource(FSRNAME))
	{
			/* Get a pointer to the first list entry. */

		FSE = (struct FileSysEntry *)FSR -> fsr_FileSysEntries . lh_Head;

			/* Scan until list is exhausted or the
			 * approriate DosType has been found.
			 */

		while(FSE -> fse_Node . ln_Succ && !Success)
		{
				/* Does the DosType match? */

			if(DevNode -> fse_DosType == DosEnvec -> de_DosType)
			{
				ULONG	*From,*To;
				WORD	 i;

					/* Get pointers to the
					 * data areas to be patched.
					 */

				From	= &FSE -> fse_Type;
				To	= &DevNode -> dn_Type;

					/* The PatchFlags entry is
					 * bitmapped, test the bits
					 * and copy only those
					 * long words which are
					 * really required.
					 */

				for(i = 0 ; i < 32 ; i++)
				{
					if(FSE -> fse_PatchFlags & (1L << i))
						To[i] = From[i];
				}

					/* The DeviceNode structure
					 * is now ready to be used.
					 */

				Success = TRUE;
			}
		}

		/* Note that there is no such `CloseResource'. */
	}

	return(Success);
}

>- Why isn't there a DOS\0 entry for old floppies? Is this handled by a
>  higher version?

The rom filing system will take care of that.

>Basically what I need is a DOS\1 seglist bptr, and I can't get it here!

You won't find any and you won't need any, the rom filing system is
smart enough to handle DOS1 type filing devices.

>    Michael Warner                  cbmaus!phoenix!michael
>    Phoenix MicroTechnologies

--
Olaf Barthel, Brabeckstrasse 35, D-3000 Hannover 71 (MXM, ETG030)


