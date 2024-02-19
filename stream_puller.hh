#ifndef STREAM_PULLER_HH
#define STREAM_PULLER_HH

#include "media_capture/camera_capture.hh"
#include "utils/blocking_queue.hh"
#include <string>
#include <boost/thread.hpp>
#include <QObject>
#include "media_capture/frame.hh"

class StreamPullerListener {
public:
  virtual void OnPullFrame(const std::string& stream_id, std::shared_ptr<AVFRAME> frame);
};

class StreamPuller :
public QObject {
  Q_OBJECT
public:
  StreamPuller(const std::string& stream_id, const std::string& ip, int port);
  ~StreamPuller(); 

  int CodecFrameFromServer(); 
  void RegisterListener(StreamPullerListener* listener);
  void UnRegisterListener(StreamPullerListener* listener);

private:
  std::string stream_id_;
  std::string ip_;
  int port_;
  
  StreamPullerListener* listener_ = nullptr;
};

#endif // STREAM_PUSHER_HH 
