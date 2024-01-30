#ifndef HTTP_SERVICE_HH
#define HTTP_SERVICE_HH

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

class HttpService : public QObject {
  Q_OBJECT

 public:
  HttpService(const HttpService&) = delete;
  HttpService& operator=(const HttpService&) = delete;

  static HttpService& getInstance();

  QNetworkReply* get(const QUrl& url);
  QNetworkReply* post(const QUrl& url, const QByteArray& data);

 private:
  explicit HttpService(QObject* parent = nullptr);
  QNetworkAccessManager* manager_;
};

#endif  // HTTP_SERVICE_HH
