#ifndef LOOP_WAVELET_TRANSFORM_HPP
#define LOOP_WAVELET_TRANSFORM_HPP

/**
 * @file     loop_wavelet_transform.hpp
 * @brief    Defines the Loop wavelet transform.
 */

#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/ptq_impl/loop_wavelet_operations.hpp>
#include <wtlib/ptq_impl/mesh_vertex_info.hpp>
#include <wtlib/ptq_impl/subdivision_modifier.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>
#include <wtlib/wavelet_operations.hpp>

namespace wtlib
{

/**
 * Definitions of several wrappers that will be used to instantiate the Wavelet_analyze and Wavelet_synthesize. 
 */

template <class Mesh, class Mesh_ops>
bool loop_analyze_classify(Mesh& mesh, const Mesh_ops& mesh_ops, int num_levels, std::vector<typename Mesh::Vertex_handle>& vertices,
  std::vector<typename Mesh::Vertex_handle*>& bands)
{
  using PTQ_classify = ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
  return PTQ_classify::classify(mesh, mesh_ops, num_levels, vertices, bands, true);
}

template <class Mesh, class Mesh_ops>
void loop_analyze_initialize(Mesh& mesh, const Mesh_ops& mesh_ops)
{
  using Loop = ptq_impl::Loop_analysis_operations<Mesh, Mesh_ops>;
  Loop::initialize(mesh, mesh_ops);
}

template <class Mesh, class Mesh_ops>
void loop_analyze_cleanup(Mesh& mesh, const Mesh_ops& mesh_ops)
{
  using Loop = ptq_impl::Loop_analysis_operations<Mesh, Mesh_ops>;
  Loop::clean_up(mesh, mesh_ops);
}

template <class Mesh, class Mesh_ops>
void loop_analyze_lift(Mesh& mesh, const Mesh_ops& mesh_ops,
  typename Mesh::Vertex_handle** first_band, typename Mesh::Vertex_handle** last_band)
{
  using Loop = ptq_impl::Loop_analysis_operations<Mesh, Mesh_ops>;
  Loop::lift(mesh, mesh_ops, first_band, last_band);
}

template <class Mesh, class Mesh_ops>
void loop_analyze_coarsen(Mesh& mesh, const Mesh_ops& mesh_ops, int level)
{
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;
  PTQ_modifier::coarsen(mesh, mesh_ops, level);
}


template <class Mesh, class Mesh_ops>
int loop_get_num_types(Mesh& mesh, const Mesh_ops& mesh_ops)
{
  // There are two types of vertices (i.e., old and edge).
  return 2;
}

template <class Mesh, class Mesh_ops>
int loop_synthesize_get_mesh_size(Mesh& mesh, const Mesh_ops& mesh_ops, int num_levels)
{
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;
  return PTQ_modifier::get_mesh_size(mesh, mesh_ops, num_levels);
}

template <class Mesh, class Mesh_ops>
void loop_synthesize_initialize(Mesh& mesh, const Mesh_ops& mesh_ops, int level)
{
  using Loop = ptq_impl::Loop_synthesis_operations<Mesh, Mesh_ops>;
  Loop::initialize(mesh, mesh_ops, level);
}

template <class Mesh, class Mesh_ops>
void loop_synthesize_cleanup(Mesh& mesh, const Mesh_ops& mesh_ops)
{
  using Loop = ptq_impl::Loop_synthesis_operations<Mesh, Mesh_ops>;
  Loop::clean_up(mesh, mesh_ops);
}

template <class Mesh, class Mesh_ops>
void loop_synthesize_refine(Mesh& mesh, const Mesh_ops& mesh_ops, int level, std::vector<typename Mesh::Vertex_handle>& vertices, std::vector<typename Mesh::Vertex_handle*>& bands)
{
  using PTQ_modifier = ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;
  return PTQ_modifier::refine(mesh, mesh_ops, level, vertices, bands);
}

template <class Mesh, class Mesh_ops>
void loop_synthesize_lift(Mesh& mesh, const Mesh_ops& mesh_ops,
  typename Mesh::Vertex_handle** first_band, typename Mesh::Vertex_handle** last_band)
{
  using Loop = ptq_impl::Loop_synthesis_operations<Mesh, Mesh_ops>;
  return Loop::lift(mesh, mesh_ops, first_band, last_band);
}



/**
 * @brief    The Loop forward wavelet transform.
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
bool loop_analyze(Mesh& mesh,
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
  using Analysis_ops = Wavelet_analysis_ops<Mesh,
                                            Mesh_ops,
                                            int (*) (Mesh&, const Mesh_ops&),
                                            bool (*) (Mesh&,
                                                      const Mesh_ops&, 
                                                      int,
                                                      std::vector<Vertex_handle>&,
                                                      std::vector<Vertex_handle*>&),
                                            void (*) (Mesh&, const Mesh_ops&),
                                            void (*) (Mesh&, const Mesh_ops&),
                                            void (*) (Mesh&,
                                                      const Mesh_ops&,
                                                      Vertex_handle**,
                                                      Vertex_handle**),
                                            void (*) (Mesh&, const Mesh_ops&, int)
>;
  using Wavelet_analyze = Wavelet_analyze<Mesh_ops, Analysis_ops>;

  using std::placeholders::_1;
  using std::placeholders::_2;

  assert(!mesh.empty() && mesh.is_pure_triangle());

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

  return Wavelet_analyze(mesh_ops, 
                         Analysis_ops(
                            loop_get_num_types<Mesh, Mesh_ops>,
                            loop_analyze_classify<Mesh, Mesh_ops>,
                            loop_analyze_initialize<Mesh, Mesh_ops>,
                            loop_analyze_cleanup<Mesh, Mesh_ops>,
                            loop_analyze_lift<Mesh, Mesh_ops>,
                            loop_analyze_coarsen<Mesh, Mesh_ops>
                            )
                         )(mesh, coefs, num_levels);
}

/**
 * @brief    Overloaded loop_analyze.
 * 
 */
template <class Mesh, class Mesh_ops>
bool loop_analyze(Mesh& mesh, Mesh_ops& mesh_ops,
  std::vector<std::vector<typename Mesh::Traits::Vector_3>>& coefs, int num_levels)
{
  using Analysis_ops = wtlib::Wavelet_analysis_ops<Mesh, Mesh_ops,
    int (*) (Mesh&, const Mesh_ops&),
    bool (*) (Mesh&, const Mesh_ops&, int, std::vector<typename Mesh::Vertex_handle>&, std::vector<typename Mesh::Vertex_handle*>&),
    void (*) (Mesh&, const Mesh_ops&),
    void (*) (Mesh&, const Mesh_ops&),
    void (*) (Mesh&, const Mesh_ops&, typename Mesh::Vertex_handle**, typename Mesh::Vertex_handle**),
    void (*) (Mesh&, const Mesh_ops&, int)
>;
  using Wavelet_analyze = Wavelet_analyze<Mesh_ops, Analysis_ops>;

  assert(!mesh.empty() && mesh.is_pure_triangle());

  return Wavelet_analyze(mesh_ops,
                         Analysis_ops(
                            loop_get_num_types<Mesh, Mesh_ops>,
                            loop_analyze_classify<Mesh, Mesh_ops>,
                            loop_analyze_initialize<Mesh, Mesh_ops>,
                            loop_analyze_cleanup<Mesh, Mesh_ops>,
                            loop_analyze_lift<Mesh, Mesh_ops>,
                            loop_analyze_coarsen<Mesh, Mesh_ops>
                            )
                         )(mesh, coefs, num_levels);
}



/**
 * @brief    The Loop inverse wavelet transform.
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
void loop_synthesize(Mesh& mesh,
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
  using Synthesis_ops = Wavelet_synthesis_ops<Mesh,
                                              Mesh_ops,
                                              int (*)(Mesh&, const Mesh_ops&),
                                              int (*)(Mesh&, const Mesh_ops&, int),
                                              void (*)(Mesh&, const Mesh_ops&, int),
                                              void (*)(Mesh&, const Mesh_ops&),
                                              void (*)(Mesh&,
                                                       const Mesh_ops&,
                                                       int,
                                                       std::vector<Vertex_handle>&,
                                                       std::vector<Vertex_handle*>&),
                                              void (*)(Mesh&,
                                                       const Mesh_ops&,
                                                       Vertex_handle**,
                                                       Vertex_handle**)
                                              >;
  using Wavelet_synthesize = Wavelet_synthesize<Mesh_ops, Synthesis_ops>;

  using std::placeholders::_1;
  using std::placeholders::_2;

  assert(!mesh.empty() && mesh.is_pure_triangle());

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
  Wavelet_synthesize(mesh_ops,
                     Synthesis_ops(
                        loop_get_num_types<Mesh, Mesh_ops>,
                        loop_synthesize_get_mesh_size<Mesh, Mesh_ops>,
                        loop_synthesize_initialize<Mesh, Mesh_ops>,
                        loop_synthesize_cleanup<Mesh, Mesh_ops>,
                        loop_synthesize_refine<Mesh, Mesh_ops>,
                        loop_synthesize_lift<Mesh, Mesh_ops>
                        )
                     )(mesh, coefs, num_levels);
}

/**
 * @brief    Overloaded loop_synthesize.
 * 
 */
template <class Mesh, class Mesh_ops>
void loop_synthesize(Mesh& mesh, Mesh_ops& mesh_ops,
  std::vector<std::vector<typename Mesh::Traits::Vector_3>>& coefs, int num_levels)
{
  using Synthesis_ops = Wavelet_synthesis_ops<Mesh, Mesh_ops,
    int (*)(Mesh&, const Mesh_ops&),
    int (*)(Mesh&, const Mesh_ops&, int),
    void (*)(Mesh&, const Mesh_ops&, int),
    void (*)(Mesh&, const Mesh_ops&),
    void (*)(Mesh&, const Mesh_ops&, int, std::vector<typename Mesh::Vertex_handle>&, std::vector<typename Mesh::Vertex_handle*>&),
    void (*)(Mesh&, const Mesh_ops&, typename Mesh::Vertex_handle**, typename Mesh::Vertex_handle**)
  >;
  using Wavelet_synthesize = Wavelet_synthesize<Mesh_ops, Synthesis_ops>;
  Wavelet_synthesize(mesh_ops,
                     Synthesis_ops(
                        loop_get_num_types<Mesh, Mesh_ops>,
                        loop_synthesize_get_mesh_size<Mesh, Mesh_ops>,
                        loop_synthesize_initialize<Mesh, Mesh_ops>,
                        loop_synthesize_cleanup<Mesh, Mesh_ops>,
                        loop_synthesize_refine<Mesh, Mesh_ops>,
                        loop_synthesize_lift<Mesh, Mesh_ops>
                        )
                     )(mesh, coefs, num_levels);
}

}
#endif