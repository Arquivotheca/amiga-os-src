
#include <exec/nodes.h>
#include <exec/lists.h>


struct BuffEntry
   {
   struct MinNode be_link;
   UWORD be_offset;
   UWORD be_length;
   UWORD be_physicallength;
   UBYTE *be_data;
   };

struct Buffer
   {
   struct MinNode buff_link;
   struct MinList buff_list;
   UWORD          buff_timeout;
   };


