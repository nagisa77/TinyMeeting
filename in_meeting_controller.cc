
#include "in_meeting_controller.hh"

#include <spdlog/spdlog.h>
#include "window_manager.hh"

InMeetingController::InMeetingController(InMeetingView* view)
: view_(view) {
  MeetingModel::getInstance().Register(this);
}

InMeetingController::~InMeetingController() {
  MeetingModel::getInstance().UnRegister(this);
}

void InMeetingController::HandleVideoClick() {
  MeetingModel::getInstance().EnableMedia(kMediaTypeVideo, true);
}

