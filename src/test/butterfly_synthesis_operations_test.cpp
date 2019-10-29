#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <wtlib/ptq_impl/butterfly_wavelet_operations.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/IO/Polyhedron_iostream.h>


#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif


using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Vertex_handle = typename Mesh::Vertex_handle;
using Halfedge_handle =typename Mesh::Halfedge_handle;
using Point = typename Mesh::Traits::Point_3;

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

using Classify = wtlib::ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;
using Modifier = wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, Mesh_ops>;

using Butterfly_analysis = wtlib::ptq_impl::Butterfly_analysis_operations<Mesh, Mesh_ops>;
using Butterfly_synthesis = wtlib::ptq_impl::Butterfly_synthesis_operations<Mesh, Mesh_ops>;

TEST_CASE("Check zero coefficients",
          "[Butterfly synthesis operations]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    Mesh ctrl_mesh {Utils::loadMesh(file)};
    Mesh test_mesh {Utils::loadMesh(file)};

    if (!ctrl_mesh.is_closed())
    {
      continue;
    }

    Mesh_ops ctrl_mesh_ops {Utils::initMeshOps()};
    Mesh_ops test_mesh_ops {Utils::initMeshOps()};

    Utils::initMeshInfo(ctrl_mesh, ctrl_mesh_ops);
    Utils::initMeshInfo(test_mesh, test_mesh_ops);

    int num_levels = 3;
    int num_types = Butterfly_synthesis::get_num_types(ctrl_mesh, ctrl_mesh_ops);

    for (int i = 0; i < num_levels; ++i)
    {
      Utils::butterfly_subdivision_step(ctrl_mesh, ctrl_mesh_ops, i);
    }

    Butterfly_synthesis b_syn;
    b_syn.initialize(test_mesh, test_mesh_ops, num_levels);

    std::vector<Vertex_handle> test_vertices;
    test_vertices.reserve(ctrl_mesh.size_of_vertices());
    std::vector<Vertex_handle*> test_bands;
    test_bands.reserve(num_levels + 2);

    for (auto v = test_mesh.vertices_begin();
         v != test_mesh.vertices_end(); ++v)
    {
      test_vertices.push_back(v);
    }

    test_bands.push_back(&test_vertices.front());
    test_bands.push_back(&test_vertices.back() + 1);

    std::vector<Vertex_handle*> tmp_bands;
    tmp_bands.resize(num_types + 1);

    for (int level = 0; level < num_levels; ++level)
    {
      Modifier::refine(test_mesh, test_mesh_ops, level, test_vertices, test_bands);
      tmp_bands[0] = test_bands[0];
      tmp_bands[1] = test_bands[level + 1];
      tmp_bands[2] = test_bands[level + 2];
      b_syn.lift(test_mesh, test_mesh_ops, &tmp_bands[0], &tmp_bands[2]);
    }

    REQUIRE(ctrl_mesh.size_of_vertices() ==
            test_mesh.size_of_vertices());
    for (auto [c_v, t_v] = std::make_pair(ctrl_mesh.vertices_begin(),
                                          test_mesh.vertices_begin());
         c_v != ctrl_mesh.vertices_end(); ++t_v, ++c_v)
    {
      REQUIRE(t_v != c_v);
      REQUIRE(test_mesh_ops.get_vertex_id(t_v) == ctrl_mesh_ops.get_vertex_id(c_v));

      REQUIRE(t_v->point().x() == Approx(c_v->point().x()));
      REQUIRE(t_v->point().y() == Approx(c_v->point().y()));
      REQUIRE(t_v->point().z() == Approx(c_v->point().z()));
   }

  }
}


TEST_CASE("Check synthesis after analysis",
          "[Butterfly synthesis operations]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    std::vector<int> vsize_levels {Utils::getSubdivisionLevels(file)};
    int num_levels = vsize_levels.size() - 1;
    int num_types = 2;

    REQUIRE(num_levels > 0);

    Mesh m {Utils::loadMesh(file)};

    if (!m.is_closed())
    {
      continue;
    }

    std::vector<Point> origin_points;
    origin_points.reserve(m.size_of_vertices());

    Mesh_ops aly_m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, aly_m_ops);

    std::vector<std::vector<Point>> coefs;
    coefs.resize((num_types - 1) * num_levels);

    // Classify vertices
    std::vector<Vertex_handle> aly_vertices;
    aly_vertices.reserve(m.size_of_vertices());
    std::vector<Vertex_handle*> aly_bands;
    aly_bands.reserve(num_levels + 2);
    REQUIRE(Classify::classify(m,
                               aly_m_ops,
                               num_levels,
                               aly_vertices,
                               aly_bands,
                               true));
    REQUIRE(aly_bands.size() == (num_types - 1) * num_levels + 2);
    REQUIRE(aly_vertices.size() == m.size_of_vertices());

    // Butterfly operator
    Butterfly_analysis analysis;
    analysis.initialize(m, aly_m_ops);


    for (const Vertex_handle& v : aly_vertices)
    {
      origin_points.push_back(v->point());
    }

    std::vector<Vertex_handle*> aly_tmp_bands(num_types + 1);

    // Do analysis
    for (int l = num_levels; l > 0; --l)
    {
      aly_tmp_bands[0] = aly_bands[0];
      aly_tmp_bands[1] = aly_bands[l];
      aly_tmp_bands[2] = aly_bands[l + 1];
      analysis.lift(m, aly_m_ops, &aly_tmp_bands[0], &aly_tmp_bands[2]);

      // Dump results to coefs array.
      std::vector<Point>& band_coef = coefs[l - 1];
      band_coef.reserve(aly_tmp_bands[2] - aly_tmp_bands[1]);
      for (Vertex_handle* p = aly_tmp_bands[1]; p != aly_tmp_bands[2]; ++p)
      {
        Vertex_handle v = *p;
        band_coef.push_back(v->point());
      }
      REQUIRE(band_coef.size() == vsize_levels[l] - vsize_levels[l - 1]);

      // Coarsen mesh.
      Modifier::coarsen(m, aly_m_ops, l);
    }

    REQUIRE(m.size_of_vertices() == vsize_levels[0]);

    Mesh_ops syn_m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, syn_m_ops);

    Butterfly_synthesis synthesis;
    synthesis.initialize(m, syn_m_ops, num_levels);

    std::vector<Vertex_handle> syn_vertices;
    syn_vertices.reserve(vsize_levels[vsize_levels.size() - 1]);
    std::vector<Vertex_handle*> syn_bands;
    syn_bands.reserve(num_levels + 2);

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      syn_vertices.push_back(v);
    }

    syn_bands.push_back(&syn_vertices.front());
    syn_bands.push_back(&syn_vertices.back() + 1);

    std::vector<Vertex_handle*> syn_tmp_bands;
    syn_tmp_bands.resize(3);
    // Do synthesis
    for (int level = 0; level < num_levels; ++level)
    {
      Modifier::refine(m, syn_m_ops, level, syn_vertices, syn_bands);
      REQUIRE(m.size_of_vertices() == vsize_levels[level + 1]);
      syn_tmp_bands[0] = syn_bands[0];
      syn_tmp_bands[1] = syn_bands[level + 1];
      syn_tmp_bands[2] = syn_bands[level + 2];

      // Load coefs
      const std::vector<Point>& band_coef = coefs[level];
      REQUIRE(band_coef.size() == syn_tmp_bands[2] - syn_tmp_bands[1]);
      for (Vertex_handle* p = syn_tmp_bands[1]; p != syn_tmp_bands[2]; ++p)
      {
        Vertex_handle v = *p;
        int idx = p - syn_tmp_bands[1];
        v->point() = band_coef[idx];
      }
      // Lifting
      synthesis.lift(m, syn_m_ops, &syn_tmp_bands[0], &syn_tmp_bands[2]);
    }

    REQUIRE(m.size_of_vertices() == vsize_levels[num_levels]);
    // Check match
    for (auto [v, idx] = std::make_pair(m.vertices_begin(), 0);
         v != m.vertices_end(); ++v, ++idx)
    {
      REQUIRE(syn_m_ops.get_vertex_id(v) == idx);

      const Point& op = origin_points[idx];
      REQUIRE(op.x() == Approx(v->point().x()).margin(1e-10));
      REQUIRE(op.y() == Approx(v->point().y()).margin(1e-10));
      REQUIRE(op.z() == Approx(v->point().z()).margin(1e-10)); 
    }
  }
}

TEST_CASE("Check analysis after synthesis",
          "[Butterfly synthesis operations]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    int num_levels = 3;
    int num_types = 2;
    Mesh m {Utils::loadMesh(file)};

    int facets_size = m.size_of_facets();
    int vertices_size = m.size_of_vertices();
    int edges_size = m.size_of_halfedges() / 2.0;
    int pow2k = std::pow(2, num_levels);

    if (!m.is_closed())
    {
      continue;
    }

    std::vector<Point> origin_points;
    origin_points.reserve(m.size_of_vertices());

    Mesh_ops syn_m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, syn_m_ops);

    std::vector<std::vector<Point>> coefs;
    coefs.resize((num_types - 1) * num_levels);

    coefs.resize(num_levels);
    for (int level = 1; level <= num_levels; ++level)
    {
      int band_size; 
      if (level == 1)
      {
        band_size = Modifier::get_mesh_size(m, syn_m_ops, level) - m.size_of_vertices();
      }
      else
      {
        band_size = Modifier::get_mesh_size(m, syn_m_ops, level)
                    - Modifier::get_mesh_size(m, syn_m_ops, level - 1);
      }
      coefs[level - 1].resize(band_size, Point{0, 0, 0});
    }


    Butterfly_synthesis synthesis;
    synthesis.initialize(m, syn_m_ops, num_levels);

    std::vector<Vertex_handle> syn_vertices;
    syn_vertices.reserve(0.5 * (pow2k - 1) * (pow2k - 2) * facets_size 
                             + (pow2k - 1) * edges_size
                             + vertices_size);
    std::vector<Vertex_handle*> syn_bands;
    syn_bands.reserve(num_levels + 2);

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      syn_vertices.push_back(v);
      origin_points.push_back(v->point());
    }

    syn_bands.push_back(&syn_vertices.front());
    syn_bands.push_back(&syn_vertices.back() + 1);

    std::vector<Vertex_handle*> syn_tmp_bands(num_types + 1);

    // Do synthesis
    int band_no = 0;
    for (int level = 0; level < num_levels; ++level)
    {
      Modifier::refine(m, syn_m_ops, level, syn_vertices, syn_bands);
      REQUIRE(m.size_of_vertices() == syn_vertices.size());


      syn_tmp_bands[0] = syn_bands[0];
      for (int i = 0; i < num_types; ++i) {
        syn_tmp_bands[i + 1] = syn_bands[(num_types - 1) * level + i + 1];
      }

      for (int i = 0; i < num_types - 1; ++i)
      {
        Vertex_handle* start = syn_tmp_bands[i + 1];
        Vertex_handle* end = syn_tmp_bands[i + 2];

        std::vector<Point>& band_coefs = coefs[band_no + i];
        REQUIRE(band_coefs.size() == end - start);
        for (Vertex_handle* p = start; p != end; ++p)
        {
          Vertex_handle v = *p;
          int idx = p - syn_tmp_bands[1];
          v->point() = band_coefs[idx];
        }
      }
      
      // Lifting
      synthesis.lift(m, syn_m_ops, &syn_tmp_bands[0], &syn_tmp_bands[2]);
      band_no += num_types - 1;
    }
    REQUIRE(band_no == num_levels);

    Mesh_ops aly_m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, aly_m_ops);

    // Classify vertices
    std::vector<Vertex_handle> aly_vertices;
    aly_vertices.reserve(m.size_of_vertices());
    std::vector<Vertex_handle*> aly_bands;
    aly_bands.reserve(num_levels + 2);
    REQUIRE(Classify::classify(m,
                                   aly_m_ops,
                                   num_levels,
                                   aly_vertices,
                                   aly_bands,
                                   true));
    REQUIRE(aly_bands.size() == (num_types - 1) * num_levels + 2);
    REQUIRE(aly_vertices.size() == m.size_of_vertices());

    // Butterfly operator
    Butterfly_analysis analysis;
    analysis.initialize(m, aly_m_ops);

    std::vector<Vertex_handle*> aly_tmp_bands(num_types + 1);

    // Do analysis
    for (int l = num_levels; l > 0; --l)
    {
      aly_tmp_bands[0] = aly_bands[0];
      aly_tmp_bands[1] = aly_bands[l];
      aly_tmp_bands[2] = aly_bands[l + 1];
      analysis.lift(m, aly_m_ops, &aly_tmp_bands[0], &aly_tmp_bands[2]);

      // Dump results to coefs array.
      std::vector<Point>& band_coef = coefs[l - 1];
      REQUIRE(band_coef.size() == (aly_tmp_bands[2] - aly_tmp_bands[1]));
      for (Vertex_handle* p = aly_tmp_bands[1]; p != aly_tmp_bands[2]; ++p)
      {
        Vertex_handle v = *p;
        band_coef[p - aly_tmp_bands[1]] = v->point();
      }
      // Coarsen mesh.
      Modifier::coarsen(m, aly_m_ops, l);
    }

    // Check match
    for (auto [v, idx] = std::make_pair(m.vertices_begin(), 0);
         v != m.vertices_end(); ++v, ++idx)
    {
      REQUIRE(aly_m_ops.get_vertex_id(v) == idx);

      const Point& op = origin_points[idx];
      REQUIRE(op.x() == Approx(v->point().x()).margin(1e-10));
      REQUIRE(op.y() == Approx(v->point().y()).margin(1e-10));
      REQUIRE(op.z() == Approx(v->point().z()).margin(1e-10));
    }

    // Check coefficients
    for (int i = 0; i < coefs.size(); ++i)
    {
      for (int j = 0; j < coefs[i].size(); ++j)
      {
        const Point& p = coefs[i][j];
        REQUIRE(p.x() == Approx(0.0).margin(1e-10));
        REQUIRE(p.y() == Approx(0.0).margin(1e-10));
        REQUIRE(p.z() == Approx(0.0).margin(1e-10));
      }
    }
  }
}


