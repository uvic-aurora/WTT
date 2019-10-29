#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <test_utils.hpp>

#include <wtlib/ptq_impl/vertex_classification.hpp>
#include <wtlib/wavelet_mesh_operations.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>

#ifndef TEST_DATA_DIR
#define TEST_DATA_DIR "."
#endif

using Vertex_const_handle = typename Mesh::Vertex_const_handle;
using Vertex_handle = typename Mesh::Vertex_handle;
using Point = typename Mesh::Traits::Point_3;

using Get_vertex_id = std::function<int(Vertex_const_handle)>;
using Set_vertex_id = std::function<void(Vertex_handle, int)>;
using Get_vertex_level = std::function<int(Vertex_const_handle)>;
using Set_vertex_level = std::function<void(Vertex_handle, int)>;
using Get_vertex_type = std::function<int(Vertex_const_handle)>;
using Set_vertex_type = std::function<void(Vertex_handle, int)>;
using Get_vertex_border = std::function<bool(Vertex_const_handle)>;
using Set_vertex_border = std::function<void(Vertex_handle, bool)>;

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

using Mesh_info = wtlib::ptq_impl::Mesh_info<Mesh>;
using Utils = Wtlib_test_helper<Mesh, Mesh_ops>;
using Classify = wtlib::ptq_impl::PTQ_classify_vertices<Mesh, Mesh_ops>;

#if defined (WTLIB_USE_UNORDERED_MAP)
using Vertex_tracker = std::unordered_map<Vertex_handle, std::pair<Vertex_handle, Vertex_handle>>;
#else
using Vertex_tracker = std::map<Vertex_handle, std::pair<Vertex_handle, Vertex_handle>>;
#endif

void subdivisionSetId(Mesh& m, const Mesh_ops& m_ops, Vertex_tracker& v_tracker)
{
  using Halfedge_handle = typename Mesh::Halfedge_handle;
  using Edge_iterator = typename Mesh::Edge_iterator;
  using Facet_handle = typename Mesh::Facet_handle;

  int edge_size = m.size_of_halfedges() / 2;
  int vertices_size = m.size_of_vertices();
  int facet_size = m.size_of_facets();

  auto edge_compair = [&m_ops](const Edge_iterator& lhs, const Edge_iterator& rhs) -> bool
                      {
                        int s_lhs = m_ops.get_vertex_id(lhs->vertex());
                        int l_lhs = m_ops.get_vertex_id(lhs->opposite()->vertex());
                        if (s_lhs > l_lhs)
                        {
                          std::swap(s_lhs, l_lhs);
                        }

                        int s_rhs = m_ops.get_vertex_id(rhs->vertex());
                        int l_rhs = m_ops.get_vertex_id(rhs->opposite()->vertex());
                        if (s_rhs > l_rhs)
                        {
                          std::swap(s_rhs, l_rhs);
                        }

                        return s_lhs == s_rhs ? l_lhs < l_rhs : s_lhs < s_rhs;
                      };
  std::set<Edge_iterator, decltype(edge_compair)> edges_set(edge_compair);

  for (Edge_iterator e = m.edges_begin(); e != m.edges_end(); ++e)
  {
    REQUIRE(edges_set.find(e) == edges_set.end());
    edges_set.insert(e);
  }

  assert(edges_set.size() == edge_size);

  // Insert a new vertex on each edge, and assign id starting from base mesh vertices size.
  for (auto [eitr, idx] = std::make_pair(edges_set.begin(), vertices_size); eitr != edges_set.end(); ++eitr, ++idx)
  {
    Halfedge_handle h = (*eitr);
    Halfedge_handle hoppo = h->opposite();
    Vertex_handle p0 = h->vertex();
    Vertex_handle p1 = h->opposite()->vertex();
    Halfedge_handle hnew = m.split_vertex(hoppo->prev(), h);

    if (!h->is_border())
    {
      h->facet()->set_halfedge(hnew);
    }
    if (!hoppo->is_border())
    {
      hoppo->facet()->set_halfedge(hoppo);
    }

    Vertex_handle ve = h->vertex();
    v_tracker.insert({ve, {p0, p1}});
    m_ops.set_vertex_id(ve, idx);
  }

  assert((edge_size + vertices_size) == m.size_of_vertices());

  for (auto [f, idx] = std::make_pair(m.facets_begin(), 0); idx < facet_size; ++f, ++idx)
  {
    REQUIRE(f->facet_degree() == 6);

    Halfedge_handle h = f->facet_begin();

    Halfedge_handle e0 = h->next();
    Halfedge_handle et = e0;
    Halfedge_handle e1 = e0->next()->next();
    Halfedge_handle e2 = e1->next()->next();

    REQUIRE(h == e2->next());

    et = m.split_facet(et, e1);
    et = m.split_facet(et, e2);
    et = m.split_facet(et, e0);

    REQUIRE(et->opposite()->next() == h);
  }
}

TEST_CASE("Classify mesh with zero levels", "[PTQ classify vertices]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");

  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    Utils::initMeshInfo(m, m_ops);

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    INFO("Classify " << file << " with zero level should return false");
    REQUIRE_FALSE(Classify::classify(m, m_ops, 0, vertices, bands, true));
  }
}


TEST_CASE("Classify meshes without subdivision connectivity",
          "[PTQ classify vertices]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");

  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    Utils::initMeshInfo(m, m_ops);

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    INFO(file << " should not have subdivision connectivity");
    REQUIRE_FALSE(Classify::classify(m, m_ops, 1, vertices, bands, true));
  }
}


TEST_CASE("Classify meshes with subdivision levels more than the given level",
          "[PTQ classify vertices]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  for (const std::string& file : files)
  {
    INFO("Processing [" << file << "]");
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    std::vector<int> vsize_in_levels {Utils::getSubdivisionLevels(file)};

    INFO("Subdivision level from file name: " << vsize_in_levels.size());
    REQUIRE(vsize_in_levels.size() > 0);

    int num_of_levels = vsize_in_levels.size() - 1;

    if (num_of_levels < 2)
    {
      continue;
    }

    Utils::initMeshInfo(m, m_ops);
    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;
    INFO("Classify with level: " << num_of_levels - 1);
    REQUIRE(Classify::classify(m, m_ops, num_of_levels - 1, vertices, bands, true));

    for (auto [v, idx] = std::make_pair(m.vertices_begin(), std::size_t(0)); v != m.vertices_end(); ++v, ++idx)
    {
      INFO("v id(" << m_ops.get_vertex_id(v) << ")  idx(" << idx << ")");
      // vertex's order in each band means the level they should have.
      for (int i = 0; i < vsize_in_levels.size(); ++i)
      {
        // Check their belonging band
        if (idx < vsize_in_levels[i])
        {
          int cl = (i - 1) >= 0 ? i - 1 : 0;
          int v_level = m_ops.get_vertex_level(v);
          int v_type = m_ops.get_vertex_type(v);

          INFO("    level: " << v_level << "; type: " << v_type);
          REQUIRE(v_level == cl);
          if (cl == 0)
          {
            REQUIRE(v_type == Classify::OLD_VERTEX);
          }
          else
          {
            REQUIRE(v_type == Classify::EDGE_VERTEX);
          }
          break;
        }
      }
    }

    // Check band size is equal to known value
    INFO("Band size: " << bands.size() << " ; vsize_in_levels size: "
         << vsize_in_levels.size());
    REQUIRE(bands.size() == vsize_in_levels.size());
    for (int i = 1; i < bands.size(); ++i)
    {
      if (i == 1)
      {
        INFO("  size of vertices in band 0-1: " << bands[1] - bands[0]);
        INFO("  size of vertices in vsize_in_levels[1]: " << vsize_in_levels[1]);
        REQUIRE(bands[1] - bands[0] == vsize_in_levels[1]);
      }
      else
      {
        INFO("  bands[" << i << "] - bands[" << i - 1 << "]: "
              << bands[i] - bands[i - 1]);
        INFO("  vsize_in_levels[" << i << "] - vsize_in_levels[" << i - 1 << "]"
              << vsize_in_levels[i] - vsize_in_levels[i - 1]);
        REQUIRE(bands[i] - bands[i - 1] == vsize_in_levels[i] - vsize_in_levels[i - 1]);
      }
    }

    for (int i = 1; i < bands.size(); ++i)
    {
      for (auto p = bands[i - 1]; p != bands[i]; ++p)
      {
        int vertex_level = m_ops.get_vertex_level(*p);
        int vertex_type = m_ops.get_vertex_type(*p);
        INFO("Bands: " << i - 1 << " ~ " << i);
        INFO("  v level: " << vertex_level);
        REQUIRE(i - 1 == vertex_level);
        if (i == 1)
        {
          INFO("v type is " << vertex_type);
          REQUIRE(Classify::OLD_VERTEX == vertex_type);
        }
        else
        {
          INFO("v type is " << vertex_type);
          REQUIRE(Classify::EDGE_VERTEX == vertex_type);
        }
      }
    }
  }
}


TEST_CASE("Classify meshes with subdivision levels less than the given level",
          "[PTQ classify vertices]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  for (const std::string& file : files)
  {
    INFO("Processing [" << file << "]");
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    std::vector<int> vsize_in_levels {Utils::getSubdivisionLevels(file)};

    INFO("Subdivision level from file name: " << vsize_in_levels.size());
    REQUIRE(vsize_in_levels.size() > 0);

    Utils::initMeshInfo(m, m_ops);
    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;
    INFO("Classify with level: " << vsize_in_levels.size());
    REQUIRE_FALSE(Classify::classify(m, m_ops, vsize_in_levels.size(), vertices, bands, true));
  }
}


TEST_CASE("Classify meshes with subdivision levels equal to the given level.",
          "[PTQ classify vertices]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  for (const std::string& file : files)
  {
    INFO("Processing [" << file << "]");
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};

    std::vector<int> vsize_in_levels {Utils::getSubdivisionLevels(file)};

    INFO("Subdivision level from file name: " << vsize_in_levels.size());
    REQUIRE(vsize_in_levels.size() > 0);

    // Ignore meshes without subdivision connectivity
    if (vsize_in_levels.size() < 2)
    {
      continue;
    }

    int num_levels = vsize_in_levels.size() - 1;

    Utils::initMeshInfo(m, m_ops);
    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    INFO("Classify with level: " << num_levels);
    REQUIRE(Classify::classify(m, m_ops, num_levels, vertices, bands, true));

    // First iterate over vertices in mesh.
    for (auto [v, idx] = std::make_pair(m.vertices_begin(), std::size_t(0)); v != m.vertices_end(); ++v, ++idx)
    {
      INFO("v id(" << m_ops.get_vertex_id(v) << ")  idx(" << idx << ")");
      // vertex's order in each band means the level they should have.
      for (int i = 0; i < vsize_in_levels.size(); ++i)
      {
        int v_level = m_ops.get_vertex_level(v);
        int v_type = m_ops.get_vertex_type(v);
        INFO("    level: " << v_level << "; type: " << v_type);
        INFO("    i = " << i);
        // Check their belonging band
        if (idx < vsize_in_levels[i])
        {
          REQUIRE(v_level == i);
          if (i == 0)
          {
            REQUIRE(v_type == Classify::OLD_VERTEX);
          }
          else
          {
            REQUIRE(v_type == Classify::EDGE_VERTEX);
          }
          break;
        }
      }
    }

    // Check band size is equal to known value
    INFO("Band size - 1: " << bands.size() - 1 << " ; vsize_in_levels size: "
         << vsize_in_levels.size());
    REQUIRE(bands.size() - 1 == vsize_in_levels.size());
    REQUIRE(bands[0] == &vertices.front());

    // Check vertices size between bands
    for (int i = 1; i < bands.size(); ++i)
    {
      INFO("  size of vertices in band [" << i << "]-[0]: " << bands[i] - bands[0]);
      INFO("  size of vertices in vsize_in_levels[" << i - 1 << "]: " 
            << vsize_in_levels[i - 1]);
      REQUIRE(bands[i] - bands[0] == vsize_in_levels[i - 1]);
    }

    // Check bands level, type property.
    for (int i = 1; i < bands.size(); ++i)
    {
      INFO("Bands: " << i - 1 << " ~ " << i);
      for (auto p = bands[i - 1]; p != bands[i]; ++p)
      {
        int v_level = m_ops.get_vertex_level(*p);
        int v_type = m_ops.get_vertex_type(*p);
        // Check vertices level
        INFO("  v level: " << v_level << "; type: " << v_type);
        REQUIRE(i - 1 == v_level);
        if (i == 1)
        {
          // Check vertices type
          REQUIRE(Classify::OLD_VERTEX == v_type);
        }
        else
        {
          REQUIRE(Classify::EDGE_VERTEX == v_type);
        }
      }
    }
  }
}


TEST_CASE("Check the bands properties with input mesh with a random vertices layout",
          "[PTQ_classify_vertices]")
{
  std::vector<std::string> files;

  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "randomized_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, m_ops);

    std::vector<int> size_of_levels {Utils::getSubdivisionLevels(file)};

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    INFO("Classify with level: " << size_of_levels.size() - 1);
    REQUIRE(Classify::classify(m, m_ops, size_of_levels.size() - 1, vertices, bands, true));

    CAPTURE(bands.size());
    CAPTURE(size_of_levels.size());
    REQUIRE(bands.size() == size_of_levels.size() + 1);

    for (int i = 1; i < bands.size(); ++i)
    {
      INFO("  size of vertices in band [" << i << "]-[0]: " << bands[i] - bands[0]);
      INFO("  size of vertices in vsize_in_levels[" << i - 1 << "]: " 
            << size_of_levels[i - 1]);
      REQUIRE(bands[i] - bands[0] == size_of_levels[i - 1]);
    }

    REQUIRE(bands[0] == &vertices[0]);

    for (int i = 0; i < vertices.size(); ++i)
    {
      int vid = m_ops.get_vertex_id(vertices[i]);
      INFO("  vid: " << vid << ";   i: " << i);
      REQUIRE(i == vid);
    }

    for (auto [p, i] = std::make_pair(bands[0], 0); p != bands.back(); ++i, ++p)
    {
      int vid = m_ops.get_vertex_id(*p);
      INFO("  vid: " << vid << ";   i: " << i);
      REQUIRE(i == vid);
    }

    // Check bands level, type property.
    for (int i = 1; i < bands.size(); ++i)
    {
      INFO("  Bands: " << i - 1 << " ~ " << i);
      for (auto p = bands[i - 1]; p != bands[i]; ++p)
      {
        int v_level = m_ops.get_vertex_level(*p);
        int v_type = m_ops.get_vertex_type(*p);
        INFO("    level: " << v_level << "; type: " << v_type);
        INFO("    i: " << i);
        REQUIRE(i - 1 == v_level);
        if (i == 1)
        {
          REQUIRE(Classify::OLD_VERTEX == v_type);
        }
        else
        {
          REQUIRE(Classify::EDGE_VERTEX == v_type);
        }
      }
    }
  }
}

TEST_CASE("Classify closed mesh with random vertices layout then check id",
          "[PTQ_classify_vertices]")
{
  Mesh dragon {Utils::loadMesh(std::string(TEST_DATA_DIR)
                               + "subdivided_meshes/ico_12_42_162.off")};
  Mesh rnd_dragon {Utils::loadMesh(std::string(TEST_DATA_DIR)
                               + "randomized_meshes/rnd_ico_12_42_162.off")};

  Mesh_ops m_ops {Utils::initMeshOps()};
  Mesh_ops rnd_m_ops {Utils::initMeshOps()};

  Utils::initMeshInfo(dragon, m_ops);
  Utils::initMeshInfo(rnd_dragon, rnd_m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  std::vector<Vertex_handle> rnd_vertices;
  std::vector<Vertex_handle*> rnd_bands;

  REQUIRE(Classify::classify(dragon, m_ops, 2, vertices, bands, true));
  REQUIRE(Classify::classify(rnd_dragon, rnd_m_ops, 2, rnd_vertices, rnd_bands, true));

  auto point_comp = [](const Vertex_handle& vlhs, const Vertex_handle& vrhs)
                    {
                      Point lhs = vlhs->point();
                      Point rhs = vrhs->point();
                      if (lhs.x() != rhs.x())
                      {
                        return lhs.x() < rhs.x();
                      }
                      else
                      {
                        if (lhs.y() != rhs.y())
                        {
                          return lhs.y() < rhs.y();
                        }
                        else
                        {
                          return lhs.z() < rhs.z();
                        }
                      }
                    };

  for (int i = 1; i < bands.size(); ++i)
  {
    // Sort vertices in each bands by geometry
    std::sort(bands[i - 1],
              bands[i],
              point_comp);
  }

  for (int i = 1; i < bands.size(); ++i)
  {
    // Sort vertices in each bands by geometry
    std::sort(rnd_bands[i - 1],
              rnd_bands[i],
              point_comp);
  }

  for (int i = 0; i < vertices.size(); ++i)
  {
    Point p = vertices[i]->point();
    Point rnd_p = rnd_vertices[i]->point();
    CAPTURE(p);
    CAPTURE(rnd_p);
    REQUIRE(p == rnd_p);
  }
}

TEST_CASE("Classify open mesh with random vertices layout then check id",
          "[PTQ_classify_vertices]")
{
  Mesh bunny {Utils::loadMesh(std::string(TEST_DATA_DIR)
                               + "subdivided_meshes/cut_ico_163_645.off")};
  Mesh rnd_bunny {Utils::loadMesh(std::string(TEST_DATA_DIR)
                               + "randomized_meshes/rnd_cut_ico_163_645.off")};

  Mesh_ops m_ops {Utils::initMeshOps()};
  Mesh_ops rnd_m_ops {Utils::initMeshOps()};

  Utils::initMeshInfo(bunny, m_ops);
  Utils::initMeshInfo(rnd_bunny, rnd_m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  std::vector<Vertex_handle> rnd_vertices;
  std::vector<Vertex_handle*> rnd_bands;

  REQUIRE(Classify::classify(bunny, m_ops, 1, vertices, bands, true));
  REQUIRE(Classify::classify(rnd_bunny, rnd_m_ops, 1, rnd_vertices, rnd_bands, true));

  auto point_comp = [](const Vertex_handle& vlhs, const Vertex_handle& vrhs)
                    {
                      Point lhs = vlhs->point();
                      Point rhs = vrhs->point();
                      if (lhs.x() != rhs.x())
                      {
                        return lhs.x() < rhs.x();
                      }
                      else
                      {
                        if (lhs.y() != rhs.y())
                        {
                          return lhs.y() < rhs.y();
                        }
                        else
                        {
                          return lhs.z() < rhs.z();
                        }
                      }
                    };

  for (int i = 1; i < bands.size(); ++i)
  {
    // Sort vertices in each bands by geometry
    std::sort(bands[i - 1],
              bands[i],
              point_comp);
  }

  for (int i = 1; i < bands.size(); ++i)
  {
    // Sort vertices in each bands by geometry
    std::sort(rnd_bands[i - 1],
              rnd_bands[i],
              point_comp);
  }

  for (int i = 0; i < vertices.size(); ++i)
  {
    Point p = vertices[i]->point();
    Point rnd_p = rnd_vertices[i]->point();

    CAPTURE(p);
    CAPTURE(rnd_p);
    REQUIRE(p == rnd_p);
  }
}


TEST_CASE("Check vertex id is in correct interval",
          "[PTQ_classify_vertices]")
{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    Mesh m {Utils::loadMesh(file)};
    Mesh_ops m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, m_ops);

    std::vector<int> vsize_in_levels {Utils::getSubdivisionLevels(file)};

    int num_levels = vsize_in_levels.size() - 1;
    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    REQUIRE(Classify::classify(m, m_ops, num_levels, vertices, bands, false));

    CAPTURE(bands.size());
    CAPTURE(vsize_in_levels.size());
    REQUIRE(bands.size() - 1 == vsize_in_levels.size());

    for (int i = 1; i < bands.size(); ++i)
    {
      REQUIRE(bands[i] - bands[0] == vsize_in_levels[i - 1]);
      int lower_bound = i == 1 ? 0 : vsize_in_levels[i - 2];
      int upper_bound = vsize_in_levels[i - 1];
      for (Vertex_handle * p = bands[i - 1]; p != bands[i]; ++p)
      {
        int vid = m_ops.get_vertex_id(*p);
        CAPTURE(vid);
        REQUIRE(vid >= lower_bound);
        REQUIRE(vid < upper_bound);
      }
    }
  }
}


TEST_CASE("Check vertex id property",
          "[PTQ_classify_vertices]")
{
  std::vector<std::string> files;

  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "unsubdivided_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};

    Mesh_ops m_ops_ctrl {Utils::initMeshOps()};
    Mesh_ops m_ops_exp {Utils::initMeshOps()};

    Vertex_tracker v_tracker;

    Utils::initMeshInfo(m, m_ops_ctrl);

    for (int i = 0; i < 3; ++i)
    {
      subdivisionSetId(m, m_ops_ctrl, v_tracker);
    }

    for (auto [v, id] = std::make_pair(m.vertices_begin(), 0);
         v != m.vertices_end(); ++v, ++id)
    {
      REQUIRE(m_ops_ctrl.get_vertex_id(v) == id);
    }

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    REQUIRE(Classify::classify(m, m_ops_exp, 3, vertices, bands, true));

    REQUIRE(bands[0] == &vertices.front());


    // Check bands level, type property.
    for (int i = 1; i < bands.size(); ++i)
    {
      for (auto p = bands[i - 1]; p != bands[i]; ++p)
      {
        int v_level = m_ops_exp.get_vertex_level(*p);
        int v_type = m_ops_exp.get_vertex_type(*p);
        INFO("    level: " << v_level << "; type: " << v_type);
        INFO("    i = " << i);
        REQUIRE(i - 1 == v_level);
        if (i == 1)
        {
          REQUIRE(Classify::OLD_VERTEX == v_type);
        }
        else
        {
          REQUIRE(Classify::EDGE_VERTEX == v_type);
        }
      }
    }

    for (int i = 0; i < vertices.size(); ++i)
    {
      int v_ctrl_id = m_ops_ctrl.get_vertex_id(vertices[i]);
      CAPTURE(v_ctrl_id);
      REQUIRE(i == v_ctrl_id);
    }

    for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
    {
      int v_ctrl_id = m_ops_ctrl.get_vertex_id(v);
      int v_exp_id = m_ops_exp.get_vertex_id(v);
      REQUIRE(v_ctrl_id == v_exp_id);
    }
  }
}
