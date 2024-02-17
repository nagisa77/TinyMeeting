#ifndef PRE_MEETING_CONTROLLER_HH
#define PRE_MEETING_CONTROLLER_HH

#include <QString>
#include "meeting_model.hh"
#include "pre_meeting_view.hh"

class PreMeetingController : public MeetingModelDelegate {
 public:
  PreMeetingController(PreMeetingView* view);
  ~PreMeetingController();

  void HandleQuickMeeting(const std::string& user_id);
  void JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) override;

private:
  PreMeetingView* view_ = nullptr;
};

#endif  // PRE_MEETING_CONTROLLER_HH
