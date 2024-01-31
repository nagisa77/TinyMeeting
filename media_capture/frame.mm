#include "frame.hh"
extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}
#import <CoreMedia/CoreMedia.h>

AVFRAME::AVFRAME() {
}

AVFRAME::~AVFRAME() {
  if (frame_) {
    ReleaseAVFRAME(frame_);
  }
}

AVFRAME CMSampleBufferRefToAVFRAME(void* ref) {
  CMSampleBufferRef sampleBuffer = (CMSampleBufferRef)ref;
  if (!sampleBuffer) {
    return AVFRAME();
  }

  CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
  if (!imageBuffer) {
    return AVFRAME();
  }

  CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
  void* baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);

  size_t width = CVPixelBufferGetWidth(imageBuffer);
  size_t height = CVPixelBufferGetHeight(imageBuffer);
  size_t stride = CVPixelBufferGetBytesPerRow(imageBuffer);

  AVFrame* frame = av_frame_alloc();
  if (!frame) {
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    return AVFRAME();
  }

  OSType pixelFormat = CVPixelBufferGetPixelFormatType(imageBuffer);
  AVPixelFormat avPixelFormat;
  switch (pixelFormat) {
  case kCVPixelFormatType_422YpCbCr8: // AV_PIX_FMT_UYVY422
    avPixelFormat = AV_PIX_FMT_UYVY422;
    break;
  case kCVPixelFormatType_32BGRA:
    avPixelFormat = AV_PIX_FMT_BGRA;
    break;
  default:
    av_frame_free(&frame);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    return AVFRAME();
  }

  frame->format = avPixelFormat;
  frame->width = static_cast<int>(width);
  frame->height = static_cast<int>(height);

  if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
                     avPixelFormat, 1) < 0) {
    av_frame_free(&frame);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    return AVFRAME();
  }

  if (avPixelFormat == AV_PIX_FMT_UYVY422 ||
      avPixelFormat == AV_PIX_FMT_BGRA) {
    uint8_t* src = (uint8_t*)baseAddress;
    for (int i = 0; i < height; ++i) {
      memcpy(frame->data[0] + i * frame->linesize[0], src + i * stride, stride);
    }
  }

  CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
  AVFRAME f;
  f.frame_ = frame;
  return f;
}

void ReleaseAVFRAME(void* frame) {
  if (!frame) {
    return;
  }
  AVFrame* av_frame = (AVFrame*)frame;
  av_frame_free(&av_frame);
}
