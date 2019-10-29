#include <wtlib/ptq_impl/vertex_classification.hpp>

#include <map>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

// Some handy type definitions.
using Mesh = CGAL::Polyhedron_3<CGAL::Simple_cartesian<double>>;
using Vertex_handle = Mesh::Vertex_handle;
using Vertex_const_handle = Mesh::Vertex_const_handle;
using Vertices = std::vector<Vertex_handle>;
using Bands = std::vector<Vertex_handle*>;
struct Vertex_info {
    int  level;
    int  type;
    int  id;
    bool border;
};

/**
 * Defining a lifted wavelet transform can be decomposed into the steps as
 * follows. First, instantiate the Wavelet_mesh_operations class that defines
 * access to vertex properties. Then, instantiate Wavelet_analysis_ops and
 * Wavelet_synthesis_ops classes that abstract operations in the FWT and IWT,
 * respectively. Next, use the instantiated Wavelet_mesh_operations and
 * Wavelet_analysis_ops (Wavelet_synthesis_ops) to instantiate Wavelet_analyze
 * (Wavelet_synthesize) for computing FWT (IWT). The code below illustrates the
 * class instantiations step by step.
 */
int main() {
  /**     Wavelet_mesh_operations    instantiation */
  // Construct a vertex property map that stores vertex properties.
  std::map<Vertex_const_handle, Vertex_info> props;

  // Construct lambda functions that define access to vertex properties.
  auto get_level = [&props](Vertex_const_handle v) -> int { 
    return props.at(v).level;
  };
  auto set_level = [&props](Vertex_handle v, int level) -> void {
    props[v].level = level;
  };

  auto get_type = [&props](Vertex_const_handle v) -> int {
    return props.at(v).type;
  };
  auto set_type = [&props](Vertex_handle v, int type) -> void {
    props[v].type = type;
  };

  auto get_id = [&props](Vertex_const_handle v) -> int {
    return props.at(v).id;
  };
  auto set_id = [&props](Vertex_handle v, int id) -> void {
    props[v].id = id;
  };

  auto get_border = [&props](Vertex_const_handle v) -> bool {
    return props.at(v).border;
  };
  auto set_border = [&props](Vertex_handle v, bool border) -> void {
    props[v].border = border;
  }; 

  // Use the lambda functions to instantiate Wavelet_mesh_operations.
  using Mesh_ops = wtlib::Wavelet_mesh_operations<Mesh, 
                                                  decltype(get_id),
                                                  decltype(set_id),
                                                  decltype(get_level),
                                                  decltype(set_level),
                                                  decltype(get_type),
                                                  decltype(set_type),
                                                  decltype(get_border),
                                                  decltype(set_border)>; 

  // Construct a mesh_ops object, which will be used in computing FWT and IWT.
  Mesh_ops mesh_ops(get_id, set_id, 
                    get_level, set_level,
                    get_type, set_type,
                    get_border, set_border); 

  /**     Wavelet_analysis_ops    instantiation */
  // Construct lambda functions that define the operations in the FWT.
  auto get_num_types = [](Mesh& mesh, const Mesh_ops& mesh_ops) -> int {
    // This function should return the number of types of the vertices. Here we
    // use 2 as an example.
    return 2;
  }; 

  auto classify = [](Mesh& mesh, const Mesh_ops& mesh_ops, 
                    int levels, Vertices& vertices, Bands& bands) -> bool {
    // Since vertex classification is a non-trivial task, in order to minimize
    // the page, the implementation in WTT is used here.
    return wtlib::ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>:: 
                classify(mesh, mesh_ops, levels, vertices, bands, true); 
  };

  auto fwt_initialize = [](Mesh& mesh, const Mesh_ops& mesh_ops, int levels) -> void {  
    // Do any necessary initialization before lifting.
  }; 

  auto fwt_cleanup = [](Mesh& mesh, const Mesh_ops& mesh_ops) -> void {  
    // Cleanup acquired resources in initialization.
  }; 

  auto fwt_lift = [](Mesh& mesh, const Mesh_ops& mesh_ops, 
                    Vertex_handle** first,
                    Vertex_handle** last) -> void {
    // Do lifting calculation. Vertices [*first, *(first+1)) are the old
    // vertices, and [*(first+1), *last) are the new vertices. Incrementing
    // *(first+1) obtains the vertices of the next type.
  }; 

  auto coarsen = [](Mesh& mesh, 
                    const Mesh_ops& mesh_ops,
                    int level) -> void {
    // If PTQ is used, the implementation in WTT can be used.
    wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>:: 
        coarsen(mesh, mesh_ops, level); 
  }; 

  // Use the lambda functions to instantiate Wavlet_analysis_ops.
  using FWT_ops = wtlib::Wavelet_analysis_ops<Mesh, Mesh_ops, 
                                              decltype(get_num_types),
                                              decltype(classify),
                                              decltype(fwt_initialize),
                                              decltype(fwt_cleanup),
                                              decltype(fwt_lift),
                                              decltype(coarsen)>;

  // Construct an object which will be used in computing the FWT.                 
  FWT_ops fwt_ops(get_num_types, classify, fwt_initialize, fwt_cleanup, fwt_lift, coarsen);

  /**     Wavelet_synthesis_ops    instantiation */
  // Construct lambda functions that define operations in the IWT.
  auto get_mesh_size = [](Mesh& mesh, 
                          const Mesh_ops& mesh_ops, 
                          int levels) -> int {
      // Predicate the final number of vertices after computing the given levels of the IWT.
      return wtlib::ptq_impl::    
              PTQ_subdivision_modifier<Mesh, Mesh_ops>::
                get_mesh_size(mesh, mesh_ops, levels);  
  }; 

  auto iwt_initialize = [](Mesh& mesh, const Mesh_ops& mesh_ops, int levels) -> void {  
    // Do any necessary initialization before lifting.
  };

  auto iwt_cleanup = [](Mesh& mesh, const Mesh_ops& mesh_ops) -> void { 
    // Cleanup resources acquired in initialization after the IWT is done.
  };

  auto refine = [](Mesh& mesh, const Mesh_ops& mesh_ops, 
                  int level, Vertices& vertices, Bands& bands) -> void {
    // If PTQ is used, the implementation in WTT can be used.
    wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>:: 
        refine(mesh, mesh_ops, level, vertices, bands); 
  };

  auto iwt_lift = [](Mesh& mesh, const Mesh_ops& mesh_ops, 
                    Vertex_handle** first,
                    Vertex_handle** last) -> void { 
    // Do lifting calculation. Vertices [*first, *(first+1)) are the old
    // vertices, and [*(first+1), *last) are the new vertices. Incrementing
    // *(first+1) obtains the vertices of the next type.
  };

  // Use the lambda functions to instantiate Wavelet_synthesis_ops.
  using IWT_ops = wtlib::Wavelet_synthesis_ops<
                        Mesh, Mesh_ops,
                        decltype(get_num_types), decltype(get_mesh_size),
                        decltype(iwt_initialize), decltype(iwt_cleanup),
                        decltype(refine), decltype(iwt_lift)>;
  
  // Construct an object which will be used in computing the IWT.
  IWT_ops iwt_ops(get_num_types, get_mesh_size,
                  iwt_initialize, iwt_cleanup,
                  refine, iwt_lift);

  // Instantiate Wavelet_analyze
  using FWT = wtlib::Wavelet_analyze<Mesh_ops, FWT_ops>; 
  // Instantiate Wavelet_synthesize
  using IWT = wtlib::Wavelet_synthesize<Mesh_ops, IWT_ops>;

  // The constructed objects can be used as function calls to compute the FWT and IWT.
  FWT fwt(mesh_ops, fwt_ops); 
  IWT iwt(mesh_ops, iwt_ops);

  Mesh mesh;
  std::cin >> mesh;
  std::vector<std::vector<Vector3>> coefs;
  int num_levels = 4;

  fwt(mesh, coefs, num_levels); // Compute the FWT. 
  iwt(mesh, coefs, num_levels); // Compute the IWT.
}