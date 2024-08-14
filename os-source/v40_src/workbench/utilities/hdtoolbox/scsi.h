/* scsi.h */

#define S_FORMAT_UNIT		0x04
#define S_MODE_SELECT		0x15
#define S_MODE_SENSE		0x1a
#define S_READ			0x08
#define S_WRITE			0x0a
#define S_READ10		0x28
#define S_WRITE10		0x2a
#define S_READ_CAPACITY 	0x25
#define S_INQUIRY		0x12
#define S_VERIFY		0x2F
#define S_READ_DEFECT_DATA	0x37
#define S_REASSIGN_BLOCKS	0x07

/* sense keys */
#define RECOVERED_ERROR		0x01
#define MEDIUM_ERROR		0x03
#define HARDWARE_ERROR		0x04
#define ILLEGAL_REQUEST		0x05
