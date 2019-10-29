#ifndef PTQ_IMPL_BUTTERFLY_WAVELET_OPERATIONS_HPP
#define PTQ_IMPL_BUTTERFLY_WAVELET_OPERATIONS_HPP

/**
 * @file     butterfly_wavelet_operations.hpp
 * @brief    Defines the lifting steps and other operations of the Butterfly wavelet tranform.
 * @copyright Copyright (c) 2019
 * 
 */


#include <wtlib/ptq_impl/subdivision_modifier.hpp>

#include <CGAL/Origin.h>

#include <vector>
#if defined (WTLIB_USE_UNORDERED_MAP)
#include <unordered_map>
#else
#include <map>
#endif  // if defined WTLIB_USE_UNORDERED_MAP

namespace wtlib::ptq_impl
{
template <class Mesh, class Mesh_ops, bool analysis>
class Butterfly_lift_operations
{
public:
  using Point3 = typename Mesh::Traits::Point_3;
  using Vec3 = typename Mesh::Traits::Vector_3;
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  using Halfedge_around_vertex_circulator = typename Mesh::Halfedge_around_vertex_circulator;
  using Modifier = PTQ_subdivision_modifier<Mesh, Mesh_ops>;
  using Halfedge_pair = typename Modifier::Halfedge_pair;

#if defined (WTLIB_USE_UNORDERED_MAP)
  using Vertex_scale = std::unordered_map<Vertex_handle, double>;
#else
  using Vertex_scale = std::map<Vertex_handle, double>;
#endif

  Butterfly_lift_operations():max_level_(-1) {}

  static int get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops)
  {
    return 2;
  }

  /**
   * @brief      Get the old vertex of type B
   *              c1----b0----c0
   *               \   / \   /
   *                \ /   \ /
   *                a0--e--a1
   *                / \   / \
   *               /   \ /   \
   *              c0----b1----c1
   * @param[in]  h     Halfedge from edge vertex to old vertex
   *
   * @return     The target vertex
   */
  static Vertex_handle get_vertex_B(Halfedge_handle h);

  /**
   * @brief      Gets the old vertex of type C0
   *              c1----b0----c0
   *               \   / \   /
   *                \ /   \ /
   *                a0--e--a1
   *                / \   / \
   *               /   \ /   \
   *              c0----b1----c1
   * @param[in]  h     Halfedge from edge vertex to old vertex
   *
   * @return     The target vertex
   */
  static Vertex_handle get_vertex_C0(Halfedge_handle h);

  /**
   * @brief      Get the old vertex of type C1
   *              c1----b0----c0
   *               \   / \   /
   *                \ /   \ /
   *                a0--e--a1
   *                / \   / \
   *               /   \ /   \
   *              c0----b1----c1
   * @param[in]  h     Halfedge from edge vertex to old vertex
   *
   * @return     The target vertex
   */
  static Vertex_handle get_vertex_C1(Halfedge_handle h);

  /**
   * @brief      Using edge vertices to update old vertices' scales
   *
   * @param      mesh         Input mesh
   * @param[in]  m_ops        Mesh operations
   * @param      edges_start  Start of edge vertices
   * @param      edges_end    End of edge vertices
   */
  void update_scale(Mesh& mesh,
                    const Mesh_ops& m_ops,
                    Vertex_handle* edges_start,
                    Vertex_handle* edges_end);

  /**
   * @brief      Lifting: using old vertices to modify edge vertices
   *
   * @param      mesh         The mesh
   * @param[in]  m_ops        The mesh operations
   * @param      edges_start  Start of edge vertices
   * @param      edges_end    End of edge vertices
   */
  void olds_to_edges(Mesh& mesh,
                     const Mesh_ops& m_ops,
                     Vertex_handle* edges_start,
                     Vertex_handle* edges_end) const;

  /**
   * @brief      Lifting: using edge vertices to modify old vertices
   *
   * @param      mesh         The mesh
   * @param[in]  m_ops        The mesh operations
   * @param      edges_start  Start of edge vertices
   * @param      edges_end    End of edge vertices
   */
  void edges_to_olds(Mesh& mesh,
                     const Mesh_ops& m_ops,
                     Vertex_handle* edges_start,
                     Vertex_handle* edges_end) const;

  /**
   * @brief      Set the scale for vertex
   *
   * @param[in]  v     The target vertex
   * @param[in]  s     scale
   */
  void set_vertex_scale(Vertex_handle v, double s);

  /**
   * @brief      Read the scale for given vertex
   *
   * @param[in]  v     The target vertex
   *
   * @return     The vertex scale.
   */
  double get_vertex_scale(Vertex_handle v) const;

  /**
   * @brief      Calculate scale ratio used in lift edges to olds
   *
   * @param[in]  old    The old vertex
   * @param[in]  edge   The edge vertex
   * @param[in]  m_ops  The mesh operation
   *
   * @return     The scale ratio.
   */
  double get_scale_ratio(Vertex_handle old,
                         Vertex_handle edge,
                         const Mesh_ops& m_ops) const; 

  void init_scale(Vertex_handle* head,
                  Vertex_handle* edge,
                  Vertex_handle* end)
  {
    for (Vertex_handle* p = head; p != end; ++p)
    {
      scales_.insert({*p, 1.0});
    }
  }

  void cleanup(Mesh& mesh,
               const Mesh_ops& mesh_ops)
  {
    scales_.clear();
    max_level_ = -1;
  }

protected:
  Vertex_scale scales_;
  int max_level_;
};  // class Butterfly_lift_operations


template <class Mesh, class Mesh_ops>
class Butterfly_analysis_operations: public Butterfly_lift_operations<Mesh,
                                                                      Mesh_ops,
                                                                      true>
{
public:
  using Base = Butterfly_lift_operations<Mesh, Mesh_ops, true>;
  using Vertex_handle = typename Base::Vertex_handle;

  void lift(Mesh& mesh,
            const Mesh_ops& mesh_ops,
            Vertex_handle** first_band,
            Vertex_handle** last_band)
  {
    assert(this->max_level_ >= 0);
    assert(last_band - first_band == 2);
    Vertex_handle* old_start = *first_band;
    Vertex_handle* edge_end = *(last_band--);
    Vertex_handle* edge_start = *last_band;

    assert(last_band - first_band == 1);

    this->olds_to_edges(mesh,
                        mesh_ops,
                        edge_start,
                        edge_end);

    this->edges_to_olds(mesh,
                        mesh_ops,
                        edge_start,
                        edge_end);

  }

  void initialize(Mesh &mesh,
                  const Mesh_ops &mesh_ops)
  {
    this->scales_.clear(); 
    this->max_level_ = -1;

    assert(mesh.is_closed());
    // Check if mesh is closed, and find the max level.
    for (Vertex_handle v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v)
    {
      int v_level = mesh_ops.get_vertex_level(v);
      if (v_level > this->max_level_)
      {
        this->max_level_ = v_level;
      }
    }
  }
};  // class Butterfly_analysis_operations



template <class Mesh, class Mesh_ops>
class Butterfly_synthesis_operations: public Butterfly_lift_operations<Mesh,
                                                                       Mesh_ops,
                                                                       false>
{
public:
  using Base = Butterfly_lift_operations<Mesh, Mesh_ops, false>;
  using Vertex_handle = typename Base::Vertex_handle;

  void lift(Mesh& mesh,
            const Mesh_ops& m_ops,
            Vertex_handle** first_band,
            Vertex_handle** last_band)
  {
    assert(this->max_level_ >= 0);
    assert(last_band - first_band == 2);
    Vertex_handle* old_start = *first_band;
    Vertex_handle* edge_end = *(last_band--);
    Vertex_handle* edge_start = *last_band;

    assert(last_band - first_band == 1);

    this->edges_to_olds(mesh,
                        m_ops,
                        edge_start,
                        edge_end);

    this->olds_to_edges(mesh,
                        m_ops,
                        edge_start,
                        edge_end);

  }

  void initialize(Mesh& mesh, const Mesh_ops& m_ops, int max_level)
  {
    this->scales_.clear();
    assert(mesh.is_closed());
    this->max_level_ = max_level;
  }
};  // class Butterfly_synthesis_operations



template <class Mesh, class Mesh_ops, bool analysis>
typename Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::Vertex_handle
Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::get_vertex_B(Halfedge_handle h)
{
  h = h->prev()->opposite();
  h = h->next()->opposite();
  h = h->next();

  return h->vertex();
}

template <class Mesh, class Mesh_ops, bool analysis>
typename Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::Vertex_handle
Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::get_vertex_C0(Halfedge_handle h)
{
  h = h->next()->opposite();
  h = h->prev()->opposite();
  h = h->next()->opposite();

  return h->next()->vertex();
}

template <class Mesh, class Mesh_ops, bool analysis>
typename Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::Vertex_handle
Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::get_vertex_C1(Halfedge_handle h)
{
  h = h->opposite();
  h = h->prev()->opposite();
  h = h->next()->opposite();
  h = h->prev()->opposite();

  return h->next()->vertex();
}

template <class Mesh, class Mesh_ops, bool analysis>
void Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::set_vertex_scale(
                                                  Vertex_handle v,
                                                  double s)
{
  scales_[v] = s;
}


template <class Mesh, class Mesh_ops, bool analysis>
double Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::get_vertex_scale(
                                                  Vertex_handle v) const
{
  return scales_.at(v);
}

template <class Mesh, class Mesh_ops, bool analysis>
double Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::get_scale_ratio(
                                                  Vertex_handle old,
                                                  Vertex_handle edge,
                                                  const Mesh_ops& m_ops) const
{
  /**
   * For closed mesh, vertex scale is related only to its valence and the
   * current processing level, so the vertex scale can be calculated directly.
   */
  double no = static_cast<double>(old->degree());

  assert(this->max_level_ >= 0);

  int level = m_ops.get_vertex_level(edge);
  assert(level > 0);
  int l = max_level_ - level;
  assert(l >= 0);
  double edge_integral = std::pow(4.0, l);
  double old_integral = (std::pow(4.0, l + 1) - 1) / 6.0 * no + 1.0;
  
  return edge_integral / (2.0 * old_integral);
}


template <class Mesh, class Mesh_ops, bool analysis>
void Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::update_scale(
                                                  Mesh &mesh,
                                                  const Mesh_ops &m_ops,
                                                  Vertex_handle *edges_start,
                                                  Vertex_handle *edges_end)
{
  for (Vertex_handle *p = edges_start; p != edges_end; ++p)
  {
    Vertex_handle e = *p;
    double se = scales_.at(e);
    Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(e, m_ops)};

    Vertex_handle a0 = hps.first->vertex();
    Vertex_handle a1 = hps.second->vertex();

    Vertex_handle b0 = get_vertex_B(hps.first);
    Vertex_handle b1 = get_vertex_B(hps.second);

    Vertex_handle c0 = get_vertex_C0(hps.first);
    Vertex_handle c1 = get_vertex_C1(hps.first);
    Vertex_handle c2 = get_vertex_C0(hps.second);
    Vertex_handle c3 = get_vertex_C1(hps.second);

    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(a0));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(a1));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(b0));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(b1));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(c0));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(c1));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(c2));
    assert(m_ops.get_vertex_level(e) > m_ops.get_vertex_level(c3));

    scales_[a0] += 0.5 * se;
    scales_[a1] += 0.5 * se;
    scales_[b0] += 0.125 * se;
    scales_[b1] += 0.125 * se;
    scales_[c0] -= 0.0625 * se;
    scales_[c1] -= 0.0625 * se;
    scales_[c2] -= 0.0625 * se;
    scales_[c3] -= 0.0625 * se;
  }
}

template <class Mesh, class Mesh_ops, bool analysis>
void Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::olds_to_edges(
                                                  Mesh &mesh,
                                                  const Mesh_ops &m_ops,
                                                  Vertex_handle *edges_start,
                                                  Vertex_handle *edges_end) const
{
  for (Vertex_handle* p = edges_start; p != edges_end; ++p)
  {
    Vertex_handle e = *p;
    assert(!m_ops.get_vertex_border(e) && "Open mesh is not supported");

    Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(e, m_ops)};
    Vertex_handle a0 = hps.first->vertex();
    Vertex_handle a1 = hps.second->vertex();
    Vertex_handle b0 = get_vertex_B(hps.first);
    Vertex_handle b1 = get_vertex_B(hps.second);
    Vertex_handle c0 = get_vertex_C0(hps.first);
    Vertex_handle c1 = get_vertex_C1(hps.first);
    Vertex_handle c2 = get_vertex_C0(hps.second);
    Vertex_handle c3 = get_vertex_C1(hps.second);

    assert(!m_ops.get_vertex_border(a0) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(a1) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(b0) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(b1) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(c0) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(c1) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(c2) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(c3) && "Open mesh is not supported");

    Vec3 ve  {CGAL::ORIGIN,  e->point()};
    Vec3 va0 {CGAL::ORIGIN, a0->point()};
    Vec3 va1 {CGAL::ORIGIN, a1->point()};
    Vec3 vb0 {CGAL::ORIGIN, b0->point()};
    Vec3 vb1 {CGAL::ORIGIN, b1->point()};
    Vec3 vc0 {CGAL::ORIGIN, c0->point()};
    Vec3 vc1 {CGAL::ORIGIN, c1->point()};
    Vec3 vc2 {CGAL::ORIGIN, c2->point()};
    Vec3 vc3 {CGAL::ORIGIN, c3->point()};

    if (analysis)
    {
      ve = ve - 0.5 * (va0 + va1)
              - 0.125 * (vb0 + vb1)
              + 0.0625 * (vc0 + vc1 + vc2 + vc3);
    }
    else
    {
      ve = ve + 0.5 * (va0 + va1)
              + 0.125 * (vb0 + vb1)
              - 0.0625 * (vc0 + vc1 + vc2 + vc3);
    }
    // Write result back to mesh vertex
    e->point() = CGAL::ORIGIN + ve;
  }
}


template <class Mesh, class Mesh_ops, bool analysis>
void Butterfly_lift_operations<Mesh, Mesh_ops, analysis>::edges_to_olds(
                                                  Mesh &mesh,
                                                  const Mesh_ops &m_ops,
                                                  Vertex_handle *edges_start,
                                                  Vertex_handle *edges_end) const
{
  for (Vertex_handle* p = edges_start; p != edges_end; ++p)
  {
    Vertex_handle e = *p;
    assert(!m_ops.get_vertex_border(e) && "Open mesh is not supported");

    Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(e, m_ops)};
    Vertex_handle a0 = hps.first->vertex();
    Vertex_handle a1 = hps.second->vertex();

    assert(!m_ops.get_vertex_border(a0) && "Open mesh is not supported");
    assert(!m_ops.get_vertex_border(a1) && "Open mesh is not supported");

    double ra0 = get_scale_ratio(a0, e, m_ops);
    double ra1 = get_scale_ratio(a1, e, m_ops);

    Vec3 ve  {CGAL::ORIGIN, e->point()};
    Vec3 va0 {CGAL::ORIGIN, a0->point()};
    Vec3 va1 {CGAL::ORIGIN, a1->point()};

    if (analysis)
    {
      va0 = va0 - ra0 * ve;
      va1 = va1 - ra1 * ve;
    }
    else
    {
      va0 = va0 + ra0 * ve;
      va1 = va1 + ra1 * ve;
    }
    // Write result back to mesh vertex
    a0->point() = CGAL::ORIGIN + va0;
    a1->point() = CGAL::ORIGIN + va1;
  }
}
}  // define wtlib::ptq_impl

#endif  // define PTQ_IMPL_BUTTERFLY_WAVELET_OPERATIONS_HPP