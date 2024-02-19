
#include "in_meeting_view.hh"
#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include "in_meeting_controller.hh"
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QPainter>
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

VideoView::VideoView(QWidget* parent)
: QWidget(parent)
{}

VideoView::~VideoView() {}

void VideoView::OnFrame(std::shared_ptr<AVFRAME> frame) {
  if (frame) {
    QImage image = ConvertToQImage(frame);
    emit frameReady(image);
  }
}

void VideoView::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  if (!current_frame_.isNull()) {
    QSize windowSize = size();
    QImage scaledFrame = current_frame_.scaled(windowSize, Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
    int startX = (windowSize.width() - scaledFrame.width()) / 2;
    int startY = (windowSize.height() - scaledFrame.height()) / 2;
    painter.drawImage(startX, startY, scaledFrame);
  }
}

void VideoView::renderFrame(QImage frame) {
  current_frame_ = frame;
  update();
}

QImage VideoView::ConvertToQImage(std::shared_ptr<AVFRAME> f) {
  AVFrame* frame = (AVFrame*)f->frame_;
  if (!frame) {
    return QImage();
  }
  
  static SwsContext* sws_ctx = nullptr;
  static AVPixelFormat last_format = AV_PIX_FMT_NONE;
  AVPixelFormat src_pix_fmt = (AVPixelFormat)frame->format;
  AVPixelFormat dst_pix_fmt = AV_PIX_FMT_RGBA;

  // 检查是否需要重新创建转换上下文
  if (!sws_ctx || frame->format != last_format) {
    last_format = src_pix_fmt;
    if (sws_ctx) {
      sws_freeContext(sws_ctx);
    }
    sws_ctx = sws_getContext(frame->width, frame->height, src_pix_fmt,
                             frame->width, frame->height, dst_pix_fmt,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!sws_ctx) {
      return QImage();
    }
  }

  uint8_t* dest[4] = {nullptr};
  int dest_linesize[4] = {0};
  if (av_image_alloc(dest, dest_linesize, frame->width, frame->height,
                     dst_pix_fmt, 1) < 0) {
    return QImage();
  }

  sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height, dest,
            dest_linesize);

  QImage img(dest[0], frame->width, frame->height, dest_linesize[0],
             QImage::Format_RGBA8888);

  av_freep(&dest[0]);

  return img;
}

UserInfoView::UserInfoView(const UserStatus& us, QWidget* parent)
: QWidget(parent), status_(us) {
  setFixedSize(100, 100);
  setAttribute(Qt::WA_StyledBackground, true); // 启用样式表背景
  setStyleSheet("QWidget { background-color: #b9929f; border: 2px solid #2f0147; border-radius: 10px; }");
  
  layout_ = new QVBoxLayout(this);
  
  QLabel* name = new QLabel();
  name->setText(status_.user_id.c_str());
  name->setStyleSheet("QLabel {"
                      "border: none;"
                      "background-color: #9c528b;"
                      "color: #b9929f;"
                      "}");
  name->setAlignment(Qt::AlignCenter);
  
  layout_->addWidget(name);
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
  const int maxColumn = width() / view->width(); // 计算每行最多容纳的UserInfoView数量

  if (currentColumn_ >= maxColumn) { // 需要换行
    currentRow_++;
    currentColumn_ = 0;
  }

  layout->addWidget(view, currentRow_, currentColumn_++, Qt::AlignCenter); // 添加控件并居中对齐
}

void UserInfoViewContainer::removeAllUserInfoView() {
  currentColumn_ = 0;
  currentRow_ = 0;
  
  if (layout != nullptr) {
    // 从布局中移除所有项目
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr) {
      // 如果项目是一个小部件，删除小部件
      if (item->widget()) {
          delete item->widget();
      }
      // 如果项目是一个布局，则递归地删除所有子项目
      delete item;
    }
  }

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

    // 初始化连接
    MakeConnections();
}

void InMeetingView::UpdateTitle(const std::string& user_id, const std::string& meeting_id) {
  std::string title = "user_id: " + user_id + ", meeting id: " + meeting_id;
  setWindowTitle(title.c_str());
}

void InMeetingView::UpdateUserStatus(const std::vector<UserStatus>& user_status) {
  user_info_view_container_->removeAllUserInfoView();
  
  for (auto us : user_status) {
    user_info_view_container_->addUserInfoView(new UserInfoView(us));
  }
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

