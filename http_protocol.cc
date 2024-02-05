#include "http_protocol.hh"
#include <QJsonDocument>
#include <QJsonObject>
#include "http_service.hh"

BaseProtocol::BaseProtocol(QObject* parent) : QObject(parent) {
  connect(&watcher, &QFutureWatcher<QNetworkReply*>::finished,
          [this]() { emit finished(watcher.result()); });
}

// QuickMeetingProtocol
QuickMeetingProtocol::QuickMeetingProtocol(QObject* parent)
    : BaseProtocol(parent) {}

void QuickMeetingProtocol::MakeRequest(const ResponseHandler& handler) {
  auto& service = HttpService::getInstance();
  auto* reply =
      service.post(QUrl("http://localhost:3000/quick_meeting"), QByteArray());

  connect(reply, &QNetworkReply::finished, [reply, handler]() {
    handler(reply);
    reply->deleteLater();
  });
}

QuickMeetingProtocol::~QuickMeetingProtocol() {}

JoinMeetingProtocol::JoinMeetingProtocol(const std::string& user_id, const std::string& meeting_id, QObject* parent)
    : BaseProtocol(parent), user_id_(user_id), meeting_id_(meeting_id) {}

JoinMeetingProtocol::~JoinMeetingProtocol() {}

void JoinMeetingProtocol::MakeRequest(const ResponseHandler& handler) {
  auto& service = HttpService::getInstance();

  // 创建一个包含 user_id 和 meeting_id 的 JSON 对象
  QJsonObject json;
  json["user_id"] = QString::fromStdString(user_id_);
  json["meeting_id"] = QString::fromStdString(meeting_id_);

  // 将 JSON 对象转换为 QByteArray
  QJsonDocument doc(json);
  QByteArray data = doc.toJson();

  // 发送 POST 请求
  auto* reply = service.post(QUrl("http://localhost:3000/join_meeting"), data);

  connect(reply, &QNetworkReply::finished, [reply, handler]() {
    handler(reply);
    reply->deleteLater();
  });
}

RequestUpStreamProtocol::RequestUpStreamProtocol(const std::string& user_id, const std::string& meeting_id, int media_type, QObject* parent) : BaseProtocol(parent), user_id_(user_id), meeting_id_(meeting_id), media_type_(media_type) {
}

RequestUpStreamProtocol::~RequestUpStreamProtocol() {
}

void RequestUpStreamProtocol::MakeRequest(const ResponseHandler& handler) {
  auto& service = HttpService::getInstance();

  // 创建一个包含 user_id 和 meeting_id 的 JSON 对象
  QJsonObject json;
  json["user_id"] = QString::fromStdString(user_id_);
  json["meeting_id"] = QString::fromStdString(meeting_id_);
  json["media_type"] = media_type_;

  // 将 JSON 对象转换为 QByteArray
  QJsonDocument doc(json);
  QByteArray data = doc.toJson();

  // 发送 POST 请求
  auto* reply = service.post(QUrl("http://localhost:3000/request_up_stream"), data);

  connect(reply, &QNetworkReply::finished, [reply, handler]() {
    handler(reply);
    reply->deleteLater();
  });
}

UserStatusProtocol::UserStatusProtocol(const std::string& meeting_id, const UserStatus& user_status, QObject* parent)
    : BaseProtocol(parent), meeting_id_(meeting_id), user_status_(user_status) {}

UserStatusProtocol::~UserStatusProtocol() {}

void UserStatusProtocol::MakeRequest(const ResponseHandler& handler) {
  auto& service = HttpService::getInstance();

  QJsonObject json;
  json["meeting_id"] = QString::fromStdString(meeting_id_);
  json["user_id"] = QString::fromStdString(user_status_.user_id);
  json["is_audio_on"] = user_status_.is_audio_on;
  json["is_video_on"] = user_status_.is_video_on;
  json["is_screen_on"] = user_status_.is_screen_on;
  json["audio_stream_id"] = QString::fromStdString(user_status_.audio_stream_id);
  json["video_stream_id"] = QString::fromStdString(user_status_.video_stream_id);
  json["screen_stream_id"] = QString::fromStdString(user_status_.screen_stream_id);

  QJsonDocument doc(json);
  QByteArray data = doc.toJson();

  auto* reply = service.post(QUrl("http://localhost:3000/user_status"), data);

  connect(reply, &QNetworkReply::finished, [reply, handler]() {
    handler(reply);
    reply->deleteLater();
  });
}


RequestUserStatusProtocol::RequestUserStatusProtocol(const std::string& meeting_id, QObject* parent)
  : BaseProtocol(parent), meeting_id_(meeting_id) {}

RequestUserStatusProtocol::~RequestUserStatusProtocol() {
}

void RequestUserStatusProtocol::MakeRequest(const ResponseHandler& handler) {
  auto& service = HttpService::getInstance();

  // 创建一个包含 user_id 和 meeting_id 的 JSON 对象
  QJsonObject json;
  json["meeting_id"] = QString::fromStdString(meeting_id_);

  // 将 JSON 对象转换为 QByteArray
  QJsonDocument doc(json);
  QByteArray data = doc.toJson();

  // 发送 POST 请求
  auto* reply = service.post(QUrl("http://localhost:3000/request_user_status"), data);

  connect(reply, &QNetworkReply::finished, [reply, handler]() {
    handler(reply);
    reply->deleteLater();
  });
}
