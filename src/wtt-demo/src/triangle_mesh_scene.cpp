#include <triangle_mesh_scene.hpp>

#include <QOpenGLShaderProgram>

TriangleMeshScene::TriangleMeshScene(QObject* parent)
: SceneObject(parent),
  vpos_(QOpenGLBuffer::VertexBuffer),
  vnormal_(QOpenGLBuffer::VertexBuffer),
  fnormal_(QOpenGLBuffer::VertexBuffer),
  vbarycentric_(QOpenGLBuffer::VertexBuffer),
  tri_size_(0),
  show_edge_(true),
  debug(DebugLogger(QString("[TriangleMeshScene]"))),
  critical(FatalLogger(QString("[TriangleMeshScene]")))
{
  model_.setToIdentity();
  view_.setToIdentity();
  proj_.setToIdentity();
}

TriangleMeshScene::~TriangleMeshScene()
{
  vpos_.release();
  vpos_.destroy();
  vnormal_.release();
  vnormal_.destroy();
  fnormal_.release();
  fnormal_.destroy();
  vbarycentric_.release();
  vbarycentric_.destroy();
  vao_.release();
  vao_.destroy();
}

void TriangleMeshScene::renderEdge(bool on) {
  show_edge_ = on;
}
void TriangleMeshScene::render(SHADINGTYPE type)
{
  this->glsl_program_->bind();
  this->vao_.bind();
  this->glsl_program_->setUniformValue("model_mat",  model_);
  this->glsl_program_->setUniformValue("view_mat", view_);
  this->glsl_program_->setUniformValue("projection_mat", proj_);
  this->glsl_program_->setUniformValue("show_edge", show_edge_);

  vpos_.bind();
  GLuint pos_location = this->glsl_program_->attributeLocation("v_pos");
  this->glsl_program_->enableAttributeArray(pos_location);
  this->glsl_program_->setAttributeArray(pos_location, GL_FLOAT, 0, 3);

  if (type == SHADINGTYPE::SMOOTH) {
    vnormal_.bind();
  } else {
    fnormal_.bind();
  }
  GLuint normal_location = this->glsl_program_->attributeLocation("v_normal");
  this->glsl_program_->enableAttributeArray(normal_location);
  this->glsl_program_->setAttributeArray(normal_location, GL_FLOAT, 0, 3);

  vbarycentric_.bind();
  GLuint barycenter_loc = this->glsl_program_->attributeLocation("v_bary_center");
  this->glsl_program_->enableAttributeArray(barycenter_loc);
  this->glsl_program_->setAttributeArray(barycenter_loc, GL_FLOAT, 0, 3);

  glDrawArrays(GL_TRIANGLES, 0, tri_size_);
  this->glsl_program_->disableAttributeArray(pos_location);
  this->glsl_program_->disableAttributeArray(normal_location);
  this->glsl_program_->disableAttributeArray(barycenter_loc);
  this->vao_.release();
  this->glsl_program_->release();
}

void TriangleMeshScene::init()
{
  initializeOpenGLFunctions();
  loadShader();
  vao_.create();
  vao_.bind();
  if (!vao_.isCreated()) {
    critical() << " VAO creation failed";
  }
  if (!vpos_.create()) {
    critical() << " Unable to create position VBO";
  }
  if (!vnormal_.create()){
    critical() << " Unable to create vertex normal VBO";
  }
  if (!fnormal_.create()) {
    critical() << "Unable to create face nomral VBO";
  }
  if (!vbarycentric_.create()) {
    critical() << " Unable to create barycentric VBO";
  }
}

void TriangleMeshScene::loadShader()
{
  if (!this->glsl_program_->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                             ":/shader/triangle_mesh.vertex"))
  {
    critical() << " Vertex shader compile error: "
                << this->glsl_program_->log(); 
  }

  if (!this->glsl_program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                             ":/shader/triangle_mesh.fragment"))
  {
    critical() << " Fragment shader compile error: "
                << this->glsl_program_->log();
  }

  if (!this->glsl_program_->link())
  {
    critical() << " Shaders link error: "
                << this->glsl_program_->log();
  }

  debug() << "Shader compile and linked";
}

void TriangleMeshScene::setModelMat(const QMatrix4x4 &model)
{
  model_ = model;
}

QMatrix4x4 TriangleMeshScene::modelMat() const
{
  return model_;
}

void TriangleMeshScene::setViewMat(const QMatrix4x4 &view)
{
  view_ = view;
}

void TriangleMeshScene::setProjMat(const QMatrix4x4 &projection)
{
  proj_ = projection;
}

void TriangleMeshScene::rotate(float angle, const QVector3D &vector)
{
  model_.rotate(angle, vector);
}

void TriangleMeshScene::rotate(const QQuaternion &q)
{
  model_.rotate(q);
}

void TriangleMeshScene::allocatePos(int count)
{
  if (vpos_.bind()) {
    vpos_.allocate(count);
  } else {
    critical() << " Unable to bind position VBO while try to allocate pos buffer";
  }
}

void TriangleMeshScene::updatePos(int offset, const void *data, int count)
{
  if (vpos_.bind()) {
    vpos_.write(offset, data, count);
  } else {
    critical() << " Unable to bind position VBO while try to write pos buffer";
  }
}

void TriangleMeshScene::allocateVNormal(int count)
{
  if (vnormal_.bind()){
    vnormal_.allocate(count);
  } else {
    critical() << "Unable to bind vertex normal VBO while try to allocate vertex normal buffer";
  }
}

void TriangleMeshScene::updateVNormal(int offset, const void *data, int count)
{
  if (vnormal_.bind()){
    vnormal_.write(offset, data, count);
  } else {
    critical() << "Unable to bind vertex normal VBO while try to write vertex normal buffer";
  }
}

void TriangleMeshScene::allocateFNormal(int count)
{
  if (fnormal_.bind()){
    fnormal_.allocate(count);
  } else {
    critical() << "Unable to bind face normal VBO while try to allocate face normal buffer";
  }
}

void TriangleMeshScene::updateFNormal(int offset, const void *data, int count)
{
  if (fnormal_.bind()){
    fnormal_.write(offset, data, count);
  } else {
    critical() << "Unable to bind face normal VBO while try to write face normal buffer";
  }
}

void TriangleMeshScene::allocateBC(int count)
{
  if (vbarycentric_.bind()){
    vbarycentric_.allocate(count);
  } else {
    critical() << "Unable to bind barycentric VBO while try to allocate barycentric buffer";
  }
}

void TriangleMeshScene::updateBC(int offset, const void *data, int count)
{
  if (vbarycentric_.bind()){
    vbarycentric_.write(offset, data, count);
  } else {
    critical() << "Unable to bind barycentric VBO while try to write barycentric buffer";
  }
}

void TriangleMeshScene::allocateVboData(int count, unsigned int vbo)
{
  switch (vbo)
  {
    case VBO::POSITION:
    {
      allocatePos(count);
      break;
    }
    case VBO::VNORMAL:
    {
      allocateVNormal(count);
      break;
    }
    case VBO::FNORMAL:
    {
      allocateFNormal(count);
      break;
    }
    case VBO::BARYCENTRIC:
    {
      allocateBC(count);
      break;
    }
    default:
    {
      critical() << "Try to access unsupported vbo " << vbo;
      break;
    }
  }
}

void TriangleMeshScene::updateVboData(int offset,
                                      const void *data,
                                      int count,
                                      unsigned int vbo)
{
  switch (vbo)
  {
    case VBO::POSITION:
    {
      updatePos(offset, data, count);
      break;
    }
    case VBO::VNORMAL:
    {
      updateVNormal(offset, data, count);
      break;
    }
    case VBO::FNORMAL:
    {
      updateFNormal(offset, data, count);
      break;
    }
    case VBO::BARYCENTRIC:
    {
      updateBC(offset, data, count);
      break;
    }
    default:
    {
      critical() << "Try to access unsupported vbo " << vbo;
      break;
    }
  }
}

void TriangleMeshScene::setPrimitiveSize(std::size_t size)
{
  tri_size_ = size;
}

std::size_t TriangleMeshScene::primitiveSize() const
{
  return tri_size_;
}