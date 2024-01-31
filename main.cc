#include <spdlog/spdlog.h>

#include <QApplication>

#include "pre_meeting_view.hh"
#include "window_manager.hh"

int main(int argc, char *argv[]) {
  spdlog::info("client init");

  QApplication app(argc, argv);

  WindowManager::getInstance().PushPremeetingView();

  return app.exec();
}
