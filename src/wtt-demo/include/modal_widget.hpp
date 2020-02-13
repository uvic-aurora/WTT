#ifndef WTT_DEMO_INCLUDE_MODAL_WIDGET_HPP
#define WTT_DEMO_INCLUDE_MODAL_WIDGET_HPP

#include <QWidget>

class QEventLoop;

class ModalWidget: public QWidget {
  Q_OBJECT
public:
  explicit ModalWidget(QWidget* parent = 0);
  virtual ~ModalWidget();

  virtual void open();
  virtual int exec();
  virtual void setBackgroundOpacity(uint opacity);
  enum ModalWidgetCode {Accepted = 1, Rejected = 0};

public slots:
  virtual void done(int code = 1);

signals:
  void finished(int code);

protected:
  virtual void paintEvent(QPaintEvent* e);

  QEventLoop* event_loop_ptr_;
  uint opacity_;
};

#endif