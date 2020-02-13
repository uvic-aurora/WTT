#include "input_prop.hpp"
#include "ui_input_prop.h"

#include <QDebug>
#include <QFile>
#include <QPushButton>

InputProp::InputProp(QWidget* parent):
  ModalWidget(parent),
  ui_ptr_(new Ui::InputProp) {
  ui_ptr_->setupUi(this);
  QFile qss(":/qss/input_prop.qss");
  if (qss.open(QFile::ReadOnly)) {
    this->setStyleSheet(QString(qss.readAll()));
  } else {
    qCritical() << "[InputProp] Fail to load QSS: " << qss.fileName();
  }

  connect(ui_ptr_->accept_button, &QPushButton::clicked, this, &InputProp::accept);
  connect(ui_ptr_->reject_button, &QPushButton::clicked, this, &InputProp::reject);
  setDescription("Set compression rate (%) to");

  ui_ptr_->accept_button->setText("Confirm");
  ui_ptr_->reject_button->setText("Cancel");
}

InputProp::~InputProp() {
  delete ui_ptr_;
}

void InputProp::setBodySize(const QSize &size) {
  ui_ptr_->msg_body->setMinimumSize(size);
}

void InputProp::resetButtonSize(const QSize &size) {
  for (int i = 0; i < ui_ptr_->buttons_layout->count(); ++i) {
    QWidget* b = ui_ptr_->buttons_layout->itemAt(i)->widget();
    if (!b) {
      continue;
    }
    b->setFixedSize(size);
  }
}
double InputProp::getValue() {
  QString txt = ui_ptr_->user_input->text();
  bool ok;
  double val = txt.toDouble(&ok);
  if (!ok) {
    return 100;
  } else {
    return val;
  }
}



void InputProp::clearInput() {
  ui_ptr_->user_input->clear();
}


void InputProp::accept() {
  QString txt = ui_ptr_->user_input->text();
  bool ok;
  double val = txt.toDouble(&ok);
  if (ok && val >= 0.0 && val <= 100.0) {
    setDescription("Set compression rate (%) to ");
    done(ModalWidget::Accepted);
  } else {
    setDescription("Value should be in between 0.0 to 100.0");
  }
}
void InputProp::reject() {
  done(ModalWidget::Rejected);
}

void InputProp::setDescription(const QString &d) {
  ui_ptr_->description->setText(d);
}



