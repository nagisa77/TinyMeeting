#ifndef MEETING_MODEL_HH
#define MEETING_MODEL_HH

#include "http_protocol.hh"
#include <string>
#include <set>
#include <memory>
#include <vector>
#include "stream_pusher.hh"
#include "stream_puller.hh"
#include <QTimer>
#include "header.hh"
#include <QObject>

class MeetingModelDelegate {
public:
  virtual void JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {}
  virtual void LeaveMeetingComplete(LeaveMeetingResult result, const std::string& msg) {}
  virtual void PushMediaComplete(MediaType media_type, PushMediaResult result, const std::string& msg) {}
  virtual void OnUserStatusUpdate(const std::vector<UserStatus>& user_status) {}
};

class MeetingModel : 
public QObject,
public StreamPusherListener, 
public StreamPullerListener
{
public:
  MeetingModel();
  
  static MeetingModel& getInstance();
  
  // subscribe
  void Register(MeetingModelDelegate* delegate);
  void UnRegister(MeetingModelDelegate* delegate);
  
  // handle
  void QuickMeeting(const std::string& userId);
  void EnableMedia(MediaType media_type, bool enable);
  void JoinMeeting(const std::string& userId, const std::string& meetingId);
  void ExitMeeting();
  void HandleUserStatus(const QString& meetingId, bool mic, bool video,
                  bool screenShare);
  
  // getter
  std::string GetMeetingId(); 
  std::string GetSelfUserId();
  std::shared_ptr<StreamPuller> GetStreamPuller(const std::string& user_id, MediaType media_type); 
  
  // StreamPusherListener
  void OnPusherStreamServerError(MediaType media_type) override;
  
  // StreamPullerListener
  void OnPullFrame(const std::string& user_id, MediaType media_type, std::shared_ptr<AVFRAME> frame) override;

private:
  void NotifyJoinComplete(JoinMeetingResult result, const std::string& msg);
  void NotifyLeaveComplete(LeaveMeetingResult result, const std::string& msg);
  void NotifyPushMediaCompelete(MediaType media_type, PushMediaResult result, const std::string& msg);
  void NotifyUserStatusUpdate(const std::vector<UserStatus>& user_status);
  void StartRequestUserStatusTimer(bool enable);
  void SyncUserStatus();
  void LoadUserId();
  void SaveUserId();
  
private:
  std::set<MeetingModelDelegate*> delegates_;
  std::string current_meeting_id_;
  struct UserStatus self_user_status_;
  std::shared_ptr<QTimer> request_user_status_timer_;

  // pusher && puller
  std::map<MediaType, std::shared_ptr<StreamPusher>> stream_pushers_;
  std::map<std::string, std::map<MediaType, std::shared_ptr<StreamPuller>>> stream_pullers_;
  
  std::map<std::string, UserStatus> user_map_;
};

#endif  // MEETING_MODEL_HH
