#ifndef HTTP_PROTOCOL_HH
#define HTTP_PROTOCOL_HH

#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkReply>
#include <string>
#include <QObject>
#include "header.hh"

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

class RequestUpStreamProtocol : public BaseProtocol {
  Q_OBJECT

 public:
  explicit RequestUpStreamProtocol(const std::string& user_id, const std::string& meeting_id, int media_type, QObject* parent = nullptr);
  ~RequestUpStreamProtocol() override;

  void MakeRequest(const ResponseHandler& handler) override;
  
private:
  std::string user_id_;
  std::string meeting_id_;
  int media_type_;
};

class UserStatusProtocol : public BaseProtocol {
  Q_OBJECT

 public:
  explicit UserStatusProtocol(const std::string& meeting_id, const UserStatus& user_status, QObject* parent = nullptr);
  ~UserStatusProtocol() override;

  void MakeRequest(const ResponseHandler& handler) override;
  
private:
  UserStatus user_status_;
  std::string meeting_id_;
};

class RequestUserStatusProtocol : public BaseProtocol {
  Q_OBJECT

 public:
  explicit RequestUserStatusProtocol(const std::string& meeting_id, QObject* parent = nullptr);
  ~RequestUserStatusProtocol() override;

  void MakeRequest(const ResponseHandler& handler) override;
  
private:
  std::string meeting_id_;
};


#endif  // HTTP_PROTOCOL_HH
