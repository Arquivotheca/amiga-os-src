/* The autoload buffer looks like this:
 *    - First there is a 16-bit value describing how many 
 *      AutoEntry structures follow
 *    - Then there are a number of AutoEntry structures
 * To find a free AutoEntry structure, check the AppID field.  
 * If this is zero, you can use this field
 */

#define AUTOLOAD_ENTRYCOUNT   4   /* Number of entries in buffer */

#define AUTOLOAD_APPID        123
#define AUTOLOAD_LOCALID      2


struct   AutoEntry
   {
   ULONG    AppID;
   USHORT   LocalID;
   };

struct AutoLoad {
   USHORT Entries;
   struct AutoEntry Entry[AUTOLOAD_ENTRYCOUNT];
   };
