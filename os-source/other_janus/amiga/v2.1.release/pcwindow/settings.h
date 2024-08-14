#include <exec/types.h>

struct PCWSettings {
      BOOL  FullSizeWindow;
      BOOL  BordersOn;
	  BOOL  WorkbenchScreen;
	  ULONG CursorBlinkRate;
	  ULONG Depth;
	  ULONG DisplayTaskPriority;
      BOOL  InterlaceOn;
   };

extern struct PCWSettings LastReadSettings,CurrentSettings,DefaultSettings;
