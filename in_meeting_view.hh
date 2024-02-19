
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
#include "stream_puller.hh"

class InMeetingController;
class VideoViewController;

class VideoView : public QWidget {
  Q_OBJECT
  
public:
  explicit VideoView(std::shared_ptr<StreamPuller> puller, QWidget* parent = nullptr);
  ~VideoView() override;
  
  void OnFrame(std::shared_ptr<AVFRAME> frame);
  void paintEvent(QPaintEvent* event) override;
  void RenderFrame(QImage frame);
  
signals:
  void frameReady(const QImage& image);
  
private:
  QImage ConvertToQImage(std::shared_ptr<AVFRAME> frame);
  
private:
  QImage current_frame_;
  std::shared_ptr<VideoViewController> controller_;
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

class InMeetingViewContainer : public QWidget {
  Q_OBJECT

private:
  QGridLayout *layout_;

public:
  explicit InMeetingViewContainer(QWidget* parent = nullptr);
  void addUserInfoView(UserInfoView* view);
  void addVideoView(VideoView* view);
  void removeAllSubView();
protected:
  void resizeEvent(QResizeEvent* event) override;
  
private:
  int user_info_current_row_ = 0;
  int user_info_current_column_ = 0;
  int video_view_current_row_ = 0;
  int video_view_current_column_ = 0;
};

class InMeetingView : public QWidget {
  Q_OBJECT

public:
  explicit InMeetingView(QWidget* parent = nullptr);
  ~InMeetingView() override;
  
  void UpdateTitle(const std::string& user_id, const std::string& meeting_id);
  void UpdateUserInfoViews(const std::vector<UserStatus>& user_status);
  void UpdateVideoViews(const std::vector<UserStatus>& user_status, const std::vector<std::shared_ptr<StreamPuller>>& stream_pullers);

private slots:
  void onAudioClicked();
  void onVideoClicked();
  void onScreenClicked();

private:
  void MakeConnections();
  void ShowToast(const std::string& toast);

private:
  std::shared_ptr<InMeetingController> controller_;
  InMeetingViewContainer* user_info_view_container_ = nullptr;
  QWidget* stream_control_container_ = nullptr;
  QWidget* memberListContainer_ = nullptr;
  QPushButton* audio_button_ = nullptr;
  QPushButton* video_button_ = nullptr;
  QPushButton* screen_button_ = nullptr;
};

#endif  // In_MEETING_VIEW_HH
