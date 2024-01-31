#include "meeting_model.hh"

#include <spdlog/spdlog.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QWidget>
#include "media_capture/camera_capture.hh"

#include "http_protocol.hh"

MeetingModel::MeetingModel() {}

MeetingModel& MeetingModel::getInstance() {
  static MeetingModel* model = new MeetingModel();
  return *model;
}

void MeetingModel::Register(MeetingModelDelegate* delegate) {
  delegates_.insert(delegate);
}

void MeetingModel::UnRegister(MeetingModelDelegate* delegate) {
  delegates_.erase(delegate);
}

void MeetingModel::QuickMeeting() {
  QuickMeetingProtocol quick_meeting;
  quick_meeting.MakeRequest([=](QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
      auto response = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        auto jsonObject = jsonDoc.object();
        QString meeting_id = jsonObject["meeting_id"].toString();
        
        spdlog::info("quick meeting id: {}", meeting_id.toStdString());
        
        JoinMeetingProtocol join_meeting("tim", meeting_id.toStdString());
        join_meeting.MakeRequest([=](QNetworkReply* reply) {
          if (reply->error() == QNetworkReply::NoError) {
            auto response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
              auto jsonObject = jsonDoc.object();
              int result = jsonObject["result"].toBool();
              QString msg = jsonObject["msg"].toString();
              
              spdlog::info("join meeting result: {}, msg: {}", result, msg.toStdString());
              current_meeting_id_ = meeting_id.toStdString();
              NotifyJoinComplete((JoinMeetingResult)result, msg.toStdString());
            }
          } else {
            spdlog::info("Error in network reply: {}",
                         reply->errorString().toStdString());
          }
        });
      }
    } else {
      spdlog::info("Error in network reply: {}",
                   reply->errorString().toStdString());
    }
  });
}

void MeetingModel::EnableMedia(MediaType media_type, bool enable) {
  spdlog::info("enable media: {}, enable: {}", (int)media_type, enable);
  if (media_type == kMediaTypeVideo && self_user_status_.is_video_on == enable ||
      media_type == kMediaTypeScreen && self_user_status_.is_screen_on == enable ||
      media_type == kMediaTypeAudio && self_user_status_.is_audio_on == enable) {
    return;
  }
  if (enable) {
    RequestUpStreamProtocol request_up_stream("tim", current_meeting_id_, media_type);
    request_up_stream.MakeRequest([=](QNetworkReply* reply) {
      if (reply->error() == QNetworkReply::NoError) {
        auto response = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
          auto jsonObject = jsonDoc.object();
          int result = jsonObject["result"].toBool();
          QString msg = jsonObject["msg"].toString();
          spdlog::info("request up stream: {}, msg: {}", result, msg.toStdString());
          
          if (media_type == kMediaTypeVideo) {
            CameraCapture::getInstance()->Start(true);
          }
        }
      } else {
        spdlog::info("Error in network reply: {}",
                     reply->errorString().toStdString());
      }
    });
  }
}

void MeetingModel::JoinMeeting(const QString& meetingId) {
  // 实现加入会议逻辑
}

void MeetingModel::UserStatus(const QString& meetingId, bool mic, bool video,
                              bool screenShare) {
  // 实现用户状态更新逻辑
}

void MeetingModel::NotifyJoinComplete(JoinMeetingResult result, const std::string& msg) {
  auto delegates(delegates_);
  for (auto delegate : delegates) {
    delegate->JoinMeetingComplete(result, msg);
  }
}
