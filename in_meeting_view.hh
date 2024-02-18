
#ifndef IN_MEETING_VIEW_HH
#define IN_MEETING_VIEW_HH

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <memory>
#include <QGridLayout>
#include <QVBoxLayout>
#include "media_capture/frame.hh"
#include "header.hh"

class InMeetingController;

class VideoView : public QWidget {
  Q_OBJECT

public:
  explicit VideoView(QWidget* parent = nullptr);
  ~VideoView() override;
  
  void OnFrame(std::shared_ptr<AVFRAME> frame);
  void paintEvent(QPaintEvent* event) override;
  void renderFrame(QImage frame);
  
signals:
  void frameReady(const QImage& image);
  
private:
  QImage ConvertToQImage(std::shared_ptr<AVFRAME> frame);

private:
  QImage current_frame_;
};

class UserInfoView : public QWidget {
  Q_OBJECT

public:
  explicit UserInfoView(const UserStatus& us, QWidget* parent = nullptr);
  ~UserInfoView() override;
  
private:
  UserStatus status_; 
  
  QVBoxLayout* layout_ = nullptr;
};

class UserInfoViewContainer : public QWidget {
  Q_OBJECT

private:
  QGridLayout *layout;

public:
  explicit UserInfoViewContainer(QWidget* parent = nullptr);
  void addUserInfoView(UserInfoView* view);
  void removeAllUserInfoView();
protected:
  void resizeEvent(QResizeEvent* event) override;
  
private:
  int currentRow_ = 0;
  int currentColumn_ = 0;
};

class InMeetingView : public QWidget {
  Q_OBJECT

public:
  explicit InMeetingView(QWidget* parent = nullptr);
  ~InMeetingView() override;
  
  void UpdateTitle(const std::string& meeting_id);
  void UpdateUserStatus(const std::vector<UserStatus>& user_status);

private slots:
  void onAudioClicked();
  void onVideoClicked();
  void onScreenClicked();

private:
  void MakeConnections();
  void ShowToast(const std::string& toast);

private:
  std::shared_ptr<InMeetingController> controller_;
  UserInfoViewContainer* user_info_view_container_ = nullptr;
  QWidget* streamControlContainer_ = nullptr;
  QWidget* memberListContainer_ = nullptr;
  QPushButton* audioButton_ = nullptr;
  QPushButton* videoButton_ = nullptr;
  QPushButton* screenButton_ = nullptr;
};

#endif  // In_MEETING_VIEW_HH
