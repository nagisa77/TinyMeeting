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

void MeetingModel::StartRequestUserStatusTimer(bool enable) {
  if (enable) {
    request_user_status_timer_ = std::make_shared<QTimer>();
    connect(request_user_status_timer_.get(), &QTimer::timeout, this, [=] {
      RequestUserStatusProtocol request_user_status(current_meeting_id_);
      request_user_status.MakeRequest([=](QNetworkReply* reply) {
        if (reply->error() == QNetworkReply::NoError) {
          auto response = reply->readAll();
          QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
          if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            auto jsonObject = jsonDoc.object();
            int result = jsonObject["result"].toBool();
            QString msg = jsonObject["msg"].toString();
            
            QJsonObject userList = jsonObject["user_list"].toObject();

            std::map<std::string, UserStatus> user_map;
            for (const QString& userId : userList.keys()) {
              QJsonObject userStatusJson = userList[userId].toObject();

              UserStatus userStatus;
              userStatus.user_id = userId.toStdString();
              userStatus.is_audio_on = userStatusJson["audio_on"].toBool();
              userStatus.audio_stream_id = userStatusJson["audio_stream_id"].toString().toStdString();
              userStatus.is_screen_on = userStatusJson["screen_on"].toBool();
              userStatus.screen_stream_id = userStatusJson["screen_stream_id"].toString().toStdString();
              userStatus.is_video_on = userStatusJson["video_on"].toBool();
              userStatus.video_stream_id = userStatusJson["video_stream_id"].toString().toStdString();

              // 将用户状态添加到 map 中
              user_map[userStatus.user_id] = userStatus;
            }
            
            if (user_map != user_map_) {
              spdlog::info("user change! {}", QJsonDocument(jsonObject).toJson(QJsonDocument::Compact).toStdString());
              user_map_ = user_map;
            }
          }
        } else {
          spdlog::info("Error in network reply: {}",
                       reply->errorString().toStdString());
        }
      });
    });
    request_user_status_timer_->start(1000);
  }
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
              
              if (result == kJoinMeetingResultSuccess) {
                StartRequestUserStatusTimer(true); 
              }
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
  if ((media_type == kMediaTypeVideo && self_user_status_.is_video_on == enable) ||
      (media_type == kMediaTypeScreen && self_user_status_.is_screen_on == enable) ||
      (media_type == kMediaTypeAudio && self_user_status_.is_audio_on == enable)) {
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
            camera_pusher_ = std::make_shared<StreamPusher>("tim-camera", "127.0.0.1", 10086);
            CameraCapture::getInstance()->Register(camera_pusher_.get());
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

void MeetingModel::HandleUserStatus(const QString& meetingId, bool mic, bool video,
                                    bool screenShare) {
  // 实现用户状态更新逻辑
}

void MeetingModel::NotifyJoinComplete(JoinMeetingResult result, const std::string& msg) {
  auto delegates(delegates_);
  for (auto delegate : delegates) {
    delegate->JoinMeetingComplete(result, msg);
  }
}
