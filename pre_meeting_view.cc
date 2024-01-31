#include "pre_meeting_view.hh"

#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include "pre_meeting_controller.hh"

PreMeetingView::~PreMeetingView() {
}

PreMeetingView::PreMeetingView(QWidget* parent) : QWidget(parent) {
  quick_meeting_button_ = new QPushButton(QString("Quick Meeting"));
  quick_meeting_button_->setFixedSize(200, 100);

  join_meeting_button_ = new QPushButton(QString("Join Meeting"));
  join_meeting_button_->setFixedSize(200, 100);

  meeting_id_edit_ = new QLineEdit(this);
  meeting_id_edit_->setFixedSize(200, 30);

  controller_ = std::make_shared<PreMeetingController>(this);
  
  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(0);
  layout->addWidget(meeting_id_edit_);
  layout->addSpacing(15);
  layout->addWidget(join_meeting_button_);
  layout->addSpacing(15);
  layout->addWidget(quick_meeting_button_);

  MakeConnections();
}

void ShowToast(const std::string& toast) {
  spdlog::info("showtoast: {}", toast);
}

void PreMeetingView::MakeConnections() {
  connect(quick_meeting_button_, &QPushButton::clicked, this,
          [=]() { controller_->HandleQuickMeeting(); });
}
