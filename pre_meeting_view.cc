#include "pre_meeting_view.hh"

#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include <QFont>

#include "pre_meeting_controller.hh"
#include "ui_utils/toast.hh"

PreMeetingView::~PreMeetingView() {
}

PreMeetingView::PreMeetingView(QWidget* parent) : QWidget(parent) {
  controller_ = std::make_shared<PreMeetingController>(this);

  setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);

  setFixedWidth(450);
  setFixedHeight(450);
  // Set background color and border radius
  setStyleSheet(
      "PreMeetingView {"
      "background-color: #e2c2c6;"
      "border-radius: 10px;"
      "}"
  );

  
  QLabel* meeting_label = new QLabel();
  meeting_label->setText("Meeting");
  {
    // Set up the font with the desired properties
    QFont font;
    font.setFamily("Roboto");
    font.setWeight(QFont::Weight::Bold); // For Qt 6, use QFont::Weight enum for setting weight
    font.setPointSize(36); // Set the font size to 36 points
    
    // Apply the font to the QLabel
    meeting_label->setFont(font);
    
    // Set the text color using a style sheet
    meeting_label->setStyleSheet("color: #2F0147;");
    
    meeting_label->setAlignment(Qt::AlignCenter);
  }

  QLabel* join_a_meeting_label = new QLabel();
  join_a_meeting_label->setText("Join a meeting");
  {
    // Configure the font properties
    QFont font;
    font.setFamily("Roboto");
    font.setPointSize(18); // Set the font size to 18 points
    font.setWeight(QFont::Weight::Normal); // QFont::Weight::Normal is typically 400

    // Apply the font to the QLabel
    join_a_meeting_label->setFont(font);

    // Set additional text properties using a style sheet
    join_a_meeting_label->setStyleSheet(
        "QLabel { "
        "color: #000000; " // Example color, change as needed
        "letter-spacing: 0px; " // Sets letter-spacing to 0px
        "text-align: left; " // Text alignment is managed by the layout, not directly applicable through QLabel's stylesheet
        "}"
    );

    // Note on line-height and text-align:
    // QLabel does not support line-height directly. For precise control over line spacing, consider using a QTextEdit in read-only mode.
    // Text alignment in QLabel can be set programmatically if you're aligning text within the label's bounds:
    join_a_meeting_label->setAlignment(Qt::AlignCenter);
  }

  quick_meeting_button_ = new QPushButton(QString("Quick Meeting"));
  quick_meeting_button_->setFixedSize(403, 50);
  quick_meeting_button_->setStyleSheet("QPushButton {"
                                      "    background-color: #610f7f;"  // 按钮背景颜色
                                      "    color: #e2c2c6;"            // 文字颜色
                                      "    border-radius: 10px;"       // 圆角大小
                                      "    font-size: 16px;"           // 可以根据需要设置字体大小
                                      "    border: none;"              // 无边框
                                      "}"
                                      "QPushButton:pressed {"
                                      "    background-color: #2f0147;"  // 按压时的背景颜色
                                      "}");

  join_meeting_button_ = new QPushButton(QString("Join Meeting"));
  join_meeting_button_->setFixedSize(403, 50);
  join_meeting_button_->setStyleSheet("QPushButton {"
                                      "    background-color: #610f7f;"  // 按钮背景颜色
                                      "    color: #e2c2c6;"            // 文字颜色
                                      "    border-radius: 10px;"       // 圆角大小
                                      "    font-size: 16px;"           // 可以根据需要设置字体大小
                                      "    border: none;"              // 无边框
                                      "}"
                                      "QPushButton:pressed {"
                                      "    background-color: #2f0147;"  // 按压时的背景颜色
                                      "}");

  
  meeting_id_edit_ = new QLineEdit(this);
  meeting_id_edit_->setFixedSize(403, 50);
  meeting_id_edit_->setAlignment(Qt::AlignLeft); // 文字左对齐
  meeting_id_edit_->setPlaceholderText(QStringLiteral("Meeting id")); // 设置默认灰色文字
  meeting_id_edit_->addAction(QIcon(":/meeting_icon"), QLineEdit::LeadingPosition);
  meeting_id_edit_->setStyleSheet(
      "QLineEdit {"
      "background-color: #b9929f;"
      "border: 2px solid #2f0147;"
      "border-radius: 10px;"
      "}"
  );
  
  user_name_edit_ = new QLineEdit(this);
  user_name_edit_->setFixedSize(403, 50);
  user_name_edit_->setAlignment(Qt::AlignLeft); // 文字左对齐
  user_name_edit_->setPlaceholderText(QStringLiteral("User id")); // 设置默认灰色文字
  user_name_edit_->addAction(QIcon(":/user_icon"), QLineEdit::LeadingPosition);
  user_name_edit_->setStyleSheet(
      "QLineEdit {"
      "background-color: #b9929f;"
      "border: 2px solid #2f0147;"
      "border-radius: 10px;"
      "}"
  );
  
  require_user_id_label_ = new QLabel();
  require_user_id_label_->setText("User id is required");
  require_user_id_label_->setStyleSheet("QLabel { color : red; font-size : 12px; }");
  require_user_id_label_->setVisible(false);

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(30, 30, 30, 30);
  layout->setSpacing(0);
  
  layout->addWidget(meeting_label);
  layout->addWidget(join_a_meeting_label);
  layout->addStretch();
  layout->addWidget(meeting_id_edit_);
  layout->addSpacing(15);
  layout->addWidget(require_user_id_label_);
  layout->addWidget(user_name_edit_);
  layout->addSpacing(15);
  layout->addWidget(join_meeting_button_);
  layout->addSpacing(15);
  layout->addWidget(quick_meeting_button_);

  MakeConnections();
}

void PreMeetingView::ShowToast(const std::string& toast_str) {
  Toast* toast = new Toast();
  toast->showMessage(toast_str, 3000); // 显示3秒
}

void PreMeetingView::MakeConnections() {
  connect(quick_meeting_button_, &QPushButton::clicked, this,
          [=]() {
    if (user_name_edit_->text().isEmpty()) {
      require_user_id_label_->setVisible(true);
      return;
    } else {
      require_user_id_label_->setVisible(false);
    }
    controller_->HandleQuickMeeting(user_name_edit_->text().toStdString());
  });
}
