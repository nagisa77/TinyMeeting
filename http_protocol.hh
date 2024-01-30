#ifndef HTTP_PROTOCOL_HH
#define HTTP_PROTOCOL_HH

#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkReply>
#include <string>
#include <QObject>

class BaseProtocol : public QObject {
  Q_OBJECT

 public:
  using ResponseHandler = std::function<void(QNetworkReply*)>;

  explicit BaseProtocol(QObject* parent = nullptr);
  virtual ~BaseProtocol() {}

  virtual void MakeRequest(const ResponseHandler& handler) = 0;

 signals:
  void finished(QNetworkReply* reply);

 protected:
  QFutureWatcher<QNetworkReply*> watcher;
};

class QuickMeetingProtocol : public BaseProtocol {
  Q_OBJECT

 public:
  explicit QuickMeetingProtocol(QObject* parent = nullptr);
  ~QuickMeetingProtocol() override;

  void MakeRequest(const ResponseHandler& handler) override;
};

class JoinMeetingProtocol : public BaseProtocol {
  Q_OBJECT

 public:
  explicit JoinMeetingProtocol(const std::string& user_id, const std::string& meeting_id, QObject* parent = nullptr);
  ~JoinMeetingProtocol() override;

  void MakeRequest(const ResponseHandler& handler) override;
  
private:
  std::string user_id_;
  std::string meeting_id_;
};

class UserStatusProtocol : public BaseProtocol {
  Q_OBJECT

 public:
  explicit UserStatusProtocol(QObject* parent = nullptr);
  ~UserStatusProtocol() override;

  void MakeRequest(const ResponseHandler& handler) override;
};

#endif  // HTTP_PROTOCOL_HH
