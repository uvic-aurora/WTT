#ifndef WTT_DEMO_INCLUDE_CUSTOM_MESH_TYPES_HPP
#define WTT_DEMO_INCLUDE_CUSTOM_MESH_TYPES_HPP

#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/HalfedgeDS_vertex_base.h>
#include <CGAL/HalfedgeDS_face_base.h>
#include <CGAL/HalfedgeDS_halfedge_base.h>
#include <CGAL/Polyhedron_items_3.h>

#include <QObject>

template <class Refs, class P>
class MeshVertex: public CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, P>
{
public:
  using Base = CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, P>;
  using Point = P;
  using Vertex_handle = typename Refs::Vertex_handle;
  using Halfedge_handle = typename Refs::Halfedge_handle;
  MeshVertex() {}
  MeshVertex(const Point& p): Base(p) {}

  int id;
  int type;
  int level;
  bool border;
  std::pair<Vertex_handle, Vertex_handle> parents;
};

struct MeshItems: public CGAL::Polyhedron_items_3 {
  template <class Refs, class Traits>
  struct Vertex_wrapper {
    using Point = typename Traits::Point_3;
    using Vertex = MeshVertex<Refs, Point>;
  };
};

using CustomCGALMesh = CGAL::Polyhedron_3<CGAL::Simple_cartesian<double>, MeshItems>;
using Mesh = CustomCGALMesh;

class MeshOps {
public:
  using Mesh = CustomCGALMesh;
  using Vertex_const_handle = typename Mesh::Vertex_const_handle;
  using Vertex_handle = typename Mesh::Vertex_handle;
  static int get_vertex_id(Vertex_const_handle v)
  {
    return v->id;
  }

  static void set_vertex_id(Vertex_handle v, int id)
  {
    v->id = id;
  }

  static int get_vertex_level(Vertex_const_handle v)
  {
    return v->level;
  }

  static void set_vertex_level(Vertex_handle v, int level)
  {
    v->level = level;
  }

  static int get_vertex_type(Vertex_const_handle v)
  {
    return v->type;
  }

  static void set_vertex_type(Vertex_handle v, int type)
  {
    v->type = type;
  }

  static bool get_vertex_border(Vertex_const_handle v)
  {
    return v->border;
  }

  static void set_vertex_border(Vertex_handle v, bool border)
  {
    v->border = border;
  }
};

struct BoundingBox {
  double xmin = std::numeric_limits<double>::max();
  double xmax = std::numeric_limits<double>::min();
  double ymin = std::numeric_limits<double>::max();
  double ymax = std::numeric_limits<double>::min();
  double zmin = std::numeric_limits<double>::max();
  double zmax = std::numeric_limits<double>::min();

  double xc = 0.0;
  double yc = 0.0;
  double zc = 0.0;
  int vsize = 0;
  int fsize = 0;
  bool validate() {
    return (xmin < xmax) && (ymin < ymax) && (zmin < zmax);
  }
};

Q_DECLARE_METATYPE(Mesh);
Q_DECLARE_METATYPE(BoundingBox);
#endif