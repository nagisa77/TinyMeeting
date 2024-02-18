
#ifndef IN_MEETING_CONTROLLER_HH
#define IN_MEETING_CONTROLLER_HH

#include <QString>
#include "meeting_model.hh"
#include "in_meeting_view.hh"

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
