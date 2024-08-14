/* scsi.h */

#define S_TEST_UNIT_READY	0x00
#define S_REZERO_UNIT		0x01
#define S_REQUEST_SENSE		0x03
#define S_FORMAT_UNIT		0x04
#define S_REASSIGN_BLOCKS	0x07
#define S_READ			0x08
#define S_WRITE			0x0a
#define S_SEEK			0x0b
#define S_INQUIRY		0x12
#define S_MODE_SELECT		0x15
#define S_RESERVE		0x16
#define S_RELEASE		0x17
#define S_COPY			0x18
#define S_MODE_SENSE		0x1a
#define S_START_STOP_UNIT	0x1b
#define S_PREVENT_ALLOW_REMOVAL	0x1e
#define S_READ_CAPACITY 	0x25
#define S_READ10		0x28
#define S_WRITE10		0x2a
#define S_WRITE_VERIFY		0x2e
#define S_VERIFY		0x2F
#define S_PREFETCH		0x34
#define S_SYNCHRONIZE_CACHE	0x35
#define	S_LOCK_UNLOCK_CACHE	0x36
#define S_READ_DEFECT_DATA	0x37
#define S_WRITE_BUFFER		0x3b
#define S_READ_BUFFER		0x3c
#define S_MODE_SELECT_10	0x55
#define S_MODE_SENSE_10		0x5a

/* sense codes */
#define CHECK_CONDITION		0x02

/* sense keys */
#define RECOVERED_ERROR		0x01
#define MEDIUM_ERROR		0x03
#define HARDWARE_ERROR		0x04
#define ILLEGAL_REQUEST		0x05
