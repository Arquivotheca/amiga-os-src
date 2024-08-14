€ˆLIBRARIES_ROMBOOT_BASE_H€LIBRARIES_ROMBOOT_BASE_HˆEXEC_TYPES_HŒ<exec/types.h>‡ˆµŒ<exec/nodes.h>‡ˆ·Œ<exec/lists.h>‡ˆEXEC_LIBRARIES_HŒ<exec/libraries.h>‡ˆEXEC_EXECBASE_HŒ<exec/execbase.h>‡ˆEXEC_EXECNAME_HŒ<exec/execname.h>‡
ƒRomBootBase
{
ƒLibrary LibNode;
ƒExecBase*ExecBase;
ƒ®BootList;
—Reserved[4];
};
ƒBootNode
{
ƒ¬bn_Node;
‰bn_Flags;
CPTR bn_DeviceNode;
};€ROMBOOT_NAME "romboot.library"‡