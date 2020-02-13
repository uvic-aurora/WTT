#ifndef WTT_DEMO_INCLUDE_MAINWINDOW_HPP
#define WTT_DEMO_INCLUDE_MAINWINDOW_HPP
#include "custom_mesh_types.hpp"
#include "logger.hpp"
#include <QMainWindow>
class QResizeEvent;
class OpenGLWidget;
class SceneObject;

class IntegerSetter;
class InputProp;

class WTTManager;

namespace Ui
{
  class MainWindow;
  class MeshInfoLabel;
}

class MessageBox;
class ActionPanel;
class MainWindow: public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = 0);
  virtual ~MainWindow();

protected:
  virtual void resizeEvent(QResizeEvent* e) override;

  void initializeGeometry();
  void initWidgets();

  void setupConnections();

public slots:
  void onUserAction(int);
  void onOpenGLLoadMesh();
  void onOpenGLReady();
  void onMeshLoaded(BoundingBox b, QString err);

  void onMeshReset();
  void onFWTLevelSet(int);
  void onIWTLevelSet(int);

  void onWTTypeSet(int);
  void onCompressRateSet(int);
  void onDenoiseLevelSet(int);
  void onFWTDone(bool, int, QString err);
  void onIWTDone(bool, int, QString err);
  void onCompressDone(QString msg);
  void onDenoiseDone(QString msg);

  void onUpdateMeshInfo(int, int);

signals:
  void openGLContextReady();
  void dataReady(const Mesh&);

  void loadMesh(QString filename);
  void resetMesh();
  void setWTType(int);

  void doFWT(int type, int level);
  void doIWT(int type, int level);
  void doCompress(double perc);
  void doDenoise(int level);

protected:
  Ui::MainWindow* ui_ptr_;
  QWidget* info_label_;
  Ui::MeshInfoLabel* info_label_ui_;
  OpenGLWidget* opengl_widget_ptr_;
  ActionPanel* action_panel_ptr_;
  MessageBox* proc_diag_ptr_;
  MessageBox* msg_prop_ptr_;
  MessageBox* wt_type_setter_ptr_;
  IntegerSetter* fwt_level_setter_ptr_;
  IntegerSetter* iwt_level_setter_ptr_;
  IntegerSetter* denoise_level_setter_ptr_;
  InputProp* compress_rate_setter_ptr_;
  WTTManager* wtt_manager_;
  int wt_type_;

  DebugLogger debug;
  FatalLogger critical;
};

#endif  // define WTT_DEMO_INCLUDE_MAINWINDOW_HPP