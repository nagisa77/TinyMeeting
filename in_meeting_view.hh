
#ifndef IN_MEETING_VIEW_HH
#define IN_MEETING_VIEW_HH

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <memory>

class InMeetingController;

class InMeetingView : public QWidget {
  Q_OBJECT

public:
  explicit InMeetingView(QWidget* parent = nullptr);
  ~InMeetingView() override;

private slots:
  void onAudioClicked();
  void onVideoClicked();
  void onScreenClicked();

private:
  void MakeConnections();
  void ShowToast(const std::string& toast);

private:
  std::shared_ptr<InMeetingController> controller_;
  QWidget* renderViewContainer_ = nullptr;
  QWidget* streamControlContainer_ = nullptr;
  QWidget* memberListContainer_ = nullptr;
  QPushButton* audioButton_ = nullptr;
  QPushButton* videoButton_ = nullptr;
  QPushButton* screenButton_ = nullptr;
};

#endif  // In_MEETING_VIEW_HH
