#include "opengl_widget.hpp"
#include "glview_control_panel.hpp"
#include "triangle_mesh_scene.hpp"
#include "threaded_gl_buffer_uploader.hpp"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QOffscreenSurface>
#include <QFileDialog>
#include <cmath>
#include <cassert>

OpenGLWidget::OpenGLWidget(QWidget* parent):
QOpenGLWidget(parent),
camera_(QVector3D(5.0, 0.0, 0.0),
        QVector3D(0.0, 0.0, 0.0),
        QVector3D(0.0, 0.0, 1.0)),
view_(camera_.getViewMatrix()),
show_edge_(false),
save_dialog_(new QFileDialog(this)),
shading_type_(SceneObject::SHADINGTYPE::SMOOTH),
control_panel_(new GLViewControlPanel(this)),
debug(DebugLogger("[OpenGL Widget]")),
critical(FatalLogger("[OpenGL Widget]"))
{
  model_.setToIdentity();
  projection_.setToIdentity();
  this->setFocusPolicy(Qt::StrongFocus);
  this->setMouseTracking(false);

  save_dialog_->setFileMode(QFileDialog::AnyFile);
  save_dialog_->setDirectory("/home/sywe1");
  save_dialog_->setAcceptMode(QFileDialog::AcceptSave);
  connect(control_panel_, &GLViewControlPanel::action, this, &OpenGLWidget::onPanelAction);
  connect(save_dialog_, &QFileDialog::directoryEntered, this, &OpenGLWidget::onEnterDirectory);
}

OpenGLWidget::~OpenGLWidget()
{
  if (scene_ptr_) {
    delete scene_ptr_;
  }
}

void OpenGLWidget::onPanelAction(int action) {
  switch (action) {
    case GLViewControlPanel::RESETCAM:
      resetCamera();
      break;
    case GLViewControlPanel::TOGGLEEDGE:
      onToggleEdge();
      break;
    case GLViewControlPanel::SMOOTHSHADING:
      shading_type_ = SceneObject::SHADINGTYPE::SMOOTH;
      break;
    case GLViewControlPanel::FLATSHADING:
      shading_type_ = SceneObject::SHADINGTYPE::FLAT;
      break;
    case GLViewControlPanel::CAPTUREFRAME:
      onSaveImage();
      return;
  }
  this->update();
}

void OpenGLWidget::resizeEvent(QResizeEvent *e) {
  QSize cp_size = control_panel_->minimumSize();
  control_panel_->setGeometry(0, 0.5 * (this->height() - cp_size.height()), cp_size.width(), cp_size.height());
  debug() << "GL window geometry: " << this->mapToGlobal(this->geometry().topLeft());
  QOpenGLWidget::resizeEvent(e);
}

void OpenGLWidget::mousePressEvent(QMouseEvent* e)
{
  mouse_last_pos_ = e->pos();
  QOpenGLWidget::mousePressEvent(e);
}

void OpenGLWidget::wheelEvent(QWheelEvent* e) {

  bool positive = e->angleDelta().y() > 0;
  if (e->inverted()) {
    positive = !positive;
  }
  if (last_bbox_.validate()) {
    if (positive) {
      model_.scale(1.05);
    } else {
      model_.scale(0.95);
    }
  }
  QOpenGLWidget::wheelEvent(e);
  this->update();
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent *e)
{
  QPointF mouse_mov = e->pos() - mouse_last_pos_;
  qreal window_radius = std::sqrt(this->width() * this->width() + 
                                  this->height() * this->height());
  if (e->modifiers() == Qt::ShiftModifier)
  {
    camera_.shift(QVector2D(mouse_mov.x() / window_radius, mouse_mov.y() / window_radius));
  }
  else
  {
    camera_.drag(QVector2D(mouse_mov.x() / window_radius, mouse_mov.y() / window_radius));
  }

  view_ = camera_.getViewMatrix();

  mouse_last_pos_ = e->pos();


  QOpenGLWidget::mouseMoveEvent(e);
  this->update();
}

void OpenGLWidget::keyPressEvent(QKeyEvent* e)
{
  switch (e->key())
  {
    case Qt::Key_Up:
    {

      break;
    }
    case Qt::Key_Down:
    {

      break;
    }
    case Qt::Key_Left:
    {

      break;
    }
    case Qt::Key_Right:
    {

      break;
    }
  }

  QOpenGLWidget::keyPressEvent(e);
}

void OpenGLWidget::alignCamera(BoundingBox b) {
  last_bbox_ = b;
  resetCamera();
}

void OpenGLWidget::resetCamera() {
  double scale = 1.0;
  double xc = last_bbox_.xc;
  double yc = last_bbox_.yc;
  double zc = last_bbox_.zc;
  if (last_bbox_.validate()) {
    camera_.setFocus(QVector3D(xc, yc, zc));
    camera_.setPosition(QVector3D(xc, yc, zc) + QVector3D(5.0, 0.0, 0.0));
    camera_.setUp(QVector3D(0.0, 0.0, 1.0));
    double xr = (last_bbox_.xmax - last_bbox_.xmin) / 2.0;
    double yr = (last_bbox_.ymax - last_bbox_.ymin) / 2.0;
    double zr = (last_bbox_.zmax - last_bbox_.zmin) / 2.0;
    double r = std::numeric_limits<double>::min();
    r = std::max(r, xr);
    r = std::max(r, yr);
    r = std::max(r, zr);
    if (r != 0) {
      scale = 1.0 / r;
    }
  } else {
    critical() << "Invalid Bounding box, reset camera to default";
    camera_ = ArcballCamera{QVector3D(5.0, 0.0, 0.0),
                            QVector3D(0.0, 0.0, 0.0),
                            QVector3D(0.0, 0.0, 1.0)};
  }
  view_ = camera_.getViewMatrix();
  model_.setToIdentity();
  model_.scale(scale);
  this->update();
}
void OpenGLWidget::initializeGL()
{
  debug() << "initializeGL()";
  initializeOpenGLFunctions();
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
  f->glEnable(GL_CULL_FACE);
  f->glEnable(GL_DEPTH_TEST);
  f->glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  scene_ptr_ = new TriangleMeshScene();
  scene_ptr_->init();
  emit openglReady();
}

void OpenGLWidget::resizeGL(int w, int h)
{
  debug() << "resizeGL(" << w << ", " << h << ")";
  if (this->context())
  {
    glViewport(0, 0,  w, h);
  }
  projection_.setToIdentity();
  projection_.perspective(45, float(w) / float(h), 1.0, 1000.0);
  this->update();
}

void OpenGLWidget::paintGL()
{
  QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();
  f->glClearColor(1.0, 1.0, 1.0, 1.0);
  f->glClear(GL_COLOR_BUFFER_BIT);
  f->glClear(GL_DEPTH_BUFFER_BIT);

  if (scene_ptr_) {
    scene_ptr_->renderEdge(show_edge_);
    scene_ptr_->setModelMat(model_);
    scene_ptr_->setViewMat(view_);
    scene_ptr_->setProjMat(projection_);
    scene_ptr_->render(shading_type_);
  } else {
    critical() << "Scene pointer is NULL";
  }
}

void OpenGLWidget::onBufferUpdated() {
  this->update();
}
void OpenGLWidget::onToggleEdge() {
  show_edge_ = !show_edge_;
  this->update();
}

void OpenGLWidget::shareContextWith(ThreadedGLBufferUploader *uploader) {
  debug() << "Share context with threaded uploader";
  QOpenGLContext* ctx = context();
  ctx->doneCurrent();
  QOpenGLContext* shared = uploader->getContext();
  shared->setFormat(ctx->format());
  shared->setShareContext(ctx);
  shared->create();
  shared->moveToThread(uploader);
  QOffscreenSurface* surface = uploader->getSurface();
  surface->setFormat(ctx->format());
  surface->create();
  surface->moveToThread(uploader);
}


void OpenGLWidget::onSaveImage() {
  QImage f {this->grabFramebuffer()};
  if (save_dialog_->exec()) {
    QString file_path = save_dialog_->selectedFiles().first();
    debug() << "Save frame to " << file_path;
    f.save(file_path, "jpg", 100);
  }
  save_dialog_->setDirectory(last_save_dir_);
}


void OpenGLWidget::onEnterDirectory(const QString &path) {
  last_save_dir_ = path;
}
