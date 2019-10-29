#ifndef WTLIB_MESH_TYPES_HPP
#define WTLIB_MESH_TYPES_HPP

/**
 * @file     mesh_types.hpp
 * @brief    Defines a custom mesh instantiation.
 */

#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#if defined (WTLIB_USE_CUSTOM_MESH)
#include <CGAL/HalfedgeDS_vertex_base.h>
#include <CGAL/HalfedgeDS_face_base.h>
#include <CGAL/HalfedgeDS_halfedge_base.h>
#include <CGAL/Polyhedron_items_3.h>
#endif

#if defined (WTLIB_USE_CUSTOM_MESH)
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

using Mesh = CGAL::Polyhedron_3<CGAL::Simple_cartesian<double>, MeshItems>;
#else
using Mesh = CGAL::Polyhedron_3<CGAL::Simple_cartesian<double>>;
#endif

#endif