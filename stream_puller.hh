#ifndef STREAM_PULLER_HH
#define STREAM_PULLER_HH

#include "media_capture/camera_capture.hh"
#include "utils/blocking_queue.hh"
#include <string>
#include <boost/thread.hpp>
#include <QObject>


class StreamPuller :
public QObject {
  Q_OBJECT
public:
  StreamPuller(const std::string& stream_id, const std::string& ip, int port);
  ~StreamPuller(); 

  int CodecFrameFromServer(); 

private:
  std::string stream_id_;
  std::string ip_;
  int port_;
};

#endif // STREAM_PUSHER_HH 
