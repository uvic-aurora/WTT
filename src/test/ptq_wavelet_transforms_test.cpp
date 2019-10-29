#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <wtlib/butterfly_wavelet_transform.hpp>
#include <wtlib/loop_wavelet_transform.hpp>
#include <wtlib/ptq_impl/subdivision_modifier.hpp>

#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/Simple_cartesian.h>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

using Point = typename Mesh::Traits::Point_3;

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
using Get_mesh_size = std::function<int(Mesh&, int)>;

TEST_CASE("Check loop analyze expect death",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "invalid_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs;
#if defined (DEBUG)
    ASSERT_DEATH(wtlib::loop_analyze(m, coefs, 1));
#endif
  }
}

TEST_CASE("Check loop analyze expect false",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "disconnected_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs;
    REQUIRE_FALSE(wtlib::loop_analyze(m, coefs, 1));
  }
}

TEST_CASE("Check butterfly analyze expect false",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    if (!m.is_closed())
    {
      continue;
    }

    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs;
    REQUIRE_FALSE(wtlib::butterfly_analyze(m, coefs, 1));
  }
}

TEST_CASE("Check loop analyze interface",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    std::vector<int> vsize_levels {Utils::getSubdivisionLevels(file)};
    int num_levels = vsize_levels.size() - 1;

    Mesh m0 {Utils::loadMesh(file)};
    Mesh m1 {m0};

    Mesh_ops m1_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m1, m1_ops);
    
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs0;
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs1;

    REQUIRE(wtlib::loop_analyze(m0, coefs0, num_levels));
    REQUIRE(wtlib::loop_analyze(m1, m1_ops, coefs1, num_levels));

    REQUIRE(m0.size_of_vertices() == m1.size_of_vertices());
    REQUIRE(m0.size_of_facets() == m1.size_of_facets());
    REQUIRE(m0.size_of_halfedges() == m1.size_of_halfedges());
    REQUIRE(coefs0.size() == coefs1.size());

    for (auto [v0, v1] = std::make_pair(m0.vertices_begin(), m1.vertices_begin());
         v0 != m0.vertices_end(); ++v0, ++v1)
    {
      const Point& p0 = v0->point();
      const Point& p1 = v1->point();

      REQUIRE(p0.x() == Approx(p1.x())); 
      REQUIRE(p0.y() == Approx(p1.y())); 
      REQUIRE(p0.z() == Approx(p1.z())); 
    }

    for (int i = 0; i < coefs0.size(); ++i)
    {
      REQUIRE(coefs0[i].size() == coefs1[i].size());
      for (int j = 0; j < coefs0[i].size(); ++j)
      {
        REQUIRE(coefs0[i][j].x() == Approx(coefs1[i][j].x())); 
        REQUIRE(coefs0[i][j].y() == Approx(coefs1[i][j].y())); 
        REQUIRE(coefs0[i][j].z() == Approx(coefs1[i][j].z())); 
      }
    }
  }
}


TEST_CASE("Check loop synthesize interface",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    int num_levels = 3;

    Mesh m0 {Utils::loadMesh(file)};
    Mesh m1 {m0};

    int base_vsize = m0.size_of_vertices();

    Mesh_ops m1_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m1, m1_ops);
    
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs0(num_levels);
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs1(num_levels);

    Get_mesh_size get_mesh_size = &wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, int>::ptq_mesh_size;

    for (int i = 0; i < num_levels; ++i)
    {
      int vsize = get_mesh_size(m0, i + 1) - get_mesh_size(m0, i);
      coefs0[i].resize(vsize, Mesh::Traits::Vector_3 {0.0, 0.0, 0.0});
      coefs1[i].resize(vsize, Mesh::Traits::Vector_3 {0.0, 0.0, 0.0});
    }

    wtlib::loop_synthesize(m0, coefs0, num_levels);
    wtlib::loop_synthesize(m1, m1_ops, coefs1, num_levels);

    REQUIRE(m0.size_of_vertices() == m1.size_of_vertices());
    REQUIRE(m0.size_of_facets() == m1.size_of_facets());
    REQUIRE(m0.size_of_halfedges() == m1.size_of_halfedges());
    REQUIRE((coefs0.empty() && coefs1.empty()));

    for (auto [v0, v1] = std::make_pair(m0.vertices_begin(), m1.vertices_begin());
         v0 != m0.vertices_end(); ++v0, ++v1)
    {
      const Point& p0 = v0->point();
      const Point& p1 = v1->point();

      REQUIRE(p0.x() == Approx(p1.x())); 
      REQUIRE(p0.y() == Approx(p1.y())); 
      REQUIRE(p0.z() == Approx(p1.z())); 
    }
  }
}


TEST_CASE("Check butterfly analyze interface",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    std::vector<int> vsize_levels {Utils::getSubdivisionLevels(file)};
    int num_levels = vsize_levels.size() - 1;

    Mesh m0 {Utils::loadMesh(file)};

    if (!m0.is_closed())
    {
      continue;
    }
    Mesh m1 {m0};

    Mesh_ops m1_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m1, m1_ops);
    
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs0;
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs1;

    wtlib::butterfly_analyze(m0, coefs0, num_levels);
    wtlib::butterfly_analyze(m1, m1_ops, coefs1, num_levels);

    REQUIRE(m0.size_of_vertices() == m1.size_of_vertices());
    REQUIRE(m0.size_of_facets() == m1.size_of_facets());
    REQUIRE(m0.size_of_halfedges() == m1.size_of_halfedges());
    REQUIRE(coefs0.size() == coefs1.size());

    for (auto [v0, v1] = std::make_pair(m0.vertices_begin(), m1.vertices_begin());
         v0 != m0.vertices_end(); ++v0, ++v1)
    {
      const Point& p0 = v0->point();
      const Point& p1 = v1->point();

      REQUIRE(p0.x() == Approx(p1.x()));
      REQUIRE(p0.y() == Approx(p1.y()));
      REQUIRE(p0.z() == Approx(p1.z()));
    }

    for (int i = 0; i < coefs0.size(); ++i)
    {
      REQUIRE(coefs0[i].size() == coefs1[i].size());
      for (int j = 0; j < coefs0[i].size(); ++j)
      {
        REQUIRE(coefs0[i][j].x() == Approx(coefs1[i][j].x())); 
        REQUIRE(coefs0[i][j].y() == Approx(coefs1[i][j].y())); 
        REQUIRE(coefs0[i][j].z() == Approx(coefs1[i][j].z())); 
      }
    }
  }
}


TEST_CASE("Check butterfly synthesize interface",
          "[PTQ wavelet transform]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());

  for (const std::string& file : files)
  {
    int num_levels = 3;

    Mesh m0 {Utils::loadMesh(file)};

    if (!m0.is_closed())
    {
      continue;
    }
    Mesh m1 {m0};

    int base_vsize = m0.size_of_vertices();

    Mesh_ops m1_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m1, m1_ops);
    
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs0(num_levels);
    std::vector<std::vector<Mesh::Traits::Vector_3>> coefs1(num_levels);

    Get_mesh_size get_mesh_size = &wtlib::ptq_impl::PTQ_subdivision_modifier<Mesh, int>::ptq_mesh_size;

    for (int i = 0; i < num_levels; ++i)
    {
      int vsize = get_mesh_size(m0, i + 1) - get_mesh_size(m0, i);
      coefs0[i].resize(vsize, Mesh::Traits::Vector_3 {0.0, 0.0, 0.0});
      coefs1[i].resize(vsize, Mesh::Traits::Vector_3 {0.0, 0.0, 0.0});
    }

    wtlib::butterfly_synthesize(m0, coefs0, num_levels);
    wtlib::butterfly_synthesize(m1, m1_ops, coefs1, num_levels);

    REQUIRE(m0.size_of_vertices() == m1.size_of_vertices());
    REQUIRE(m0.size_of_facets() == m1.size_of_facets());
    REQUIRE(m0.size_of_halfedges() == m1.size_of_halfedges());
    REQUIRE((coefs0.empty() && coefs1.empty()));

    for (auto [v0, v1] = std::make_pair(m0.vertices_begin(), m1.vertices_begin());
         v0 != m0.vertices_end(); ++v0, ++v1)
    {
      const Point& p0 = v0->point();
      const Point& p1 = v1->point();

      REQUIRE(p0.x() == Approx(p1.x())); 
      REQUIRE(p0.y() == Approx(p1.y())); 
      REQUIRE(p0.z() == Approx(p1.z())); 
    }
  }
}
