#include "modal_widget.hpp"

#include <QEventLoop>
#include <QList>
#include <QPainter>
#include <QDebug>

ModalWidget::ModalWidget(QWidget* parent): QWidget(parent),
                                           opacity_(255),
                                           event_loop_ptr_(new QEventLoop(this)) {
  this->hide();
}

ModalWidget::~ModalWidget() {}

void ModalWidget::open() {
  this->raise();
  this->show();
}

void ModalWidget::done(int code) {
  emit finished(code);
  event_loop_ptr_->exit(code);
  this->hide();
}

void ModalWidget::setBackgroundOpacity(uint opacity) {
  opacity_ = opacity > 255 ? 255 : opacity;
}

void ModalWidget::paintEvent(QPaintEvent *e) {
  QColor background("#FAFAFA");
  background.setAlpha(opacity_);
  QPainter painter(this);
  painter.fillRect(this->rect(), background);
  QWidget::paintEvent(e);
}

int ModalWidget::exec() {
  open();
  return event_loop_ptr_->exec();
}