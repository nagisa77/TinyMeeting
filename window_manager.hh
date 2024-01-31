#ifndef WINDOW_MANAGER_HH
#define WINDOW_MANAGER_HH

#include "in_meeting_view.hh"
#include "pre_meeting_view.hh"
#include <memory>

class WindowManager {
public: 
  WindowManager(); 
  static WindowManager& getInstance();
  
  void PushPremeetingView();
  void PopPremeetingView();
  void PushInmeetingView();
  void PopInmeetingView();
private:
  std::shared_ptr<PreMeetingView> pre_meeting_view_;
  std::shared_ptr<InMeetingView> in_meeting_view_;
};

#endif
