#include "frame.hh"
extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}
#import <CoreMedia/CoreMedia.h>

AVFRAME CMSampleBufferRefToAVFRAME(void* ref) {
  CMSampleBufferRef sampleBuffer = (CMSampleBufferRef)ref;
  if (!sampleBuffer) {
    return nullptr;
  }

  CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
  if (!imageBuffer) {
    return nullptr;
  }

  CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
  void* baseAddress = CVPixelBufferGetBaseAddress(imageBuffer);

  size_t width = CVPixelBufferGetWidth(imageBuffer);
  size_t height = CVPixelBufferGetHeight(imageBuffer);
  size_t stride = CVPixelBufferGetBytesPerRow(imageBuffer);

  AVFrame* frame = av_frame_alloc();
  if (!frame) {
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    return nullptr;
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
    return nullptr;
  }

  frame->format = avPixelFormat;
  frame->width = static_cast<int>(width);
  frame->height = static_cast<int>(height);

  if (av_image_alloc(frame->data, frame->linesize, frame->width, frame->height,
                     avPixelFormat, 1) < 0) {
    av_frame_free(&frame);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    return nullptr;
  }

  if (avPixelFormat == AV_PIX_FMT_UYVY422 ||
      avPixelFormat == AV_PIX_FMT_BGRA) {
    uint8_t* src = (uint8_t*)baseAddress;
    for (int i = 0; i < height; ++i) {
      memcpy(frame->data[0] + i * frame->linesize[0], src + i * stride, stride);
    }
  }

  CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
  return frame;
}

void ReleaseAVFRAME(AVFRAME frame) {
  if (!frame) {
    return;
  }
  AVFrame* av_frame = (AVFrame*)frame;
  av_frame_free(&av_frame);
}
