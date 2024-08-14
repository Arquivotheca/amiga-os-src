
#include <exec/types.h>
#include "memory.h"
#include "s2io.h"
#include "ip.h"
#include "rdp.h"

void test_input();
void test_status();

extern struct MinList DeviceList;
extern struct RDPConnection *OpenActive();
extern struct RDPConnection *OpenPassive();

struct RDPConnection *connection;

void init_test()
{

 struct RDPConnection *ourpassive;
 struct RDPConnection *ouractive;

 if ( ((struct sanadev *) (DeviceList.mlh_Head))->sanadev_IPAddress == 0x87070715 )
   {
   /* Do the passive */
   ourpassive = OpenPassive(500,&test_input,&test_status);
   kprintf("ourpassive = %lx\n",ourpassive);
   connection = ourpassive;
   }
 else
   {
   /* Do the active */
   ouractive = OpenActive(0x87070715,500,&test_input,&test_status);
   connection = ouractive;
   kprintf("ouractive = %lx\n",ouractive);
   }
}

void test_input(c,buff)
struct RDPConnection *c;
struct Buffer *buff;
{

 struct BuffEntry *be;
 UBYTE *data;
 kprintf("Received Buffer %lx on connection %lx\n",c,buff);

// be = (struct BuffEntry *) buff->buff_list.mlh_Head;
 data = (UBYTE *) BuffPointer(buff,0L,&be);
 kprintf("String is %s %lx\n",data,DataLength(buff));
 FreeBuffer(buff);
 kprintf("Freed it\n");
 kprintf("Trying to CLOSE this sucker.\n");
 CloseConnection(c);

}

void test_status(conn)
struct RDPConnection *conn;
{

 if (conn->conn_State == STATE_OPEN)
   if ( ((struct sanadev *) (DeviceList.mlh_Head))->sanadev_IPAddress == 0x87070715 )
      rdp_output(conn,"Here's a test RDP message",26);

 if (conn->conn_State == STATE_CLOSE)
   {
   CloseConnection(conn);
   kprintf("Closing the connection\n");
   }

 kprintf("Connection %lx state is now %lx\n",conn,conn->conn_State);

}

