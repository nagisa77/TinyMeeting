#ifndef PRE_MEETING_VIEW_HH
#define PRE_MEETING_VIEW_HH

#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#include <memory>
#include <QLabel>

class PreMeetingController; 

class PreMeetingView : public QWidget {
  Q_OBJECT

 public:
  explicit PreMeetingView(QWidget* parent = nullptr);
  ~PreMeetingView();
  void MakeConnections();
  
  void ShowToast(const std::string& toast);

 private:
  QPushButton* quick_meeting_button_ = nullptr;
  QPushButton* join_meeting_button_ = nullptr;
  QLineEdit* meeting_id_edit_ = nullptr;
  QLineEdit* user_name_edit_ = nullptr;
  QLabel* require_user_id_label_ = nullptr;
  QLabel* require_meeting_id_label_ = nullptr;
  std::shared_ptr<PreMeetingController> controller_;
};

#endif  // PRE_MEETING_VIEW_HH
