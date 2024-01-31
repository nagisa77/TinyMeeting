
#include "window_manager.hh"

WindowManager::WindowManager() {}

WindowManager& WindowManager::getInstance() {
  static WindowManager manager;
  return manager;
}

void WindowManager::PushPremeetingView() {
  pre_meeting_view_ = std::make_shared<PreMeetingView>();
  pre_meeting_view_->show();
}

void WindowManager::PopPremeetingView() {
  if (pre_meeting_view_) {
    pre_meeting_view_->close();
    pre_meeting_view_.reset();
  }
}

void WindowManager::PushInmeetingView() {
  in_meeting_view_ = std::make_shared<InMeetingView>();
  in_meeting_view_->show();
}

void WindowManager::PopInmeetingView() {
  if (in_meeting_view_) {
    in_meeting_view_->close();
    in_meeting_view_.reset();
  }
}
