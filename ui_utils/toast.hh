#ifndef TOAST_HH
#define TOAST_HH

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QScreen>

class Toast : public QWidget {
public:
  Toast(QWidget* parent = nullptr) : QWidget(parent, Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint) {
    setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明
    auto* layout = new QVBoxLayout(this);
    label = new QLabel(this);
    label->setStyleSheet("QLabel { color: white; background-color: black; padding: 10px; border-radius: 5px; }");
    layout->addWidget(label, 0, Qt::AlignCenter);
    setLayout(layout);
  }

  void showMessage(const std::string& message, int duration_ms) {
    label->setText(message.c_str());
    adjustSize(); // 调整大小以适应文本

    if (parentWidget() != nullptr) {
      // 如果存在parent，将消息框移动到parent中间
      move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 2);
    } else {
      // 如果不存在parent，使用原来的逻辑将消息框移动到屏幕中间
      auto screen = QApplication::primaryScreen(); // todo: 窗口所在screen
      auto screenSize = screen->geometry();
      move(screenSize.center() - rect().center());
    }

    QTimer::singleShot(duration_ms, this, &QWidget::hide); // duration_ms毫秒后自动隐藏
    show();
  }

private:
    QLabel* label;
};

#endif
