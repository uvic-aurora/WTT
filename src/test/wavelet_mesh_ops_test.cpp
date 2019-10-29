#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include <test_utils.hpp> 

#include <wtlib/wavelet_mesh_operations.hpp>
#include <wtlib/ptq_impl/mesh_vertex_info.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#include <boost/filesystem.hpp>

#include <fstream>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

namespace fs = boost::filesystem;


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

TEST_CASE("Check set/get vertex_id",
          "[Wavelet mesh operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    Mesh_ops m_ops {Utils::initMeshOps()};

#if defined (UES_UNORDERED_MAP)
    std::unordered_map<Vertex_handle, int> vids;
#else
    std::map<Vertex_handle, int> vids;
#endif

    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      m_ops.set_vertex_id(v, id);
      vids[v] = id;
    }

    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      REQUIRE(m_ops.get_vertex_id(v) == vids.at(v));
    }
  }
}

TEST_CASE("Check set_get_vertex_level",
          "[Wavelet mesh operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

#if defined (UES_HASHTABLE)
    std::unordered_map<Vertex_handle, int> v_levels;
#else
    std::map<Vertex_handle, int> v_levels;
#endif

    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      m_ops.set_vertex_id(v, id);
      v_levels[v] = id;
    }

    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      REQUIRE(m_ops.get_vertex_id(v) == v_levels.at(v));
    }
  }
}

TEST_CASE("Check set_get_vertex_type",
          "[Wavelet mesh operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    Mesh_ops m_ops {Utils::initMeshOps()};

#if defined (UES_HASHTABLE)
    std::unordered_map<Vertex_handle, int> v_types;
#else
    std::map<Vertex_handle, int> v_types;
#endif

    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      m_ops.set_vertex_id(v, id);
      v_types[v] = id;
    }

    for (auto [v, id] = std::make_pair(m.vertices_begin(), int(0));
         v != m.vertices_end();
         ++v, ++id)
    {
      REQUIRE(m_ops.get_vertex_id(v) == v_types.at(v));
    }
  }
}

TEST_CASE("Check set_get_vertex_border",
          "[Wavelet mesh operations]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    Mesh_ops m_ops {Utils::initMeshOps()};

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      m_ops.set_vertex_border(v, false);
    }

    if (m.normalized_border_is_valid())
    {
      for (auto h = m.border_halfedges_begin(); h != m.halfedges_end(); ++h)
      {
        m_ops.set_vertex_border(h->vertex(), true);
        m_ops.set_vertex_border(h->opposite()->vertex(), true);
      }
    }
    else
    {
      for (auto h = m.halfedges_begin(); h != m.halfedges_end(); ++h)
      {
        if (h->is_border())
        {
          m_ops.set_vertex_border(h->vertex(), true);
          m_ops.set_vertex_border(h->opposite()->vertex(), true);          
        }
      }
    }

    for (Halfedge_handle h = m.halfedges_begin(); h != m.halfedges_end(); ++h)
    {
      if (h->is_border())
      {
        REQUIRE(m_ops.get_vertex_border(h->vertex()));
        REQUIRE(m_ops.get_vertex_border(h->opposite()->vertex()));
      }
    }
  }
}
