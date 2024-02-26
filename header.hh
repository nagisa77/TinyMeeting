#ifndef HEADER_HH
#define HEADER_HH
#include <string> 

#define SEVER_IP "192.168.50.165"

#define COMPANY_NAME "tim"

#define APP_NAME "TinyMeeting"

enum RequestUserStatusResult {
  kRequestUserStatusResultSuccess = 0,
  kRequestUserStatusResultFailed, 
};

enum JoinMeetingResult {
  kJoinMeetingResultSuccess = 0,
  kJoinMeetingResultFailed,
};

enum LeaveMeetingResult {
  kLeaveMeetingResultSuccess = 0,
  kLeaveMeetingResultUserNotExist,
  kLeaveMeetingResultMeetingNotExist,
  kLeaveMeetingResultNetworkError,
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

#endif
