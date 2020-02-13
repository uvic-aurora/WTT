#include "wtt_manager.hpp"
#include "triangle_mesh_scene.hpp"

#include <wtlib/loop_wavelet_transform.hpp>
#include <wtlib/butterfly_wavelet_transform.hpp>

#include <QVector3D>
#include <QOpenGLContext>
#include <QOffscreenSurface>

#include <QDebug>
#include <QFile>

#include <sstream>

WTTManager::WTTManager():
ThreadedGLBufferUploader(),
debug(DebugLogger("[WTTManager]")),
critical(FatalLogger("[WTTManager]"))
{

}

void WTTManager::onLoadMesh(QString filename) {
  debug() << "on loadMesh request";
  QFile mesh_file(filename);
  mesh_origin_.clear();
  mesh_for_wt_.clear();
  coefs_.clear();
  if (mesh_file.open(QFile::ReadOnly)) {
    std::stringstream mesh_stream;
    mesh_stream << QString(mesh_file.readAll()).toStdString();
    mesh_stream >> mesh_origin_;
  } else {
    critical() << "Unable to open mesh file " << mesh_file.fileName();
    emit meshLoaded(BoundingBox{}, "Fail to open " + mesh_file.fileName());
    prepareBuffer(mesh_origin_);
    return;
  }

  if (mesh_origin_.size_of_vertices() == 0) {
    critical() << "No vertices data.";
    emit meshLoaded(BoundingBox{}, "No data found");
    prepareBuffer(mesh_origin_);
    return;
  }

  if (!mesh_origin_.is_pure_triangle()) {
    critical() << "The mesh is not pure triangle.";
    emit meshLoaded(BoundingBox{}, "Input mesh is not pure triangle.");
    prepareBuffer(mesh_origin_);
    return;
  }

  mesh_origin_.normalize_border();
  for (auto [v, idx] = std::make_pair(mesh_origin_.vertices_begin(), 0); v != mesh_origin_.vertices_end(); ++v, ++idx) {
    v->id = idx;
  }
  mesh_for_wt_ = mesh_origin_;
  BoundingBox b = computeBBox(mesh_origin_);
  prepareBuffer(mesh_origin_);
  emit meshLoaded(b, "");
}

void WTTManager::onResetMesh() {
  mesh_for_wt_ = mesh_origin_;
  prepareBuffer(mesh_for_wt_);
  emit meshReset();
}


BoundingBox WTTManager::computeBBox(const Mesh &mesh) {
  BoundingBox b;
  b.vsize = mesh.size_of_vertices();
  b.fsize = mesh.size_of_facets();
  for (Vertex v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v) {
    b.xc += v->point().x();
    b.yc += v->point().y();
    b.zc += v->point().z();
    if (v->point().x() > b.xmax) {
      b.xmax = v->point().x();
    }
    if (v->point().x() < b.xmin) {
      b.xmin = v->point().x();
    }

    if (v->point().y() > b.ymax) {
      b.ymax = v->point().y();
    }
    if (v->point().y() < b.ymin) {
      b.ymin = v->point().y();
    }

    if (v->point().z() > b.zmax) {
      b.zmax = v->point().z();
    }
    if (v->point().z() < b.zmin) {
      b.zmin = v->point().z();
    }
  }
  b.xc /= static_cast<double>(mesh.size_of_vertices());
  b.yc /= static_cast<double>(mesh.size_of_vertices());
  b.zc /= static_cast<double>(mesh.size_of_vertices());
  return b;
}

void WTTManager::prepareBuffer(const Mesh& mesh) {
  using Halfedge_circulator = typename Mesh::Halfedge_around_vertex_const_circulator;
  debug() << "Prepare buffers for rendering";
  std::vector<GLfloat> vpos;
  std::vector<GLfloat> vbcs;
  std::vector<GLfloat> vnorms;
  std::vector<GLfloat> fnorms;
  vpos.reserve(mesh.size_of_facets() * 9);
  vbcs.reserve(mesh.size_of_facets() * 9);
  vnorms.reserve(mesh.size_of_facets() * 9);
  fnorms.reserve(mesh.size_of_facets() * 9);
  std::vector<QVector3D> vnorm_buffer(mesh.size_of_vertices());
  for (Vertex v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v) {
    QVector3D wn {0.0, 0.0, 0.0};
    Halfedge_circulator hc = v->vertex_begin();
    do {
      Vertex v1 = hc->opposite()->vertex();
      Vertex v2 = hc->next()->vertex();
      QVector3D vp(v->point().x(), v->point().y(), v->point().z());
      QVector3D v1p(v1->point().x(), v1->point().y(), v1->point().z());
      QVector3D v2p(v2->point().x(), v2->point().y(), v2->point().z());
      QVector3D normal {QVector3D::normal(v2p - vp, v1p - vp)};
      float area = 0.5 * QVector3D::crossProduct(v2p - vp, v1p - vp).length();
      wn += area * normal;
    } while (++hc != v->vertex_begin());
    wn.normalize();
    vnorm_buffer[v->id] = wn;
  }
  for (Facet f = mesh.facets_begin(); f != mesh.facets_end(); ++f)
  {
    Halfedge hc = f->facet_begin();
    Vertex v0 = hc->vertex();
    Vertex v1 = hc->next()->vertex();
    Vertex v2 = hc->next()->next()->vertex();

    vpos.push_back(static_cast<GLfloat>(v0->point().x()));
    vpos.push_back(static_cast<GLfloat>(v0->point().y()));
    vpos.push_back(static_cast<GLfloat>(v0->point().z()));

    vpos.push_back(static_cast<GLfloat>(v1->point().x()));
    vpos.push_back(static_cast<GLfloat>(v1->point().y()));
    vpos.push_back(static_cast<GLfloat>(v1->point().z()));

    vpos.push_back(static_cast<GLfloat>(v2->point().x()));
    vpos.push_back(static_cast<GLfloat>(v2->point().y()));
    vpos.push_back(static_cast<GLfloat>(v2->point().z()));

    vbcs.push_back(1.0);
    vbcs.push_back(0.0);
    vbcs.push_back(0.0);

    vbcs.push_back(0.0);
    vbcs.push_back(1.0);
    vbcs.push_back(0.0);

    vbcs.push_back(0.0);
    vbcs.push_back(0.0);
    vbcs.push_back(1.0);

    const QVector3D& vnormal0 = vnorm_buffer[v0->id];
    const QVector3D& vnormal1 = vnorm_buffer[v1->id];
    const QVector3D& vnormal2 = vnorm_buffer[v2->id];

    vnorms.push_back(static_cast<GLfloat>(vnormal0.x()));
    vnorms.push_back(static_cast<GLfloat>(vnormal0.y()));
    vnorms.push_back(static_cast<GLfloat>(vnormal0.z()));

    vnorms.push_back(static_cast<GLfloat>(vnormal1.x()));
    vnorms.push_back(static_cast<GLfloat>(vnormal1.y()));
    vnorms.push_back(static_cast<GLfloat>(vnormal1.z()));

    vnorms.push_back(static_cast<GLfloat>(vnormal2.x()));
    vnorms.push_back(static_cast<GLfloat>(vnormal2.y()));
    vnorms.push_back(static_cast<GLfloat>(vnormal2.z()));


    QVector3D v0p(v0->point().x(), v0->point().y(), v0->point().z());
    QVector3D v1p(v1->point().x(), v1->point().y(), v1->point().z());
    QVector3D v2p(v2->point().x(), v2->point().y(), v2->point().z());

    QVector3D fnormal(QVector3D::normal(v1p - v0p, v2p - v0p));

    fnorms.push_back(static_cast<GLfloat>(fnormal.x()));
    fnorms.push_back(static_cast<GLfloat>(fnormal.y()));
    fnorms.push_back(static_cast<GLfloat>(fnormal.z()));

    fnorms.push_back(static_cast<GLfloat>(fnormal.x()));
    fnorms.push_back(static_cast<GLfloat>(fnormal.y()));
    fnorms.push_back(static_cast<GLfloat>(fnormal.z()));

    fnorms.push_back(static_cast<GLfloat>(fnormal.x()));
    fnorms.push_back(static_cast<GLfloat>(fnormal.y()));
    fnorms.push_back(static_cast<GLfloat>(fnormal.z()));
  }
  uploadBuffer(vpos, vnorms, fnorms, vbcs);
  emit bufferUploaded();
  emit updateMeshInfo(mesh.size_of_vertices(), mesh.size_of_facets());
}

void WTTManager::uploadBuffer(const std::vector<GLfloat> &vpos, const std::vector<GLfloat> &vnorms, const std::vector<GLfloat>& fnorms, const std::vector<GLfloat> &vbcs) {
  debug() << "Update vertex buffers";
  if (!scene_ptr_) {
    critical() << "Scene is NULL";
    return;
  }
  this->context_->makeCurrent(this->surface_);

  scene_ptr_->allocateVboData(sizeof(GLfloat) * vpos.size(),
                              TriangleMeshScene::VBO::POSITION);
  scene_ptr_->updateVboData(0,
                            vpos.data(),
                            sizeof(GLfloat) * vpos.size(),
                            TriangleMeshScene::VBO::POSITION);
  scene_ptr_->allocateVboData(sizeof(GLfloat) * vnorms.size(),
                              TriangleMeshScene::VBO::VNORMAL);
  scene_ptr_->updateVboData(0,
                            vnorms.data(),
                            sizeof(GLfloat) * vnorms.size(),
                            TriangleMeshScene::VBO::VNORMAL);
  scene_ptr_->allocateVboData(sizeof(GLfloat) * fnorms.size(),
                              TriangleMeshScene::VBO::FNORMAL);
  scene_ptr_->updateVboData(0,
                            fnorms.data(),
                            sizeof(GLfloat) * fnorms.size(),
                            TriangleMeshScene::VBO::FNORMAL);
  scene_ptr_->allocateVboData(sizeof(GLfloat) * vbcs.size(),
                              TriangleMeshScene::VBO::BARYCENTRIC);
  scene_ptr_->updateVboData(0,
                            vbcs.data(),
                            sizeof(GLfloat) * vbcs.size(),
                            TriangleMeshScene::VBO::BARYCENTRIC);
  scene_ptr_->setPrimitiveSize(vpos.size() / 3);

  this->context_->doneCurrent();
}

void WTTManager::onDoFWT(int type, int level) {
  MeshOps meshops;
  bool res = false;
  coefs_.clear();
  if (type == WTType::LOOP) {
    debug() << "Performing " << level << " levels Loop FWT";
    res = wtlib::loop_analyze(mesh_for_wt_, meshops, coefs_, level);
  } else {
    debug() << "Performing " << level << " levels Butterfly FWT";
    if (!mesh_for_wt_.is_closed()) {
      emit fwtDone(false, level, "Butterfly WT is not supported on meshes with boundaries.");
      return;
    }
    res = wtlib::butterfly_analyze(mesh_for_wt_, meshops, coefs_, level);
  }
  if (!res) {
    emit fwtDone(false, level, "The mesh does not have " + QString::number(level) + " levels subdivision connectivity.");
    return;
  }
  prepareBuffer(mesh_for_wt_);
  emit fwtDone(true, level, "");
}

void WTTManager::onDoIWT(int type, int level) {
  using Modifier = wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, MeshOps>;
  MeshOps meshops;
  bool padding = false;
  if (coefs_.size() < level) {
    padding = true;
    coefs_.resize(level);
  }
  for (int i = 0; i < coefs_.size(); ++i) {
    int expect_size = Modifier::get_mesh_size(mesh_for_wt_, MeshOps{}, i + 1) - Modifier::get_mesh_size(mesh_for_wt_, MeshOps{}, i);
    if (coefs_[i].size() != expect_size) {
      padding = true;
      coefs_[i].resize(expect_size, Vector3 {0.0, 0.0, 0.0});
    }
  }

  if (type == WTType::BUTTERFLY) {
    debug() << "Performing " << level << " Butterfly IWT";
    if (!mesh_for_wt_.is_closed()) {
      emit iwtDone(false, level, "Butterfly WT is not supported on meshes with boundaries.");
      return;
    }
    wtlib::butterfly_synthesize(mesh_for_wt_, meshops, coefs_, level);
  } else {
    debug() << "Performing " << level << " Loop IWT";
    wtlib::loop_synthesize(mesh_for_wt_, meshops, coefs_, level);
  }

  QString msg;
  if (padding) {
    msg = QString("Zero wavelet coefficients padded.");
  } 

  prepareBuffer(mesh_for_wt_);
  emit iwtDone(true, level,  msg);
}

void WTTManager::onCompress(double perc) {
  debug() << "Performing compressing with compression rate " << perc << "%";
  int size = 0;
  for (const auto& v : coefs_) {
    size += v.size();
  }
  std::vector<std::pair<int, int>> idxmap;
  idxmap.reserve(size);
  for (int b = 0; b < coefs_.size(); ++b) {
    for (int i = 0; i < coefs_[b].size(); ++i) {
      idxmap.emplace_back(b, i);
    }
  }

  auto compare = [this](const auto& l, const auto& r) {
    const Vector3& lv = coefs_[l.first][l.second];
    const Vector3& rv = coefs_[r.first][r.second];
    return lv.squared_length() > rv.squared_length();
  };

  int desired_length = int(idxmap.size() * perc / 100.0);
  
  if (desired_length < size) {
    for (int i = desired_length; i < idxmap.size(); ++i) {
      std::pair<int, int> idx = idxmap[i];
      coefs_[idx.first][idx.second] = Vector3{0.0, 0.0, 0.0};
    }
  }
  QString msg = "Set " + QString::number(size > desired_length ? size - desired_length : 0) + " out of " + QString::number(size) + " wavelet coefficients to 0";

  emit compressDone(msg);
}

void WTTManager::onDenoise(int level) {
  debug() << "Performing " << level << " levels denosing";
  for (int l = 0; l < coefs_.size(); ++l) {
    if (l + 1 <= level) {
      continue;
    }
    std::vector<Vector3>& band_coefs = coefs_[l];
    for (Vector3& v : band_coefs) {
      v = Vector3{0.0, 0.0, 0.0};
    }
  }
  emit denoiseDone("Set wavelet coefficients in level " + QString::number(level) + " and above to 0");
}