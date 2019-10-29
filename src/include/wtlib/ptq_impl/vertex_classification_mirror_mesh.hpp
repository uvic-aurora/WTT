#ifndef PTQ_IMPL_CLASSIFY_VERTICES_MIRROR_MESH_HPP
#define PTQ_IMPL_CLASSIFY_VERTICES_MIRROR_MESH_HPP

/**
 * @file  vertex_classification_mirror_mesh.hpp
 * @brief This file defines several basic types that are used to instantiate
 *        CGAL::Polyhedron_3 and then define the mirror mesh. The mirror mesh is
 *        used for avoding breaking the original mesh structure while
 *        identifying subdivision connectivity and classifying vertices.
 *
 */

#include <CGAL/HalfedgeDS_face_base.h>
#include <CGAL/HalfedgeDS_vertex_base.h>
#include <CGAL/HalfedgeDS_halfedge_base.h>

namespace wtlib::ptq_impl
{

/**
 * @brief The type traits of the mirror mesh. The type traits follow the
 *        requirement of CGAL, so the CGAL halfedge data structure can be used
 *        to construct the mirror mesh.
 *
 * @tparam M 
 */
template <class M>
class Traits
{
public:
  /**
   * @brief The point type of the mirror, which is a vertex handle of the mesh
   *        that is mirrored.
   *
   */
  using Point_3 = typename M::Vertex_handle;
  using Point = Point_3;
  
  class No_plane {};
  using Plane_3 = No_plane;

  using size_type = std::size_t;
  using FT = double;
};

/**
 * @brief The vertex definition of the mirror mesh, which holds a vertex handle
 *        of the original mesh as its point. 
 *
 * @tparam Refs   The type of a container that hold the vertices.
 * @tparam P      Point type, which will be instantiated as Vertex_handle.
 */
template <class Refs, class P>
class Mirror_vertex: public CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, P>
{
public:
  using Base = CGAL::HalfedgeDS_vertex_base<Refs, CGAL::Tag_true, P>;
  using Point = P;
  using Target = Point;

  
  Mirror_vertex(): Base(), border_(false), finer_(false) {};
  Mirror_vertex(const Point& p): Base(p), border_(false), finer_(false) {};

  void set_border(bool b)
  {
    border_ = b;
  }
  
  bool is_border() const
  {
    return border_;
  }
  
  void set_finer(bool finer)
  {
    finer_ = finer;
  }

  bool is_finer() const
  {
    return finer_;
  }

private:
  bool border_;

  /**
   * @brief Flags whether the mirror as well as the mirrored vertex is a finer
   *        vertex.
   *
   */
  bool finer_;
};


template <class Refs>
class Mirror_face: public CGAL::HalfedgeDS_face_base<Refs>
{
public:
  void set_visited(bool v)
  {
    visited_ = v;
  }

  bool visited() const
  {
    return visited_;
  }

private:
  bool visited_;
};

/**
 * @brief    Items wrapper. It is required by CGAL::Polyhedron_3 specifications.
 * 
 */
class Mirror_mesh_items
{
public:
  template <class Refs, class Traits>
  struct Vertex_wrapper
  {
    using Point = typename Traits::Point_3;
    using Vertex = Mirror_vertex<Refs, Point>;
  };

  template <class Refs, class Traits>
  struct Halfedge_wrapper
  {
    using Halfedge = CGAL::HalfedgeDS_halfedge_base<Refs>;
  };

  template <class Refs, class Traits>
  struct Face_wrapper
  {
    using Face = Mirror_face<Refs>;
  };
};
}  // namespace wtlib::ptq_impl
#endif
