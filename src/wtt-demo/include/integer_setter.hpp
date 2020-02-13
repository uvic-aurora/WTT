#ifndef WTT_DEMO_INCLUDE_INTEGER_SETTER_HPP
#define WTT_DEMO_INCLUDE_INTEGER_SETTER_HPP

#include "modal_widget.hpp"
namespace Ui {
  class IntegerSetter;
}

class QLabel;
class IntegerSetter: public ModalWidget {
  Q_OBJECT
public:
  explicit IntegerSetter(QWidget* parent = 0);
  virtual ~IntegerSetter();
  int getValue();
  void setValue(int v);
  void setMax(int);
  void setMin(int);
  void setBodySize(const QSize& size);
  void resetButtonSize(const QSize& size);
  void setDescription(const QString& d);

public slots:
  void accept();
  void reject();

  void onAdd();
  void onSubtract();

protected:
  Ui::IntegerSetter* ui_ptr_;
  int max_;
  int min_;
};
#endif