        TTL    '$Id: device.asm,v 36.2 90/08/29 17:52:27 mks Exp $'
**********************************************************************
*                                                                    *
*   Copyright 1986, Commodore-Amiga Inc.   All rights reserved.      *
*   No part of this program may be reproduced, transmitted,          *
*   transcribed, stored in retrieval system, or translated into      *
*   any language or computer language, in any form or by any         *
*   means, electronic, mechanical, magnetic, optical, chemical,      *
*   manual or otherwise, without the prior written permission of     *
*   Commodore-Amiga Incorporated, 983 University Ave. Building #D,   *
*   Los Gatos, California, 95030                                     *
*                                                                    *
**********************************************************************
*
* $Id: device.asm,v 36.2 90/08/29 17:52:27 mks Exp $
*
* $Locker:  $
*
* $Log:	device.asm,v $
* Revision 36.2  90/08/29  17:52:27  mks
* Changed revision to 36...  Change ownership to MKS...
* Changed $Header to $Id
* Removed the /command/ in the autodoc titles...
* 
* Revision 33.1  86/05/05  18:00:08  sam
* minor changes to audiodocs
*
* Revision 32.1  86/01/14  21:22:46  sam
* revision set to 32
*
* Revision 1.1  86/01/14  20:48:00  sam
* Initial revision
*
*
**********************************************************************

        SECTION         audio

*------ Included Files -----------------------------------------------

        INCLUDE         "exec/types.i"
        INCLUDE         "exec/nodes.i"
        INCLUDE         "exec/lists.i"
        INCLUDE         "exec/memory.i"
        INCLUDE         "exec/ports.i"
        INCLUDE         "exec/libraries.i"
        INCLUDE         "exec/devices.i"
        INCLUDE         "exec/tasks.i"
        INCLUDE         "exec/io.i"
        INCLUDE         "exec/interrupts.i"
        INCLUDE         "exec/errors.i"
        INCLUDE         "exec/initializers.i"
        INCLUDE         "hardware/custom.i"

        INCLUDE         "macros.i"
        INCLUDE         "audio.i"
        INCLUDE         "device.i"

*------ Imported Functions -------------------------------------------

        XREF            _CADOpen
        XREF            _CADClose
        XREF            _CADExpunge
        XREF            _CADBeginIO
        XREF            _CADAbortIO

*------ Exported Functions -------------------------------------------

        XDEF            ADOpen
        XDEF            ADClose
        XDEF            ADExpunge
        XDEF            ADExtFunc
        XDEF            ADBeginIO
        XDEF            ADAbortIO


******* audio.device/OpenDevice **********************************************
*
*   NAME
*       OpenDevice - open the audio device
*
*   SYNOPSIS
*       error = OpenDevice("audio.device", unitNumber, iORequest, flags);
*
*   FUNCTION
*       The OpenDevice routine grants access to the audio device.  It takes an
*       I/O audio request block (iORequest) and if it can successfully open
*       the audio device, it loads the device pointer (io_Device) and the
*       allocation key (ioa_AllocKey); otherwise, it returns an error
*       (IOERR_OPENFAIL).  OpenDevice increments the open count keeping the
*       device from being expunged (Expunge).  If the length (ioa_Length) is
*       non-zero, OpenDevice tries to allocate (ADCMD_ALLOCATE) audio channels
*       from a array of channel combination options (ioa_Data). If the
*       allocation succeeds, the allocated channel combination is loaded into
*       the unit field (ioa_Unit); otherwise, OpenDevice returns an error
*       (ADIOERR_ALLOCFAILED).  OpenDevice does not wait for allocation to
*       succeed and closes (CloseDevice) the audio device if it fails.  To
*       allocate channels, OpenDevice also requires a properly initialized
*       reply port (mn_ReplyPort) with an allocated signal bit.
*
*   INPUTS
*       unitNumber- not used
*       iORequest - pointer to audio request block (struct IOAudio)
*               ln_Pri      - allocation precedence (-128 thru 127), only
*                             necessary for allocation (non-zero length)
*               mn_ReplyPort- pointer to message port for allocation, only
*                             necessary for allocation (non-zero length)
*               ioa_AllocKey- allocation key; zero to generate new key.
*                             Otherwise, it must be set by (or copied from I/O
*                             block that is set by) previous OpenDevice
*                             function or ADCMD_ALLOCATE command (non-zero
*                             length)
*               ioa_Data    - pointer to channel combination options (byte
*                             array, bits 0 thru 3 correspond to channels 0
*                             thru 3), only necessary for allocation (non-zero
*                             length)
*               ioa_Length  - length of the channel combination option array
*                             (0 thru 16), zero for no allocation
*       flags     - not used
*
*   OUTPUTS
*       iORequest - pointer to audio request block (struct IOAudio)
*               io_Device   - pointer to device node if OpenDevice succeeds,
*                             otherwise -1
*               io_Unit     - bit map of successfully allocated channels (bits
*                             0 thru 3 correspond to channels 0 thru 3)
*               io_Error    - error number:
*                             0                   - no error
*                             IOERR_OPENFAIL      - open failed
*                             ADIOERR_ALLOCFAILED - allocation failed, no open
*               ioa_AllocKey- allocation key, set to a unique number if passed
*                             a zero and OpenDevice succeeds
*       error     - copy of io_Error
*
******************************************************************************
ADOpen:
                MOVEM.L D1/A6,-(SP)
                MOVEM.L D0/A1,-(SP)
                BSR     _CADOpen
                LEA     16(SP),SP
                RTS


******* audio.device/CloseDevice *********************************************
*
*   NAME
*       CloseDevice - terminate access to the audio device
*
*   SYNOPSIS
*       CloseDevice(iORequest);
*                       A1
*
*   FUNCTION
*       The CloseDevice routine notifies the audio device that it will no
*       longer be used.  It takes an I/O audio request block (IOAudio) and
*       clears the device pointer (io_Device).  If there are any channels
*       allocated with the same allocation key (ioa_AllocKey), CloseDevice
*       frees (ADCMD_FREE) them. CloseDevice decrements the open count, and if
*       it falls to zero and an expunge (Expunge) is pending, the device is
*       expunged.
*
*   INPUTS
*       iORequest   - pointer to audio request block (struct IOAudio)
*               io_Device   - pointer to device node, must be set by (or
*                             copied from I/O block set by) open (OpenDevice)
*               io_Unit     - bit map of channels to free (ADCMD_FREE) (bits 0
*                             thru 3 correspond to channels 0 thru 3)
*               ioa_AllocKey- allocation key, used to free channels
*
*   OUTPUTS
*       iORequest - pointer to audio request block (struct IOAudio)
*               io_Device   - set to -1
*               io_Unit     - set to zero
*
******************************************************************************
ADClose:
                MOVEM.L A1/A6,-(SP)
                BSR     _CADClose
                ADDQ.L  #8,SP
                RTS


******* audio.device/Expunge *************************************************
*
*   NAME
*       EXPUNGE - indicate a desire to remove the Audio device
*
*   FUNCTION
*       The Expunge routine is called when a user issues a RemDevice call.  By
*       the time it is called, the device has already been removed from the
*       device list, so no new opens will succeed.  The existence of any other
*       users of the device, as determined by the device open count being
*       non-zero, will cause the Expunge to be deferred.  When the device is
*       not in use, or no longer in use, the Expunge is actually performed.
*
******************************************************************************
ADExpunge:
                MOVE.L  A6,-(SP)
                BSR     _CADExpunge
                ADDQ.L  #4,SP
ADExtFunc:
                RTS


******* audio.device/BeginIO *************************************************
*
*   NAME
*       BeginIO - dispatch a device command
*
*   SYNOPSIS
*       BeginIO(iORequest);
*                   A1
*
*   FUNCTION
*       BeginIO has the responsibility of dispatching all device commands.
*       Immediate commands are always called directly, and all other commands
*       are queued to make them single threaded.
*
*   INPUTS
*       iORequest -- pointer to the I/O Request for this command
*
******************************************************************************
ADBeginIO:
                MOVEM.L A1/A6,-(SP)
                BSR     _CADBeginIO
                ADDQ.L  #8,SP
                RTS


******* audio.device/AbortIO *************************************************
*
*   NAME
*       AbortIO - abort a device command
*
*   SYNOPSIS
*       AbortIO(iORequest);
*                   A1
*
*   FUNCTION
*       AbortIO tries to abort a device command.  It is allowed to be
*       unsuccessful.  If the Abort is successful, the io_Error field of the
*       iORequest contains an indication that IO was aborted.
*
*   INPUTS
*       iORequest -- pointer to the I/O Request for the command to abort
*
******************************************************************************
ADAbortIO:
                MOVEM.L A1/A6,-(SP)
                BSR     _CADAbortIO
                ADDQ.L  #8,SP
                RTS

                END
