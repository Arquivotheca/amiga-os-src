/* "camd.library"*/
/**/
/* --------------------- Locks*/
/**/
#pragma libcall CamdBase LockCAMD 1E 001
#pragma libcall CamdBase UnlockCAMD 24 801
/**/
/* --------------------- MidiNode*/
/**/
#pragma libcall CamdBase CreateMidiA 2A 801
#pragma libcall CamdBase DeleteMidi 30 801
#pragma libcall CamdBase SetMidiAttrsA 36 9802
#pragma libcall CamdBase GetMidiAttrsA 3C 9802
#pragma libcall CamdBase NextMidi 42 801
#pragma libcall CamdBase FindMidi 48 901
#pragma libcall CamdBase FlushMidi 4E 801
/**/
/* --------------------- MidiLink*/
/**/
#pragma libcall CamdBase AddMidiLinkA 54 90803
#pragma libcall CamdBase RemoveMidiLink 5A 801
#pragma libcall CamdBase SetMidiLinkAttrsA 60 9802
#pragma libcall CamdBase GetMidiLinkAttrsA 66 9802
#pragma libcall CamdBase NextClusterLink 6C 09803
#pragma libcall CamdBase NextMidiLink 72 09803
#pragma libcall CamdBase MidiLinkConnected 78 801
/**/
/* --------------------- MidiCluster*/
/**/
#pragma libcall CamdBase NextCluster 7E 801
#pragma libcall CamdBase FindCluster 84 801
/**/
/* --------------------- Message*/
/**/
#pragma libcall CamdBase PutMidi 8A 0802
#pragma libcall CamdBase GetMidi 90 9802
#pragma libcall CamdBase WaitMidi 96 9802
#pragma libcall CamdBase PutSysEx 9C 9802
#pragma libcall CamdBase GetSysEx A2 09803
#pragma libcall CamdBase QuerySysEx A8 801
#pragma libcall CamdBase SkipSysEx AE 801
#pragma libcall CamdBase GetMidiErr B4 801
#pragma libcall CamdBase MidiMsgType BA 801
#pragma libcall CamdBase MidiMsgLen C0 001
#pragma libcall CamdBase ParseMidi C6 09803
/**/
/* --------------------- Device*/
/**/
#pragma libcall CamdBase OpenMidiDevice CC 801
#pragma libcall CamdBase CloseMidiDevice D2 801
/**/
/* --------------------- External functions*/
/**/
#pragma libcall CamdBase RethinkCAMD D8 00
#pragma libcall CamdBase StartClusterNotify DE 801
#pragma libcall CamdBase EndClusterNotify E4 801
/*pragma libcall CamdBase PutMidiNoWait EA 9802*/
/**/
