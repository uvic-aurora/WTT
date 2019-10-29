#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <wtlib/ptq_impl/subdivision_modifier.hpp>
#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/ptq_impl/mesh_vertex_info.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif


using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Vertex_handle = typename Mesh::Vertex_handle;
using Halfedge_handle = typename Mesh::Halfedge_handle;
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


TEST_CASE("Check get_halfedges_to_old and get_halfedges_to_borders",
          "[PTQ subdivision modifier]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    std::vector<int> levels {Utils::getSubdivisionLevels(file)};
    int max_level = levels.size() - 1;
    CAPTURE(levels.size());
    REQUIRE(levels.size() > 1);
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    Utils::initMeshInfo(m, m_ops);

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    CAPTURE(max_level);
    REQUIRE(Classify::classify(m, m_ops, max_level, vertices, bands, true));

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      int v_l = m_ops.get_vertex_level(v);
      if (v_l == max_level)
      {
          Modifier::Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(v, m_ops)};

          if (!m_ops.get_vertex_border(v))
          {
            INFO("  Vertex on border");
            int tmp_vid = m_ops.get_vertex_id(v);
            int tmp_void0 = m_ops.get_vertex_id(hps.first->vertex());
            int tmp_void1 = m_ops.get_vertex_id(hps.second->vertex());
            INFO("  vid: " << tmp_vid);
            INFO("  old0: " << tmp_void0);
            INFO("  old1: " << tmp_void1);
            REQUIRE_FALSE((hps.first->is_border() || hps.second->is_border()));
          }
          int old0_level = m_ops.get_vertex_level(hps.first->vertex());
          int old1_level = m_ops.get_vertex_level(hps.second->vertex());
          CAPTURE(old0_level);
          CAPTURE(old1_level);
          REQUIRE(old0_level < v_l);
          REQUIRE(old1_level < v_l);
          
          // If mesh only has one level, then the retrieved old neighbor should
          // all be old vertices
          if (max_level == 1)
          {
            REQUIRE(m_ops.get_vertex_type(hps.first->vertex()) == Classify::OLD_VERTEX);
            REQUIRE(m_ops.get_vertex_type(hps.first->vertex()) == Classify::OLD_VERTEX);
          }
      }

      if (m_ops.get_vertex_border(v))
      {
        Modifier::Halfedge_pair hps {Modifier::get_halfedges_to_borders(v)};
        REQUIRE(hps.first->is_border() != hps.second->is_border());
        // Two neighbors should also be border vertices
        REQUIRE(m_ops.get_vertex_border(hps.first->vertex()));
        REQUIRE(m_ops.get_vertex_border(hps.second->vertex()));
      }
    }
  }
  SUCCEED("[OK] Get vertex out halfedges");
}


TEST_CASE("Check coarsen", "[PTQ subdivision modifier]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "randomized_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, m_ops);

    std::vector<int> levels {Utils::getSubdivisionLevels(file)};
    int num_levels = levels.size() - 1;

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    REQUIRE(Classify::classify(m, m_ops, num_levels, vertices, bands, true));

    for (int level = num_levels; level > 0; --level)
    {
      Modifier::coarsen(m, m_ops, level);

      for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
      {
        int id = m_ops.get_vertex_id(v);
        REQUIRE(id <= m.size_of_vertices());
        REQUIRE(v == vertices[id]);
      }
    }
  }
}


TEST_CASE("Check bands after refine", "[PTQ subdivision modifier]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    int num_levels = 3;
    int finest_vsize = Modifier::get_mesh_size(m, m_ops, num_levels);

    Utils::initMeshInfo(m, m_ops);
    std::vector<Vertex_handle> vertices;
    vertices.reserve(finest_vsize);
    std::vector<Vertex_handle*> bands;
    bands.reserve(num_levels + 2);

    for (auto v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      vertices.push_back(v);
    }
    bands.push_back(&vertices.front());
    bands.push_back(&vertices.back() + 1);
    REQUIRE(bands.size() == 2);;

    for (int level = 0; level < num_levels; ++level)
    {
      int current_vsize = m.size_of_vertices();
      int expect_vsize = Modifier::get_mesh_size(m, m_ops, 1);
      Modifier::refine(m, m_ops, level, vertices, bands);

      INFO("  level: " << level + 1);
      INFO("  expect size: " << expect_vsize);
      INFO("  actual size: " << m.size_of_vertices());
      REQUIRE(m.size_of_vertices() == expect_vsize);
      int vsize_increments = m.size_of_vertices() - current_vsize;

      INFO("  band difference: " << bands[bands.size() - 1] - bands[bands.size() - 2]);
      INFO("  vertices size increment: " << vsize_increments);
      REQUIRE(bands.size() == level + 3);
      REQUIRE(bands[bands.size() - 1] - bands[bands.size() - 2] == vsize_increments);
    }
  }
}

TEST_CASE("Check refine", "[PTQ subdivision modifier]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    Mesh m {Utils::loadMesh(file)};
    m.normalize_border();
    Mesh_ops m_ops {Utils::initMeshOps()};

    Utils::initMeshInfo(m, m_ops);

    std::vector<int> levels {Utils::getSubdivisionLevels(file)};
    int num_levels = levels.size() - 1;

    int facets_size = m.size_of_facets();
    int vertices_size = m.size_of_vertices();
    int edges_size = m.size_of_halfedges() / 2.0;
    int border_edges_size = m.size_of_border_edges();

    std::vector<Vertex_handle> origin_vertices;
    origin_vertices.reserve(m.size_of_vertices());
    std::vector<Vertex_handle*> origin_bands;
    origin_bands.reserve(num_levels + 1);

    REQUIRE(Classify::classify(m, m_ops, num_levels, origin_vertices, origin_bands, true));

    // Using 3 vertex ids to denote a triangle and a vector of this kind of
    // triangle to denote the mesh topology.
    std::vector<std::vector<int>> origin;

    for (auto f = m.facets_begin(); f != m.facets_end(); ++f)
    {
      Halfedge_handle h = f->halfedge();
      // Vertices ids in this face.
      std::vector<int> vids;

      do
      {
        vids.push_back(m_ops.get_vertex_id(h->vertex()));
        h = h->next();
      }
      while (h != f->halfedge());

      std::sort(vids.begin(), vids.end());
      origin.push_back(vids);
    }

    // Sort all the faces by their covered vertices id.
    std::sort(origin.begin(), origin.end());
    REQUIRE(origin.size() == facets_size);;

    // First coarsen mesh
    for (int l = num_levels; l > 0; --l)
    {
      Modifier::coarsen(m, m_ops, l);
    }

    // PTQ primitives quantities check
    int base_vertices_size = m.size_of_vertices();
    int base_facets_size = m.size_of_facets();
    int base_edges_size = m.size_of_halfedges() / 2;
    int pow2k = std::pow(2, num_levels);

    REQUIRE(vertices_size == 0.5 * (pow2k - 1) * (pow2k - 2) * base_facets_size 
                             + (pow2k - 1) * base_edges_size
                             + base_vertices_size);
    REQUIRE(facets_size == std::pow(4, num_levels) * base_facets_size);;
    REQUIRE(edges_size == 3 * std::pow(2, num_levels - 1) * (pow2k -1) * base_facets_size
                          + pow2k * base_edges_size);

    std::vector<Vertex_handle> vertices;
    vertices.reserve(vertices_size);
    std::vector<Vertex_handle*> bands;
    bands.reserve(num_levels + 2);

    vertices.resize(m.size_of_vertices());

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      vertices[m_ops.get_vertex_id(v)] = v;
    }

    bands.push_back(&vertices.front());
    bands.push_back(&vertices.back() + 1);

    REQUIRE(m.size_of_vertices() == bands[1] - vertices.data());;
    // Refine mesh back
    for (int l = 0; l < num_levels; ++l)
    {
      Modifier::refine(m, m_ops, l, vertices, bands);
    }

    m.normalize_border();

    REQUIRE(m.size_of_vertices() == vertices_size);;
    REQUIRE(m.size_of_facets() == facets_size);;
    REQUIRE(m.size_of_border_halfedges() == border_edges_size);;
    REQUIRE(m.size_of_halfedges() == edges_size * 2);;

    REQUIRE(vertices.size() == vertices_size);
    REQUIRE(bands.size() == levels.size() + 1);

    for (const Vertex_handle& v : vertices)
    {
      int id = &v - vertices.data();
      REQUIRE(id == m_ops.get_vertex_id(v));
    }

    for (int i = 1; i < bands.size(); ++i)
    {
      REQUIRE(bands[i] - vertices.data() == levels[i - 1]);

      for (auto p = bands[i - 1]; p != bands[i]; ++p)
      {
        Vertex_handle v = *p;
        REQUIRE(m_ops.get_vertex_level(v) == i - 1);

        if (i == 1)
        {
          REQUIRE(m_ops.get_vertex_type(v) == 0);;
        }
        else
        {
          REQUIRE(m_ops.get_vertex_type(v) == 1);;
          REQUIRE(v->point() == CGAL::ORIGIN);
        }
      }
    }

    std::vector<std::vector<int>> after;

    for (auto f = m.facets_begin(); f != m.facets_end(); ++f)
    {
      Halfedge_handle h = f->halfedge();
      // Vertices ids in this face.
      std::vector<int> vids;

      do
      {
        vids.push_back(m_ops.get_vertex_id(h->vertex()));
        h = h->next();
      }
      while (h != f->halfedge());

      std::sort(vids.begin(), vids.end());
      after.emplace_back(vids);
    }

    // Sort all the faces.
    std::sort(after.begin(), after.end());
    REQUIRE(after.size() == facets_size);;

    // After refinement, faces should equal to before.
    REQUIRE(origin == after);
  }
}
