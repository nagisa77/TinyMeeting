
#ifndef CAMERA_CAPTURE_H
#define CAMERA_CAPTURE_H

#include <set>

#include "frame.hh"

class CameraCaptureListener {
public:
  virtual ~CameraCaptureListener() {}
  virtual void OnCameraFrame(AVFRAME frame) = 0;
};

class CameraCapture {
public:
  void Start(bool enable);
  void Register(CameraCaptureListener* listener);
  void UnRegister(CameraCaptureListener* listener);
  static CameraCapture* getInstance();
  void InjectFrame(AVFRAME frame);

private:
  CameraCapture();
  ~CameraCapture();

  void StartCapture();
  void StopCapture();

  void* capture_session_ = nullptr;
  std::set<CameraCaptureListener*> listeners_;
  bool capture_running_ = false;
};

#endif // CAMERA_CAPTURE_H
