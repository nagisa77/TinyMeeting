
#include "in_meeting_view.hh"
#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include "in_meeting_controller.hh"
#include <QApplication>
#include <QScreen>

UserInfoView::UserInfoView(QWidget* parent)
  : QWidget(parent) {
  setFixedSize(100, 100);
  setAttribute(Qt::WA_StyledBackground, true); // 启用样式表背景

  setStyleSheet("QWidget { background-color: #b9929f; border: 2px solid #2f0147; border-radius: 10px; }");
}

UserInfoView::~UserInfoView() {
}

UserInfoViewContainer::UserInfoViewContainer(QWidget* parent) : QWidget(parent) {
  setFixedSize(970, 675);
  setAttribute(Qt::WA_StyledBackground, true); // 启用样式表背景
  setStyleSheet("QWidget { background-color: #b9929f; border: 2px solid #2f0147; border-radius: 10px; }");
//
  layout = new QGridLayout(this);
  layout->setAlignment(Qt::AlignCenter); // 设置布局整体居中
}

void UserInfoViewContainer::addUserInfoView(UserInfoView* view) {
  static int currentRow = 0;
  static int currentColumn = 0;
  const int maxColumn = width() / view->width(); // 计算每行最多容纳的UserInfoView数量

  if (currentColumn >= maxColumn) { // 需要换行
    currentRow++;
    currentColumn = 0;
  }

  layout->addWidget(view, currentRow, currentColumn++, Qt::AlignCenter); // 添加控件并居中对齐
}

void UserInfoViewContainer::resizeEvent(QResizeEvent* event) {
  QWidget::resizeEvent(event);
  // 可以在这里处理resize事件，动态调整布局
}

InMeetingView::InMeetingView(QWidget* parent)
    : QWidget(parent), controller_(std::make_shared<InMeetingController>(this)) {
      
      setFixedSize(1000, 800);
      setStyleSheet("QWidget { background-color: #e2c2c6; }");

      // 设置布局
      QVBoxLayout* layout = new QVBoxLayout(this);
      layout->setSpacing(0);
      layout->setContentsMargins(15, 15, 15, 15);

      user_info_view_container_ = new UserInfoViewContainer();
      user_info_view_container_->addUserInfoView(new UserInfoView());
      user_info_view_container_->addUserInfoView(new UserInfoView());
      user_info_view_container_->addUserInfoView(new UserInfoView()); 
      user_info_view_container_->addUserInfoView(new UserInfoView()); 
      user_info_view_container_->addUserInfoView(new UserInfoView()); 
      user_info_view_container_->addUserInfoView(new UserInfoView()); 
      user_info_view_container_->addUserInfoView(new UserInfoView()); 
      user_info_view_container_->addUserInfoView(new UserInfoView()); 
      user_info_view_container_->addUserInfoView(new UserInfoView());
      
      layout->addWidget(user_info_view_container_);

      // StreamControlContainer
      streamControlContainer_ = new QWidget(this);
      streamControlContainer_->setStyleSheet("QWidget { background-color: #b9929f; border: 2px solid #2f0147; border-radius: 10px; }");
      QHBoxLayout* streamControlContainer_layout = new QHBoxLayout(streamControlContainer_);
      streamControlContainer_->setFixedSize(970, 80);
      streamControlContainer_->setLayout(streamControlContainer_layout);

      // 创建按钮
      audioButton_ = new QPushButton(this);
      videoButton_ = new QPushButton(this);
      screenButton_ = new QPushButton(this);
      
      audioButton_->setIcon(QIcon(":/audio_icon"));
      videoButton_->setIcon(QIcon(":/video_icon"));
      screenButton_->setIcon(QIcon(":/screen_share_icon"));
      
      audioButton_->setFixedSize(50, 50);
      videoButton_->setFixedSize(50, 50);
      screenButton_->setFixedSize(50, 50);

      // 设置按钮样式
      QString buttonStyle =     "QPushButton {"
      "background-color: #b9929f;" // 正常状态下的背景颜色
      "border-radius: 10px;" // 圆角
      "border: 2px solid #2f0147;" // 无边框
      "}"
      "QPushButton:pressed {"
      "background-color: #9c528b;" // 按下状态下的背景颜色
      "}";

      audioButton_->setStyleSheet(buttonStyle);
      videoButton_->setStyleSheet(buttonStyle);
      screenButton_->setStyleSheet(buttonStyle);


      // 添加按钮到布局
      streamControlContainer_layout->addStretch();
      streamControlContainer_layout->addWidget(audioButton_);
      streamControlContainer_layout->addWidget(videoButton_);
      streamControlContainer_layout->addWidget(screenButton_);
      streamControlContainer_layout->addStretch();

      layout->addSpacing(15);
      layout->addWidget(streamControlContainer_);

      // MemberListContainer
      memberListContainer_ = new QWidget(this);
      layout->addWidget(memberListContainer_); // 添加占位

      // 设置标题
      setWindowTitle("meeting id: xxx");

      // 初始化连接
      MakeConnections();
}

InMeetingView::~InMeetingView() {
    // 析构函数逻辑（如有必要）
}

void InMeetingView::MakeConnections() {
    connect(audioButton_, &QPushButton::clicked, this, &InMeetingView::onAudioClicked);
    connect(videoButton_, &QPushButton::clicked, this, &InMeetingView::onVideoClicked);
    connect(screenButton_, &QPushButton::clicked, this, &InMeetingView::onScreenClicked);
}

void InMeetingView::onAudioClicked() {
    spdlog::info("audio click");
    // 这里可以添加额外的逻辑
}

void InMeetingView::onVideoClicked() {
  controller_->HandleVideoClick();
}

void InMeetingView::onScreenClicked() {
    spdlog::info("screen click");
    // 这里可以添加额外的逻辑
}

void InMeetingView::ShowToast(const std::string& toast) {
    // 显示提示信息
}

