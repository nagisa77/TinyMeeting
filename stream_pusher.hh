#ifndef STREAM_PUSHER_HH
#define STREAM_PUSHER_HH

#include "media_capture/camera_capture.hh"
#include "utils/blocking_queue.hh"
#include <string>
#include <boost/thread.hpp>
#include <QObject>

class StreamPusher;
class StreamPusherListener {
public:
  virtual void OnStreamServerError(StreamPusher* pusher) = 0;
};

class StreamPusher :
public QObject,
public CameraCaptureListener {
  Q_OBJECT
public:
  StreamPusher(const std::string& stream_id, const std::string& ip, int port);
  ~StreamPusher(); 
  void OnCameraFrame(std::shared_ptr<AVFRAME> frame) override;
  int CodecFrameToServer(); 
  void RegisterListener(StreamPusherListener* listener);
  void UnRegisterListener(StreamPusherListener* listener);
  
signals:
  void ShouldStopPushing();
  
public slots:
  void OnShouldStopPushing();
  
private:
  BlockingQueue<std::shared_ptr<AVFRAME>> frame_queue_; 
  std::string stream_id_;
  std::string ip_;
  int port_;
  std::unique_ptr<boost::thread> codec_thread_;
  StreamPusherListener* listener_ = nullptr;
};

#endif // STREAM_PUSHER_HH 
