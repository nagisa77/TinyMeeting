#ifndef FRAME_H
#define FRAME_H

//#define AVFRAME void*

struct AVFRAME {
  AVFRAME();
  ~AVFRAME();
  void* frame_ = nullptr;
};

void* CMSampleBufferRefToAVFRAME(void* ref);

void ReleaseAVFRAME(void* frame);

#endif
