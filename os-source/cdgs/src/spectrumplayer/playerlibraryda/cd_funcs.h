
void  GetDiskInfo(struct V *V);
int   GetQCodePacket(struct V *V);
int   GetValidQCode(struct V *V);
void  SkipTrack(UBYTE KeyPress, struct V *V);
UBYTE PlayNextPrevTrack(struct V *V);
void  Pause(struct V *V);
void  Resume(struct V *V);
void  FastForward(struct V *V);
void  FastReverse(struct V *V);
void  NormalPlay(struct V *V);
void  Play(struct V *V);
void  AbortPlay(struct V *V);
void  SeekPaused(struct V *V);
void  Stop(struct V *V);
void  InstallSampler(struct V *V);
void  RemoveSampler(struct V *V);