#define ID_EFSC     MAKE_ID('E','F','S','C')
#define ID_VOLM     MAKE_ID('V','O','L','M')

#define IFFPrefChunkCnt     1

static LONG far IffPrefChunks[]={ID_EFSC,ID_VOLM};
struct Library *IFFParseBase;

