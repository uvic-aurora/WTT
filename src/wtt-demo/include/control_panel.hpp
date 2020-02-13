#ifndef WTT_DEMO_INCLUDE_CONTROL_PANEL_HPP
#define WTT_DEMO_INCLUDE_CONTROL_PANEL_HPP

#include "logger.hpp"
#include <QWidget>
#include <QBoxLayout>

namespace Ui {
  class ControlPanel;
}

class QPushButton;
class ControlPanel: public QWidget {
  Q_OBJECT
public:
  explicit ControlPanel(QWidget* parent = 0);
  virtual ~ControlPanel();

signals:
  void action(int);
public slots:
  void setMainLayoutDirection(QBoxLayout::Direction direction);
  virtual void addButton(QPushButton* b);

protected:
  Ui::ControlPanel* ui_ptr_;
  QBoxLayout* main_layout_;
};

#endif