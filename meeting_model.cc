#include "meeting_model.hh"

#include <spdlog/spdlog.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QWidget>

#include "http_protocol.hh"

MeetingModel::MeetingModel() {}

MeetingModel& MeetingModel::getInstance() {
  static MeetingModel model;
  return model;
}

void MeetingModel::Register(MeetingModelDelegate* delegate) {
  delegates_.insert(delegate);
}

void MeetingModel::UnRegister(MeetingModelDelegate* delegate) {
  delegates_.erase(delegate);
}

void MeetingModel::QuickMeeting() {
  QuickMeetingProtocol quick_meeting;
  quick_meeting.MakeRequest([](QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
      auto response = reply->readAll();
      QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
      if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        auto jsonObject = jsonDoc.object();
        QString meeting_id = jsonObject["meeting_id"].toString();
        
        spdlog::info("quick meeting id: {}", meeting_id.toStdString());
        
        JoinMeetingProtocol join_meeting("tim", meeting_id.toStdString());
        join_meeting.MakeRequest([](QNetworkReply* reply) {
          if (reply->error() == QNetworkReply::NoError) {
            auto response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
              auto jsonObject = jsonDoc.object();
              int result = jsonObject["result"].toBool();
              QString msg = jsonObject["msg"].toString();
              
              spdlog::info("join meeting result: {}, msg: {}", result, msg.toStdString());
              for (auto delegate : delegates_) {
                delegate->JoinMeetingComplete(result, msg.toStdString());
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

void MeetingModel::JoinMeeting(const QString& meetingId) {
  // 实现加入会议逻辑
}

void MeetingModel::UserStatus(const QString& meetingId, bool mic, bool video,
                              bool screenShare) {
  // 实现用户状态更新逻辑
}
