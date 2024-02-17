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
    auto screen = QApplication::primaryScreen();
    auto screenSize = screen->geometry();
    move(screenSize.center() - rect().center()); // 使用QScreen居中显示
    QTimer::singleShot(duration_ms, this, &QWidget::hide); // duration_ms毫秒后自动隐藏
    show();
  }

private:
    QLabel* label;
};

#endif
