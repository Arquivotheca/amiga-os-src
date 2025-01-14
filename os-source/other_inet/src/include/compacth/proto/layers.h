��GRAPHICS_LAYERS_H�<graphics/layers.h>�
���*�;
�InitLayers(�Layer_Info*);
�Layer*CreateUpfrontLayer(�Layer_Info*,�BitMap*,�,�,�,�,�,�BitMap*);
�Layer*CreateBehindLayer(�Layer_Info*,�BitMap*,�,�,�,�,�,�BitMap*);
�UpfrontLayer(�Layer_Info*,�Layer*);
�BehindLayer(�Layer_Info*,�Layer*);
�MoveLayer(�Layer_Info*,�Layer*,�,�);
�SizeLayer(�Layer_Info*,�Layer*,�,�);
�ScrollLayer(�Layer_Info*,�Layer*,�,�);
�BeginUpdate(�Layer*);
�EndUpdate(�Layer*,�);
�DeleteLayer(�Layer_Info*,�Layer*);
�LockLayer(�Layer_Info*,�Layer*);
�UnlockLayer(�Layer*);
�LockLayers(�Layer_Info*);
�UnlockLayers(�Layer_Info*);
�LockLayerInfo(�Layer_Info*);
�SwapBitsRastPortClipRect(�RastPort*,�ClipRect*);
�Layer*WhichLayer(�Layer_Info*,�,�);
�UnlockLayerInfo(�Layer_Info*);
�Layer_Info*NewLayerInfo(�);
�DisposeLayerInfo(�Layer_Info*);
�FattenLayerInfo(�Layer_Info*);
�ThinLayerInfo(�Layer_Info*);
�MoveLayerInFrontOf(�Layer*,�Layer*);
�Region*InstallClipRegion(�Layer*,�Region*);�NO_PRAGMAS���InitLayers 1e����UpfrontLayer 30����BehindLayer 36����MoveLayer 3c 109804���SizeLayer 42 109804���ScrollLayer 48 109804���BeginUpdate 4e����EndUpdate 54����DeleteLayer 5a����LockLayer 60����UnlockLayer 66����LockLayers 6c����UnlockLayers 72����LockLayerInfo 78����SwapBitsRastPortClipRect 7e����WhichLayer 84 10803���UnlockLayerInfo 8a����NewLayerInfo 90 0���DisposeLayerInfo 96����FattenLayerInfo 9c����ThinLayerInfo a2����MoveLayerInFrontOf a8����InstallClipRegion ae��