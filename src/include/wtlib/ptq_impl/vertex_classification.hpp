#ifndef PTQ_IMPL_VERTEX_CLASSIFICATION_HPP
#define PTQ_IMPL_VERTEX_CLASSIFICATION_HPP

/**
 * @brief    This header file defines several functions to implement Taubin's
 *           subdivision detection algorithm for PTQ and vertex classification.
 *
 */

#include <wtlib/ptq_impl/vertex_classification_mirror_mesh.hpp>

#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>

#include <cassert>
#include <queue>
#include <vector>

#if defined (WTLIB_HASH_IN_PLACE_LIST_ITERATOR)
#include <wtlib/ptq_impl/in_place_list_iterator_hash.hpp>
#endif

#if defined (WTLIB_USE_UNORDERED_MAP)
#include <unordered_map>
#else
#include <map>
#endif

namespace wtlib::ptq_impl
{
/**
 * @brief    This class is designed to collect all the center triangles produced
 *           in PTQ. Starting from a seed and assuming it is the center
 *           triangle, the other center triangles are collected by recursively
 *           looking for the opposite triangles. The opposite triangle of a
 *           triangle is defined as the one sharing a vertex with the triangle
 *           and is three triangles away from it along the vertex's triangle
 *           fan.
 *
 * @tparam   Mirror_mesh    the type of a mirror mesh.
 */
template <class Mirror_mesh>
class PTQ_finer_faces_crawler
{
public:
  using Facet_handle = typename Mirror_mesh::Facet_handle;
  using Vertex_handle = typename Mirror_mesh::Vertex_handle;
  using Halfedge_handle = typename Mirror_mesh::Halfedge_handle;
  using Halfedge_around_vertex_circulator = typename Mirror_mesh::Halfedge_around_vertex_circulator;
  using Faces = std::vector<Facet_handle>;

  PTQ_finer_faces_crawler(Mirror_mesh& mirror, Facet_handle seed)
  : mirror_(mirror),
    seed_(seed),
    success_(false)
  {
  }

  PTQ_finer_faces_crawler(const PTQ_finer_faces_crawler&) = delete;

  PTQ_finer_faces_crawler& operator=(const PTQ_finer_faces_crawler&) = delete;

  PTQ_finer_faces_crawler(PTQ_finer_faces_crawler&& c)
  : mirror_(c.mirror_),
    seed_(c.seed_),
    faces_(std::move(c.faces_)),
    success_(c.success_)
  {
  }

  PTQ_finer_faces_crawler& operator=(PTQ_finer_faces_crawler&& c)
  {
    mirror_ = c.mirror_;
    seed_ = c.seed_;
    faces_ = std::move(c.faces_);
    success_ = c.success_;
    return *this;
  }

  bool success() const
  {
    return success_;
  }

  /**
   * @brief    Starting from a seed face, iteratively search and buffer the
   *           opposite center triangles. Then, check if the number of all the
   *           faces is 4 times the number of center triangles.
   *
   * @return true 
   * @return false 
   */
  bool crawl();

  /**
   * @brief    A helper function to enqueue the adjacent center triangles of a
   *           center triangle. The queue is used to implement recursion.
   *
   * @param    v         A vertex in a triangle.
   * @param    v_prev    The previous vertex of v in the triangle. 
   * @param    queue     The faces queue.
   */
  void enqueue_opposite_face(Vertex_handle v,
                             Vertex_handle v_prev,
                             std::queue<Facet_handle>& queue) const;

  /**
   * @brief    Check if the number of center triangles is a quarter of the
   *           number of all the triangles.
   *
   * @return true      implies subdivision connectivity found.
   * @return false 
   */
  bool is_equivalence() const;

  Faces& faces()
  {
    return faces_;
  }

  const Faces& faces() const
  {
    return faces_;
  }
private:
  Mirror_mesh& mirror_;
  Facet_handle seed_;
  Faces faces_;
  bool success_;
};  // class PTQ_finer_faces_crawler

template <class Mirror_mesh>
void PTQ_finer_faces_crawler<Mirror_mesh>::enqueue_opposite_face(
                                          Vertex_handle v,
                                          Vertex_handle v_prev,
                                          std::queue<Facet_handle> &queue) const
{
  if (!v->is_border())
  {
    Halfedge_around_vertex_circulator hcir = v->vertex_begin();

    // Get the halfedge circulator from v_prev to v.
    do
    {
      if (hcir->opposite()->vertex() == v_prev)
      {
        break;
      }
      ++hcir;
    }
    while (hcir != v->vertex_begin());

    // Circulate to its incident fine face.
    ++hcir;
    ++hcir;
    ++hcir;

    Facet_handle f = hcir->facet();

    if (!f->visited())
    {
      queue.push(f);
    }
  }
}

template <class Mirror_mesh>
bool PTQ_finer_faces_crawler<Mirror_mesh>::crawl()
{
// Use a queue instead of recursive function
  std::queue<Facet_handle> face_queue;
  face_queue.push(seed_);

  while (!face_queue.empty())
  {
    // Current processing face.
    Facet_handle f = face_queue.front();
    face_queue.pop();

    // Check whether this face is visited, since duplicate face maybe added
    // to the queue.
    if (f->visited())
    {
      continue;
    }

    // Get its three incident vertices
    Halfedge_handle h0 = f->facet_begin();
    Halfedge_handle h1 = h0->next();
    Halfedge_handle h2 = h1->next();

    // If the triangle is on the boundary, return false. Since PTQ cannot
    // produce center triangle on boundary.
    if (   h0->is_border_edge()
        || h1->is_border_edge()
        || h2->is_border_edge())
    {
      success_ = false;
      return false;
    }
    Vertex_handle v0 = h0->vertex();
    Vertex_handle v1 = h1->vertex();
    Vertex_handle v2 = h2->vertex();

    assert(h2->next() == h0);

    // Check if valences of the three vertices are all 6(inner) or 4(border)
    // since subdivision can only produce regular vertices
    int val_v0 = v0->degree();
    int val_v1 = v1->degree();
    int val_v2 = v2->degree();

    // If one of the valences is not 6 or 4, terminate crawl.
    if (   (v0->is_border() ? val_v0 != 4 : val_v0 != 6)
        || (v1->is_border() ? val_v1 != 4 : val_v1 != 6)
        || (v2->is_border() ? val_v2 != 4 : val_v2 != 6))
    {
      success_ = false;
      return false;
    }

    // Mark the current face as visited
    f->set_visited(true);

    // Passed valence check, vertices are all regular vertices
    // Push the current faces to the candidate faces container
    faces_.emplace_back(f);

    // Enqueue its three opposite faces. The opposite face share one vertex with
    // current face f, and is three triangles away along the triangle fan.
    enqueue_opposite_face(v0, v2, face_queue);
    enqueue_opposite_face(v1, v0, face_queue);
    enqueue_opposite_face(v2, v1, face_queue);
  }

  success_ = is_equivalence();
  return success_;
}

template <class Mirror_mesh>
bool PTQ_finer_faces_crawler<Mirror_mesh>::is_equivalence() const
{
  return faces_.size() == 0 ? false :
            (mirror_.size_of_facets() % faces_.size() == 0 ? 
                mirror_.size_of_facets() / faces_.size() == 4 : false);
}


/**
 * @brief    Track the parents of a vertex.
 * 
 * @tparam   Mesh 
 */
template <class Mesh>
class V_tracker
{
public:
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Halfedge_handle = typename Mesh::Halfedge_handle;
#if defined (WTLIB_USE_CUSTOM_MESH)
  std::pair<Vertex_handle, Vertex_handle> get_hs_to_parents(Vertex_handle v) const
  {
    return v->parents;
  }

  void set_hs_to_parents(Vertex_handle v, std::pair<Vertex_handle, Vertex_handle> parents) const
  {
    v->parents = parents;
  }
#else
  std::pair<Vertex_handle, Vertex_handle> get_hs_to_parents(Vertex_handle v) const
  {
    return tracker_.at(v);
  }

  void set_hs_to_parents(Vertex_handle v, std::pair<Vertex_handle, Vertex_handle> parents)
  {
    tracker_.insert({v, parents});
  }
#endif

private:
#if !defined (WTLIB_USE_CUSTOM_MESH)
#if defined (WTLIB_USE_UNORDERED_MAP)
  std::unordered_map<Vertex_handle, std::pair<Vertex_handle, Vertex_handle>> tracker_;
#else
  std::map<Vertex_handle, std::pair<Vertex_handle, Vertex_handle>> tracker_;
#endif
#endif
};


/**
 * @brief    Implement Taubin's PTQ subdivision detection algorithm.
 * 
 * @tparam   Mesh 
 * @tparam   Mesh_ops 
 */
template <class Mesh, class Mesh_ops>
class PTQ_classify_vertices
{
public:
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  using Facet_handle = typename Mesh::Facet_handle;

  using Mirror_mesh = CGAL::Polyhedron_3<Traits<Mesh>, Mirror_mesh_items>;
  using Mirror_vertex_handle = typename Mirror_mesh::Vertex_handle;
  using Mirror_facet_handle = typename Mirror_mesh::Facet_handle;
  using Mirror_halfedge_handle = typename Mirror_mesh::Halfedge_handle;
  using Crawler = PTQ_finer_faces_crawler<Mirror_mesh>;

  using Vertex_tracker = V_tracker<Mesh>;

#if !defined(WTLIB_USE_CUSTOM_MESH)
#if defined (WTLIB_USE_UNORDERED_MAP)
  using Vertex_property = std::unordered_map<Vertex_handle, int>;
#else
  using Vertex_property = std::map<Vertex_handle, int>;
#endif  // WTLIB_USE_UNORDERED_MAP
#endif
  enum Vertex_type
  {
    OLD_VERTEX = 0,
    EDGE_VERTEX = 1
  };


public:
  bool operator()(
                Mesh& mesh,
                const Mesh_ops& mesh_ops,
                int num_levels,
                std::vector<Vertex_handle>& vertices,
                std::vector<Vertex_handle*>& bands) const;
  /**
   * @brief      Detect the given level of subdivision connectivity and classify
   *             vertices based on their resolution. Vertex handles are placed
   *             in the array vertices and sorted by level, type, and id in
   *             ascending order. The pointers to the starting vertex handle of
   *             each type are stored in the array bands. If the function return
   *             false, mesh, vertices, and bands are left unmodified. 
   *
   * @param      mesh               Input mesh
   * @param[in]  mesh_ops           Mesh operations
   * @param[in]  num_levels         The number levels
   * @param      vertices           The array of vertices
   * @param      bands              The bands
   * @param[in]  reset_vertices_id  Reset vertices id tag
   *
   * @return     True if has subdivision connectivity, false otherwise.
   */
  static bool classify(
                Mesh& mesh,
                const Mesh_ops& mesh_ops,
                int num_levels,
                std::vector<Vertex_handle>& vertices,
                std::vector<Vertex_handle*>& bands,
                bool reset_vertices_id
               );

  /**
   * @brief      Gets the number of vertex types.
   *
   * @param      mesh      The mesh
   * @param[in]  mesh_ops  The mesh ops
   *
   * @return     The number of vertex types.
   */
  static int get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops)
  {
    return 2;
  }

protected:
  /**
   * @brief      Construct a mirror mesh from the input mesh.
   *
   * @param      mesh  The input mesh
   *
   */
  static void extract(Mesh& mesh, Mirror_mesh& mirror);


  /**
   * @brief      Check number of triangles faces and connected irregular vertices
   *
   * @param      mirror
   *
   * @return     pass or fail
   */
  static bool precheck(Mirror_mesh& mirror);


  /**
   * @brief      Reset all the faces of the mirror to unvisited.
   *
   * @param      mirror
   */
  static void reset_facets_marks(Mirror_mesh& mirror);


  /**
   * @brief      Compute size of border halfedges
   *
   * @param      mirror
   *
   * @return     size of border halfedges
   */
  static int size_of_borders(Mirror_mesh& mirror);


  /**
   * @brief      Gets the seed faces for crawling
   *
   * @param      mirror
   *
   * @return     The seed faces.
   */
  static std::vector<Mirror_facet_handle> get_seed_faces(Mirror_mesh& mirror);


  /**
   * @brief      Coarsen the mirror mesh, dump the finer vertices, assign level and type to them.
   *
   * @param      mirror      The mirror mesh
   * @param[in]  fine_faces  The crawled finer faces
   * @param[in]  m_ops       The mesh operations
   * @param      vertices    The array of vertices
   * @param      bands       The array of band delimiters
   * @param[in]  level       The current processing level
   * @param[in]  v_back_idx  The vertices index from last to first
   * @param      v_tracker   The vertex parent tracker
   */
  static void coarsen_mirror_dump_vertices(
                               Mirror_mesh& mirror,
                               const std::vector<Mirror_facet_handle>& fine_faces,
                               const Mesh_ops& m_ops,
                               std::vector<Vertex_handle>& vertices,
                               std::vector<Vertex_handle*>& bands,
                               int level,
                               int& v_back_idx,
                               Vertex_tracker& v_tracker);


  /**
   * @brief      Sort vertices in the array vertices based on id and parent id
   *
   * @param[in]  v_tracker  Vertex parent tracker
   * @param[in]  m_ops      mesh operations
   * @param      vertices   The array of vertices
   * @param      bands      The band delimiters
   * @param[in]  reset_id   if reset id. Should be true.
   */
  static void sort_vertices(
                const Vertex_tracker& v_tracker,
                const Mesh_ops& m_ops,
                std::vector<Vertex_handle>& vertices,
                std::vector<Vertex_handle*>& bands,
                bool reset_id);

  /**
   * @brief      Check if a border loop is composed of only two halfedges
   *
   * @param[in]  mirror
   *
   * @return     Pass or fail
   */
  static bool border_post_check(const Mirror_mesh& mirror);


  /**
   * @brief      Calculate the sum of ids of all finer vertices
   *
   * @param[in]  faces  The finer faces
   * @param[in]  m_ops  The mesh_operations
   *
   * @return     sum
   */
  static int sum_of_vids(const std::vector<Mirror_facet_handle>& faces,
                         const Mesh_ops& m_ops);


  /**
   * @brief      This class is used to access Mirror_mesh protected member hds
   *             since there is no public method to access it before CGAL 4.5
   */
  class Extract_mesh: public CGAL::Modifier_base<typename Mirror_mesh::HDS>
  {
  public:
    Extract_mesh(Mesh& mesh);
    void operator()(typename Mirror_mesh::HDS& hds);
  private:
    Mesh& mesh_;
  };  // class Extract_mesh

  /**
   * @brief      This class is used to access Mirror_mesh protected member hds
   *             since there is no public method to access it before CGAL 4.5
   */
  class Join_vertex: public CGAL::Modifier_base<typename Mirror_mesh::HDS>
  {
  public:
    Join_vertex(Mirror_halfedge_handle h);
    void operator()(typename Mirror_mesh::HDS& hds);
  private:
    Mirror_halfedge_handle h_;
  };  // class Join_vertex

};  // class PTQ_classify_vertices

template <class Mesh, class Mesh_ops>
bool PTQ_classify_vertices<Mesh, Mesh_ops>::operator()(
                    Mesh &mesh,
                    const Mesh_ops &mesh_ops,
                    int num_levels,
                    std::vector<Vertex_handle> &vertices,
                    std::vector<Vertex_handle *> &bands) const
{
  return classify(mesh, mesh_ops, num_levels, vertices, bands, true);
}

template <class Mesh, class Mesh_ops>
bool PTQ_classify_vertices<Mesh, Mesh_ops>::classify(
                    Mesh &mesh,
                    const Mesh_ops &mesh_ops,
                    int num_levels,
                    std::vector<Vertex_handle>& vertices,
                    std::vector<Vertex_handle*>& bands,
                    bool reset_vertices_id)
{
  assert(!mesh.empty() && mesh.is_pure_triangle());
  if (num_levels < 1)
  {
    return false;
  }

  // Initialize vertices level, type and id which is determined by layout in memory
  for (auto [v, id] = std::make_pair(mesh.vertices_begin(), int(0));
       v != mesh.vertices_end();
       ++v, ++id)
  {
    mesh_ops.set_vertex_type(v, OLD_VERTEX);
    mesh_ops.set_vertex_level(v, num_levels);
    mesh_ops.set_vertex_id(v, id);
  }

  int v_back_idx = mesh.size_of_vertices();
  vertices.resize(mesh.size_of_vertices());
  bands.resize(num_levels + 2);
  bands[num_levels + 1] = &vertices.back() + 1;
  bands[0] = &vertices.front();

  // Construct a mesh mirror, which copies the topology of the original mesh.
  // The subdivision detection is performed on the mirror, then mapped back to
  // the original mesh. The reason to do so is to avoid breaking the topology of
  // the original mesh.
  Mirror_mesh mirror;
  extract(mesh, mirror);
  int border_size = size_of_borders(mirror);

  // The vertex parents tracker.
  Vertex_tracker v_tracker;

  int level = num_levels;

  // Identify the given levels of subdivision connectivity.
  for (; level > 0; --level)
  {
    if (!precheck(mirror))
    {
      break;
    }

    // Reset all facets to unvisited.
    reset_facets_marks(mirror);

    // Find seed faces for crawling
    std::vector<Mirror_facet_handle> seeds {get_seed_faces(mirror)};
    std::vector<Crawler> crawlers;

    // Construct crawlers
    for (const Mirror_facet_handle& seed : seeds)
    {
      crawlers.emplace_back(mirror, seed);
    }
    assert(crawlers.size() > 0 && "At least one crawler");
    assert(crawlers.size() == seeds.size());

    // TODO: Multi-thread
    bool success = false;
    int crawler_id = 0;
    for (int i = 0; i < crawlers.size(); ++i)
    {
      if (crawlers[i].crawl())
      {
        success = true;
#ifndef IS_RUNNING_TESTS
        crawler_id = i;
        break;
#endif
      }
    }

    if (!success)
    {
      break;
    }

// Used in tests.
#if defined(IS_RUNNING_TESTS)
    // Always find the successful crawler with maximum sum of finer ids
    // to make sure the preset old vertices in test meshes are classified
    // as old vertices.
    for (auto [i, max] = std::make_pair(std::size_t(0), std::numeric_limits<int>::min());
         i < crawlers.size();
         ++i)
    {
      if (crawlers[i].success())
      {
        int vid_sum = sum_of_vids(crawlers[i].faces(), mesh_ops);
        if (vid_sum > max)
        {
          max = vid_sum;
          crawler_id = i;
        }
      }
    }
#endif

    // Found subdivision connectivity. Then coarsen the mirror for the next
    // level detection.
    const std::vector<Mirror_facet_handle>& ffs = crawlers[crawler_id].faces();
    coarsen_mirror_dump_vertices(mirror,
                              ffs,
                              mesh_ops,
                              vertices,
                              bands,
                              level,
                              v_back_idx,
                              v_tracker);


    // In case some mesh borders will be closed (caused by the CGAL HDS) after coarsen
    if (border_size > 0 && !border_post_check(mirror))
    {
      break;
    }
  }

  if (level == 0)
  {
    // Now only base level vertices in mesh_ext
    for (Mirror_vertex_handle v = mirror.vertices_begin();
         v != mirror.vertices_end();
         ++v)
    {
      --v_back_idx;
      vertices[v_back_idx] = v->point();
    }

    // v_back_idx should be zero;
    assert(v_back_idx == 0);

    // Bands should have at least 3 elements
    assert(bands.size() >= 3);
    sort_vertices(v_tracker,
                  mesh_ops,
                  vertices,
                  bands,
                  reset_vertices_id);
    return true;
  }

  return false;
}

template <class Mesh, class Mesh_ops>
void PTQ_classify_vertices<Mesh, Mesh_ops>::sort_vertices(
                  const Vertex_tracker &v_tracker,
                  const Mesh_ops& m_ops,
                  std::vector<Vertex_handle> &vertices,
                  std::vector<Vertex_handle *> &bands,
                  bool reset_id)
{
  using Parent = std::pair<Vertex_handle, Vertex_handle>;
  using Vitr = typename std::vector<Vertex_handle>::iterator;

  // Compare by parent vertices' id, used to sort vertices.
  auto vcomp = [&v_tracker, &m_ops](const Vertex_handle& lhs,
                                    const Vertex_handle& rhs)
                {
                  Parent p_lhs = v_tracker.get_hs_to_parents(lhs);
                  Parent p_rhs = v_tracker.get_hs_to_parents(rhs);

                  int s_lhs = m_ops.get_vertex_id(p_lhs.first);
                  int l_lhs = m_ops.get_vertex_id(p_lhs.second);
                  if (s_lhs > l_lhs)
                  {
                    std::swap(s_lhs, l_lhs);
                  }

                  int s_rhs = m_ops.get_vertex_id(p_rhs.first);
                  int l_rhs = m_ops.get_vertex_id(p_rhs.second);
                  if (s_rhs > l_rhs)
                  {
                    std::swap(s_rhs, l_rhs);
                  }

                  return s_lhs != s_rhs ? s_lhs < s_rhs : l_lhs < l_rhs;
                };

  // First sort base resolution vertices
  assert(vertices.begin() == Vitr {bands[0]});
  assert(bands[0] == &vertices[0]);
  std::sort(bands[0],
            bands[1], 
            [&m_ops](const Vertex_handle& lhs,
                     const Vertex_handle& rhs)
            {
              return m_ops.get_vertex_id(lhs) < m_ops.get_vertex_id(rhs);
            });

  if (reset_id)
  {
    for (int i = 0; i < (bands[1] - bands[0]); ++i)
    {
      m_ops.set_vertex_id(vertices[i], i);
    }
  }


  // Sort vertices in each level
  // Since [bands[0], bands[1]) are old vertices, so i start from 2.
  for (int i = 2; i < bands.size(); ++i)
  {
    Vertex_handle* start {bands[i - 1]};
    Vertex_handle* end {bands[i]};
    std::sort(start, end, vcomp);

    // Reset id of vertices in current processing level.
    if (reset_id)
    {
      int lower_v_size = bands[i - 1] - bands[0];
      assert(m_ops.get_vertex_id(*(start - 1)) == lower_v_size - 1);
      // Set id for vertices in current processing band.
      for (auto [vitr, id] = std::make_pair(start, lower_v_size);
           vitr != end; ++vitr, ++id)
      {
        m_ops.set_vertex_id(*vitr, id);
      }
    }
  }
}

template <class Mesh, class Mesh_ops>
void PTQ_classify_vertices<Mesh, Mesh_ops>::extract(Mesh& mesh, Mirror_mesh& mirror)
{
  Extract_mesh ext(mesh);
  mirror.delegate(ext);

  assert(mirror.size_of_facets() == mesh.size_of_facets());
  assert(mirror.size_of_vertices() == mesh.size_of_vertices());

  for (Mirror_halfedge_handle h_h = mirror.edges_begin();
       h_h != mirror.edges_end();
       ++h_h)
  {
    if (h_h->is_border_edge())
    {
      h_h->vertex()->set_border(true);
      h_h->opposite()->vertex()->set_border(true);
    }
  }
}

template <class Mesh, class Mesh_ops>
void PTQ_classify_vertices<Mesh, Mesh_ops>::reset_facets_marks(Mirror_mesh& mirror)
{
  for (Mirror_facet_handle f = mirror.facets_begin(); f != mirror.facets_end(); ++f)
  {
    f->set_visited(false);
  }
}


template <class Mesh, class Mesh_ops>
bool PTQ_classify_vertices<Mesh, Mesh_ops>::precheck(Mirror_mesh& mirror)
{
  assert(!mirror.empty());
  // Face size should be a factor of 4
  if (mirror.size_of_facets() % 4 != 0)
  {
    return false;
  }

  // Irregular vertices should not connected.
  for (auto h = mirror.edges_begin(); h != mirror.edges_end(); ++h)
  {
    bool irregular = true;

    Mirror_vertex_handle v0 = h->vertex();
    Mirror_vertex_handle v1 = h->opposite()->vertex();

    // Regular border vertex has valence 4.
    // Regular inner vertex has valence 6.
    irregular = v0->is_border() ? 
                (irregular && v0->degree() != 4) : (irregular && v0->degree() != 6);

    irregular = v1->is_border() ? 
                (irregular && v1->degree() != 4) : (irregular && v1->degree() != 6);

    if (irregular)
    {
      return false;
    }
  }
  return true;
}

template <class Mesh, class Mesh_ops>
int PTQ_classify_vertices<Mesh, Mesh_ops>::size_of_borders(Mirror_mesh& mirror)
{
  int size = 0;
  for (Mirror_halfedge_handle h = mirror.halfedges_begin();
       h != mirror.halfedges_end(); ++h)
  {
    if (h->is_border())
    {
      ++size;
    }
  }
  return size;
}

template <class Mesh, class Mesh_ops>
std::vector<typename PTQ_classify_vertices<Mesh, Mesh_ops>::Mirror_facet_handle>
PTQ_classify_vertices<Mesh, Mesh_ops>::get_seed_faces(Mirror_mesh& mesh)
{
  std::vector<Mirror_facet_handle> seeds;

  Mirror_facet_handle f = mesh.facets_begin();

  seeds.push_back(f);

  Mirror_halfedge_handle h = f->facet_begin();

  do
  {
    if (!h->is_border_edge())
    {
      seeds.push_back(h->opposite()->face());
    }
    h = h->next();
  }
  while (h != f->facet_begin());

  return seeds;
}

template <class Mesh,class Mesh_ops>
void PTQ_classify_vertices<Mesh, Mesh_ops>::coarsen_mirror_dump_vertices(
                               Mirror_mesh& mirror,
                               const std::vector<Mirror_facet_handle>& fine_faces,
                               const Mesh_ops& m_ops,
                               std::vector<Vertex_handle>& vertices,
                               std::vector<Vertex_handle*>& bands,
                               int level,
                               int& v_back_idx,
                               Vertex_tracker& v_tracker)
{
  // Remove faces
  for (const Mirror_facet_handle& f : fine_faces)
  {
    Mirror_halfedge_handle h = f->facet_begin();
    std::vector<int> ids(3);
    for (int i = 0; i < 3; ++i)
    {
      assert(m_ops.get_vertex_level(h->vertex()->point()) == level);
      ids[i] = m_ops.get_vertex_id(h->vertex()->point());
      h->vertex()->set_finer(true);
      h = mirror.join_facet(h);
      assert(h == f->facet_begin());
    }
    assert(h == f->facet_begin());
    assert(f->facet_degree() == 6);
  }

  assert(v_back_idx == mirror.size_of_vertices());

  // Dump the finer vertices to the array and remove from mirror
  for (Mirror_vertex_handle v = mirror.vertices_begin();
       v != mirror.vertices_end();)
  {
    Mirror_vertex_handle tmp = v++;
    if (tmp->is_finer())
    {
      assert(tmp->degree() == 2);
      // Add its neighbors to v_tracker
      Vertex_handle v0 = tmp->point();
      Mirror_halfedge_handle h0 = tmp->vertex_begin();
      Mirror_halfedge_handle h1 = h0->next();
      assert(h1->opposite()->vertex() == tmp);
      // Parent vertices.
      // h0 is pointed to tmp
      Vertex_handle p0 = h0->opposite()->vertex()->point();
      // h1->opposite() is pointed to tmp
      Vertex_handle p1 = h1->vertex()->point();
      assert(p0 != p1);
      assert(m_ops.get_vertex_level(v0) == level);

      // Set this vertex type to EDGE_VERTEX
      m_ops.set_vertex_type(v0, EDGE_VERTEX);

      // Set the level of its parents to level - 1
      m_ops.set_vertex_level(p0, level - 1);
      m_ops.set_vertex_level(p1, level - 1);

      // Check vertex type
      assert(m_ops.get_vertex_type(p0) == OLD_VERTEX);
      assert(m_ops.get_vertex_type(p1) == OLD_VERTEX);

      // Insert v0 and the halfedges to its parent to vertex tracker.
      v_tracker.set_hs_to_parents(v0, {p0, p1});

      --v_back_idx;
      vertices[v_back_idx] = v0;
      assert(v_back_idx > 0);

      // Coarsen mirror
      Join_vertex join_vertex(h0->opposite());
      mirror.delegate(join_vertex);
    }
  }
  assert(v_back_idx == mirror.size_of_vertices());
  bands[level] = &vertices[v_back_idx];
}

template <class Mesh, class Mesh_ops>
bool PTQ_classify_vertices<Mesh, Mesh_ops>::border_post_check(const Mirror_mesh& mirror)
{
  for (auto h = mirror.halfedges_begin(); h != mirror.halfedges_end(); ++h)
  {
    if (h->is_border())
    {
      if (h->next()->next() == h)
      {
        return false;
      }
    }
  }
  return true;
}

template <class Mesh, class Mesh_ops>
int PTQ_classify_vertices<Mesh, Mesh_ops>::sum_of_vids(
                  const std::vector<Mirror_facet_handle>& faces,
                  const Mesh_ops& m_ops)
{
  int sum = 0;
  for (const Mirror_facet_handle& f : faces)
  {
    Mirror_halfedge_handle h = f->facet_begin();
    do
    {
      if (h->vertex()->is_border())
      {
        sum += m_ops.get_vertex_id(h->vertex()->point());
      }
      else
      {
        sum += m_ops.get_vertex_id(h->vertex()->point()) / 2.0;
      }
      h = h->next();
    }
    while (h != f->facet_begin());
  }

  return sum;
}

template <class Mesh, class Mesh_ops>
PTQ_classify_vertices<Mesh, Mesh_ops>::Extract_mesh::Extract_mesh(Mesh& mesh): mesh_(mesh)
{}


template <class Mesh, class Mesh_ops>
void PTQ_classify_vertices<Mesh, Mesh_ops>::Extract_mesh::operator()(typename Mirror_mesh::HDS &hds)
{
#if !defined (WTLIB_USE_CUSTOM_MESH)
  Vertex_property vids;
#endif
  CGAL::Polyhedron_incremental_builder_3<typename Mirror_mesh::HDS> b(hds);

  b.begin_surface(mesh_.size_of_vertices(), mesh_.size_of_facets());

  for (auto [v, id] = std::make_pair(mesh_.vertices_begin(), 0);
       v != mesh_.vertices_end();
       ++v, ++id)
  {
    b.add_vertex(v);
#if !defined (WTLIB_USE_CUSTOM_MESH)
    vids[v] = id;
#else
    v->id = id;
#endif
  }

  for (auto f = mesh_.facets_begin(); f != mesh_.facets_end(); ++f)
  {
    b.begin_facet();

    Halfedge_handle h = f->facet_begin();
    do
    {
      Vertex_handle v = h->vertex();
#if !defined (WTLIB_USE_CUSTOM_MESH)
      int id = vids.at(v);
#else
      int id = v->id;
#endif
      b.add_vertex_to_facet(id);
      h = h->next();
    }
    while (h != f->facet_begin());

    b.end_facet();
  }
  b.end_surface();
}


template <class Mesh, class Mesh_ops>
PTQ_classify_vertices<Mesh, Mesh_ops>::Join_vertex::Join_vertex(Mirror_halfedge_handle h):h_(h)
{}


template <class Mesh, class Mesh_ops>
void PTQ_classify_vertices<Mesh, Mesh_ops>::Join_vertex::operator()(typename Mirror_mesh::HDS &hds)
{
  CGAL::HalfedgeDS_decorator<typename Mirror_mesh::HDS> d(hds);
  d.join_vertex(h_);
}

}
#endif