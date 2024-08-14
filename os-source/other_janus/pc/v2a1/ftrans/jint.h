#define JANUS				0x0b
#define JANUS_INT			JANUS		/* kludge to mesh with 500/2000 manual */
											/* funtion codes */
#define J_GET_SERVICE	0
#define J_GET_BASE		1
#define J_ALLOC_MEM		2
#define J_FREE_MEM		3
#define J_SET_PARAM		4
#define J_SET_SERVICE	5
#define J_STOP_SERVICE	6
#define J_CALL_AMIGA		7
#define J_WAIT_AMIGA		8
#define J_CHECK_AMIGA	9
											/* status returns	*/
#define J_NO_SERVICE		0xff
#define J_PENDING			0
#define J_FINISHED		1
#define J_OK				0
#define J_NO_MEMORY		3
#define J_ILL_FNCTN		4

 
#define AMIGAFUN 16
 
unsigned char j_amiga(unsigned char); 
unsigned char amigainit();
short amiga(unsigned short,unsigned short);
 
