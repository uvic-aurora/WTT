#ifndef PTQ_IMPL_SUBDIVISION_MODIFIER_HPP
#define PTQ_IMPL_SUBDIVISION_MODIFIER_HPP

/**
 * @file     subdivision_modifier.hpp
 * @brief    Defines some helper functions used in the wavelet transform,
 *           including PTQ, looking up connected old/new vertices, etc.
 *
 */

#include <vector>
#include <set>

#include <CGAL/HalfedgeDS_decorator.h>
#include <CGAL/Modifier_base.h>
#include <CGAL/Origin.h>

namespace wtlib::ptq_impl
{
template <class Mesh, class Mesh_ops>
class PTQ_subdivision_modifier
{
public:
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  using Halfedge_pair = std::pair<Halfedge_handle, Halfedge_handle>;
  using Facet_handle = typename Mesh::Facet_handle;
  using Halfedge_around_vertex_circulator = typename Mesh::Halfedge_around_vertex_circulator;

  /**
   * @brief      Calculate number of vertices in the most refined mesh after
   *             num_levels PTQ subdivision
   *
   * @param      mesh        The mesh
   * @param[in]  mesh_ops    The mesh ops
   * @param[in]  num_levels  The number levels
   *
   * @return     The mesh size of vertices.
   */
  static int get_mesh_size(Mesh& mesh, const Mesh_ops& mesh_ops, int num_levels);

  static int ptq_mesh_size(Mesh& mesh, int num_levels);

  /**
   * @brief      Helper function to get two outgoing halfedges from a new
   *             vertex to two old vertices.
   *
   * @param[in]  v      target vertex.
   * @param[in]  m_ops  The mesh_ops
   *
   * @return     Two outgoing halfedges
   */
  static Halfedge_pair get_halfedges_to_old_vertices(Vertex_handle v,
                                                      const Mesh_ops& m_ops);

  /**
   * @brief      Helper function to get two outgoing halfedges from v to
   *             vertices that are on the border.
   *
   * @param[in]  v      Target vertex, should on border
   *
   * @return     The two outgoing halfedges.
   */
  static Halfedge_pair get_halfedges_to_borders(Vertex_handle v);

  /**
   * @brief      Refine mesh topology based on Primal Triangle Quadrisection
   *             rule, assign level, border, type, id to edge vertices. The
   *             level of newly added vertices will be (level + 1)
   *
   * @param      mesh      Input mesh
   * @param[in]  mesh_ops  The mesh operations
   * @param[in]  level     The current resolution level
   * @param      vertices  The Vertex_handles array
   * @param      bands     The vertices band.
   */
  static void refine(
                  Mesh& mesh,
                  const Mesh_ops& mesh_ops,
                  int level,
                  std::vector<Vertex_handle>& vertices,
                  std::vector<Vertex_handle*>& bands);

  /**
   * @brief      Coarsen mesh, remove vertices with given level. 
   *
   * @param      mesh      The input mesh to be coarsened
   * @param[in]  mesh_ops  The mesh_operations
   * @param[in]  level     The vertices level to be removed.
   */
  static void coarsen(
                  Mesh& mesh,
                  const Mesh_ops& mesh_ops,
                  int level);

protected:
  /**
   * @brief      This class is used to access Mesh protected member hds
   *             since there is no public method to access it before CGAL 4.5
   */
  class Join_vertex: public CGAL::Modifier_base<typename Mesh::HDS>
  {
  public:
    Join_vertex(Halfedge_handle h);
    void operator()(typename Mesh::HDS& hds);

  private:
    Halfedge_handle h_;
  };  // class Join_vertex

};  // class PTQ_subdivision_modifier

template <class Mesh, class Mesh_ops>
int PTQ_subdivision_modifier<Mesh, Mesh_ops>::get_mesh_size(
                                Mesh &mesh,
                                const Mesh_ops &mesh_ops,
                                int num_levels)
{
  return ptq_mesh_size(mesh, num_levels);
}

template <class Mesh, class Mesh_ops>
int PTQ_subdivision_modifier<Mesh, Mesh_ops>::ptq_mesh_size(Mesh &mesh, int num_levels)
{
  assert(num_levels >= 0);
  int vertices_size = mesh.size_of_vertices();
  int facets_size = mesh.size_of_facets();
  int edges_size = mesh.size_of_halfedges() / 2;
  int pow2k = std::pow(2, num_levels);

  return num_levels == 0 ? vertices_size : 
                           0.5 * (pow2k - 1) * (pow2k - 2) * facets_size 
                           + (pow2k - 1) * edges_size
                           + vertices_size;
}

template <class Mesh, class Mesh_ops>
typename PTQ_subdivision_modifier<Mesh, Mesh_ops>::Halfedge_pair
PTQ_subdivision_modifier<Mesh, Mesh_ops>::get_halfedges_to_old_vertices(
                                                  Vertex_handle v,
                                                  const Mesh_ops &m_ops)
{
#if defined (WTLIB_USE_CUSTOM_MESH)
  // Find the halfedges from v0 to p0, p1
  std::pair<Halfedge_handle, Halfedge_handle> hs;
  std::pair<Vertex_handle, Vertex_handle> parents = v->parents;
  Halfedge_around_vertex_circulator hcir = v->vertex_begin();
  do
  {
    if (hcir->opposite()->vertex() == parents.first) {
      hs.first = hcir->opposite();
    }
    if (hcir->opposite()->vertex() == parents.second) {
      hs.second = hcir->opposite();
    }
    ++hcir;
  } while (hcir != v->vertex_begin());
  assert(hs.first != Halfedge_handle {});
  assert(hs.second != Halfedge_handle {});
  assert(hs.first != hs.second);
  return hs;
#else
  int v_level = m_ops.get_vertex_level(v);
  
  Halfedge_pair hs;

  Halfedge_around_vertex_circulator hcir = v->vertex_begin();

  int count = 0;

  do
  {
    if (m_ops.get_vertex_level(hcir->opposite()->vertex()) < v_level)
    {
      ++count;
      if (hs.first == Halfedge_handle {})
      {
        hs.first = hcir->opposite();
      }
      else if (hs.second == Halfedge_handle {})
      {
        hs.second = hcir->opposite();
      }
    }
    else
    {
      // Otherwise the adjacent vertex should be in the same level
      assert(m_ops.get_vertex_level(hcir->opposite()->vertex()) == v_level);
    }
    ++hcir;
  }
  while (hcir != v->vertex_begin());

  assert(count == 2);
  assert(hs.first != Halfedge_handle {});
  assert(hs.second != Halfedge_handle {});
  assert(hs.first != hs.second);
  return hs;
#endif
}


template <class Mesh, class Mesh_ops>
typename PTQ_subdivision_modifier<Mesh, Mesh_ops>::Halfedge_pair
PTQ_subdivision_modifier<Mesh, Mesh_ops>::get_halfedges_to_borders(
                                            Vertex_handle v)
{
  Halfedge_pair hs;

  Halfedge_around_vertex_circulator hcir = v->vertex_begin();

  // For assertion
  int count = 0;

  do
  {
    if (hcir->is_border_edge())
    {
      ++count;
      if (hs.first == Halfedge_handle {})
      {
        hs.first = hcir->opposite();
      }
      else if (hs.second == Halfedge_handle {})
      {
        hs.second = hcir->opposite();
      }
    }
    ++hcir;
  }
  while (hcir != v->vertex_begin());

  assert(hs.first != Halfedge_handle {});
  assert(hs.second != Halfedge_handle {});
  assert(hs.first != hs.second);
  assert(count == 2);
  return hs;
}


template <class Mesh, class Mesh_ops>
void PTQ_subdivision_modifier<Mesh, Mesh_ops>::refine(
                              Mesh& m,
                              const Mesh_ops& m_ops,
                              int level,
                              std::vector<Vertex_handle>& vertices,
                              std::vector<Vertex_handle*>& bands)
{
  using Edge_iterator = typename Mesh::Edge_iterator;
  int edges_size = m.size_of_halfedges() / 2;
  int vertices_size = m.size_of_vertices();
  int facets_size = m.size_of_facets();
  assert(!m.empty());
  assert(m.is_pure_triangle());


  // Used to compare edge based on its two end vertices id
  auto edge_compair = [&m_ops](const Edge_iterator& lhs,
                               const Edge_iterator& rhs) -> bool
                      {
                        int s_lhs = m_ops.get_vertex_id(lhs->vertex());
                        int l_lhs = m_ops.get_vertex_id(lhs->opposite()->vertex());
                        if (s_lhs > l_lhs)
                        {
                          std::swap(s_lhs, l_lhs);
                        }

                        int s_rhs = m_ops.get_vertex_id(rhs->vertex());
                        int l_rhs = m_ops.get_vertex_id(rhs->opposite()->vertex());
                        if (s_rhs > l_rhs)
                        {
                          std::swap(s_rhs, l_rhs);
                        }

                        return s_lhs == s_rhs ? l_lhs < l_rhs : s_lhs < s_rhs;
                      };

  // Store mesh edges in a sorted order.
  std::set<Edge_iterator, decltype(edge_compair)> edges_set(edge_compair);

  // Sort all the mesh edges based on their end vertices id.
  for (Edge_iterator e = m.edges_begin(); e != m.edges_end(); ++e)
  {
    assert(edges_set.find(e) == edges_set.end());
    edges_set.insert(e);
  }

  assert(edges_set.size() == edges_size);

  // Insert a new vertex on every edge, push the new vertex in vertices, and
  // assign new vertex an id starting from the current vertices_size
  for (auto [eitr, idx] = std::make_pair(edges_set.begin(), vertices_size);
       eitr != edges_set.end(); ++eitr, ++idx)
  {
    // Two end vertices
    Vertex_handle p0 = (*eitr)->vertex();
    Vertex_handle p1 = (*eitr)->opposite()->vertex();

    // The inserted new vertex
    Halfedge_handle h = *eitr;
    Halfedge_handle hoppo = h->opposite();
    Halfedge_handle hnew = m.split_vertex(hoppo->prev(), h);
    
    if (!h->is_border())
    {
      h->facet()->set_halfedge(hnew);
    }
    if (!hoppo->is_border())
    {
      hoppo->facet()->set_halfedge(hoppo);
    }

    Vertex_handle ve = h->vertex();
    ve->point() = CGAL::ORIGIN;
    vertices.push_back(ve);
    // Assign its parents, if WTLIB_USE_CUSTOM_MESH is enabled
  #if defined (WTLIB_USE_CUSTOM_MESH)
    ve->parents = std::make_pair(p0, p1);
  #endif
  
    // Assign id, level, type to edge vertex
    m_ops.set_vertex_id(ve, idx);
    m_ops.set_vertex_level(ve, level + 1);
    m_ops.set_vertex_type(ve, 1);

    // If this edge is border edge, then the edge vertex is border vertex.
    if (h->is_border_edge())
    {
      m_ops.set_vertex_border(ve, true);
    }
    else
    {
      m_ops.set_vertex_border(ve, false);
    }
  }

  // Push the position of the one next to the last vertex to bands.
  bands.push_back(&vertices.back() + 1);

  assert(edges_size + vertices_size == m.size_of_vertices());

  // Connect edge vertices inside a same facet
  for (auto [f, idx] = std::make_pair(m.facets_begin(), 0);
       idx < facets_size; ++f, ++idx)
  {
    // Every facet should be hexagon
    assert(f->facet_degree() == 6);

    // This halfedge should always point to an old vertex
    Halfedge_handle h = f->facet_begin();
    assert(m_ops.get_vertex_id(h->vertex()) < vertices_size);

    // Then this halfedge point to an edge vertex
    Halfedge_handle e0 = h->next();
    Halfedge_handle et = e0;

    // The next two halfedges that pointed to edge vertices
    Halfedge_handle e1 = e0->next()->next();
    Halfedge_handle e2 = e1->next()->next();

    // Assert they are in a loop.
    assert(h == e2->next());

    // Link edge vertices to split facet.
    et = m.split_facet(et, e1);
    et = m.split_facet(et, e2);
    et = m.split_facet(et, e0);

    // Post condition.
    assert(et->opposite()->next() == h);
  }
  assert(facets_size * 4 == m.size_of_facets());
}

template <class Mesh, class Mesh_ops>
void PTQ_subdivision_modifier<Mesh, Mesh_ops>::coarsen(
                              Mesh &m,
                              const Mesh_ops &m_ops,
                              int level)
{
  // Remove halfedges between vertices with level
  for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
  {
    // Start from old vertices
    if (m_ops.get_vertex_level(v) < level)
    {
      // Loop all halfedges incident to an old vertex
      Halfedge_around_vertex_circulator hcir = v->vertex_begin();

      // The incident vertex's level should equal to level
      assert(m_ops.get_vertex_level(hcir->opposite()->vertex()) == level);

      do
      {
        if (!hcir->is_border())
        {
          // hcir->prev() is an halfedge linked two edge vertices.
          m.join_facet(hcir->prev());
        }
        ++hcir;
      }
      while (hcir != v->vertex_begin());
    }
  }

  // Now all facet is hexagon
  assert([&m]()->bool
         {
          for (Facet_handle f = m.facets_begin(); f != m.facets_end(); ++f)
          {
            if (f->facet_degree() != 6)
            {
              return false;
            }
          }
          return true;
         }());

  // Remove vertices of level
  for (Vertex_handle vidx = m.vertices_begin(); vidx != m.vertices_end();)
  {
    Vertex_handle v = vidx++;
    if (m_ops.get_vertex_level(v) == level)
    {
      assert(v->degree() == 2);

      Halfedge_handle hcir = v->vertex_begin();

      Join_vertex join_vertex(hcir->opposite());
      m.delegate(join_vertex);
    }
  }
}

template <class Mesh, class Mesh_ops>
PTQ_subdivision_modifier<Mesh, Mesh_ops>::Join_vertex::Join_vertex(Halfedge_handle h):h_(h)
{}

template <class Mesh, class Mesh_ops>
void PTQ_subdivision_modifier<Mesh, Mesh_ops>::Join_vertex::operator()(typename Mesh::HDS &hds)
{
  CGAL::HalfedgeDS_decorator<typename Mesh::HDS> d(hds);
  // Using this low-level join_vertex method instead of the higher level
  // Polyhedron::join_vertex just because the higher level one has safety
  // check that prevent it from working on a border vertex, but that check
  // is unnecessary in this case. 
  d.join_vertex(h_);
}

}  // namespace wtlib::ptq_impl
#endif