#ifndef MEETING_MODEL_HH
#define MEETING_MODEL_HH

#include "http_protocol.hh"
#include <string>
#include <set>

enum JoinMeetingResult {
  kJoinMeetingResultSuccess = 0,
  kJoinMeetingResultFailed,
};

class MeetingModelDelegate {
public:
  virtual void JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {}
};

class MeetingModel {
 public:
  MeetingModel();

  static MeetingModel& getInstance();

  void Register(MeetingModelDelegate* delegate);
  void UnRegister(MeetingModelDelegate* delegate);
  void QuickMeeting();
  void JoinMeeting(const QString& meetingId);
  void UserStatus(const QString& meetingId, bool mic, bool video,
                  bool screenShare);
  
private:
  std::set<MeetingModelDelegate*> delegates_;
};

#endif  // MEETING_MODEL_HH
