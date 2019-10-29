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

using Butterfly = wtlib::ptq_impl::Butterfly_lift_operations<Mesh, Mesh_ops, true>;

using Butterfly_analysis = wtlib::ptq_impl::Butterfly_analysis_operations<Mesh, Mesh_ops>;


TEST_CASE("Check get vertex b c",
          "[Butterfly analysis operations]")

{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "randomized_meshes/butterfly_mask_8_21.off")};

  Mesh_ops m_ops {Utils::initMeshOps()};
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;
  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, true));

  Vertex_handle e = m.vertices_begin();

  REQUIRE(8 == m_ops.get_vertex_id(e));
  REQUIRE(e == *bands[1]);

  Modifier::Halfedge_pair hps {Modifier::get_halfedges_to_old_vertices(e, m_ops)};

  Halfedge_handle h0 = hps.first;
  Halfedge_handle h1 = hps.second;

  if (m_ops.get_vertex_id(h0->vertex()) > m_ops.get_vertex_id(h1->vertex()))
  {
    std::swap(h0, h1);
  }

  Vertex_handle a0 = h0->vertex();
  Vertex_handle a1 = h1->vertex();

  Vertex_handle b0 = Butterfly::get_vertex_B(h0);
  Vertex_handle b1 = Butterfly::get_vertex_B(h1);

  Vertex_handle c0 = Butterfly::get_vertex_C0(h0);
  Vertex_handle c1 = Butterfly::get_vertex_C1(h0);
  Vertex_handle c2 = Butterfly::get_vertex_C0(h1);
  Vertex_handle c3 = Butterfly::get_vertex_C1(h1);

  REQUIRE(a0 == vertices[0]);
  REQUIRE(a1 == vertices[1]);

  REQUIRE(b0 == vertices[3]);
  REQUIRE(b1 == vertices[2]);

  REQUIRE(c0 == vertices[5]);
  REQUIRE(c1 == vertices[4]);
  REQUIRE(c2 == vertices[6]);
  REQUIRE(c3 == vertices[7]);
}

TEST_CASE("Check initialize",
          "[Butterfly analysis operations]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "randomized_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    Mesh m {Utils::loadMesh(file)};
    Butterfly_analysis butterfly;

    Mesh_ops m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, m_ops);

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, false));

    if (!m.is_closed())
    {
#ifdef DEBUG
      ASSERT_DEATH({butterfly.initialize(m, m_ops);}, "");
#endif
      continue;
    }

    REQUIRE(bands.size() == 3);
    butterfly.initialize(m, m_ops);
    butterfly.init_scale(bands[0], bands[1], bands[2]);
    for (auto p = bands[1]; p != bands[2]; ++p)
    {
      REQUIRE(Classify::EDGE_VERTEX == m_ops.get_vertex_type(*p));
      REQUIRE(1.0 == butterfly.get_vertex_scale(*p));
    }

    for (auto p = bands[0]; p != bands[1]; ++p)
    {
      REQUIRE(Classify::OLD_VERTEX == m_ops.get_vertex_type(*p));
      REQUIRE(1.0 == butterfly.get_vertex_scale(*p));
    }
  }
}

TEST_CASE("Check update scale",
          "[Butterfly analysis operations]")

{
  std::vector<std::string> files;
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "subdivided_meshes/");
  Utils::loadFiles(files, std::string(TEST_DATA_DIR) + "randomized_meshes/");
  REQUIRE_FALSE(files.empty());
  for (const std::string& file : files)
  {
    INFO("Processing " << file);
    Mesh m {Utils::loadMesh(file)};
    Butterfly_analysis butterfly;

    std::vector<int> size_of_levels {Utils::getSubdivisionLevels(file)};
    int num_levels = size_of_levels.size() - 1;

    Mesh_ops m_ops {Utils::initMeshOps()};
    Utils::initMeshInfo(m, m_ops);

    std::vector<Vertex_handle> vertices;
    std::vector<Vertex_handle*> bands;

    REQUIRE(Classify::classify(m, m_ops, num_levels, vertices, bands, true));

    if (!m.is_closed())
    {
      continue;
    }

    REQUIRE(bands.size() == num_levels + 2);
    butterfly.initialize(m, m_ops);
    butterfly.init_scale(bands[0], bands[num_levels], bands[num_levels + 1]);

    for (int level = num_levels; level > 0; --level)
    {
      butterfly.update_scale(m, m_ops, bands[level], bands[level + 1]);

      for (auto p = bands[0]; p != bands[level]; ++p)
      {
        Vertex_handle o = *p;
        int n = o->degree();
        int vid = m_ops.get_vertex_id(o);
        double expect_scale = (std::pow(4.0, num_levels - level + 1) - 1) / 6.0 * n + 1.0;
        INFO("    vid: " << vid);
        INFO("    degree: " << n);
        INFO("    expect_scale: " << expect_scale);
        REQUIRE(expect_scale  == butterfly.get_vertex_scale(o));
      }

      for (auto p = bands[level]; p != bands[level + 1]; ++p)
      {
        Vertex_handle e = *p;
        Modifier::Halfedge_pair hp = Modifier::get_halfedges_to_old_vertices(e, m_ops);
        Vertex_handle a0 = hp.first->vertex();
        Vertex_handle a1 = hp.second->vertex();

        int edge_id = m_ops.get_vertex_id(e);

        double calc_ratio_a0 = butterfly.get_scale_ratio(a0, e, m_ops);
        double calc_ratio_a1 = butterfly.get_scale_ratio(a1, e, m_ops);

        double se = butterfly.get_vertex_scale(e);
        double sa0 = butterfly.get_vertex_scale(a0);
        double sa1 = butterfly.get_vertex_scale(a1);

        INFO("   edge id: " << edge_id);
        INFO("   scale at e: " << se);
        INFO("   scale at a0: " << sa0);
        INFO("   scale at a1: " << sa1);

        REQUIRE(calc_ratio_a0 == se / (2 * sa0));
        REQUIRE(calc_ratio_a1 == se / (2 * sa1));
      }

      Modifier::coarsen(m, m_ops, level);
    }
  }
}

TEST_CASE("Check lift one loop close5",
          "[Butterfly analysis operations]")

{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/butterfly_test5_12_42.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, true));
  REQUIRE(bands.size() == 3);

  Butterfly_analysis b_analy;

  b_analy.initialize(m, m_ops);
  b_analy.lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_matlab = {
    Point{  0.000000285714,               0,  2.956600000000},
    Point{ -0.000000285714,              -0, -2.956600000000},
    Point{  1.964253714286, -0.000000142857,  1.018865714286},
    Point{  0.606988785714,  1.868114642857,  1.018867428571},
    Point{ -1.589111642857,  1.154558928571,  1.018868285714},
    Point{ -1.589111642857, -1.154558785714,  1.018868285714},
    Point{  0.606988785714, -1.868114642857,  1.018867428571},
    Point{  1.589111642857,  1.154558785714, -1.018868285714},
    Point{ -0.606988785714,  1.868114642857, -1.018867428571},
    Point{ -1.964253714286,  0.000000142857, -1.018865714286},
    Point{ -0.606988785714, -1.868114642857, -1.018867428571},
    Point{  1.589111642857, -1.154558928571, -1.018868285714},
    Point{  0.156251500000,               0,  0.060760000000},
    Point{  0.048278750000,  0.148598750000,  0.060760000000},
    Point{ -0.126405500000,  0.091840500000,  0.060760000000},
    Point{ -0.126405500000, -0.091840500000,  0.060760000000},
    Point{  0.048278750000, -0.148598750000,  0.060760000000},
    Point{  0.126405500000,  0.091840500000, -0.060760000000},
    Point{ -0.048278750000,  0.148598750000, -0.060760000000},
    Point{ -0.156251500000,               0, -0.060760000000},
    Point{ -0.048278750000, -0.148598750000, -0.060760000000},
    Point{  0.126405500000, -0.091840500000, -0.060760000000},
    Point{  0.051929250000,  0.037728750000, -0.096410000000},
    Point{  0.051929250000, -0.037728750000, -0.096410000000},
    Point{ -0.004943000000, -0.001604500000,               0},
    Point{ -0.004943000000,  0.001605500000,               0},
    Point{ -0.019833750000,  0.061036250000, -0.096420000000},
    Point{ -0.003057750000, -0.004193750000, -0.000002000000},
    Point{               0, -0.005202500000,               0},
    Point{ -0.064190000000,               0, -0.096420000000},
    Point{  0.003057750000, -0.004193750000,  0.000002000000},
    Point{  0.004943000000, -0.001605500001,               0},
    Point{ -0.019833750000, -0.061036250000, -0.096420000000},
    Point{  0.004943000000,  0.001604500000,               0},
    Point{  0.003057750000,  0.004193750000,  0.000002000000},
    Point{               0,  0.005202500000,               0},
    Point{ -0.003057750000,  0.004193750000, -0.000002000000},
    Point{  0.019833750000,  0.061036250000,  0.096420000000},
    Point{  0.064190000000,               0,  0.096420000000},
    Point{ -0.051929250000,  0.037728750000,  0.096410000000},
    Point{ -0.051929250000, -0.037728750000,  0.096410000000},
    Point{  0.019833750000, -0.061036250000,  0.096420000000},
  };

  for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
  {
    Point p = v->point();
    int id = m_ops.get_vertex_id(v);
    Point pc = res_from_matlab[id];
    INFO("    id: " << id);
    INFO("  p.x() - pc.x(): " << std::setprecision(25) << p.x() - pc.x());
    INFO("  p.y() - pc.y(): " << std::setprecision(25) << p.y() - pc.y());
    REQUIRE(p.x() == Approx(pc.x()).margin(std::numeric_limits<double>::epsilon()));
    REQUIRE(p.y() == Approx(pc.y()).margin(std::numeric_limits<double>::epsilon()));
    REQUIRE(p.z() == Approx(pc.z()).margin(std::numeric_limits<double>::epsilon()));
  }
}

TEST_CASE("Check lift one loop close4",
          "[Butterfly analysis operations]")

{
  Mesh m {Utils::loadMesh(std::string(TEST_DATA_DIR) + "subdivided_meshes/loop_test_close4_6_18.off")};
  Mesh_ops m_ops {Utils::initMeshOps()};
  Utils::initMeshInfo(m, m_ops);

  std::vector<Vertex_handle> vertices;
  std::vector<Vertex_handle*> bands;

  REQUIRE(Classify::classify(m, m_ops, 1, vertices, bands, true));
  REQUIRE(bands.size() == 3);

  Butterfly_analysis b_analy;

  b_analy.initialize(m, m_ops);
  b_analy.lift(m, m_ops, &bands[0], &bands[2]);

  std::vector<Point> res_from_matlab = {
    Point{  0.019609607950,  0.003109018100,  1.090832942567},
    Point{  0.002991825283,  0.010435876833, -1.076993598233},
    Point{  1.606843593633, -0.033031115550,  0.022919935867},
    Point{  0.020188004183,  1.656610338000, -0.014364363850},
    Point{ -1.650159727267,  0.038050616617, -0.028079892017},
    Point{ -0.013406737617, -1.615758513700,  0.018226354433},
    Point{ -0.135849024200,  0.068805142400, -0.167441060000},
    Point{ -0.045616805000, -0.214419304300, -0.116494557200},
    Point{  0.145908810600,  0.029881459900, -0.085074169700},
    Point{ -0.082100629100,  0.097078593400, -0.175987868500},
    Point{ -0.216359997000,  0.092274391400,  0.013072837300},
    Point{ -0.032018025800, -0.303215334000,  0.196291293700},
    Point{  0.288788111500, -0.124098708800,  0.224990199600},
    Point{ -0.058361040400,  0.272424390400,  0.027607258800},
    Point{ -0.201921971000, -0.141777793200, -0.026331523300},
    Point{ -0.086930569600,  0.178884952700,  0.043180130800},
    Point{  0.158428776700, -0.280249596500,  0.032720969900},
    Point{  0.307832664800,  0.146163145700, -0.004157647700},
  };

  for (Vertex_handle v = m.vertices_begin(); v != m.vertices_end(); ++v)
  {
    Point p = v->point();
    int id = m_ops.get_vertex_id(v);
    Point pc = res_from_matlab[id];

    INFO("    id: " << id);

    REQUIRE(p.x() == Approx(pc.x()));
    REQUIRE(p.y() == Approx(pc.y()));
    REQUIRE(p.z() == Approx(pc.z()));
  }
}
