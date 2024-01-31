#ifndef FRAME_H
#define FRAME_H

#define AVFRAME void*

AVFRAME CMSampleBufferRefToAVFRAME(void* ref);

void ReleaseAVFRAME(AVFRAME frame);

#endif
