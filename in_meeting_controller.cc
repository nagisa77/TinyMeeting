
#include "in_meeting_controller.hh"

#include <spdlog/spdlog.h>
#include "window_manager.hh"


VideoViewController::VideoViewController(std::shared_ptr<StreamPuller> puller, VideoView* view)
: puller_(puller), view_(view) {
  puller_->RegisterListener(this);
}

VideoViewController::~VideoViewController() {
  puller_->UnRegisterListener(this);
}

void VideoViewController::OnPullFrame(const std::string& user_id, MediaType media_type, std::shared_ptr<AVFRAME> frame) {
  view_->OnFrame(frame); 
}

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

void InMeetingController::HandleMeetingInfomationClick() {
  view_->UpdateMeeingInfoToClipboard(MeetingModel::getInstance().GetMeetingId()); 
  view_->ShowToast("Meeting information copied to clipboard.");
}

void InMeetingController::HandleExitClick() {
  MeetingModel::getInstance().ExitMeeting();
}

void InMeetingController::PushMediaComplete(MediaType media_type, PushMediaResult result, const std::string& msg) {
  
}

void InMeetingController::OnUserStatusUpdate(const std::vector<UserStatus>& user_status) {
  bool has_video_stream = false;
  std::vector<std::shared_ptr<StreamPuller>> stream_pullers(user_status.size());
  for (int i = 0; i < user_status.size(); ++i) {
    auto us = user_status[i];
    if (us.is_video_on) {
      has_video_stream = true;
      
      // todo: 用户可能既开视频又开共享，这一块逻辑需要处理
      stream_pullers[i] = MeetingModel::getInstance().GetStreamPuller(us.user_id, kMediaTypeVideo);
    }
  }
  
  if (has_video_stream) {
    view_->UpdateVideoViews(user_status, stream_pullers);
  } else {
    view_->UpdateUserInfoViews(user_status);
  }
}

void InMeetingController::LeaveMeetingComplete(LeaveMeetingResult result, const std::string& msg) {
  if (result == kLeaveMeetingResultSuccess) {
    WindowManager::getInstance().PushPremeetingView();
    WindowManager::getInstance().PopInmeetingView();
  } else {
    view_->ShowToast(msg);
  }
}
