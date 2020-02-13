#ifndef WTT_DEMO_INCLUDE_INPUT_PROP_HPP
#define WTT_DEMO_INCLUDE_INPUT_PROP_HPP

#include "modal_widget.hpp"
namespace Ui {
  class InputProp;
}

class QLabel;
class InputProp: public ModalWidget {
  Q_OBJECT
public:
  explicit InputProp(QWidget* parent = 0);
  virtual ~InputProp();
  double getValue();
  void setBodySize(const QSize& size);

  void resetButtonSize(const QSize& size);
  void setDescription(const QString& d);

public slots:
  void accept();
  void reject();
  void clearInput();

protected:
  Ui::InputProp* ui_ptr_;
};
#endif