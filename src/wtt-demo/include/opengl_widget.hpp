#ifndef WTT_DEMO_INCLUDE_OPENGL_WIDGET_HPP
#define WTT_DEMO_INCLUDE_OPENGL_WIDGET_HPP
#include "logger.hpp"
#include "custom_mesh_types.hpp"
#include "arcball_camera.hpp"
#include "scene_object.hpp"

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPointF>
#include <QOpenGLBuffer>

class SceneObject;
class ThreadedGLBufferUploader;

class GLViewControlPanel;
class QOpenGLShaderProgram;
class QMouseEvent;
class QKeyEvent;
class QPushButton;
class QFileDialog;
class OpenGLWidget: public QOpenGLWidget, public QOpenGLFunctions
{
  Q_OBJECT
public:
  explicit OpenGLWidget(QWidget* parent = 0);
  virtual ~OpenGLWidget();

  virtual void paintGL() override;
  virtual void resizeGL(int w, int h) override;
  virtual void initializeGL() override;

  SceneObject* getScene() { return scene_ptr_;}

signals:
  void openglReady();
  void meshRendered();

public slots:
  void shareContextWith(ThreadedGLBufferUploader* uploader);
  void alignCamera(BoundingBox b);
  void onBufferUpdated();

  void onToggleEdge();
  void resetCamera();

  void onSaveImage();

  void onEnterDirectory(const QString& path);
  void onPanelAction(int);

protected:
  virtual void resizeEvent(QResizeEvent* e) override;
  virtual void mousePressEvent(QMouseEvent* e) override;
  virtual void mouseMoveEvent(QMouseEvent* e) override;
  virtual void keyPressEvent(QKeyEvent* e) override;

  virtual void wheelEvent(QWheelEvent* e) override;

protected:

  ArcballCamera camera_;

  SceneObject* scene_ptr_;
  BoundingBox last_bbox_;
  QMatrix4x4 model_;
  QMatrix4x4 view_;
  QMatrix4x4 projection_;

  GLViewControlPanel* control_panel_;
  QPointF mouse_last_pos_;
  bool show_edge_;
  QString last_save_dir_;
  QFileDialog* save_dialog_;
  SceneObject::SHADINGTYPE shading_type_;
  DebugLogger debug;
  FatalLogger critical;
};

#endif  // define WTT_DEMO_INCLUDE_OPENGL_WIDGET_HPP