#include "pre_meeting_controller.hh"

#include <spdlog/spdlog.h>

PreMeetingController::PreMeetingController(PreMeetingView* view)
: view_(view) {
  MeetingModel::getInstance().Register(this);
}

PreMeetingController::~PreMeetingController() {
  MeetingModel::getInstance().UnRegister(this);
}

void PreMeetingController::HandleQuickMeeting() {
  MeetingModel::getInstance().QuickMeeting();
}

void PreMeetingController::JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {
  if (result == JoinMeetingResult::kJoinMeetingResultSuccess) {
    
  }
}
