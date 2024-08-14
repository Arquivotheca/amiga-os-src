€ˆGRAPHICS_LAYERS_H€GRAPHICS_LAYERS_Hˆ·Œ<exec/lists.h>‡ˆEXEC_SEMAPHORES_HŒ<exec/semaphores.h>‡€LAYERSIMPLE 1€LAYERSMART 2€LAYERSUPER 4€LAYERUPDATING 16€LAYERBACKDROP 64€LAYERREFRESH 128€LAYER_CLIPRECTS_LOST 256€LMN_REGION -1
ƒLayer_Info
{
ƒLayer*top_layer;
ƒLayer*check_lp;
ƒLayer*obs;
ƒMinList FreeClipRects;
ƒSignalSemaphore Lock;
ƒ®gs_Head;
’longreserved;
‰¦;
šfatten_count;
šLockLayersCount;
‰LayerInfo_extra_size;
˜*blitbuff;
ƒLayerInfo_extra*LayerInfo_extra;
};€NEWLAYERINFO_CALLED 1€ALERTLAYERSNOMEM 0x83010000‡