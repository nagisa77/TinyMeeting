
#include "in_meeting_view.hh"
#include <QVBoxLayout>
#include <spdlog/spdlog.h>
#include "in_meeting_controller.hh"
#include "ui_utils/toast.hh"
#include <QApplication>
#include <QScreen>
#include <QLabel>
#include <QPainter>
#include <QClipboard>

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

VideoView::VideoView(std::shared_ptr<StreamPuller> puller, QWidget* parent)
: QWidget(parent)
{
  controller_ = std::make_shared<VideoViewController>(puller, this);
  
  setAttribute(Qt::WA_StyledBackground, true); // 启用样式表背景
  setStyleSheet("QWidget { background-color: green; border: 2px solid #2f0147; border-radius: 10px; }");
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  
  connect(this, &VideoView::frameReady, this, &VideoView::RenderFrame);
}

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

void VideoView::RenderFrame(QImage frame) {
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

InMeetingViewContainer::InMeetingViewContainer(QWidget* parent) : QWidget(parent) {
  setFixedSize(970, 675);
  setAttribute(Qt::WA_StyledBackground, true); // 启用样式表背景
  setStyleSheet("QWidget { background-color: #b9929f; border: 2px solid #2f0147; border-radius: 10px; }");
//
  layout_ = new QGridLayout(this);
  layout_->setAlignment(Qt::AlignCenter); // 设置布局整体居中
}

void InMeetingViewContainer::addUserInfoView(UserInfoView* view) {
  const int maxColumn = width() / view->width(); // 计算每行最多容纳的UserInfoView数量

  if (user_info_current_column_ >= maxColumn) { // 需要换行
    user_info_current_row_++;
    user_info_current_column_ = 0;
  }

  layout_->addWidget(view, user_info_current_row_, user_info_current_column_++, Qt::AlignCenter); // 添加控件并居中对齐
}

void InMeetingViewContainer::addVideoView(VideoView* view) {
  // Check if we need to move to the next row.
  if (video_view_current_column_ >= 3) {
    video_view_current_column_ = 0; // Reset column for the next row.
    video_view_current_row_++; // Move to the next row.
  }

  // Add the video view to the current position in the grid.
  layout_->addWidget(view, video_view_current_row_, video_view_current_column_);

  // Prepare for the next video view addition.
  video_view_current_column_++; // Move to the next column for the next addition.
}

void InMeetingViewContainer::removeAllSubView() {
  user_info_current_column_ = 0;
  user_info_current_row_ = 0;
  
  if (layout_ != nullptr) {
    // 从布局中移除所有项目
    QLayoutItem* item;
    while ((item = layout_->takeAt(0)) != nullptr) {
      // 如果项目是一个小部件，删除小部件
      if (item->widget()) {
          delete item->widget();
      }
      // 如果项目是一个布局，则递归地删除所有子项目
      delete item;
    }
  }

}

void InMeetingViewContainer::resizeEvent(QResizeEvent* event) {
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

      user_info_view_container_ = new InMeetingViewContainer();
      
      layout->addWidget(user_info_view_container_);

      // StreamControlContainer
      stream_control_container_ = new QWidget(this);
      stream_control_container_->setStyleSheet("QWidget { background-color: #b9929f; border: 2px solid #2f0147; border-radius: 10px; }");
      QHBoxLayout* stream_control_container_layout = new QHBoxLayout(stream_control_container_);
      stream_control_container_->setFixedSize(970, 80);
      stream_control_container_->setLayout(stream_control_container_layout);

      // 创建按钮
      meeting_information_button_ = new QPushButton(this);
      audio_button_ = new QPushButton(this);
      video_button_ = new QPushButton(this);
      screen_button_ = new QPushButton(this);
      exit_button_ = new QPushButton(this);
      
      meeting_information_button_->setIcon(QIcon(":/information_icon"));
      audio_button_->setIcon(QIcon(":/audio_icon"));
      video_button_->setIcon(QIcon(":/video_icon"));
      screen_button_->setIcon(QIcon(":/screen_share_icon"));
      exit_button_->setIcon(QIcon(":/exit_icon"));
      
      meeting_information_button_->setFixedSize(50, 50);
      audio_button_->setFixedSize(50, 50);
      video_button_->setFixedSize(50, 50);
      screen_button_->setFixedSize(50, 50);
      exit_button_->setFixedSize(50, 50);

      // 设置按钮样式
      QString buttonStyle =     "QPushButton {"
      "background-color: #b9929f;" // 正常状态下的背景颜色
      "border-radius: 10px;" // 圆角
      "border: 2px solid #2f0147;" // 无边框
      "}"
      "QPushButton:pressed {"
      "background-color: #9c528b;" // 按下状态下的背景颜色
      "}";

      meeting_information_button_->setStyleSheet(buttonStyle);
      audio_button_->setStyleSheet(buttonStyle);
      video_button_->setStyleSheet(buttonStyle);
      screen_button_->setStyleSheet(buttonStyle);
      exit_button_->setStyleSheet(buttonStyle);


      // 添加按钮到布局
      stream_control_container_layout->addStretch();
      stream_control_container_layout->addWidget(meeting_information_button_);
      stream_control_container_layout->addWidget(audio_button_);
      stream_control_container_layout->addWidget(video_button_);
      stream_control_container_layout->addWidget(screen_button_);
      stream_control_container_layout->addWidget(exit_button_);
      stream_control_container_layout->addStretch();

      layout->addSpacing(15);
      layout->addWidget(stream_control_container_);

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

void InMeetingView::UpdateUserInfoViews(const std::vector<UserStatus>& user_status) {
  user_info_view_container_->removeAllSubView();
  
  for (auto us : user_status) {
    user_info_view_container_->addUserInfoView(new UserInfoView(us));
  }
}

void InMeetingView::UpdateVideoViews(const std::vector<UserStatus>& user_status, const std::vector<std::shared_ptr<StreamPuller>>& stream_pullers) {
  user_info_view_container_->removeAllSubView();

  for (int i = 0; i < user_status.size(); ++i) {
    auto us = user_status[i];
    if (us.is_video_on || us.is_screen_on) {
      user_info_view_container_->addVideoView(new VideoView(stream_pullers[i]));
    }
  }
}

void InMeetingView::UpdateMeeingInfoToClipboard(const std::string& meeting_id) {
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(meeting_id.c_str());
}

InMeetingView::~InMeetingView() {
    // 析构函数逻辑（如有必要）
}

void InMeetingView::MakeConnections() {
    connect(audio_button_, &QPushButton::clicked, this, &InMeetingView::OnAudioClicked);
    connect(video_button_, &QPushButton::clicked, this, &InMeetingView::OnVideoClicked);
    connect(screen_button_, &QPushButton::clicked, this, &InMeetingView::OnScreenClicked);
    connect(meeting_information_button_, &QPushButton::clicked, this, &InMeetingView::OnInfomationClicked);
    connect(exit_button_, &QPushButton::clicked, this, &InMeetingView::OnExitClicked);
}

void InMeetingView::OnAudioClicked() {
    spdlog::info("audio click");
    // 这里可以添加额外的逻辑
}

void InMeetingView::OnVideoClicked() {
  controller_->HandleVideoClick();
}

void InMeetingView::OnScreenClicked() {
    spdlog::info("screen click");
    // 这里可以添加额外的逻辑
}

void InMeetingView::OnInfomationClicked() {
  spdlog::info("information click");
  controller_->HandleMeetingInfomationClick();
}

void InMeetingView::OnExitClicked() {
  spdlog::info("exit click"); 
  controller_->HandleExitClick();
}

void InMeetingView::ShowToast(const std::string& toast_str) {
  Toast* toast = new Toast(this);
  toast->showMessage(toast_str, 3000); // 显示3秒
}

void InMeetingView::closeEvent(QCloseEvent *event) {
  OnExitClicked();
}
