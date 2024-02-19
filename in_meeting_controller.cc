
#include "in_meeting_controller.hh"

#include <spdlog/spdlog.h>
#include "window_manager.hh"

InMeetingController::InMeetingController(InMeetingView* view)
: view_(view) {
  MeetingModel::getInstance().Register(this);
  
  view_->UpdateTitle(MeetingModel::getInstance().GetSelfUserId(), MeetingModel::getInstance().GetMeetingId()); 
}

InMeetingController::~InMeetingController() {
  MeetingModel::getInstance().UnRegister(this);
}

void InMeetingController::HandleVideoClick() {
  MeetingModel::getInstance().EnableMedia(kMediaTypeVideo, true);
}

void InMeetingController::PushMediaComplete(MediaType media_type, PushMediaResult result, const std::string& msg) {
  
}

void InMeetingController::OnUserStatusUpdate(const std::vector<UserStatus>& user_status) {
  view_->UpdateUserStatus(user_status); 
}

