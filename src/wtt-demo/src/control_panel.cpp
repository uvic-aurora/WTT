#include "control_panel.hpp"

#include "ui_control_panel.h"

#include <QPushButton>
ControlPanel::ControlPanel(QWidget* parent): 
QWidget(parent),
main_layout_(new QBoxLayout(QBoxLayout::Direction::TopToBottom, this)),
ui_ptr_(new Ui::ControlPanel())
{
  ui_ptr_->setupUi(this);
  this->setLayout(main_layout_);
}


ControlPanel::~ControlPanel() {
  delete ui_ptr_;
}

void ControlPanel::setMainLayoutDirection(QBoxLayout::Direction direction) {
  main_layout_->setDirection(direction);
}

void ControlPanel::addButton(QPushButton *b) {
  main_layout_->addWidget(b);
}