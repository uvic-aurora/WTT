#ifndef PTQ_IMPL_LOOP_ANALYSIS_OPERATIONS_HPP
#define PTQ_IMPL_LOOP_ANALYSIS_OPERATIONS_HPP

/**
 * @file     loop_wavelet_operations.hpp
 * @brief    Defines the lifting steps of the Loop wavelet transform.
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <wtlib/ptq_impl/loop_math_utils.hpp>
#include <wtlib/ptq_impl/subdivision_modifier.hpp>

#include <CGAL/Origin.h>

#include <vector>
#include <cassert>



namespace wtlib::ptq_impl
{
template <class Mesh, class Mesh_ops, bool analysis>
class Loop_lift_operations
{
public:
  using Point3 = typename Mesh::Traits::Point_3;
  using Vec3 = typename Mesh::Traits::Vector_3;
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  using Halfedge_around_vertex_circulator = typename Mesh::Halfedge_around_vertex_circulator;
  using Modifier = PTQ_subdivision_modifier<Mesh, Mesh_ops>;
  using Halfedge_pair = typename Modifier::Halfedge_pair;

  /**
   * @brief      Lifting steps.
   *
   * @param      m           input mesh
   * @param[in]  m_ops       The mesh operations
   * @param      first_band  Pointer to pointer to the start vertex handle.
   * @param      last_band   Pointer to pointer to the end vertex handle.
   */
  static void lift(Mesh& m, 
                   const Mesh_ops& m_ops,
                   Vertex_handle** first_band,
                   Vertex_handle** last_band);

  static void border_edges_to_border_olds(
                          Mesh& m, 
                          const Mesh_ops& m_ops,
                          Vertex_handle* old_start,
                          Vertex_handle* edge_start,
                          Vertex_handle* edge_end);

  static void border_olds_to_border_edges(
                          Mesh& m, 
                          const Mesh_ops& m_ops,
                          Vertex_handle* old_start,
                          Vertex_handle* edge_start,
                          Vertex_handle* edge_end);

  static void inner_edges_to_inner_olds(
                          Mesh& m, 
                          const Mesh_ops& m_ops,
                          Vertex_handle* old_start,
                          Vertex_handle* edge_start,
                          Vertex_handle* edge_end);

  static void inner_olds_to_inner_edges(
                          Mesh& m, 
                          const Mesh_ops& m_ops,
                          Vertex_handle* old_start,
                          Vertex_handle* edge_start,
                          Vertex_handle* edge_end);

  static void border_edges_to_border_olds_dual(
                          Mesh& m, 
                          const Mesh_ops& m_ops,
                          Vertex_handle* old_start,
                          Vertex_handle* edge_start,
                          Vertex_handle* edge_end);

  static void inner_edges_to_inner_olds_dual(
                          Mesh& m, 
                          const Mesh_ops& m_ops,
                          Vertex_handle* old_start,
                          Vertex_handle* edge_start,
                          Vertex_handle* edge_end);

  static void initialize(
                    Mesh& m,
                    const Mesh_ops& m_ops,
                    int num_levels)
  {
    return;
  }

  static void cleanup(
                    Mesh& m,
                    const Mesh_ops& m_ops)
  {
    return;
  }

  /**
   * @brief      Navigate to halfedge opposite vertex on loop mask
   *
   * @param[in]  h     Halfedge
   *
   * @return     The target vertex
   */
  static Vertex_handle opposite_vertex(Halfedge_handle h);

  static int get_num_types(Mesh& mesh,
                           const Mesh_ops& m_ops)
  {
    return 2;
  }
};  // class Loop_lift_operations


template <class Mesh, class Mesh_ops>
class Loop_analysis_operations
{
public:
  using Lift = Loop_lift_operations<Mesh, Mesh_ops, true>;
  using Vertex_handle = typename Lift::Vertex_handle;

  static void lift(
                Mesh& m, 
                const Mesh_ops& m_ops,
                Vertex_handle** first_band,
                Vertex_handle** last_band)
  {
    assert(last_band - first_band == 2);
    Vertex_handle* old_start = *first_band;
    Vertex_handle* edge_end = *(last_band--);
    Vertex_handle* edge_start = *last_band;
    assert(last_band - first_band == 1);

    Lift::border_edges_to_border_olds(m, 
                                      m_ops,
                                      old_start,
                                      edge_start,
                                      edge_end);
    Lift::border_olds_to_border_edges(m, 
                                      m_ops,
                                      old_start,
                                      edge_start,
                                      edge_end);
    Lift::inner_edges_to_inner_olds(m, 
                                    m_ops,
                                    old_start,
                                    edge_start,
                                    edge_end);
    Lift::inner_olds_to_inner_edges(m, 
                                    m_ops,
                                    old_start,
                                    edge_start,
                                    edge_end);
    Lift::border_edges_to_border_olds_dual(m, 
                                           m_ops,
                                           old_start,
                                           edge_start,
                                           edge_end);
    Lift::inner_edges_to_inner_olds_dual(m, 
                                         m_ops,
                                         old_start,
                                         edge_start,
                                         edge_end);
  }

  static int get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops)
  {
    return 2;
  }

  static void initialize(Mesh& mesh, const Mesh_ops& m_ops)
  {
    return;
  }

  static void clean_up(Mesh& mesh, const Mesh_ops& m_ops)
  {
    return;
  }
};  // class Loop_analysis_operations

template <class Mesh, class Mesh_ops>
class Loop_synthesis_operations
{
public:
  using Lift = Loop_lift_operations<Mesh, Mesh_ops, false>;
  using Vertex_handle = typename Lift::Vertex_handle;

  static void initialize(Mesh& mesh, const Mesh_ops& m_ops, int levels)
  {
    return;
  }

  static void clean_up(Mesh& mesh, const Mesh_ops& m_ops)
  {
    return;
  }

  static int get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops)
  {
    return 2;
  }

  static void lift(
                Mesh& m, 
                const Mesh_ops& m_ops,
                Vertex_handle** first_band,
                Vertex_handle** last_band)
  {
    assert(last_band - first_band == 2);
    Vertex_handle* old_start = *first_band;
    Vertex_handle* edge_end = *(last_band--);
    Vertex_handle* edge_start = *last_band;
    assert(last_band - first_band == 1);

    Lift::inner_edges_to_inner_olds_dual(m, 
                                         m_ops,
                                         old_start,
                                         edge_start,
                                         edge_end);
    Lift::border_edges_to_border_olds_dual(m, 
                                           m_ops,
                                           old_start,
                                           edge_start,
                                           edge_end);
    Lift::inner_olds_to_inner_edges(m, 
                                    m_ops,
                                    old_start,
                                    edge_start,
                                    edge_end);
    Lift::inner_edges_to_inner_olds(m, 
                                    m_ops,
                                    old_start,
                                    edge_start,
                                    edge_end);
    Lift::border_olds_to_border_edges(m, 
                                      m_ops,
                                      old_start,
                                      edge_start,
                                      edge_end);
    Lift::border_edges_to_border_olds(m, 
                                      m_ops,
                                      old_start,
                                      edge_start,
                                      edge_end);
  }
};  // class Loop_synthesis_operations


template <class Mesh, class Mesh_ops, bool analysis>
typename Loop_lift_operations<Mesh, Mesh_ops, analysis>::Vertex_handle
Loop_lift_operations<Mesh, Mesh_ops, analysis>::opposite_vertex(Halfedge_handle h)
{
  assert(h != Halfedge_handle {});
  assert(!h->is_border());

  h = h->prev();
  h = h->opposite();

  assert(!h->is_border());

  h = h->next();
  h = h->opposite();

  assert(!h->is_border());
  h = h->next();

  return h->vertex();
}

template <class Mesh, class Mesh_ops, bool analysis>
void Loop_lift_operations<Mesh, Mesh_ops, analysis>::border_edges_to_border_olds(
                                                Mesh &m, 
                                                const Mesh_ops &m_ops, 
                                                Vertex_handle *first_band, 
                                                Vertex_handle *edge_start,
                                                Vertex_handle *last_band)
{
  for (Vertex_handle* v_ptr = first_band; v_ptr != edge_start; ++v_ptr)
  {
    Vertex_handle vo = *v_ptr;
    if (m_ops.get_vertex_border(vo))
    {
      Halfedge_pair hps {Modifier::get_halfedges_to_borders(vo)};

      Vertex_handle e0 = hps.first->vertex();
      Vertex_handle e1 = hps.second->vertex();

      assert(m_ops.get_vertex_border(e0));
      assert(m_ops.get_vertex_border(e1));
      assert(m_ops.get_vertex_level(e0) > m_ops.get_vertex_level(vo));
      assert(m_ops.get_vertex_level(e1) > m_ops.get_vertex_level(vo));

      Vec3 res {CGAL::ORIGIN, vo->point()};
      Vec3 ve0 {CGAL::ORIGIN, e0->point()};
      Vec3 ve1 {CGAL::ORIGIN, e1->point()};

      if (analysis)
      {
        res = res - 0.25 * (ve0 + ve1);
        res = res * 2.0;
      }
      else
      {
        res = res / 2.0;
        res = res + 0.25 * (ve0 + ve1);
      }

      vo->point() = CGAL::ORIGIN + res;
    }
  }
}

template <class Mesh, class Mesh_ops, bool analysis>
void Loop_lift_operations<Mesh, Mesh_ops, analysis>::border_olds_to_border_edges(
                                                Mesh &m,
                                                const Mesh_ops &m_ops,
                                                Vertex_handle *first_band,
                                                Vertex_handle *edge_start,
                                                Vertex_handle *last_band)
{
  for (Vertex_handle* v_ptr = edge_start; v_ptr != last_band; ++v_ptr)
  {
    Vertex_handle ve = *v_ptr;
    if (m_ops.get_vertex_border(ve))
    {
      Halfedge_pair hps {Modifier::get_halfedges_to_borders(ve)};
      Vertex_handle o0 = hps.first->vertex();
      Vertex_handle o1 = hps.second->vertex();
      
      assert(m_ops.get_vertex_border(o0));
      assert(m_ops.get_vertex_border(o1));
      assert(m_ops.get_vertex_level(o0) < m_ops.get_vertex_level(ve));
      assert(m_ops.get_vertex_level(o1) < m_ops.get_vertex_level(ve));

      Vec3 res {CGAL::ORIGIN, ve->point()};
      Vec3 vo0 {CGAL::ORIGIN, o0->point()};
      Vec3 vo1 {CGAL::ORIGIN, o1->point()};

      if (analysis)
      {
        res = res - 0.5 * (vo0 + vo1);
      }
      else
      {
        res = res + 0.5 * (vo0 + vo1);
      }

      ve->point() = CGAL::ORIGIN + res;
    }
  }
}

template <class Mesh, class Mesh_ops, bool analysis>
void Loop_lift_operations<Mesh, Mesh_ops, analysis>::inner_edges_to_inner_olds(
                                                Mesh &m,
                                                const Mesh_ops &m_ops,
                                                Vertex_handle *first_band,
                                                Vertex_handle *edge_start,
                                                Vertex_handle *last_band)
{
  for (Vertex_handle* v_ptr = first_band; v_ptr != edge_start; ++v_ptr)
  {
    // The center old vertex of current mask.
    Vertex_handle v = *v_ptr;

    // The old vertex should be interior vertex.
    if (!m_ops.get_vertex_border(v))
    {
      double delta {Loop_math::delta(v->degree())};
      double beta {Loop_math::beta(v->degree())};

      Halfedge_around_vertex_circulator hcir = v->vertex_begin();

      Vec3 o {CGAL::ORIGIN, v->point()};
      Vec3 e_sum {0.0, 0.0, 0.0};
      do
      {
        Vertex_handle ve = hcir->opposite()->vertex();
        assert(!m_ops.get_vertex_border(ve));
        assert(m_ops.get_vertex_level(ve) > m_ops.get_vertex_level(v)); 
        Vec3 e {CGAL::ORIGIN, ve->point()};
        e_sum = e_sum + e;
        ++hcir;
      }
      while (hcir != v->vertex_begin());

      if (analysis)
      {
        o = o - delta * (e_sum);
        o = o / beta;
      }
      else
      {
        o = o * beta;
        o = o + delta * (e_sum);
      }

      v->point() = CGAL::ORIGIN + o;
    }
  }
}

template <class Mesh, class Mesh_ops, bool analysis>
void Loop_lift_operations<Mesh, Mesh_ops, analysis>::inner_olds_to_inner_edges(
                                                Mesh &m,
                                                const Mesh_ops &m_ops,
                                                Vertex_handle *first_band,
                                                Vertex_handle *edge_start,
                                                Vertex_handle *last_band)
{
  // Using Old vertices to modify inner edge vertices
  for (Vertex_handle* v_ptr = edge_start; v_ptr != last_band; ++v_ptr)
  {
    // The center edge vertex of the current mask
    Vertex_handle v = *v_ptr;

    // The edge vertex should be interior vertex
    if (!m_ops.get_vertex_border(v))
    {
      Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(v, m_ops)};
      Halfedge_handle h0 = hps.first;
      Halfedge_handle h1 = hps.second;

      Vertex_handle vo0 = h0->vertex();
      Vertex_handle vo1 = h1->vertex();

      Vertex_handle vo2 = opposite_vertex(h0);
      Vertex_handle vo3 = opposite_vertex(h1);
      assert(vo0 != Vertex_handle {});
      assert(vo1 != Vertex_handle {});
      assert(vo2 != Vertex_handle {});
      assert(vo3 != Vertex_handle {});
      assert(m_ops.get_vertex_level(vo2) < m_ops.get_vertex_level(v));
      assert(m_ops.get_vertex_level(vo3) < m_ops.get_vertex_level(v));

      Vec3 o0 {CGAL::ORIGIN, vo0->point()};
      Vec3 o1 {CGAL::ORIGIN, vo1->point()};
      Vec3 o2 {CGAL::ORIGIN, vo2->point()};
      Vec3 o3 {CGAL::ORIGIN, vo3->point()};
      Vec3 e {CGAL::ORIGIN, v->point()};

      if (analysis)
      {
        e = e - 0.375 * (o0 + o1) - 0.125 * (o2 + o3);
      }
      else
      {
        e = e + 0.375 * (o0 + o1) + 0.125 * (o2 + o3);
      }

      v->point() = CGAL::ORIGIN + e;
    }
  }
}

template <class Mesh, class Mesh_ops, bool analysis>
void Loop_lift_operations<Mesh, Mesh_ops, analysis>::border_edges_to_border_olds_dual(
                                                Mesh &m,
                                                const Mesh_ops &m_ops,
                                                Vertex_handle *first_band,
                                                Vertex_handle *edge_start,
                                                Vertex_handle *last_band)
{
  for (Vertex_handle* v_ptr = edge_start; v_ptr != last_band; ++v_ptr)
  {
    // The center edge vertex of the current mask
    Vertex_handle v = *v_ptr;

    // The edge vertex should be border vertex
    if (m_ops.get_vertex_border(v))
    {
      // The two outgoing halfedges from v to its old neighbors
      Halfedge_pair hps {Modifier::get_halfedges_to_borders(v)};

      Halfedge_handle h0 = hps.first;
      Halfedge_handle h1 = hps.second;

      // The two closet old border neighbors
      Vertex_handle vo0 = h0->vertex();
      Vertex_handle vo1 = h1->vertex();

      Vertex_handle vo2;
      Vertex_handle vo3;

      // Navigate to its farther old border neighbors
      if (h0->is_border())
      {
        vo2 = h0->next()->next()->vertex();
      }
      else
      {
        vo2 = h0->opposite()->prev()->prev()->opposite()->vertex();
      }

      if (h1->is_border())
      {
        vo3 = h1->next()->next()->vertex();
      }
      else
      {
        vo3 = h1->opposite()->prev()->prev()->opposite()->vertex();
      }

      assert(vo0 != Vertex_handle {});
      assert(vo1 != Vertex_handle {});
      assert(vo2 != Vertex_handle {});
      assert(vo3 != Vertex_handle {});

      // Precondition: o0-o3 should have less levels
      assert(m_ops.get_vertex_level(vo0) < m_ops.get_vertex_level(v));
      assert(m_ops.get_vertex_level(vo1) < m_ops.get_vertex_level(v));
      assert(m_ops.get_vertex_level(vo2) < m_ops.get_vertex_level(v));
      assert(m_ops.get_vertex_level(vo3) < m_ops.get_vertex_level(v));

      // Precondition: o2,o3 should on border
      assert(m_ops.get_vertex_border(vo2));
      assert(m_ops.get_vertex_border(vo3));

      Vec3 e {CGAL::ORIGIN, v->point()};
      Vec3 o0 {CGAL::ORIGIN, vo0->point()};
      Vec3 o1 {CGAL::ORIGIN, vo1->point()};
      Vec3 o2 {CGAL::ORIGIN, vo2->point()};
      Vec3 o3 {CGAL::ORIGIN, vo3->point()};

      double eta0 = -0.525336;
      double eta1 = -0.525336;
      double eta2 =  0.189068;
      double eta3 =  0.189068;

      if (analysis)
      {
        o0 = o0 - eta0 * e;
        o1 = o1 - eta1 * e;
        o2 = o2 - eta2 * e;
        o3 = o3 - eta3 * e;
      }
      else
      {
        o0 = o0 + eta0 * e;
        o1 = o1 + eta1 * e;
        o2 = o2 + eta2 * e;
        o3 = o3 + eta3 * e;
      }

      vo0->point() = CGAL::ORIGIN + o0;
      vo1->point() = CGAL::ORIGIN + o1;
      vo2->point() = CGAL::ORIGIN + o2;
      vo3->point() = CGAL::ORIGIN + o3;
    }
  }
}


template <class Mesh, class Mesh_ops, bool analysis>
void Loop_lift_operations<Mesh, Mesh_ops, analysis>::inner_edges_to_inner_olds_dual(
                                                Mesh &m,
                                                const Mesh_ops &m_ops,
                                                Vertex_handle *first_band,
                                                Vertex_handle *edge_start,
                                                Vertex_handle *last_band)
{
  // Using inner edge vertices to modify old vertices on Loop mask.
  for (Vertex_handle* v_ptr = edge_start; v_ptr != last_band; ++v_ptr)
  {
    // The center edge vertex of the current mask
    Vertex_handle v = *v_ptr;

    // The edge vertex should be interior vertex
    if (!m_ops.get_vertex_border(v))
    {
      Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(v, m_ops)};
      Halfedge_handle h0 = hps.first;
      Halfedge_handle h1 = hps.second;

      Vertex_handle vo0 = h0->vertex();
      Vertex_handle vo1 = h1->vertex();

      Vertex_handle vo2 = opposite_vertex(h0);
      Vertex_handle vo3 = opposite_vertex(h1);

      assert(vo0 != Vertex_handle {});
      assert(vo1 != Vertex_handle {});
      assert(vo2 != Vertex_handle {});
      assert(vo3 != Vertex_handle {});

      assert(m_ops.get_vertex_level(vo2) < m_ops.get_vertex_level(v));
      assert(m_ops.get_vertex_level(vo3) < m_ops.get_vertex_level(v));

      Vec3 o0 {CGAL::ORIGIN, vo0->point()};
      Vec3 o1 {CGAL::ORIGIN, vo1->point()};
      Vec3 o2 {CGAL::ORIGIN, vo2->point()};
      Vec3 o3 {CGAL::ORIGIN, vo3->point()};

      // Calculated weights at old vertices
      Eigen::Vector4d ws {Loop_math::get_weight(vo0->degree(),
                                                vo1->degree(), 
                                                vo2->degree(), 
                                                vo3->degree())};

      double w0 = ws[0];
      double w1 = ws[1];
      double w2 = ws[2];
      double w3 = ws[3];

      // Geometry at center edge vertex
      Vec3 e {CGAL::ORIGIN, v->point()};

      if (analysis)
      {
        o0 = o0 - w0 * e;
        o1 = o1 - w1 * e;
        o2 = o2 - w2 * e;
        o3 = o3 - w3 * e;
      }
      else
      {
        o0 = o0 + w0 * e;
        o1 = o1 + w1 * e;
        o2 = o2 + w2 * e;
        o3 = o3 + w3 * e;
      }

      vo0->point() = CGAL::ORIGIN + o0;
      vo1->point() = CGAL::ORIGIN + o1;
      vo2->point() = CGAL::ORIGIN + o2;
      vo3->point() = CGAL::ORIGIN + o3;
    }
  }
}
}  // namespace wtlib::ptq_impl

#endif  // define PTQ_IMPL_LOOP_ANALYSIS_OPERATIONS_HPP