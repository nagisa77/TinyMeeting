#include <spdlog/spdlog.h>

#include <QApplication>

#include "pre_meeting_view.hh"

int main(int argc, char *argv[]) {
  spdlog::info("client init");

  QApplication app(argc, argv);

  // 创建前会议视图并显示
  PreMeetingView preMeetingView;
  preMeetingView.show();

  return app.exec();
}
