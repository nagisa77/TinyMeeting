
#include "in_meeting_view.hh"
#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include "in_meeting_controller.hh"

InMeetingView::InMeetingView(QWidget* parent)
    : QWidget(parent), controller_(std::make_shared<InMeetingController>(this)) {
      // 设置布局
      QVBoxLayout* layout = new QVBoxLayout(this);

      // RenderViewContainer
      renderViewContainer_ = new QWidget(this);
      layout->addWidget(renderViewContainer_); // 添加占位

      // StreamControlContainer
      streamControlContainer_ = new QWidget(this);
      QHBoxLayout* hbox = new QHBoxLayout(streamControlContainer_);
      streamControlContainer_->setLayout(hbox);

      // 创建按钮
      audioButton_ = new QPushButton("audio", this);
      videoButton_ = new QPushButton("video", this);
      screenButton_ = new QPushButton("screen", this);

      // 设置按钮样式
      QString buttonStyle = "QPushButton { "
                            "width: 64px; "
                            "height: 64px; "
                            "border: 2px solid black; "
                            "border-radius: 8px; "
                            "}"
                            "QPushButton:pressed { "
                            "background-color: #A9A9A9;"  // 深灰色
                            "}";

      audioButton_->setStyleSheet(buttonStyle);
      videoButton_->setStyleSheet(buttonStyle);
      screenButton_->setStyleSheet(buttonStyle);


      // 添加按钮到布局
      hbox->addWidget(audioButton_);
      hbox->addWidget(videoButton_);
      hbox->addWidget(screenButton_);

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
