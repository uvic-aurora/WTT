#ifndef BUTTERFLY_WAVELET_TRANSFORM_HPP
#define BUTTERFLY_WAVELET_TRANSFORM_HPP

/**
 * @file     butterfly_wavelet_transform.hpp
 * @brief    Defines the Butterfly wavelet transforms.
 */

#include <wtlib/ptq_impl/butterfly_wavelet_operations.hpp>
#include <wtlib/ptq_impl/mesh_vertex_info.hpp>
#include <wtlib/ptq_impl/subdivision_modifier.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>
#include <wtlib/wavelet_operations.hpp>

namespace wtlib
{
/**
 * @brief    The Butterfly forward wavelet transform.
 *
 * @tparam   Mesh       Type of mesh
 * @param    mesh       The input mesh 
 * @param    coefs      The wavelet coefficients, where an inner vector is the
 *                      wavelet coefficients at a resolution.
 * @param    num_levels The number of transform levels.
 *
 * @return true         
 * @return false        FWT fails because the input mesh does not have enough
 *                      levels of subdivision connectivity.
 */
template<class Mesh>
bool butterfly_analyze(Mesh& mesh,
                       std::vector<std::vector<typename Mesh::Traits::Vector_3>>& coefs,
                       int num_levels)
{
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Vertex_const_handle = typename Mesh::Vertex_const_handle;
  using Get_vertex_id = std::function<int(Vertex_const_handle)>;
  using Set_vertex_id = std::function<void(Vertex_handle, int)>;
  using Get_vertex_level = std::function<int(Vertex_const_handle)>;
  using Set_vertex_level = std::function<void(Vertex_handle, int)>;
  using Get_vertex_type = std::function<int(Vertex_const_handle)>;
  using Set_vertex_type = std::function<void(Vertex_handle, int)>;
  using Get_vertex_border = std::function<bool(Vertex_const_handle)>;
  using Set_vertex_border = std::function<void(Vertex_handle, bool)>;
  using Mesh_info = ptq_impl::Mesh_info<Mesh>;
  using Mesh_ops = Wavelet_mesh_operations<
                                        Mesh,
                                        Get_vertex_id,
                                        Set_vertex_id,
                                        Get_vertex_level,
                                        Set_vertex_level,
                                        Get_vertex_type,
                                        Set_vertex_type,
                                        Get_vertex_border,
                                        Set_vertex_border
                                      >;


  using Get_num_types = std::function<int(Mesh&, const Mesh_ops&)>;
  using Classify_vertices = std::function<int(Mesh&,
                                              const Mesh_ops&,
                                              int,
                                              std::vector<Vertex_handle>&,
                                              std::vector<Vertex_handle*>&)>;
  using Cleanup = std::function<void(Mesh&, const Mesh_ops&)>;
  using Lift = std::function<void(Mesh&,
                                  const Mesh_ops&,
                                  Vertex_handle**,
                                  Vertex_handle**)>;
  using Coarsen = std::function<void(Mesh&, const Mesh_ops&, int)>;
  using Initialize = std::function<void(Mesh&, const Mesh_ops&)>;



  using Analysis_ops = Wavelet_analysis_ops<Mesh,
                                            Mesh_ops,
                                            Get_num_types,
                                            Classify_vertices,
                                            Initialize,
                                            Cleanup,
                                            Lift,
                                            Coarsen>;

  using Analyze = Wavelet_analyze<Mesh_ops, Analysis_ops>;

  using Butterfly = ptq_impl::Butterfly_analysis_operations<Mesh, Mesh_ops>;
  using PTQ_classify = ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;

  assert(!mesh.empty() && mesh.is_pure_triangle() && mesh.is_closed());

  // PTQ classify and coarsen functors
  PTQ_classify classify;
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Coarsen coarsen = &PTQ_modifier::coarsen;

  // Butterfly specific operations functors
  Butterfly butterfly;
  Initialize initialize = std::bind(&Butterfly::initialize, &butterfly, _1, _2);
  Cleanup cleanup = std::bind(&Butterfly::cleanup, &butterfly, _1, _2);
  Lift lift = std::bind(&Butterfly::lift, &butterfly, _1, _2, _3, _4);

  // Setup mesh_ops
  // Hold mesh vertex info
  Mesh_info mesh_info;
  // Mesh operations functors
  Get_vertex_id get_vertex_id = std::bind(&Mesh_info::get_vertex_id, &mesh_info,  _1);
  Set_vertex_id set_vertex_id = std::bind(&Mesh_info::set_vertex_id, &mesh_info,  _1, _2);
  Get_vertex_type get_vertex_type = std::bind(&Mesh_info::get_vertex_type, &mesh_info,  _1);
  Set_vertex_type set_vertex_type = std::bind(&Mesh_info::set_vertex_type, &mesh_info,  _1, _2);
  Get_vertex_border get_vertex_border = std::bind(&Mesh_info::get_vertex_border, &mesh_info,  _1);
  Set_vertex_border set_vertex_border = std::bind(&Mesh_info::set_vertex_border, &mesh_info,  _1, _2);
  Get_vertex_level get_vertex_level = std::bind(&Mesh_info::get_vertex_level, &mesh_info,  _1);
  Set_vertex_level set_vertex_level = std::bind(&Mesh_info::set_vertex_level, &mesh_info,  _1, _2);

  Mesh_ops mesh_ops {get_vertex_id,
                     set_vertex_id,
                     get_vertex_level,
                     set_vertex_level,
                     get_vertex_type,
                     set_vertex_type,
                     get_vertex_border,
                     set_vertex_border};

  // Create analysis operations.
  Analysis_ops analysis {get_num_types,
                         classify,
                         initialize,
                         cleanup,
                         lift,
                         coarsen};

  Analyze analyze {mesh_ops, analysis};

  return analyze(mesh, coefs, num_levels);
}


/**
 * @brief    Overloaded butterfly_analyze, which allows user to pass in custom mesh_ops.
 *
 */
template<class Mesh, class Mesh_ops>
bool butterfly_analyze(Mesh& mesh,
                       const Mesh_ops& mesh_ops,
                       std::vector<std::vector<typename Mesh::Traits::Vector_3>>& coefs,
                       int num_levels)
{
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Get_num_types = std::function<int(Mesh&, const Mesh_ops&)>;
  using Classify_vertices = std::function<int(Mesh&,
                                              const Mesh_ops&,
                                              int,
                                              std::vector<Vertex_handle>&,
                                              std::vector<Vertex_handle*>&)>;
  using Cleanup = std::function<void(Mesh&, const Mesh_ops&)>;
  using Lift = std::function<void(Mesh&,
                                  const Mesh_ops&,
                                  Vertex_handle**,
                                  Vertex_handle**)>;
  using Coarsen = std::function<void(Mesh&, const Mesh_ops&, int)>;
  using Initialize = std::function<void(Mesh&, const Mesh_ops&)>;

  using Analysis_ops = Wavelet_analysis_ops<Mesh,
                                            Mesh_ops,
                                            Get_num_types,
                                            Classify_vertices,
                                            Initialize,
                                            Cleanup,
                                            Lift,
                                            Coarsen>;

  using Analyze = Wavelet_analyze<Mesh_ops, Analysis_ops>;

  using Butterfly = ptq_impl::Butterfly_analysis_operations<Mesh, Mesh_ops>;
  using PTQ_classify = ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;

  assert(!mesh.empty() && mesh.is_pure_triangle() && mesh.is_closed());

  // PTQ classify and coarsen functors
  PTQ_classify classify;
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Coarsen coarsen = &PTQ_modifier::coarsen;

  // Butterfly specific operations functors
  Butterfly butterfly;
  Initialize initialize = std::bind(&Butterfly::initialize, &butterfly, _1, _2);
  Cleanup cleanup = std::bind(&Butterfly::cleanup, &butterfly, _1, _2);
  Lift lift = std::bind(&Butterfly::lift, &butterfly, _1, _2, _3, _4);

  // Create analysis operations.
  Analysis_ops analysis {get_num_types,
                         classify,
                         initialize,
                         cleanup,
                         lift,
                         coarsen};

  Analyze analyze {mesh_ops, analysis};

  return analyze(mesh, coefs, num_levels);
}

/**
 * @brief    The Butterfly inverse wavelet transform.
 *
 * @tparam   Mesh       Type of mesh
 * @param    mesh       The input mesh 
 * @param    coefs      The wavelet coefficients, where an inner vector should
 *                      be the wavelet coefficients at a resolution, and the
 *                      number should match the number of introduced vertices.
 * @param    num_levels The number of transform levels.
 *
 */
template<class Mesh>
void butterfly_synthesize(Mesh& mesh,
                          std::vector<std::vector<typename Mesh::Traits::Vector_3>>& coefs,
                          int num_levels)
{
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Vertex_const_handle = typename Mesh::Vertex_const_handle;


  using Get_vertex_id = std::function<int(Vertex_const_handle)>;
  using Set_vertex_id = std::function<void(Vertex_handle, int)>;
  using Get_vertex_level = std::function<int(Vertex_const_handle)>;
  using Set_vertex_level = std::function<void(Vertex_handle, int)>;
  using Get_vertex_type = std::function<int(Vertex_const_handle)>;
  using Set_vertex_type = std::function<void(Vertex_handle, int)>;
  using Get_vertex_border = std::function<bool(Vertex_const_handle)>;
  using Set_vertex_border = std::function<void(Vertex_handle, bool)>;
  using Mesh_info = ptq_impl::Mesh_info<Mesh>;
  using Mesh_ops = Wavelet_mesh_operations<
                                        Mesh,
                                        Get_vertex_id,
                                        Set_vertex_id,
                                        Get_vertex_level,
                                        Set_vertex_level,
                                        Get_vertex_type,
                                        Set_vertex_type,
                                        Get_vertex_border,
                                        Set_vertex_border
                                      >;


  using Get_num_types = std::function<int(Mesh&, const Mesh_ops&)>;
  using Get_mesh_size = std::function<int(Mesh&, const Mesh_ops&, int)>;

  using Cleanup = std::function<void(Mesh&, const Mesh_ops&)>;
  using Lift = std::function<void(Mesh&,
                                  const Mesh_ops&,
                                  Vertex_handle**,
                                  Vertex_handle**)>;
  using Refine = std::function<void(Mesh&, 
                                    const Mesh_ops&,
                                    int,
                                    std::vector<Vertex_handle>&,
                                    std::vector<Vertex_handle*>&)>;
  using Initialize = std::function<void(Mesh&, const Mesh_ops&, int)>;



  using Synthesis_ops = Wavelet_synthesis_ops<Mesh,
                                              Mesh_ops,
                                              Get_num_types,
                                              Get_mesh_size,
                                              Initialize,
                                              Cleanup,
                                              Refine,
                                              Lift>;

  using Synthesize = Wavelet_synthesize<Mesh_ops, Synthesis_ops>;

  using Butterfly = ptq_impl::Butterfly_synthesis_operations<Mesh, Mesh_ops>;
  using PTQ_classify = ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;

  assert(!mesh.empty() && mesh.is_pure_triangle() && mesh.is_closed());

  // PTQ refine and get_mesh_size functors
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Get_mesh_size get_mesh_size = &PTQ_modifier::get_mesh_size;
  Refine refine = &PTQ_modifier::refine;

  // Butterfly specific operations functors
  Butterfly butterfly;
  Initialize initialize = std::bind(&Butterfly::initialize, &butterfly, _1, _2, _3);
  Cleanup cleanup = std::bind(&Butterfly::cleanup, &butterfly, _1, _2);
  Lift lift = std::bind(&Butterfly::lift, &butterfly, _1, _2, _3, _4);

  // Setup mesh_ops
  // Hold mesh vertex info
  Mesh_info mesh_info;
  // Mesh operations functors
  Get_vertex_id get_vertex_id = std::bind(&Mesh_info::get_vertex_id, &mesh_info,  _1);
  Set_vertex_id set_vertex_id = std::bind(&Mesh_info::set_vertex_id, &mesh_info,  _1, _2);
  Get_vertex_type get_vertex_type = std::bind(&Mesh_info::get_vertex_type, &mesh_info,  _1);
  Set_vertex_type set_vertex_type = std::bind(&Mesh_info::set_vertex_type, &mesh_info,  _1, _2);
  Get_vertex_border get_vertex_border = std::bind(&Mesh_info::get_vertex_border, &mesh_info,  _1);
  Set_vertex_border set_vertex_border = std::bind(&Mesh_info::set_vertex_border, &mesh_info,  _1, _2);
  Get_vertex_level get_vertex_level = std::bind(&Mesh_info::get_vertex_level, &mesh_info,  _1);
  Set_vertex_level set_vertex_level = std::bind(&Mesh_info::set_vertex_level, &mesh_info,  _1, _2);

  Mesh_ops mesh_ops {get_vertex_id,
                     set_vertex_id,
                     get_vertex_level,
                     set_vertex_level,
                     get_vertex_type,
                     set_vertex_type,
                     get_vertex_border,
                     set_vertex_border};

  // Create analysis operations.
  Synthesis_ops synthesis {get_num_types,
                           get_mesh_size,
                           initialize,
                           cleanup,
                           refine,
                           lift};

  Synthesize synthesize {mesh_ops, synthesis};

  synthesize(mesh, coefs, num_levels);
}

/**
 * @brief    Overloaded butterfly_synthesize, which allows users to pass in custom mesh_ops.
 * 
 */
template<class Mesh, class Mesh_ops>
void butterfly_synthesize(Mesh& mesh,
                          const Mesh_ops& mesh_ops,
                          std::vector<std::vector<typename Mesh::Traits::Vector_3>>& coefs,
                          int num_levels)
{
  using Vertex_handle = typename Mesh::Vertex_handle;
  using Get_num_types = std::function<int(Mesh&, const Mesh_ops&)>;
  using Get_mesh_size = std::function<int(Mesh&, const Mesh_ops&, int)>;
  using Cleanup = std::function<void(Mesh&, const Mesh_ops&)>;
  using Lift = std::function<void(Mesh&,
                                  const Mesh_ops&,
                                  Vertex_handle**,
                                  Vertex_handle**)>;
  using Refine = std::function<void(Mesh&, 
                                    const Mesh_ops&,
                                    int,
                                    std::vector<Vertex_handle>&,
                                    std::vector<Vertex_handle*>&)>;
  using Initialize = std::function<void(Mesh&, const Mesh_ops&, int)>;



  using Synthesis_ops = Wavelet_synthesis_ops<Mesh,
                                              Mesh_ops,
                                              Get_num_types,
                                              Get_mesh_size,
                                              Initialize,
                                              Cleanup,
                                              Refine,
                                              Lift>;

  using Synthesize = Wavelet_synthesize<Mesh_ops, Synthesis_ops>;

  using Butterfly = ptq_impl::Butterfly_synthesis_operations<Mesh, Mesh_ops>;
  using PTQ_classify = ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;

  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  using std::placeholders::_4;

  assert(!mesh.empty() && mesh.is_pure_triangle() && mesh.is_closed());

  // PTQ refine and get_mesh_size functors
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Get_mesh_size get_mesh_size = &PTQ_modifier::get_mesh_size;
  Refine refine = &PTQ_modifier::refine;

  // Butterfly specific operations functors
  Butterfly butterfly;
  Initialize initialize = std::bind(&Butterfly::initialize, &butterfly, _1, _2, _3);
  Cleanup cleanup = std::bind(&Butterfly::cleanup, &butterfly, _1, _2);
  Lift lift = std::bind(&Butterfly::lift, &butterfly, _1, _2, _3, _4);

  // Create analysis operations.
  Synthesis_ops synthesis {get_num_types,
                           get_mesh_size,
                           initialize,
                           cleanup,
                           refine,
                           lift};

  Synthesize synthesize {mesh_ops, synthesis};

  synthesize(mesh, coefs, num_levels);
}
}  // namespace wtlib

#endif