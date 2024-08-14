#include "ALINKHDR.h"

word mypointword ( mymark )
word mymark;
{
   if ( simplepoint == 1L )
   {
      word diff = mymark - filemark;
      i_ptr = i_ptr + diff;
      if ( (  - 1L  <= i_ptr ) && ( i_ptr < i_end ) )
      {
         filemark = mymark;
         return 1L;
      }
   }
   filemark = mymark;
   pointword ( mymark );
   F001E_replenish_input (  );
   simplepoint = 1L;
   return 1L;
}
