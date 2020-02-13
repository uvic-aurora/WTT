#ifndef WTT_DEMO_INCLUDE_ACTION_PANEL_HPP
#define WTT_DEMO_INCLUDE_ACTION_PANEL_HPP

#include <QWidget>
namespace Ui
{
  class ActionPanel;
}

class QResizeEvent;

class ActionPanel: public QWidget
{
  Q_OBJECT
public:
  enum ActionTypes {
    OPENMESH = 0,
    RESETMESH = 1,
    SETTYPE= 2,
    FWT = 3,
    IWT = 4,
    COMPRESS = 5,
    DENOISE = 6
  };
  explicit ActionPanel(QWidget* parent);
  virtual ~ActionPanel();

  virtual QSize minimumSizeHint() const override;

signals:

  void userAction(int);

public slots:
  void onOpenMeshDone(bool);
  void onTypeSelected(int);
  void onFWTDone(bool);
  void onIWTDone(bool);
protected:
  virtual void resizeEvent(QResizeEvent* e) override;

  void initGeometry();

  void initConnections();

  void initWidgets();
  void initStyleSheet();

protected:
  Ui::ActionPanel* ui_ptr_;
  // QGraphicsOpacityEffect* fwt_effect_;
  // QGraphicsOpacityEffect* iwt_effect_;
  // QGraphicsOpacityEffect* compress_effect_;
  // QGraphicsOpacityEffect* denoise_effect_;
};

#endif  // define WTT_DEMO_INCLUDE_ACTION_PANEL_HPP