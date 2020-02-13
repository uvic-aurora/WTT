#ifndef WTT_DEMO_INCLUDE_WTT_MANAGER_HPP
#define WTT_DEMO_INCLUDE_WTT_MANAGER_HPP

#include "custom_mesh_types.hpp"
#include "threaded_gl_buffer_uploader.hpp"
#include "logger.hpp"

#include <QThread>
#include <QOpenGLFunctions>

class SceneObject;
class QOpenGLContext;
class QOffscreenSurface;

class WTTManager: public ThreadedGLBufferUploader{
  Q_OBJECT
public:
  enum WTType {
    LOOP = 0,
    BUTTERFLY = 1
  };
  using Halfedge = typename Mesh::Halfedge_const_handle;
  using Vertex = typename Mesh::Vertex_const_handle;
  using Facet = typename Mesh::Facet_const_handle;
  using Vector3 = typename Mesh::Traits::Vector_3;
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Vertex_const_handle = typename Mesh::Vertex_const_handle;
  explicit WTTManager();
  void obtainSceneInOtherThread(SceneObject* s) { scene_ptr_ = s;}

public slots:
  void onLoadMesh(QString filename);
  void onResetMesh();
  BoundingBox computeBBox(const Mesh& mesh);

  void onDoFWT(int type, int level);

  void onDoIWT(int type, int level);

  void onCompress(double perc);
  void onDenoise(int level);
  void prepareBuffer(const Mesh& mesh);

  void uploadBuffer(const std::vector<GLfloat>& vpos,
                    const std::vector<GLfloat>& vnormals,
                    const std::vector<GLfloat>& fnormals,
                    const std::vector<GLfloat>& vbcs);
signals:
  void meshLoaded(BoundingBox bbox, QString err);
  void meshReset();
  void checkSCDone(bool, int);
  void fwtDone(bool, int, QString err);
  void iwtDone(bool, int, QString msg);
  void compressDone(QString msg);
  void denoiseDone(QString msg);

  void updateMeshInfo(int vsize, int fsize);

protected:
  SceneObject* scene_ptr_;
  Mesh mesh_origin_;
  Mesh mesh_for_wt_;
  std::vector<std::vector<Vector3>> coefs_;
  DebugLogger debug;
  FatalLogger critical;
};

#endif