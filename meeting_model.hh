#ifndef MEETING_MODEL_HH
#define MEETING_MODEL_HH

#include "http_protocol.hh"
#include <string>
#include <set>
#include <memory>
#include <vector>
#include "stream_pusher.hh"
#include <QTimer>
#include <QObject>

enum RequestUserStatusResult {
  kRequestUserStatusResultSuccess = 0,
  kRequestUserStatusResultFailed, 
};

enum JoinMeetingResult {
  kJoinMeetingResultSuccess = 0,
  kJoinMeetingResultFailed,
};

enum PushMediaResult {
  kPushMediaResultSuccess = 0,
  kPushMediaResultFailed,
};

enum MediaType {
  kMediaTypeVideo = 0,
  kMediaTypeAudio,
  kMediaTypeScreen,
};

struct UserStatus {
  std::string user_id;
  bool is_video_on = false;
  bool is_audio_on = false;
  bool is_screen_on = false;
  std::string video_stream_id;
  std::string audio_stream_id;
  std::string screen_stream_id;
  
  bool operator==(const UserStatus& other) const {
    return user_id == other.user_id &&
    is_video_on == other.is_video_on &&
    is_audio_on == other.is_audio_on &&
    is_screen_on == other.is_screen_on &&
    video_stream_id == other.video_stream_id &&
    audio_stream_id == other.audio_stream_id &&
    screen_stream_id == other.screen_stream_id;
  }
};

class MeetingModelDelegate {
public:
  virtual void JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {}
  virtual void PushMediaComplete(PushMediaResult result, const std::string& msg) {}
  virtual void OnUserStatusUpdate(std::vector<UserStatus> user_status) {}
};

class MeetingModel : public QObject {
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
private:
  void NotifyJoinComplete(JoinMeetingResult result, const std::string& msg);
  void StartRequestUserStatusTimer(bool enable);
  
private:
  std::set<MeetingModelDelegate*> delegates_;
  std::string current_meeting_id_;
  struct UserStatus self_user_status_;
  std::shared_ptr<QTimer> request_user_status_timer_;

  std::shared_ptr<StreamPusher> camera_pusher_;
  std::map<std::string, UserStatus> user_map_;
};

#endif  // MEETING_MODEL_HH
