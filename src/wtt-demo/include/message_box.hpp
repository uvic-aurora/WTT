#ifndef WTT_DEMO_INCLUDE_MESSAGE_BOX_HPP
#define WTT_DEMO_INCLUDE_MESSAGE_BOX_HPP

#include "modal_widget.hpp"
namespace Ui {
  class MessageBox;
}

class QLabel;
class MessageBox: public ModalWidget {
  Q_OBJECT
public:
  explicit MessageBox(QWidget* parent = 0);
  virtual ~MessageBox();
  void addAcceptButton(const QString& name);
  void addRejectButton(const QString& name);

  void resetStyleSheet(const QString& filename);
  QLabel* getDescription();
  void setBodySize(const QSize& size);
  void resetButtonSize(const QSize& size);
  void hStackWidget(QWidget* w);

public slots:
  void accept();
  void reject();

protected:
  Ui::MessageBox* ui_ptr_;
};
#endif