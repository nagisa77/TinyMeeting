#include "frame.hh"
extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#import <CoreMedia/CoreMedia.h>
#include "spdlog/spdlog.h"

AVFRAME::AVFRAME() {
}

AVFRAME::~AVFRAME() {
  if (frame_) {
    ReleaseAVFRAME(frame_);
  }
}

void* CMSampleBufferRefToAVFRAME(void* ref) {
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
  
  if (avPixelFormat == AV_PIX_FMT_UYVY422) {
    static struct SwsContext* sws_ctx = sws_getContext(
                                                frame->width, frame->height, AV_PIX_FMT_UYVY422,    // 源格式
                                                frame->width, frame->height, AV_PIX_FMT_YUV420P,    // 目标格式
                                                SWS_BILINEAR, NULL, NULL, NULL);
    if (!sws_ctx) {
      fprintf(stderr, "Could not initialize the conversion context\n");
      exit(1);
    }
    
    AVFrame* converted_frame = av_frame_alloc();
    if (!converted_frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    converted_frame->format = AV_PIX_FMT_YUV420P;
    converted_frame->width  = frame->width;
    converted_frame->height = frame->height;

    // 为转换后的帧分配空间
    av_image_alloc(converted_frame->data, converted_frame->linesize, frame->width, frame->height, AV_PIX_FMT_YUV420P, 16);

    // 执行转换
    sws_scale(sws_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, frame->height, converted_frame->data, converted_frame->linesize);
    
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    
    // 应在此处释放原始frame的图像数据内存
    av_freep(&frame->data[0]); // 释放由av_image_alloc分配的内存
    av_frame_free(&frame);
    return converted_frame;
  }


  CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
  return frame;
}

void ReleaseAVFRAME(void* frame) {
  if (!frame) {
    return;
  }
  AVFrame* av_frame = (AVFrame*)frame;
  // 应在此处释放原始frame的图像数据内存
  av_freep(&av_frame->data[0]); // 释放由av_image_alloc分配的内存
  av_frame_free(&av_frame);
}
