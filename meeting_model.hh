#ifndef MEETING_MODEL_HH
#define MEETING_MODEL_HH

#include "http_protocol.hh"
#include <string>
#include <set>
#include <memory>
#include <vector>
#include "stream_pusher.hh"
#include <QTimer>
#include "header.hh"
#include <QObject>

class MeetingModelDelegate {
public:
  virtual void JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {}
  virtual void PushMediaComplete(MediaType media_type, PushMediaResult result, const std::string& msg) {}
  virtual void OnUserStatusUpdate(std::vector<UserStatus> user_status) {}
};

class MeetingModel : 
public QObject,
public StreamPusherListener {
public:
  MeetingModel();
  
  static MeetingModel& getInstance();
  
  void Register(MeetingModelDelegate* delegate);
  void UnRegister(MeetingModelDelegate* delegate);
  void QuickMeeting();
  void EnableMedia(MediaType media_type, bool enable);
  void JoinMeeting(const QString& meetingId);
  void HandleUserStatus(const QString& meetingId, bool mic, bool video,
                  bool screenShare);
  
  // StreamPusherListener
  void OnStreamServerError(StreamPusher* pusher) override;

private:
  void NotifyJoinComplete(JoinMeetingResult result, const std::string& msg);
  void NotifyPushMediaCompelete(MediaType media_type, PushMediaResult result, const std::string& msg);
  void StartRequestUserStatusTimer(bool enable);
  void SyncUserStatus();
  
private:
  std::set<MeetingModelDelegate*> delegates_;
  std::string current_meeting_id_;
  struct UserStatus self_user_status_;
  std::shared_ptr<QTimer> request_user_status_timer_;

  std::shared_ptr<StreamPusher> camera_pusher_;
  std::map<std::string, UserStatus> user_map_;
};

#endif  // MEETING_MODEL_HH
