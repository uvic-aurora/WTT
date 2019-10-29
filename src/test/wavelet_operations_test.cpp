#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <wtlib/ptq_impl/butterfly_wavelet_operations.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/ptq_impl/loop_wavelet_operations.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>
#include <wtlib/wavelet_operations.hpp>

#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Simple_cartesian.h>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

using Vec3 = Mesh::Traits::Vector_3;
using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Vertex_handle = typename Mesh::Vertex_handle;

using Get_vertex_id = std::function<int(Vertex_const_handle)>;
using Set_vertex_id = std::function<void(Vertex_handle, int)>;
using Get_vertex_level = std::function<int(Vertex_const_handle)>;
using Set_vertex_level = std::function<void(Vertex_handle, int)>;
using Get_vertex_type = std::function<int(Vertex_const_handle)>;
using Set_vertex_type = std::function<void(Vertex_handle, int)>;
using Get_vertex_border = std::function<bool(Vertex_const_handle)>;
using Set_vertex_border = std::function<void(Vertex_handle, bool)>;

using Mesh_info = wtlib::ptq_impl::Mesh_info<Mesh>;

using Mesh_ops = wtlib::Wavelet_mesh_operations<
                                Mesh,
                                Get_vertex_id,
                                Set_vertex_id,
                                Get_vertex_level,
                                Set_vertex_level,
                                Get_vertex_type,
                                Set_vertex_type,
                                Get_vertex_border,
                                Set_vertex_border>;
using Utils = Wtlib_test_helper<Mesh, Mesh_ops>;

using PTQ_classify = wtlib::ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
using PTQ_modifier = wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;

using Loop_analysis = wtlib::ptq_impl::Loop_analysis_operations<Mesh, Mesh_ops>;
using Loop_synthesis = wtlib::ptq_impl::Loop_synthesis_operations<Mesh, Mesh_ops>;

using Butterfly_analysis = wtlib::ptq_impl::Butterfly_analysis_operations<Mesh, Mesh_ops>;
using Butterfly_synthesis = wtlib::ptq_impl::Butterfly_synthesis_operations<Mesh, Mesh_ops>;

using Get_num_types = std::function<int(Mesh&, const Mesh_ops&)>;
using Get_mesh_size = std::function<int(Mesh&, const Mesh_ops&, int)>;
using Classify_vertices = std::function<bool(Mesh&,
                                             const Mesh_ops&,
                                             int,
                                             std::vector<Vertex_handle>&,
                                             std::vector<Vertex_handle*>&)>;
using Initialize = std::function<void(Mesh&, const Mesh_ops&)>;
using Syn_initialize = std::function<void(Mesh&, const Mesh_ops&, int)>;
using Cleanup = std::function<void(Mesh&, const Mesh_ops&)>;
using Lift = std::function<void(Mesh&,
                                const Mesh_ops&,
                                Vertex_handle**,
                                Vertex_handle**)>;
using Coarsen = std::function<void(Mesh&, const Mesh_ops&, int)>;
using Refine = std::function<void(Mesh&,
                                  const Mesh_ops&,
                                  int,
                                  std::vector<Vertex_handle>&,
                                  std::vector<Vertex_handle*>&)>;
using Analysis_ops = wtlib::Wavelet_analysis_ops<Mesh,
                                                 Mesh_ops,
                                                 Get_num_types,
                                                 Classify_vertices,
                                                 Initialize,
                                                 Cleanup,
                                                 Lift,
                                                 Coarsen>;

using Synthesis_ops = wtlib::Wavelet_synthesis_ops<Mesh,
                                                   Mesh_ops,
                                                   Get_num_types,
                                                   Get_mesh_size,
                                                   Syn_initialize,
                                                   Cleanup,
                                                   Refine,
                                                   Lift>;

using Analyze = wtlib::Wavelet_analyze<Mesh_ops, Analysis_ops>;
using Synthesize = wtlib::Wavelet_synthesize<Mesh_ops, Synthesis_ops>;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;
using std::placeholders::_5;

TEST_CASE("Check loop analysis return false",
          "[Wavelet operations]")
{
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Classify_vertices ptq_classify = std::bind(&PTQ_classify::classify, _1, _2, _3, _4, _5, true);
  Coarsen ptq_coarsen = &PTQ_modifier::coarsen;
  Initialize analysis_init = &Loop_analysis::initialize;
  Lift analysis_lift = &Loop_analysis::lift;
  Cleanup cleanup = &Loop_analysis::clean_up;
  Analysis_ops analysis {get_num_types,
                         ptq_classify,
                         analysis_init,
                         cleanup,
                         analysis_lift,
                         ptq_coarsen};

  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const auto& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops analysis_mesh_ops {Utils::initMeshOps()};

    Analyze analyze(analysis_mesh_ops, analysis);

    int num_levels = 1;
    std::vector<std::vector<Vec3>> analysis_coefs;
    REQUIRE_FALSE(analyze(m, analysis_coefs, num_levels));
  }
}


TEST_CASE("Check butterfly analysis return false",
          "[Wavelet operations]")
{
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Classify_vertices ptq_classify = std::bind(&PTQ_classify::classify, _1, _2, _3, _4, _5, true);
  Butterfly_analysis b_a;
  Coarsen ptq_coarsen = &PTQ_modifier::coarsen;
  Initialize analysis_init = std::bind(&Butterfly_analysis::initialize, &b_a, _1, _2);
  Lift analysis_lift = std::bind(&Butterfly_analysis::lift, &b_a, _1, _2, _3, _4);
  Cleanup analysis_cleanup = std::bind(&Butterfly_analysis::cleanup, &b_a, _1, _2);
  Analysis_ops analysis {get_num_types,
                         ptq_classify,
                         analysis_init,
                         analysis_cleanup,
                         analysis_lift,
                         ptq_coarsen};

  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const auto& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    
    m.normalize_border();

    if (!m.is_closed())
    {
      continue;
    }

    Mesh_ops analysis_mesh_ops {Utils::initMeshOps()};

    Analyze analyze(analysis_mesh_ops, analysis);

    int num_levels = 1;
    std::vector<std::vector<Vec3>> analysis_coefs;
    REQUIRE_FALSE(analyze(m, analysis_coefs, num_levels));
  }
}

TEST_CASE("loop synthesis analysis",
          "[Wavelet operations]")

{
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Get_mesh_size get_mesh_size = &PTQ_modifier::get_mesh_size;
  Classify_vertices ptq_classify = std::bind(&PTQ_classify::classify, _1, _2, _3, _4, _5, true);
  Coarsen ptq_coarsen = &PTQ_modifier::coarsen;
  Refine ptq_refine = &PTQ_modifier::refine;
  Initialize analysis_init = &Loop_analysis::initialize;
  Syn_initialize synthesis_init = &Loop_synthesis::initialize;
  Lift analysis_lift = &Loop_analysis::lift;
  Lift synthesis_lift = &Loop_synthesis::lift;
  Cleanup cleanup = &Loop_analysis::clean_up;

  Analysis_ops analysis {get_num_types,
                         ptq_classify,
                         analysis_init,
                         cleanup,
                         analysis_lift,
                         ptq_coarsen};

  Synthesis_ops synthesis {get_num_types,
                           get_mesh_size,
                           synthesis_init,
                           cleanup,
                           ptq_refine,
                           synthesis_lift};

  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const auto& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    m.normalize_border();
    Mesh m_copy {m};
    Mesh_ops analysis_mesh_ops {Utils::initMeshOps()};
    Mesh_ops synthesis_mesh_ops {Utils::initMeshOps()};

    Analyze analyze(analysis_mesh_ops, analysis);
    Synthesize synthesize(synthesis_mesh_ops, synthesis);

    int num_levels = 3;

    std::vector<std::vector<Vec3>> origin_coefs;
    origin_coefs.resize(num_levels);
    for (int level = 1; level <= num_levels; ++level)
    {
      int band_size;
      if (level == 1)
      {
        band_size = get_mesh_size(m, analysis_mesh_ops, level) - m.size_of_vertices();
      }
      else
      {
        band_size = get_mesh_size(m, analysis_mesh_ops, level)
                    - get_mesh_size(m, analysis_mesh_ops, level - 1);
      }
      origin_coefs[level - 1].resize(band_size, Vec3{0.01, 0.01, 0.01});
    }

    std::vector<std::vector<Vec3>> coefs {origin_coefs};
    
    // First synthesis
    synthesize(m, coefs, num_levels);
    REQUIRE(coefs.empty());

    std::vector<std::vector<Vec3>> analysis_coefs;
    m.normalize_border();
    // Then analyze
    REQUIRE(analyze(m, analysis_coefs, num_levels));

    // Check match
    REQUIRE(analysis_coefs.size() == origin_coefs.size());
    REQUIRE(m.size_of_vertices() == m_copy.size_of_vertices());
    REQUIRE(m.size_of_facets() == m_copy.size_of_facets());
    REQUIRE(m.size_of_halfedges() == m_copy.size_of_halfedges());

    // Check base mesh
    for (auto [v, vc] = std::make_pair(m.vertices_begin(), m_copy.vertices_begin());
         v != m.vertices_end(); ++v, ++vc)
    {
      REQUIRE(analysis_mesh_ops.get_vertex_id(v) == synthesis_mesh_ops.get_vertex_id(v));
      REQUIRE(v->point().x() == Approx(vc->point().x()).margin(1e-10));
      REQUIRE(v->point().y() == Approx(vc->point().y()).margin(1e-10));
      REQUIRE(v->point().z() == Approx(vc->point().z()).margin(1e-10));
    }

    // Check coefs
    for (int i = 0; i < origin_coefs.size(); ++i)
    {
      const std::vector<Vec3>& band_c0 = origin_coefs[i];
      const std::vector<Vec3>& band_c1 = analysis_coefs[i];
      REQUIRE(band_c0.size() == band_c1.size());;
      for (int j = 0; j < band_c0.size(); ++j)
      {
        REQUIRE(band_c0[j].x() == Approx(band_c1[j].x()));
        REQUIRE(band_c0[j].y() == Approx(band_c1[j].y()));
        REQUIRE(band_c0[j].z() == Approx(band_c1[j].z()));
      }
    }
  }
}


TEST_CASE("butterfly synthesis analysis",
          "[Wavelet operations]")

{
  Get_num_types get_num_types = &PTQ_classify::get_num_types;
  Get_mesh_size get_mesh_size = &PTQ_modifier::get_mesh_size;
  Classify_vertices ptq_classify = std::bind(&PTQ_classify::classify, _1, _2, _3, _4, _5, true);
  Coarsen ptq_coarsen = &PTQ_modifier::coarsen;
  Refine ptq_refine = &PTQ_modifier::refine;

  Butterfly_analysis b_a;
  Butterfly_synthesis b_s;


  Initialize analysis_init = std::bind(&Butterfly_analysis::initialize, &b_a, _1, _2);
  Syn_initialize synthesis_init = std::bind(&Butterfly_synthesis::initialize, &b_s, _1, _2, _3);
  Lift analysis_lift = std::bind(&Butterfly_analysis::lift, &b_a, _1, _2, _3, _4);
  Lift synthesis_lift = std::bind(&Butterfly_synthesis::lift, &b_s, _1, _2, _3, _4);
  Cleanup analysis_cleanup = std::bind(&Butterfly_analysis::cleanup, &b_a, _1, _2);
  Cleanup synthesis_cleanup = std::bind(&Butterfly_synthesis::cleanup, &b_s, _1, _2);

  Analysis_ops analysis {get_num_types,
                         ptq_classify,
                         analysis_init,
                         analysis_cleanup,
                         analysis_lift,
                         ptq_coarsen};

  Synthesis_ops synthesis {get_num_types,
                           get_mesh_size,
                           synthesis_init,
                           synthesis_cleanup,
                           ptq_refine,
                           synthesis_lift};

  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const auto& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    if (!m.is_closed())
    {
      continue;
    }
    Mesh m_copy {m};
    Mesh_ops analysis_mesh_ops {Utils::initMeshOps()};
    Mesh_ops synthesis_mesh_ops {Utils::initMeshOps()};

    Analyze analyze(analysis_mesh_ops, analysis);
    Synthesize synthesize(synthesis_mesh_ops, synthesis);

    int num_levels = 3;

    std::vector<std::vector<Vec3>> origin_coefs;

    origin_coefs.resize(num_levels);
    for (int level = 1; level <= num_levels; ++level)
    {
      int band_size;
      if (level == 1)
      {
        band_size = get_mesh_size(m, analysis_mesh_ops, level) - m.size_of_vertices();
      }
      else
      {
        band_size = get_mesh_size(m, analysis_mesh_ops, level)
                    - get_mesh_size(m, analysis_mesh_ops, level - 1);
      }
      origin_coefs[level - 1].resize(band_size, Vec3{0.015, 0.25, 0.03});
    }

    std::vector<std::vector<Vec3>> coefs {origin_coefs};
    
    // First synthesis
    synthesize(m, coefs, num_levels);
    REQUIRE(coefs.empty());

    std::vector<std::vector<Vec3>> analysis_coefs;
    // Then analyze
    REQUIRE(analyze(m, analysis_coefs, num_levels));

    // Check match
    REQUIRE(analysis_coefs.size() == origin_coefs.size());;
    REQUIRE(m.size_of_vertices() == m_copy.size_of_vertices());;
    REQUIRE(m.size_of_facets() == m_copy.size_of_facets());;
    REQUIRE(m.size_of_halfedges() == m_copy.size_of_halfedges());;

    // Check base mesh
    for (auto [v, vc] = std::make_pair(m.vertices_begin(), m_copy.vertices_begin());
         v != m.vertices_end(); ++v, ++vc)
    {
      REQUIRE(analysis_mesh_ops.get_vertex_id(v) == synthesis_mesh_ops.get_vertex_id(v));
      REQUIRE(v->point().x() == Approx(vc->point().x()));
      REQUIRE(v->point().y() == Approx(vc->point().y()));
      REQUIRE(v->point().z() == Approx(vc->point().z()));
    }

    // Check coefs
    for (int i = 0; i < origin_coefs.size(); ++i)
    {
      const std::vector<Vec3>& band_c0 = origin_coefs[i];
      const std::vector<Vec3>& band_c1 = analysis_coefs[i];
      REQUIRE(band_c0.size() == band_c1.size());;
      for (int j = 0; j < band_c0.size(); ++j)
      {
        REQUIRE(band_c0[j].x() == Approx(band_c1[j].x()));
        REQUIRE(band_c0[j].y() == Approx(band_c1[j].y()));
        REQUIRE(band_c0[j].z() == Approx(band_c1[j].z()));
      }
    }
  }
}
