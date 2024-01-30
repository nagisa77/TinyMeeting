#include "http_service.hh"

HttpService::HttpService(QObject* parent) : QObject(parent) {
  manager_ = new QNetworkAccessManager(this);
}

HttpService& HttpService::getInstance() {
  static HttpService service;
  return service;
}

QNetworkReply* HttpService::get(const QUrl& url) {
  QNetworkRequest request(url);
  return manager_->get(request);
}

QNetworkReply* HttpService::post(const QUrl& url, const QByteArray& data) {
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  return manager_->post(request, data);
}
