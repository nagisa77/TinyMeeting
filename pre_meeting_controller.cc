#include "pre_meeting_controller.hh"

#include <spdlog/spdlog.h>
#include <QTimer>
#include "window_manager.hh"

PreMeetingController::PreMeetingController(PreMeetingView* view)
: view_(view) {
  MeetingModel::getInstance().Register(this);
}

PreMeetingController::~PreMeetingController() {
  MeetingModel::getInstance().UnRegister(this);
}

void PreMeetingController::HandleUpdateUserId() {
  view_->UpdateUserId(MeetingModel::getInstance().GetSelfUserId());
}

void PreMeetingController::HandleQuickMeeting(const std::string& user_id) {
  MeetingModel::getInstance().QuickMeeting(user_id);
}

void PreMeetingController::HandleJoinMeeting(const std::string& user_id, const std::string& meeting_id) {
  MeetingModel::getInstance().JoinMeeting(user_id, meeting_id);
}

void PreMeetingController::JoinMeetingComplete(JoinMeetingResult result, const std::string& msg) {
  view_->ShowToast(msg);
  if (result == JoinMeetingResult::kJoinMeetingResultSuccess) {
    WindowManager::getInstance().PopPremeetingView();
    WindowManager::getInstance().PushInmeetingView();
  }
}
