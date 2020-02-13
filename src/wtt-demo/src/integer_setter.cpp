#include "integer_setter.hpp"
#include "ui_integer_setter.h"

#include <QDebug>
#include <QFile>
#include <QPushButton>
#include <QScreen>
#include <QApplication>

IntegerSetter::IntegerSetter(QWidget* parent):
  ModalWidget(parent),
  ui_ptr_(new Ui::IntegerSetter),
  max_(10),
  min_(0) {
  ui_ptr_->setupUi(this);
  QFile qss(":/qss/integer_setter.qss");
  if (qss.open(QFile::ReadOnly)) {
    this->setStyleSheet(QString(qss.readAll()));
  } else {
    qCritical() << "[IntegerSetter] Fail to load QSS: " << qss.fileName();
  }

  ui_ptr_->plus_icon->setPixmap(QPixmap(":/images/add.png"));
  ui_ptr_->plus_icon->setScaledContents(true);
  ui_ptr_->minus_icon->setPixmap(QPixmap(":/images/minus.png"));
  ui_ptr_->minus_icon->setScaledContents(true);
  qreal scale = qApp->primaryScreen()->logicalDotsPerInch() / 96.0;
  ui_ptr_->minus_button->setFixedSize(QSize(36, 36) * scale);
  ui_ptr_->plus_button->setFixedSize(QSize(36, 36) * scale);


  connect(ui_ptr_->minus_button, &QPushButton::clicked, this, &IntegerSetter::onSubtract);
  connect(ui_ptr_->plus_button, &QPushButton::clicked, this, &IntegerSetter::onAdd);
  connect(ui_ptr_->accept_button, &QPushButton::clicked, this, &IntegerSetter::accept);
  connect(ui_ptr_->reject_button, &QPushButton::clicked, this, &IntegerSetter::reject);

  ui_ptr_->accept_button->setText("Confirm");
  ui_ptr_->reject_button->setText("Cancel");
  setValue(0);
}

IntegerSetter::~IntegerSetter() {
  delete ui_ptr_;
}

void IntegerSetter::setBodySize(const QSize &size) {
  ui_ptr_->msg_body->setMinimumSize(size);
}

void IntegerSetter::resetButtonSize(const QSize &size) {
  for (int i = 0; i < ui_ptr_->buttons_layout->count(); ++i) {
    QWidget* b = ui_ptr_->buttons_layout->itemAt(i)->widget();
    if (!b) {
      continue;
    }
    b->setFixedSize(size);
  }
}

int IntegerSetter::getValue() {
  QString txt = ui_ptr_->value_label->text();
  return txt.toInt(nullptr, 10);
}

void IntegerSetter::setValue(int v) {
  QString txt {QString::number(v)};
  ui_ptr_->value_label->setText(txt);
}

void IntegerSetter::onAdd() {
  int cur = getValue();
  setValue(cur + 1 > max_ ? max_ : cur + 1);
}

void IntegerSetter::onSubtract() {
  int cur = getValue();
  setValue(cur - 1 < min_ ? min_ : cur - 1);
}
void IntegerSetter::accept() {
  done(ModalWidget::Accepted);
}
void IntegerSetter::reject() {
  done(ModalWidget::Rejected);
}

void IntegerSetter::setDescription(const QString &d) {
  ui_ptr_->description->setText(d);
}

void IntegerSetter::setMax(int max) {
  max_ = max;
}

void IntegerSetter::setMin(int min) {
  min_ = min;
}


