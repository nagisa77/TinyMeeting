#ifndef PRE_MEETING_VIEW_HH
#define PRE_MEETING_VIEW_HH

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <memory>

class PreMeetingController; 

class PreMeetingView : public QWidget {
  Q_OBJECT

 public:
  explicit PreMeetingView(QWidget* parent = nullptr);
  ~PreMeetingView();
  void MakeConnections();
  
private:
  void ShowToast(const std::string& toast);

 private:
  QPushButton* quick_meeting_button_;
  QPushButton* join_meeting_button_;
  QLineEdit* meeting_id_edit_;
  std::shared_ptr<PreMeetingController> controller_;
};

#endif  // PRE_MEETING_VIEW_HH
