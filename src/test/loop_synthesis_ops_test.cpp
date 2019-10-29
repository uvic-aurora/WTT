#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <wtlib/ptq_impl/loop_wavelet_operations.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>


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


using Loop_analysis = wtlib::ptq_impl::Loop_analysis_operations<Mesh, Mesh_ops>;
using Loop_synthesis = wtlib::ptq_impl::Loop_synthesis_operations<Mesh, Mesh_ops>;
using Loop_synthesis_lift = wtlib::ptq_impl::Loop_lift_operations<Mesh, Mesh_ops, false>;


TEST_CASE("Check lifting steps",
          "[Loop synthesis operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, m_ops);
    std::vector<Point> origin_points;

    int vsize = m.size_of_vertices();
    int vsize_after = Modifier::get_mesh_size(m, m_ops, 1);

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;
    vertices.reserve(vsize_after);
    bands.reserve(3);

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      origin_points.push_back(v->point());
      vertices.push_back(v);
    }

    bands.push_back(&vertices.front());
    bands.push_back(&vertices.back() + 1);

    Modifier::refine(m, m_ops, 0, vertices, bands);

    REQUIRE(vertices.size() == vsize_after);;
    REQUIRE(bands.size() == 3);;
    REQUIRE(bands[1] - bands[0] == vsize);;
    REQUIRE(bands[2] - bands[0] == vsize_after);;


    // Do dual lifting, old vertices should be unchanged
    // since wavelet coefficients are zeros
    Loop_synthesis_lift::border_edges_to_border_olds_dual(m, m_ops, bands[0], bands[1], bands[2]);
    Loop_synthesis_lift::inner_edges_to_inner_olds_dual(m, m_ops, bands[0], bands[1], bands[2]);

    for (auto [p, idx] = std::make_pair(bands[0], 0);
         p != bands[1]; ++p, ++idx)
    {
      Vertex_handle v = *p;
      REQUIRE(v->point().x() == Approx(origin_points[idx].x()));
      REQUIRE(v->point().y() == Approx(origin_points[idx].y()));
      REQUIRE(v->point().z() == Approx(origin_points[idx].z()));
    }

    Loop_synthesis_lift::inner_olds_to_inner_edges(m, m_ops, bands[0], bands[1], bands[2]);
    Loop_synthesis_lift::inner_edges_to_inner_olds(m, m_ops, bands[0], bands[1], bands[2]);

    // Border edges should be zero
    for (Vertex_handle* p = bands[1]; p != bands[2]; ++p)
    {
      Vertex_handle v = *p;
      if (m_ops.get_vertex_border(v))
      {
        REQUIRE(v->point() == CGAL::ORIGIN);
      }
    }
    Loop_synthesis_lift::border_olds_to_border_edges(m, m_ops, bands[0], bands[1], bands[2]);

    // Until now, the old border vertices should remain unchanged
    for (auto [p, idx] = std::make_pair(bands[0], 0);
         p != bands[1]; ++p, ++idx)
    {
      Vertex_handle v = *p;
      if (m_ops.get_vertex_border(v))
      {
        REQUIRE(v->point().x() == Approx(origin_points[idx].x()));
        REQUIRE(v->point().y() == Approx(origin_points[idx].y()));
        REQUIRE(v->point().z() == Approx(origin_points[idx].z()));
      }
    }

    // Check border vertices
    for (Vertex_handle* p = bands[1]; p != bands[2]; ++p)
    {
      Vertex_handle v = *p;
      if (m_ops.get_vertex_border(v))
      {
        Modifier::Halfedge_pair hp = Modifier::get_halfedges_to_borders(v);
        Point p0 = hp.first->vertex()->point();
        Point p1 = hp.second->vertex()->point();

        REQUIRE(v->point().x() * 2.0 == Approx(p0.x() + p1.x()));
        REQUIRE(v->point().y() * 2.0 == Approx(p0.y() + p1.y()));
        REQUIRE(v->point().z() * 2.0 == Approx(p0.z() + p1.z()));
      }
    }
  }
}

TEST_CASE("Check zero coefficients",
          "[Loop synthesis operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    Mesh ctrl_mesh {Utils::loadMesh(file)};
    Mesh test_mesh {Utils::loadMesh(file)};

    Mesh_ops ctrl_mesh_ops {Utils::initMeshOps()};
    Mesh_ops test_mesh_ops {Utils::initMeshOps()};

    Utils::initMeshInfo(ctrl_mesh, ctrl_mesh_ops);
    Utils::initMeshInfo(test_mesh, test_mesh_ops);

    int num_levels = 3;
    int num_types = 2;

    for (int i = 0; i < num_levels; ++i)
    {
      Utils::loop_subdivision_step(ctrl_mesh, ctrl_mesh_ops, i);
    }

    Loop_synthesis syn;
    syn.initialize(test_mesh, test_mesh_ops, num_levels);

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
      syn.lift(test_mesh, test_mesh_ops, &tmp_bands[0], &tmp_bands[2]);
    }

    REQUIRE(ctrl_mesh.size_of_vertices() ==
            test_mesh.size_of_vertices());
    for (auto [c_v, t_v] = std::make_pair(ctrl_mesh.vertices_begin(),
                                          test_mesh.vertices_begin());
         c_v != ctrl_mesh.vertices_end(); ++t_v, ++c_v)
    {
      REQUIRE(t_v != c_v);
      REQUIRE(test_mesh_ops.get_vertex_id(t_v) == ctrl_mesh_ops.get_vertex_id(c_v));;

      REQUIRE(t_v->point().x() == Approx(c_v->point().x()).margin(1e-10));
      REQUIRE(t_v->point().y() == Approx(c_v->point().y()).margin(1e-10));
      REQUIRE(t_v->point().z() == Approx(c_v->point().z()).margin(1e-10));
   }

  }
}


TEST_CASE("Check synthesis after analysis",
          "[Loop synthesis operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    std::vector<int> vsize_levels {Utils::getSubdivisionLevels(file)};
    int num_levels = vsize_levels.size() - 1;
    int num_types = 2;

    REQUIRE(num_levels > 0);

    Mesh m {Utils::loadMesh(file)};

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
    REQUIRE(aly_bands.size() == (num_types - 1) * num_levels + 2);;
    REQUIRE(aly_vertices.size() == m.size_of_vertices());;

    // Loop operator
    Loop_analysis analysis;
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

    REQUIRE(m.size_of_vertices() == vsize_levels[0]);;

    Mesh_ops syn_m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, syn_m_ops);

    Loop_synthesis synthesis;
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
      REQUIRE(m.size_of_vertices() == vsize_levels[level + 1]);;
      syn_tmp_bands[0] = syn_bands[0];
      syn_tmp_bands[1] = syn_bands[level + 1];
      syn_tmp_bands[2] = syn_bands[level + 2];

      // Load coefs
      const std::vector<Point>& band_coef = coefs[level];
      REQUIRE(band_coef.size() == syn_tmp_bands[2] - syn_tmp_bands[1]);;
      for (Vertex_handle* p = syn_tmp_bands[1]; p != syn_tmp_bands[2]; ++p)
      {
        Vertex_handle v = *p;
        int idx = p - syn_tmp_bands[1];
        v->point() = band_coef[idx];
      }
      // Lifting
      synthesis.lift(m, syn_m_ops, &syn_tmp_bands[0], &syn_tmp_bands[2]);
    }

    REQUIRE(m.size_of_vertices() == vsize_levels[num_levels]);;
    // Check match
    for (auto [v, idx] = std::make_pair(m.vertices_begin(), 0);
         v != m.vertices_end(); ++v, ++idx)
    {
      int syn_vid = syn_m_ops.get_vertex_id(v);
      CAPTURE(syn_vid);
      REQUIRE(syn_vid == idx);

      const Point& op = origin_points[idx];
      REQUIRE(op.x() == Approx(v->point().x()).margin(1e-10));
      REQUIRE(op.y() == Approx(v->point().y()).margin(1e-10));
      REQUIRE(op.z() == Approx(v->point().z()).margin(1e-10)); 
    }
  }
}

TEST_CASE("Check analysis after synthesis",
          "[Loop synthesis operations]")
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

    std::vector<std::vector<Point>> coefs;
    coefs.resize((num_types - 1) * num_levels);

    Mesh_ops syn_m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, syn_m_ops);

    Loop_synthesis synthesis;
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

    std::vector<Vertex_handle*> syn_tmp_bands;
    syn_tmp_bands.resize(3);
    // Do synthesis
    for (int level = 0; level < num_levels; ++level)
    {
      int tmp_vsize = m.size_of_vertices();
      Modifier::refine(m, syn_m_ops, level, syn_vertices, syn_bands);
      REQUIRE(m.size_of_vertices() == syn_vertices.size());;
      syn_tmp_bands[0] = syn_bands[0];
      syn_tmp_bands[1] = syn_bands[level + 1];
      syn_tmp_bands[2] = syn_bands[level + 2];

      // Load zero coefs
      std::vector<Point>& band_coef = coefs[level];
      band_coef.resize(syn_tmp_bands[2] - syn_tmp_bands[1], CGAL::ORIGIN);
      for (Vertex_handle* p = syn_tmp_bands[1]; p != syn_tmp_bands[2]; ++p)
      {
        Vertex_handle v = *p;
        int idx = p - syn_tmp_bands[1];
        v->point() = band_coef[idx];
      }
      // Lifting
      synthesis.lift(m, syn_m_ops, &syn_tmp_bands[0], &syn_tmp_bands[2]);
    }

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
    REQUIRE(aly_bands.size() == (num_types - 1) * num_levels + 2);;
    REQUIRE(aly_vertices.size() == m.size_of_vertices());;

    // Loop operator
    Loop_analysis analysis;
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
      REQUIRE(band_coef.size() == (aly_tmp_bands[2] - aly_tmp_bands[1]));;
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
      REQUIRE(op.x() == Approx(v->point().x()));
      REQUIRE(op.y() == Approx(v->point().y()));
      REQUIRE(op.z() == Approx(v->point().z()));
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
