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
              userStatus.is_audio_on = userStatusJson["is_audio_on"].toBool();
              userStatus.audio_stream_id = userStatusJson["audio_stream_id"].toString().toStdString();
              userStatus.is_screen_on = userStatusJson["is_screen_on"].toBool();
              userStatus.screen_stream_id = userStatusJson["screen_stream_id"].toString().toStdString();
              userStatus.is_video_on = userStatusJson["is_video_on"].toBool();
              userStatus.video_stream_id = userStatusJson["video_stream_id"].toString().toStdString();

              // 将用户状态添加到 map 中
              user_map[userStatus.user_id] = userStatus;
            }
            
            if (user_map != user_map_) {
              spdlog::info("user change! {}", QJsonDocument(jsonObject).toJson(QJsonDocument::Compact).toStdString());
              user_map_ = user_map;
              
              std::vector<UserStatus> values;
              for (const auto& entry : user_map_) {
                auto user_id = entry.first;
                if (entry.second.is_video_on) {
                  if (!stream_pullers_[user_id][kMediaTypeVideo]) {
                    if (self_user_status_.user_id  == user_id) {
//                      // 目前不拉自己的流
//                      continue;
                    }
                    auto puller = std::make_shared<StreamPuller>(entry.second.video_stream_id,
                                                                  SEVER_IP, 10086,
                                                                  entry.second.user_id, kMediaTypeVideo);
                    stream_pullers_[user_id][kMediaTypeVideo] = puller;
                    puller->RegisterListener(this); 
                  }
                }
                
                values.push_back(entry.second);
              }
              
              NotifyUserStatusUpdate(values);
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

void MeetingModel::OnPusherStreamServerError(MediaType media_type) {
  if (media_type == kMediaTypeVideo) {
    stream_pushers_[media_type]->UnRegisterListener(this);
    CameraCapture::getInstance()->UnRegister(stream_pushers_[media_type].get());
    CameraCapture::getInstance()->Start(false);
    stream_pushers_[media_type].reset();
    
    self_user_status_.is_video_on = false;
    self_user_status_.video_stream_id.clear();
    
    SyncUserStatus();
    
    NotifyPushMediaCompelete(kMediaTypeVideo, kPushMediaResultFailed, "media sever error");
  }
}

void MeetingModel::OnPullFrame(const std::string& user_id, MediaType media_type, std::shared_ptr<AVFRAME> frame) {
  spdlog::info("OnPullFrame({}, {}, ...)", user_id, (int)media_type);
}

void MeetingModel::QuickMeeting(const std::string& userId) {
  QuickMeetingProtocol quick_meeting;
  quick_meeting.MakeRequest([=](QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
      auto response = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        auto jsonObject = jsonDoc.object();
        QString meeting_id = jsonObject["meeting_id"].toString();
        spdlog::info("quick meeting id: {}", meeting_id.toStdString());
        
        JoinMeeting(userId, meeting_id.toStdString());
      }
    } else {
      spdlog::info("Error in network reply: {}",
                   reply->errorString().toStdString());
      
      NotifyJoinComplete(kJoinMeetingResultFailed, "network error");
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
    RequestUpStreamProtocol request_up_stream(self_user_status_.user_id, current_meeting_id_, media_type);
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
            std::string camere_stream_id = self_user_status_.user_id + "camera";
            stream_pushers_[media_type] = std::make_shared<StreamPusher>(camere_stream_id, SEVER_IP, 10086, media_type);
            stream_pushers_[media_type]->RegisterListener(this);
            CameraCapture::getInstance()->Register(stream_pushers_[media_type].get());
            CameraCapture::getInstance()->Start(true);
            
            self_user_status_.is_video_on = true;
            self_user_status_.video_stream_id = camere_stream_id;
            
            SyncUserStatus();
            
            NotifyPushMediaCompelete(kMediaTypeVideo, kPushMediaResultSuccess, "push camera success");
          }
        }
      } else {
        spdlog::info("Error in network reply: {}",
                     reply->errorString().toStdString());
      }
    });
  }
}

void MeetingModel::SyncUserStatus() {
  UserStatusProtocol sync_user_status(current_meeting_id_, self_user_status_);
  sync_user_status.MakeRequest([=](QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
      auto response = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        auto jsonObject = jsonDoc.object();
        int result = jsonObject["result"].toBool();
        QString msg = jsonObject["msg"].toString();
        spdlog::info("sync user status: {}, msg: {}", result, msg.toStdString());
      }
    } else {
      spdlog::info("Error in network reply: {}",
                   reply->errorString().toStdString());
    }
  });
}

void MeetingModel::JoinMeeting(const std::string& userId, const std::string& meetingId) {
  JoinMeetingProtocol join_meeting(userId, meetingId);
  
  join_meeting.MakeRequest([=](QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
      auto response = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        auto jsonObject = jsonDoc.object();
        int result = jsonObject["result"].toInt();
        QString msg = jsonObject["msg"].toString();
        
        spdlog::info("join meeting result: {}, msg: {}", result, msg.toStdString());
        current_meeting_id_ = meetingId;
        self_user_status_.user_id = userId;
        
        NotifyJoinComplete((JoinMeetingResult)result, msg.toStdString());
        
        if (result == kJoinMeetingResultSuccess) {
          StartRequestUserStatusTimer(true);
        }
      }
    } else {
      spdlog::info("Error in network reply: {}",
                   reply->errorString().toStdString());
      NotifyJoinComplete(kJoinMeetingResultFailed, "network error");
    }
  });
}

void MeetingModel::HandleUserStatus(const QString& meetingId, bool mic, bool video,
                                    bool screenShare) {
  // 实现用户状态更新逻辑
}

std::string MeetingModel::GetMeetingId() {
  return current_meeting_id_; 
}

std::string MeetingModel::GetSelfUserId() {
  return self_user_status_.user_id;
}

std::shared_ptr<StreamPuller> MeetingModel::GetStreamPuller(const std::string& user_id, MediaType media_type) {
  return stream_pullers_[user_id][media_type]; 
}

void MeetingModel::NotifyJoinComplete(JoinMeetingResult result, const std::string& msg) {
  auto delegates(delegates_);
  for (auto delegate : delegates) {
    delegate->JoinMeetingComplete(result, msg);
  }
}

void MeetingModel::NotifyPushMediaCompelete(MediaType media_type, PushMediaResult result, const std::string& msg) {
  auto delegates(delegates_);
  for (auto delegate : delegates) {
    delegate->PushMediaComplete(media_type, result, msg);
  }
}


void MeetingModel::NotifyUserStatusUpdate(const std::vector<UserStatus>& user_status) {
  auto delegates(delegates_);
  for (auto delegate : delegates) {
    delegate->OnUserStatusUpdate(user_status);
  }
}
