
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

private:
  InMeetingView* view_ = nullptr;
};

#endif  // IN_MEETING_CONTROLLER_HH
