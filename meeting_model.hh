#ifndef MEETING_MODEL_HH
#define MEETING_MODEL_HH

#include "http_protocol.hh"
#include <string>
#include <set>

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
  bool is_video_on = false;
  bool is_audio_on = false;
  bool is_screen_on = false;
};

class MeetingModelDelegate {
public:
  virtual void JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {}
  virtual void PushMediaComplete(PushMediaResult result, const std::string& msg) {}
};

class MeetingModel {
public:
  MeetingModel();
  
  static MeetingModel& getInstance();
  
  void Register(MeetingModelDelegate* delegate);
  void UnRegister(MeetingModelDelegate* delegate);
  void QuickMeeting();
  void EnableMedia(MediaType media_type, bool enable);
  void JoinMeeting(const QString& meetingId);
  void UserStatus(const QString& meetingId, bool mic, bool video,
                  bool screenShare);
private:
  void NotifyJoinComplete(JoinMeetingResult result, const std::string& msg);
  
private:
  std::set<MeetingModelDelegate*> delegates_;
  std::string current_meeting_id_;
  struct UserStatus self_user_status_;
};

#endif  // MEETING_MODEL_HH
