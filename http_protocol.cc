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


UserStatusProtocol::UserStatusProtocol(QObject* parent)
    : BaseProtocol(parent) {}

UserStatusProtocol::~UserStatusProtocol() {}

void UserStatusProtocol::MakeRequest(const ResponseHandler& handler) {
  // 实现发送用户状态请求的逻辑
  // return networkManager->post(...);
}
