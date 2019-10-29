#ifndef WAVELET_OPERATIONS_HPP
#define WAVELET_OPERATIONS_HPP

#include <vector>
#include <CGAL/Origin.h>

namespace wtlib
{

template <class M, class MO, class F1, class F2, class F3, class F4,
  class F5, class F6>
class Wavelet_analysis_ops
{
public:
  using Mesh = M;
  using Mesh_ops = MO;
  using Get_num_types = F1;
  using Classify_vertices = F2;
  using Initialize = F3;
  using Cleanup = F4;
  using Lift = F5;
  using Coarsen = F6;
  Wavelet_analysis_ops(
    Get_num_types get_num_types,
    Classify_vertices classify_vertices,
    Initialize initialize,
    Cleanup cleanup,
    Lift lift,
    Coarsen coarsen
  ) :
    get_num_types_(get_num_types),
    classify_vertices_(classify_vertices),
    initialize_(initialize),
    cleanup_(cleanup),
    lift_(lift),
    coarsen_(coarsen)
    {}

  int get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops) const
  {
    return get_num_types_(mesh, mesh_ops);
  }

  bool classify_vertices(Mesh& mesh, const Mesh_ops& mesh_ops, int num_levels,
    std::vector<typename Mesh::Vertex_handle>& vertices, std::vector<typename Mesh::Vertex_handle*>& bands) const
  {
    return classify_vertices_(mesh, mesh_ops, num_levels, vertices, bands);
  }

  void initialize(Mesh& mesh, const Mesh_ops& mesh_ops) const
  {
    initialize_(mesh, mesh_ops);
  }

  void cleanup(Mesh& mesh, const Mesh_ops& mesh_ops) const
  {
    cleanup_(mesh, mesh_ops);
  }

  void lift(Mesh& mesh, const Mesh_ops& mesh_ops,
    typename Mesh::Vertex_handle** first_band, typename Mesh::Vertex_handle** last_band) const
  {
    lift_(mesh, mesh_ops, first_band, last_band);
  }

  void coarsen(Mesh& mesh, const Mesh_ops& mesh_ops, int level) const
  {
    coarsen_(mesh, mesh_ops, level);
  }

private:
  Get_num_types get_num_types_;
  Classify_vertices classify_vertices_;
  Initialize initialize_;
  Cleanup cleanup_;
  Lift lift_;
  Coarsen coarsen_;
};  // class Wavelet_analysis_ops


template <class M, class MO, class F1, class F2, class F3, class F4,
  class F5, class F6>
class Wavelet_synthesis_ops
{
public:
  using Mesh = M;
  using Mesh_ops = MO;
  using Get_num_types = F1;
  using Get_mesh_size = F2;
  using Initialize = F3;
  using Cleanup = F4;
  using Refine = F5;
  using Lift = F6;
  Wavelet_synthesis_ops(
    Get_num_types get_num_types,
    Get_mesh_size get_mesh_size,
    Initialize initialize,
    Cleanup cleanup,
    Refine refine,
    Lift lift
  ) :
    get_num_types_(get_num_types),
    get_mesh_size_(get_mesh_size),
    initialize_(initialize),
    cleanup_(cleanup),
    refine_(refine),
    lift_(lift)
    {}

  int get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops) const
  {
    return get_num_types_(mesh, mesh_ops);
  }

  int get_mesh_size(Mesh& mesh, const Mesh_ops& mesh_ops, int num_levels) const
  {
    return get_mesh_size_(mesh, mesh_ops, num_levels);
  }

  void initialize(Mesh& mesh, const Mesh_ops& mesh_ops, int level) const
  {
    return initialize_(mesh, mesh_ops, level);
  }

  void cleanup(Mesh& mesh, const Mesh_ops& mesh_ops) const
  {
    return cleanup_(mesh, mesh_ops);
  }

  void refine(Mesh& mesh, const Mesh_ops& mesh_ops, int level, std::vector<typename Mesh::Vertex_handle>& vertices, std::vector<typename Mesh::Vertex_handle*>& bands) const
  {
    return refine_(mesh, mesh_ops, level, vertices, bands);
  }

  void lift(Mesh& mesh, const Mesh_ops& mesh_ops,
    typename Mesh::Vertex_handle** first_band, typename Mesh::Vertex_handle** last_band) const
  {
    return lift_(mesh, mesh_ops, first_band, last_band);
  }

private:
  Get_num_types get_num_types_;
  Get_mesh_size get_mesh_size_;
  Initialize initialize_;
  Cleanup cleanup_;
  Refine refine_;
  Lift lift_;
};  // class Wavelet_synthesis_ops


template<class T1, class T2>
class Wavelet_analyze
{
public:
  using Mesh_ops = T1;
  using Analysis_ops = T2;
  using Mesh = typename Mesh_ops::Mesh;
  using Vector_3 = typename Mesh::Traits::Vector_3;

  Wavelet_analyze(Mesh_ops mesh_ops, Analysis_ops analysis_ops)
  : mesh_ops_(mesh_ops), 
    analysis_ops_(analysis_ops)
  {}

  bool operator()(Mesh& mesh, std::vector<std::vector<Vector_3>>& coefs, int num_levels) const
  {
    // Initialize the border information for each vertex.
    // First, set the border flag to false for each vertex.
    for (auto v = mesh.vertices_begin(); v != mesh.vertices_end(); ++v) {
      mesh_ops_.set_vertex_border(v, false);
    }
    // Then, set the border flag to true for each border vertex.
    if (mesh.normalized_border_is_valid()) {
      // The border is normalized so we can handle this more efficiently.
      for (auto h = mesh.border_halfedges_begin(); h != mesh.halfedges_end(); ++h) {
        mesh_ops_.set_vertex_border(h->vertex(), true);
        mesh_ops_.set_vertex_border(h->opposite()->vertex(), true);
      }
    } else {
      // The border is not normalized so we must use a less efficient algorithm.
      for (auto h = mesh.halfedges_begin(); h != mesh.halfedges_end(); ++h) {
        if (h->is_border_edge()) {
          mesh_ops_.set_vertex_border(h->vertex(), true);
          mesh_ops_.set_vertex_border(h->opposite()->vertex(), true);
        }
      }
    }

    // Get the number of vertex types, which depends on the topological
    // refinement rule.
    const int num_types = analysis_ops_.get_num_types(mesh, mesh_ops_);

    // Get the total number of bands.
    const int num_bands = (num_types - 1) * num_levels;

    // Create arrays for the vertices and bands.
    std::vector<typename Mesh::Vertex_handle> vertices;
    vertices.reserve(mesh.size_of_vertices());
    std::vector<typename Mesh::Vertex_handle*> bands;
    bands.reserve(num_bands + 2);

    std::vector<typename Mesh::Vertex_handle*> tmp_bands(num_types + 1);

    // Determine the coarse mesh vertices.
    // This operation can fail if the mesh does not have the
    // correct subdivision connectivity.
    // This operation initializes the level and type information
    // for each vertex.
    // The parameters vertices and bands are set by function.
    if (!analysis_ops_.classify_vertices(mesh, mesh_ops_, num_levels, vertices, bands)) {
      return false;
    }

    // Initialize band array with an empty array for each band.
    coefs = std::move(std::vector<std::vector<Vector_3>>(num_bands));

    // Perform any initialization.
    // This may be a no-op for some wavelet transforms.
    // Note: Maybe this needs additional parameters?
    analysis_ops_.initialize(mesh, mesh_ops_);

    // Perform each of the levels of wavelet transform analysis.
    // Note: The levels must be numbered consistently in analysis and synthesis.
    // 0 is coarsest resolution
    // num_levels is finest resolution
    int band_no = num_bands;
    for (int level = num_levels - 1; level >= 0; --level) {

      tmp_bands[0] = bands[0];
      for (int i = 0; i < num_types; ++i) {
        tmp_bands[i + 1] = bands[(num_types - 1) * level + i + 1];
      }

      // Perform the lifting steps.
      assert(&tmp_bands[num_types] - &tmp_bands[0] == num_types);
      analysis_ops_.lift(mesh,
                         mesh_ops_,
                         &tmp_bands[0],
                         &tmp_bands[num_types]);

      // Copy the vertex positions to the wavelet coefficient array.
      for (int i = 0; i < num_types - 1; ++i) {
        typename Mesh::Vertex_handle* start = tmp_bands[i + 1];
        typename Mesh::Vertex_handle* end = tmp_bands[i + 2];
        std::vector<Vector_3>& band_coefs = coefs[band_no - (num_types - 1) + i];
        int band_size = end - start;
        band_coefs.reserve(band_size);
        for (typename Mesh::Vertex_handle* p = start; p != end; ++p) {
          band_coefs.push_back((*p)->point() - CGAL::ORIGIN);
        }
      }

      // Apply the inverse topological-refinment rule.
      analysis_ops_.coarsen(mesh, mesh_ops_, level + 1);

      band_no -= num_types - 1;
    }
    assert(band_no == 0);
    // Perform any cleanup.
    // This may be a no-op for some wavelet transforms.
    analysis_ops_.cleanup(mesh, mesh_ops_);

    return true;
  }

private:
  Mesh_ops mesh_ops_;
  Analysis_ops analysis_ops_;
};  // class Wavelet_analyze


template <class T1, class T2>
class Wavelet_synthesize
{
public:
  using Mesh_ops = T1;
  using Synthesis_ops = T2;
  using Mesh = typename Mesh_ops::Mesh;
  using Vector_3 = typename Mesh_ops::Mesh::Traits::Vector_3;

  Wavelet_synthesize(Mesh_ops mesh_ops, Synthesis_ops synthesis_ops)
  : mesh_ops_(mesh_ops),
    synthesis_ops_(synthesis_ops)
  {}


  void operator()(Mesh& mesh, std::vector<std::vector<Vector_3>>& coefs, int num_levels)
  {
    // Get the number of vertex types, which depends on the
    // topological refinement rule.
    int num_types = synthesis_ops_.get_num_types(mesh, mesh_ops_);

    // Determine the number of vertices in the most-refined mesh
    // (i.e., at the highest resolution level).
    int mesh_size = synthesis_ops_.get_mesh_size(mesh, mesh_ops_, num_levels);

    // Create arrays for the vertices and bands.
    std::vector<typename Mesh::Vertex_handle> vertices;
    vertices.reserve(mesh_size);
    std::vector<typename Mesh::Vertex_handle*> bands;
    bands.reserve(num_levels * (num_types - 1) + 2);

    std::vector<typename Mesh::Vertex_handle*> tmp_bands(num_types + 1);

    // Initialize the level, type, id, and border information for each vertex
    // (in the coarse mesh).
    for (auto [v, id] = std::make_pair(mesh.vertices_begin(), 0);
         v != mesh.vertices_end();
         ++v, ++id) 
    {
      vertices.push_back(v);
      mesh_ops_.set_vertex_id(v, id);
      mesh_ops_.set_vertex_level(v, 0);
      mesh_ops_.set_vertex_type(v, 0);
      mesh_ops_.set_vertex_border(v, false);
    }
    if (mesh.normalized_border_is_valid()) {
      for (auto h = mesh.border_halfedges_begin(); h != mesh.halfedges_end(); ++h) {
        mesh_ops_.set_vertex_border(h->vertex(), true);
        mesh_ops_.set_vertex_border(h->opposite()->vertex(), true);
      }
    } else {
      for (auto h = mesh.halfedges_begin(); h != mesh.halfedges_end(); ++h) {
        if (h->is_border_edge()) {
          mesh_ops_.set_vertex_border(h->vertex(), true);
          mesh_ops_.set_vertex_border(h->opposite()->vertex(), true);
        }
      }
    }

    // Initialize bands for old vertices
    bands.push_back(&vertices.front());
    bands.push_back(&vertices.back() + 1);
    assert(bands.size() == 2);
    // Perform any initialization prior to wavelet synthesis.
    // This may be a no-op for some wavelet transforms.
    // I think that this step will not be a no-op for Butterfly.
    // Note: What parameters are needed here?  I think num_levels is needed
    synthesis_ops_.initialize(mesh, mesh_ops_, num_levels);

    // For each level in the synthesis process...
    int band_no = 0;
    for (int level = 0; level < num_levels; ++level) {

      // Apply the topological refinement rule.
      synthesis_ops_.refine(mesh, mesh_ops_, level, vertices, bands);
      
      tmp_bands[0] = bands[0];
      for (int i = 0; i < num_types; ++i) {
        tmp_bands[i + 1] = bands[(num_types - 1) * level + i + 1];
      }
      assert(&tmp_bands[num_types] - &tmp_bands[0] == num_types);

      // Initialize the positions of the newly-added vertices by copying
      // their values from the wavelet coefficient arrays.
      //read_coefs(mesh, coefs /*, first_band, last_band */);
      for (int i = 0; i < num_types - 1; ++i) {
        typename Mesh::Vertex_handle* start = tmp_bands[i + 1];
        typename Mesh::Vertex_handle* end = tmp_bands[i + 2];
        const std::vector<Vector_3>& band_coefs = coefs[band_no + i];
        const Vector_3* band_coef = band_coefs.data();
        int band_size = end - start;
        assert(band_size == band_coefs.size());
        for (typename Mesh::Vertex_handle* v = start; v != end; ++v) {
          (*v)->point() = CGAL::ORIGIN + (*band_coef);
          ++band_coef;
        }
      }

      // Apply the lifting steps.
      synthesis_ops_.lift(mesh, mesh_ops_, &tmp_bands[0],
        &tmp_bands[num_types]);

      band_no += num_types - 1;
    }
    assert(band_no == num_levels * (num_types - 1));

    // Perform any cleanup after wavelet synthesis.
    synthesis_ops_.cleanup(mesh, mesh_ops_);

    // Discard the empty coefficient arrays.
    coefs.erase(coefs.begin(), coefs.begin() + band_no);
  }
private:
  Mesh_ops mesh_ops_;
  Synthesis_ops synthesis_ops_;
};  // class Wavelet_synthesize
}  // namespace wtlib

#endif  // define WAVELET_OPERATIONS_HPP