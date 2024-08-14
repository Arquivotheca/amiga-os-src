	INCLUDE	"exec/types.i"
	INCLUDE	"exec/libraries.i"

 STRUCTURE	BulletLibrary,LIB_SIZE
    LONG    bl_Segment
    APTR    bl_SysBase
    APTR    bl_DOSBase
    APTR    bl_UtilityBase
    LABEL   bl_SIZEOF
