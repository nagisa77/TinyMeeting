#ifndef STREAM_PUSHER_HH
#define STREAM_PUSHER_HH

#include "media_capture/camera_capture.hh"
#include "utils/blocking_queue.hh"
#include <string>

class StreamPusher : public CameraCaptureListener {
public:
  StreamPusher(const std::string& stream_id, const std::string& ip, int port); 
  void OnCameraFrame(std::shared_ptr<AVFRAME> frame) override;
  int CodecFrameToServer(); 
  
private:
  BlockingQueue<std::shared_ptr<AVFRAME>> frame_queue_; 
  std::string stream_id_;
  std::string ip_;
  int port_;
};

#endif // STREAM_PUSHER_HH 
