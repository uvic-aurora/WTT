#include "message_box.hpp"

#include "ui_messagebox.h"

#include <QDebug>
#include <QFile>
#include <QPushButton>
MessageBox::MessageBox(QWidget* parent):
  ModalWidget(parent),
  ui_ptr_(new Ui::MessageBox) {
  ui_ptr_->setupUi(this);
}

MessageBox::~MessageBox() {
  delete ui_ptr_;
}

void MessageBox::setBodySize(const QSize &size) {
  ui_ptr_->msg_body->setMinimumSize(size);
}


void MessageBox::resetButtonSize(const QSize &size) {
  for (int i = 0; i < ui_ptr_->buttons_layout->count(); ++i) {
    QWidget* b = ui_ptr_->buttons_layout->itemAt(i)->widget();
    if (!b) {
      continue;
    }
    b->setFixedSize(size);
  }
}

void MessageBox::addAcceptButton(const QString &name){
  QPushButton* b = new QPushButton(this);
  b->setObjectName("accept_button");
  b->setText(name);
  ui_ptr_->buttons_layout->addWidget(b);
  connect(b, &QPushButton::clicked, this, &MessageBox::accept);
}

void MessageBox::addRejectButton(const QString &name){
  QPushButton* b = new QPushButton(this);
  b->setObjectName("reject_button");
  b->setText(name);
  ui_ptr_->buttons_layout->addWidget(b);
  connect(b, &QPushButton::clicked, this, &MessageBox::reject);
}

QLabel* MessageBox::getDescription() {
  return ui_ptr_->description;
}

void MessageBox::resetStyleSheet(const QString &filename) {
  QFile qss(filename);
  if (qss.open(QFile::ReadOnly)) {
    this->setStyleSheet(QString(qss.readAll()));
  } else {
    qCritical() << "[MESSAGEBOX] Fail to load QSS: " << qss.fileName();
  }
}

void MessageBox::accept() {
  done(ModalWidget::Accepted);
}

void MessageBox::reject() {
  done(ModalWidget::Rejected);
}

