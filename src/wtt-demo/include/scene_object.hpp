#ifndef WTT_DEMO_INCLUDE_SCENE_OBJECT_HPP
#define WTT_DEMO_INCLUDE_SCENE_OBJECT_HPP

#include <QOpenGLFunctions>
class QOpenGLShaderProgram;

class SceneObject: public QObject, public QOpenGLFunctions
{
  Q_OBJECT
public:
  enum class SHADINGTYPE: int {
    SMOOTH = 0,
    FLAT = 1
  };
  explicit SceneObject(QObject* parent = 0);
  virtual ~SceneObject();

  virtual void render(SHADINGTYPE type) = 0;
  virtual void init() = 0;

  virtual void renderEdge(bool) = 0;

  virtual void setModelMat(const QMatrix4x4& model) = 0;
  virtual void setViewMat(const QMatrix4x4& view) = 0;
  virtual void setProjMat(const QMatrix4x4& projection) = 0;

  virtual void rotate(float angle, const QVector3D& vector) = 0;
  virtual void rotate(const QQuaternion& q) = 0;

  virtual void allocateVboData(int count,
                               unsigned int vbo) = 0;
  virtual void updateVboData(int offset,
                             const void* data,
                             int count,
                             unsigned int vbo) = 0;

  virtual void setPrimitiveSize(std::size_t size) = 0;
  virtual std::size_t primitiveSize() const = 0;

protected:
  QOpenGLShaderProgram* glsl_program_;
};

#endif  // define WTT_DEMO_INCLUDE_SCENE_OBJECT_HPP