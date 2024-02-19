
#ifndef IN_MEETING_CONTROLLER_HH
#define IN_MEETING_CONTROLLER_HH

#include <QString>
#include "meeting_model.hh"
#include "in_meeting_view.hh"

class VideoViewController : public StreamPullerListener {
public:
  VideoViewController(std::shared_ptr<StreamPuller> puller, VideoView* view);
  ~VideoViewController();
  void OnPullFrame(const std::string& user_id, MediaType media_type, std::shared_ptr<AVFRAME> frame) override;
  
private:
  VideoView* view_ = nullptr;
  std::shared_ptr<StreamPuller> puller_; 
};

class InMeetingController : public MeetingModelDelegate {
 public:
  InMeetingController(InMeetingView* view);
  ~InMeetingController();
  
  void HandleVideoClick(); 
  void PushMediaComplete(MediaType media_type, PushMediaResult result, const std::string& msg) override;
  void OnUserStatusUpdate(const std::vector<UserStatus>& user_status) override;

private:
  InMeetingView* view_ = nullptr;
};

#endif  // IN_MEETING_CONTROLLER_HH
